// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#include "ShopDomainService.h"
#include "IShopRepositoryInterface.h"
#include "ShopDomain.h"
#include "GameFramework/PlayerState.h"
#include "Components/ShopComponent.h"
#include "Tasks/Task.h"

UShopDomainService::UShopDomainService()
{
}

void UShopDomainService::Initialize(TScriptInterface<IShopRepositoryInterface> InRepository)
{
	Repository = InRepository;
}

// ============================================================================
// Domain Service Methods - void return types with domain events
// ============================================================================

void UShopDomainService::PurchaseItem(
	APlayerState* PlayerState, 
	int32 ShopID, 
	int32 ItemID, 
	int32 Quantity, 
	float PlayerCurrency)
{
	if (!Repository.GetInterface() || !PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid repository or player state"));
		return;
	}

	if (Quantity <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid quantity"));
		return;
	}

	// Step 1: Load shop data (Worker Thread)
	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	// Step 2: Process purchase logic with atomic transaction
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, ItemID, Quantity, PlayerCurrency, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop: %s"), *ShopResult.ErrorMessage);
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		// Validate business rules (Worker Thread)
		if (!ValidatePurchaseRules(ShopData, ItemID, Quantity, PlayerCurrency))
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Purchase validation failed"));
			return;
		}

		// Find and update item
		auto* ItemData = ShopData.ShopItems.FindByPredicate([ItemID](const FShopItemDTO& Item) 
		{ 
			return Item.ItemID == ItemID; 
		});

		if (!ItemData)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not found"));
			return;
		}

		// Execute purchase business logic - atomic transaction
		ItemData->Stock -= Quantity;
		ItemData->bIsAvailable = ItemData->Stock > 0;

		// Save changes with atomic transaction
		auto SaveTask = Repository->SaveShop(ShopData);
		bool bSaveSuccess = SaveTask.GetResult();

		if (bSaveSuccess)
		{
			// Publish domain event on GameThread
			AsyncTask(ENamedThreads::GameThread, [this, PlayerState, ItemID, Quantity]()
			{
				OnItemPurchased.Broadcast(PlayerState, ItemID, Quantity);
			});
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Failed to save purchase"));
		}
	});
}

void UShopDomainService::LoadShop(APlayerState* PlayerState, int32 ShopID)
{
	if (!Repository.GetInterface() || !PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid repository or player state"));
		return;
	}

	// Load shop data (Worker Thread)
	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	// Publish domain event on GameThread
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, LoadTask]() mutable -> void
	{
		FShopRepositoryResult Result = LoadTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, PlayerState, Result]()
		{
			if (Result.bSuccess)
			{
				OnShopLoaded.Broadcast(PlayerState, Result.ShopData);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop: %s"), *Result.ErrorMessage);
			}
		});
	});
}

void UShopDomainService::SaveShop(const FShopDomain& ShopData)
{
	if (!Repository.GetInterface())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid repository"));
		return;
	}

	// Save shop data with atomic transaction (Worker Thread)
	auto SaveTask = Repository->SaveShop(ShopData);
	
	// Publish domain event on GameThread
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopData, SaveTask]() mutable -> void
	{
		bool bSuccess = SaveTask.GetResult();
		
		AsyncTask(ENamedThreads::GameThread, [this, ShopData, bSuccess]()
		{
			if (bSuccess)
			{
				OnShopSaved.Broadcast(ShopData);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to save shop: %d"), ShopData.ShopID);
			}
		});
	});
}

void UShopDomainService::AddItemToShop(int32 ShopID, const FShopItemDTO& ItemDTO)
{
	if (!Repository.GetInterface())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid repository"));
		return;
	}

	// Step 1: Load shop data (Worker Thread)
	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	// Step 2: Add item logic with atomic transaction
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, ItemDTO, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for item addition"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		// Validate business rules
		if (!ValidateItemAdditionRules(ShopData, ItemDTO))
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item addition validation failed"));
			return;
		}

		// Add item to shop
		ShopData.ShopItems.Add(ItemDTO);

		// Save changes with atomic transaction
		auto SaveTask = Repository->SaveShop(ShopData);
		bool bSaveSuccess = SaveTask.GetResult();

		if (bSaveSuccess)
		{
			// Publish domain event on GameThread
			AsyncTask(ENamedThreads::GameThread, [this, ShopID, ItemDTO]()
			{
				OnShopItemAdded.Broadcast(ShopID, ItemDTO.ItemID, ItemDTO);
			});
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Failed to save item addition"));
		}
	});
}

void UShopDomainService::RemoveItemFromShop(int32 ShopID, int32 ItemID)
{
	if (!Repository.GetInterface())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid repository"));
		return;
	}

	// Step 1: Load shop data (Worker Thread)
	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	// Step 2: Remove item logic with atomic transaction
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, ItemID, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for item removal"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		// Find and remove item
		int32 RemovedCount = ShopData.ShopItems.RemoveAll([ItemID](const FShopItemDTO& Item) 
		{ 
			return Item.ItemID == ItemID; 
		});

		if (RemovedCount == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not found for removal"));
			return;
		}

		// Save changes with atomic transaction
		auto SaveTask = Repository->SaveShop(ShopData);
		bool bSaveSuccess = SaveTask.GetResult();

		if (bSaveSuccess)
		{
			// Publish domain event on GameThread
			AsyncTask(ENamedThreads::GameThread, [this, ShopID, ItemID]()
			{
				OnShopItemRemoved.Broadcast(ShopID, ItemID);
			});
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Failed to save item removal"));
		}
	});
}

void UShopDomainService::UpdateItemStock(int32 ShopID, int32 ItemID, int32 NewStock)
{
	if (!Repository.GetInterface())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid repository"));
		return;
	}

	if (NewStock < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid stock amount"));
		return;
	}

	// Step 1: Load shop data (Worker Thread)
	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	// Step 2: Update stock logic with atomic transaction
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, ItemID, NewStock, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for stock update"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		// Find and update item stock
		auto* ItemData = ShopData.ShopItems.FindByPredicate([ItemID](const FShopItemDTO& Item) 
		{ 
			return Item.ItemID == ItemID; 
		});

		if (!ItemData)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not found for stock update"));
			return;
		}

		// Update stock
		ItemData->Stock = NewStock;
		ItemData->bIsAvailable = NewStock > 0;

		// Save changes with atomic transaction
		auto SaveTask = Repository->SaveShop(ShopData);
		bool bSaveSuccess = SaveTask.GetResult();

		if (bSaveSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("ShopDomainService: Successfully updated stock for item %d to %d"), ItemID, NewStock);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Failed to save stock update"));
		}
	});
}

void UShopDomainService::UpdateItemPrice(int32 ShopID, int32 ItemID, float NewPrice)
{
	if (!Repository.GetInterface())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid repository"));
		return;
	}

	if (NewPrice <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid price amount"));
		return;
	}

	// Step 1: Load shop data (Worker Thread)
	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	// Step 2: Update price logic with atomic transaction
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, ItemID, NewPrice, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for price update"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		// Find and update item price
		auto* ItemData = ShopData.ShopItems.FindByPredicate([ItemID](const FShopItemDTO& Item) 
		{ 
			return Item.ItemID == ItemID; 
		});

		if (!ItemData)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not found for price update"));
			return;
		}

		// Update price
		ItemData->Price = NewPrice;

		// Save changes with atomic transaction
		auto SaveTask = Repository->SaveShop(ShopData);
		bool bSaveSuccess = SaveTask.GetResult();

		if (bSaveSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("ShopDomainService: Successfully updated price for item %d to %f"), ItemID, NewPrice);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Failed to save price update"));
		}
	});
}

void UShopDomainService::RestockShop(int32 ShopID)
{
	if (!Repository.GetInterface())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid repository"));
		return;
	}

	// Step 1: Load shop data (Worker Thread)
	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	// Step 2: Restock logic with atomic transaction
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for restock"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		// Business logic: Restock all items to full capacity
		for (auto& Item : ShopData.ShopItems)
		{
			// Restore to default stock level (business rule)
			Item.Stock = 10; // Could be configurable or based on item type
			Item.bIsAvailable = true;
		}

		// Update shop last restock time
		ShopData.LastRestockTime = FDateTime::Now();

		// Save changes with atomic transaction
		auto SaveTask = Repository->SaveShop(ShopData);
		bool bSaveSuccess = SaveTask.GetResult();

		if (bSaveSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("ShopDomainService: Successfully restocked shop %d"), ShopID);
			
			// Broadcast domain event on Game Thread
			AsyncTask(ENamedThreads::GameThread, [this, ShopID]()
			{
				OnShopRestocked.Broadcast(ShopID);
			});
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Failed to save restock data"));
		}
	});
}

void UShopDomainService::SetShopStatus(int32 ShopID, bool bIsOpen)
{
	if (!Repository.GetInterface())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid repository"));
		return;
	}

	// Step 1: Load shop data (Worker Thread)
	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	// Step 2: Update status logic with atomic transaction
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, bIsOpen, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for status update"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		// Update shop status
		ShopData.bIsOpen = bIsOpen;

		// Save changes with atomic transaction
		auto SaveTask = Repository->SaveShop(ShopData);
		bool bSaveSuccess = SaveTask.GetResult();

		if (bSaveSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("ShopDomainService: Successfully updated shop %d status to %s"), 
				ShopID, bIsOpen ? TEXT("Open") : TEXT("Closed"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Failed to save shop status update"));
		}
	});
}

void UShopDomainService::GetShopInfo(int32 ShopID)
{
	if (!Repository.GetInterface())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Invalid repository"));
		return;
	}

	// Load shop data (Worker Thread)
	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	// Execute domain event on GameThread
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, LoadTask]() mutable -> void
	{
		FShopRepositoryResult Result = LoadTask.GetResult();
		
		if (Result.bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("ShopDomainService: Successfully loaded shop info"));
			
			// Broadcast domain event on Game Thread
			AsyncTask(ENamedThreads::GameThread, [this, Result]()
			{
				OnShopLoaded.Broadcast(nullptr, Result.ShopData);
			});
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop info: %s"), *Result.ErrorMessage);
		}
	});
}

// ============================================================================
// Private Business Rule Validation Methods
// ============================================================================

bool UShopDomainService::ValidatePurchaseRules(const FShopDomain& ShopData, int32 ItemID, int32 Quantity, float PlayerCurrency) const
{
	// Business rule: Shop must be open
	if (!ShopData.bIsOpen)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Shop is closed"));
		return false;
	}

	// Find the item
	const auto* ItemData = ShopData.ShopItems.FindByPredicate([ItemID](const FShopItemDTO& Item) 
	{ 
		return Item.ItemID == ItemID; 
	});

	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not found"));
		return false;
	}

	// Business rule: Item must be available
	if (!ItemData->bIsAvailable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not available"));
		return false;
	}

	// Business rule: Sufficient stock
	if (ItemData->Stock < Quantity)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Insufficient stock"));
		return false;
	}

	// Business rule: Player must have enough currency
	const float TotalCost = ItemData->Price * Quantity;
	if (PlayerCurrency < TotalCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Insufficient currency"));
		return false;
	}

	return true;
}

bool UShopDomainService::ValidateItemAdditionRules(const FShopDomain& ShopData, const FShopItemDTO& ItemDTO) const
{
	// Business rule: Item ID must be valid
	if (ItemDTO.ItemID <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Invalid item ID"));
		return false;
	}

	// Business rule: Price must be positive
	if (ItemDTO.Price <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Invalid item price"));
		return false;
	}

	// Business rule: Stock must be non-negative
	if (ItemDTO.Stock < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Invalid item stock"));
		return false;
	}

	// Business rule: Item must not already exist in shop
	const bool bItemExists = ShopData.ShopItems.ContainsByPredicate([&ItemDTO](const FShopItemDTO& Item) 
	{ 
		return Item.ItemID == ItemDTO.ItemID; 
	});

	if (bItemExists)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item already exists in shop"));
		return false;
	}

	return true;
}

template<typename Func>
void UShopDomainService::PublishDomainEvent(Func&& EventFunction)
{
	// Ensure we're on the GameThread before publishing events
	if (IsInGameThread())
	{
		EventFunction();
	}
	else
	{
		// Should not happen since we explicitly use ENamedThreads::GameThread
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Attempting to publish domain event from non-GameThread"));
	}
}
