
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

	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerState, ItemID, Quantity, PlayerCurrency, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop: %s"), *ShopResult.ErrorMessage);
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		if (!ValidatePurchaseRules(ShopData, ItemID, Quantity, PlayerCurrency))
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Purchase validation failed"));
			return;
		}

		auto* ItemData = ShopData.ShopItems.FindByPredicate([ItemID](const FShopItemDTO& Item) 
		{ 
			return Item.ItemID == ItemID; 
		});

		if (!ItemData)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not found"));
			return;
		}

		ItemData->Stock -= Quantity;
		ItemData->bIsAvailable = ItemData->Stock > 0;

		auto SaveTask = Repository->SaveShop(ShopData);
		bool bSaveSuccess = SaveTask.GetResult();

		if (bSaveSuccess)
		{
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

	auto LoadTask = Repository->LoadShopByID(ShopID);
	
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

	auto SaveTask = Repository->SaveShop(ShopData);
	
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

	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, ItemDTO, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for item addition"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		if (!ValidateItemAdditionRules(ShopData, ItemDTO))
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item addition validation failed"));
			return;
		}

		ShopData.ShopItems.Add(ItemDTO);

		auto SaveTask = Repository->SaveShop(ShopData);
		bool bSaveSuccess = SaveTask.GetResult();

		if (bSaveSuccess)
		{
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

	auto LoadTask = Repository->LoadShopByID(ShopID);
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, ItemID, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for item removal"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		int32 RemovedCount = ShopData.ShopItems.RemoveAll([ItemID](const FShopItemDTO& Item) 
		{ 
			return Item.ItemID == ItemID; 
		});

		if (RemovedCount == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not found for removal"));
			return;
		}

		auto SaveTask = Repository->SaveShop(ShopData);
		bool bSaveSuccess = SaveTask.GetResult();

		if (bSaveSuccess)
		{
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

	auto LoadTask = Repository->LoadShopByID(ShopID);
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, ItemID, NewStock, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for stock update"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		auto* ItemData = ShopData.ShopItems.FindByPredicate([ItemID](const FShopItemDTO& Item) 
		{ 
			return Item.ItemID == ItemID; 
		});

		if (!ItemData)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not found for stock update"));
			return;
		}

		ItemData->Stock = NewStock;
		ItemData->bIsAvailable = NewStock > 0;

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

	auto LoadTask = Repository->LoadShopByID(ShopID);
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, ItemID, NewPrice, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for price update"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		auto* ItemData = ShopData.ShopItems.FindByPredicate([ItemID](const FShopItemDTO& Item) 
		{ 
			return Item.ItemID == ItemID; 
		});

		if (!ItemData)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not found for price update"));
			return;
		}

		ItemData->Price = NewPrice;

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

	auto LoadTask = Repository->LoadShopByID(ShopID);
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for restock"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		for (auto& Item : ShopData.ShopItems)
		{
			Item.Stock = 10;
			Item.bIsAvailable = true;
		}

		ShopData.LastRestockTime = FDateTime::Now();

		auto SaveTask = Repository->SaveShop(ShopData);
		bool bSaveSuccess = SaveTask.GetResult();

		if (bSaveSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("ShopDomainService: Successfully restocked shop %d"), ShopID);
			
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

	auto LoadTask = Repository->LoadShopByID(ShopID);
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, bIsOpen, LoadTask]() mutable -> void
	{
		FShopRepositoryResult ShopResult = LoadTask.GetResult();
		
		if (!ShopResult.bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Failed to load shop for status update"));
			return;
		}

		FShopDomain ShopData = ShopResult.ShopData;

		ShopData.bIsOpen = bIsOpen;
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
	auto LoadTask = Repository->LoadShopByID(ShopID);
	
	UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, LoadTask]() mutable -> void
	{
		FShopRepositoryResult Result = LoadTask.GetResult();
		
		if (Result.bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("ShopDomainService: Successfully loaded shop info"));
			
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
	if (!ShopData.bIsOpen)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Shop is closed"));
		return false;
	}

	const auto* ItemData = ShopData.ShopItems.FindByPredicate([ItemID](const FShopItemDTO& Item) 
	{ 
		return Item.ItemID == ItemID; 
	});

	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not found"));
		return false;
	}

	if (!ItemData->bIsAvailable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Item not available"));
		return false;
	}

	if (ItemData->Stock < Quantity)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Insufficient stock"));
		return false;
	}

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
	if (ItemDTO.ItemID <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Invalid item ID"));
		return false;
	}

	if (ItemDTO.Price <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Invalid item price"));
		return false;
	}

	if (ItemDTO.Stock < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopDomainService: Invalid item stock"));
		return false;
	}

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
	if (IsInGameThread())
	{
		EventFunction();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ShopDomainService: Attempting to publish domain event from non-GameThread"));
	}
}
