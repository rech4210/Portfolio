#include "ShopSubsystem.h"
#include "ShopRepository.h"
#include "ShopDomainService.h"
#include "GameFramework/PlayerState.h"
#include "Provider/DBProviderInfra.h"
void UShopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UDBProviderInfra::StaticClass());
	Super::Initialize(Collection);
	
	ShopRepository = NewObject<UShopRepository>(this, TEXT("ShopRepository"));
	if (UDBProviderInfra* Infra = GetGameInstance()->GetSubsystem<UDBProviderInfra>())
	{
		ShopRepository->Initialize(Infra);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ShopSubsystem: UDBProviderInfra not available"));
	}
	
	ShopDomainService = NewObject<UShopDomainService>(this, TEXT("ShopDomainService"));
	ShopDomainService->Initialize(ShopRepository);
}

void UShopSubsystem::Deinitialize()
{
	if (ShopDomainService)
	{
		ShopDomainService->OnItemPurchased.RemoveAll(this);
		ShopDomainService->OnShopLoaded.RemoveAll(this);
		ShopDomainService->OnShopSaved.RemoveAll(this);
	}
	
	ShopRepository = nullptr;
	ShopDomainService = nullptr;
	Super::Deinitialize();
}

TScriptInterface<IShopRepositoryInterface> UShopSubsystem::GetShopRepository() const
{
	return ShopRepository;
}

UShopDomainService* UShopSubsystem::GetDomainService() const
{
	return ShopDomainService;
}

// ============================================================================
// Use Case Orchestration - App Layer Responsibilities Only
// ============================================================================

void UShopSubsystem::RequestPurchaseItem(APlayerState* PlayerState, int32 ShopID, int32 ItemID, int32 Quantity, float PlayerCurrency)
{
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Invalid PlayerState for purchase"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Purchase requests should only be made from server"));
		return;
	}

	if (!ShopDomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopSubsystem: DomainService not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Starting purchase transaction - Player: %s, Shop: %d, Item: %d, Quantity: %d"), 
		*PlayerState->GetPlayerName(), ShopID, ItemID, Quantity);

	ShopDomainService->PurchaseItem(PlayerState, ShopID, ItemID, Quantity, PlayerCurrency);
}

void UShopSubsystem::RequestLoadShopData(APlayerState* PlayerState, int32 ShopID)
{
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Invalid PlayerState for shop loading"));
		return;
	}

	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Client should wait for replicated data, not load from server"));
		return;
	}

	if (!ShopDomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopSubsystem: DomainService not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Starting shop load transaction - Player: %s, Shop: %d"), 
		*PlayerState->GetPlayerName(), ShopID);

	ShopDomainService->LoadShop(PlayerState, ShopID);
}

void UShopSubsystem::RequestAddItemToShop(int32 ShopID, int32 ItemID, int32 Stock, float Price)
{
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Item addition should only be requested from server"));
		return;
	}

	if (!ShopDomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopSubsystem: DomainService not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Starting add item transaction - Shop: %d, Item: %d, Stock: %d, Price: %.2f"), 
		ShopID, ItemID, Stock, Price);

	FShopItemDTO NewItem(ItemID, Stock, Price, ShopID);
	ShopDomainService->AddItemToShop(ShopID, NewItem);
}

void UShopSubsystem::RequestRemoveItemFromShop(int32 ShopID, int32 ItemID)
{
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Item removal should only be requested from server"));
		return;
	}

	if (!ShopDomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopSubsystem: DomainService not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Starting remove item transaction - Shop: %d, Item: %d"), ShopID, ItemID);
	ShopDomainService->RemoveItemFromShop(ShopID, ItemID);
}

void UShopSubsystem::RequestRestockShop(int32 ShopID)
{
	if (GetGameInstance()->GetWorld()->GetNetMode() == NM_Client)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopSubsystem: Restock should only be requested from server"));
		return;
	}

	if (!ShopDomainService)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopSubsystem: DomainService not initialized"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ShopSubsystem: Starting restock transaction - Shop: %d"), ShopID);
	ShopDomainService->RestockShop(ShopID);
}
