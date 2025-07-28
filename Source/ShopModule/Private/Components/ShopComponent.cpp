
#include "Components/ShopComponent.h"
#include "Net/UnrealNetwork.h"
#include "ShopSubsystem.h"
#include "ShopDomain.h"
#include "Engine/World.h"
#include "Async/Async.h"

UShopComponent::UShopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UShopComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UShopComponent, ShopItems);
	DOREPLIFETIME(UShopComponent, ShopID);
	DOREPLIFETIME(UShopComponent, bIsShopOpen);
	DOREPLIFETIME(UShopComponent, GlobalPriceModifier);
}

void UShopComponent::BeginPlay()
{
	Super::BeginPlay();
}

// ============================================================================
// Replication Handlers
// ============================================================================

void UShopComponent::OnRep_ShopItems()
{
	OnShopStateChanged.Broadcast(ShopItems);
	
	// if (auto* ShopSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UShopSubsystem>())
	// {
	// 	ShopSubsystem->Client_OnShopStateUpdated(this);
	// }
}

void UShopComponent::OnRep_ShopID()
{
	UE_LOG(LogTemp, Log, TEXT("ShopComponent: Shop ID replicated: %d"), ShopID);
}

void UShopComponent::OnRep_ShopOpenStatus()
{
	UE_LOG(LogTemp, Log, TEXT("ShopComponent: Shop open status replicated: %s"), bIsShopOpen ? TEXT("Open") : TEXT("Closed"));
}

// ============================================================================
// Core Shop Operations
// ============================================================================

bool UShopComponent::PurchaseItemWithValidation(int32 ItemID, int32 Quantity, float PlayerCurrency)
{
	if (!ValidateServerAuthority())
	{
		return false;
	}

	// Validate purchase rules
	if (!ValidatePurchaseRules(ItemID, Quantity, PlayerCurrency))
	{
		PublishDomainEvent([this, ItemID, Quantity]()
		{
			OnItemPurchaseAttempted.Broadcast(ItemID, Quantity, false);
		});
		return false;
	}

	// Find and update item
	FShopItemState* Item = GetShopItemPtr(ItemID);
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopComponent: Item %d not found for purchase"), ItemID);
		return false;
	}

	// Process purchase
	Item->Stock = FMath::Max(0, Item->Stock - Quantity);
	Item->bIsAvailable = Item->Stock > 0;

	NotifyStateChanged();

	// Publish domain events
	PublishDomainEvent([this, ItemID, Quantity]()
	{
		OnItemPurchaseAttempted.Broadcast(ItemID, Quantity, true);
		OnItemStockUpdated.Broadcast(ItemID, GetShopItemPtr(ItemID)->Stock);
	});

	return true;
}

bool UShopComponent::UpdateItemStock(int32 ItemID, int32 NewStock)
{
	if (!ValidateServerAuthority())
	{
		return false;
	}

	FShopItemState* Item = GetShopItemPtr(ItemID);
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopComponent: Item %d not found for stock update"), ItemID);
		return false;
	}

	Item->Stock = FMath::Max(0, NewStock);
	Item->bIsAvailable = Item->Stock > 0;

	NotifyStateChanged();

	PublishDomainEvent([this, ItemID, NewStock]()
	{
		OnItemStockUpdated.Broadcast(ItemID, NewStock);
	});

	UE_LOG(LogTemp, Log, TEXT("ShopComponent: Updated stock for item %d to %d"), ItemID, NewStock);
	return true;
}

bool UShopComponent::UpdateItemPrice(int32 ItemID, float NewPrice)
{
	if (!ValidateServerAuthority())
	{
		return false;
	}

	FShopItemState* Item = GetShopItemPtr(ItemID);
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopComponent: Item %d not found for price update"), ItemID);
		return false;
	}

	Item->Price = FMath::Max(0.01f, NewPrice); // Minimum price of 0.01

	NotifyStateChanged();

	PublishDomainEvent([this, ItemID, NewPrice]()
	{
		OnItemPriceUpdated.Broadcast(ItemID, NewPrice);
	});

	UE_LOG(LogTemp, Log, TEXT("ShopComponent: Updated price for item %d to %f"), ItemID, NewPrice);
	return true;
}

// ============================================================================
// Essential Query Methods
// ============================================================================

bool UShopComponent::GetShopItem(int32 ItemID, FShopItemState& OutItem) const
{
	const FShopItemState* ItemPtr = GetShopItemPtr(ItemID);
	if (ItemPtr)
	{
		OutItem = *ItemPtr;
		return true;
	}
	return false;
}

const FShopItemState* UShopComponent::GetShopItemPtr(int32 ItemID) const
{
	return ShopItems.FindByPredicate([ItemID](const FShopItemState& Item)
	{
		return Item.ItemID == ItemID;
	});
}

FShopItemState* UShopComponent::GetShopItemPtr(int32 ItemID)
{
	return ShopItems.FindByPredicate([ItemID](const FShopItemState& Item)
	{
		return Item.ItemID == ItemID;
	});
}

bool UShopComponent::CanPurchaseItem(int32 ItemID, int32 Quantity, float PlayerCurrency) const
{
	return ValidatePurchaseRules(ItemID, Quantity, PlayerCurrency);
}

// ============================================================================
// Essential Configuration
// ============================================================================

void UShopComponent::SetShopID(int32 NewShopID)
{
	if (ValidateServerAuthority())
	{
		ShopID = NewShopID;
		UE_LOG(LogTemp, Log, TEXT("ShopComponent: Shop ID set to %d"), ShopID);
	}
}

void UShopComponent::SetShopOpenStatus(bool bNewIsOpen)
{
	if (ValidateServerAuthority())
	{
		bIsShopOpen = bNewIsOpen;
		UE_LOG(LogTemp, Log, TEXT("ShopComponent: Shop status set to %s"), bIsShopOpen ? TEXT("Open") : TEXT("Closed"));
	}
}

// ============================================================================
// Domain Integration (Essential for DDD)
// ============================================================================

void UShopComponent::SyncWithDomain(const FShopDomain& ShopData)
{
	ShopID = ShopData.ShopID;
	bIsShopOpen = ShopData.bIsOpen;
	GlobalPriceModifier = ShopData.GlobalPriceModifier;

	// Convert domain items to component state
	ShopItems.Empty();
	for (const auto& DomainItem : ShopData.ShopItems)
	{
		FShopItemState ComponentItem;
		ComponentItem.ItemID = DomainItem.ItemID;
		ComponentItem.Stock = DomainItem.Stock;
		ComponentItem.Price = DomainItem.Price;
		ComponentItem.bIsAvailable = DomainItem.bIsAvailable;
		ShopItems.Add(ComponentItem);
	}

	NotifyStateChanged();
	UE_LOG(LogTemp, Log, TEXT("ShopComponent: Synced with domain - Shop ID: %d, Items: %d"), ShopID, ShopItems.Num());
}

FShopDomain UShopComponent::ExtractDomain() const
{
	FShopDomain DomainData;
	DomainData.ShopID = ShopID;
	DomainData.bIsOpen = bIsShopOpen;
	DomainData.GlobalPriceModifier = GlobalPriceModifier;

	// Convert component items to domain objects
	DomainData.ShopItems.Empty();
	for (const auto& ComponentItem : ShopItems)
	{
		FShopItemDTO DomainItem;
		DomainItem.ItemID = ComponentItem.ItemID;
		DomainItem.Stock = ComponentItem.Stock;
		DomainItem.Price = ComponentItem.Price;
		DomainItem.bIsAvailable = ComponentItem.bIsAvailable;
		DomainData.ShopItems.Add(DomainItem);
	}

	return DomainData;
}

// ============================================================================
// Private Helper Methods
// ============================================================================

bool UShopComponent::ValidateServerAuthority() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->HasAuthority();
	}
	return false;
}

void UShopComponent::NotifyStateChanged()
{
	if (ValidateServerAuthority())
	{
		// Force replication update
		// ForceNetUpdate();
	}
}

bool UShopComponent::ValidatePurchaseRules(int32 ItemID, int32 Quantity, float PlayerCurrency) const
{
	if (!bIsShopOpen)
	{
		return false;
	}

	const FShopItemState* Item = GetShopItemPtr(ItemID);
	if (!Item || !Item->bIsAvailable || Item->Stock < Quantity)
	{
		return false;
	}

	float TotalCost = Item->Price * Quantity * GlobalPriceModifier;
	return PlayerCurrency >= TotalCost;
}

template<typename Func>
void UShopComponent::PublishDomainEvent(Func&& EventFunction)
{
	if (IsInGameThread())
	{
		EventFunction();
	}
	else
	{
		AsyncTask(ENamedThreads::GameThread, MoveTemp(EventFunction));
	}
}
