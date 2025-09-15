
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Async/TaskGraphInterfaces.h"
#include "Tasks/Task.h"
#include "GameSharedModule/Public/DTO/ShopDTOs.h"
#include "ShopDomainService.generated.h"

class APlayerState;
class UShopComponent;
class IShopRepositoryInterface;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemPurchased, APlayerState* /* PlayerState */, int32 /* ItemID */, int32 /* Quantity */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnShopLoaded, APlayerState* /* PlayerState */, const FShopDomain& /* ShopData */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopSaved, const FShopDomain& /* ShopData */);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnShopItemAdded, int32 /* ShopID */, int32 /* ItemID */, const FShopItemDTO& /* Item */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnShopItemRemoved, int32 /* ShopID */, int32 /* ItemID */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopRestocked, int32 /* ShopID */);

UCLASS()
class SHOPMODULE_API UShopDomainService : public UObject
{
	GENERATED_BODY()

public:
	UShopDomainService();

	FOnItemPurchased OnItemPurchased;
	FOnShopLoaded OnShopLoaded;
	FOnShopSaved OnShopSaved;
	FOnShopItemAdded OnShopItemAdded;
	FOnShopItemRemoved OnShopItemRemoved;
	FOnShopRestocked OnShopRestocked;

	void Initialize(TScriptInterface<IShopRepositoryInterface> InRepository);

	void PurchaseItem(
		APlayerState* PlayerState, 
		int32 ShopID, 
		int32 ItemID, 
		int32 Quantity, 
		float PlayerCurrency
	);

	void LoadShop(APlayerState* PlayerState, int32 ShopID);
	void SaveShop(const FShopDomain& ShopData);
	void AddItemToShop(int32 ShopID, const FShopItemDTO& ItemDTO);
	void RemoveItemFromShop(int32 ShopID, int32 ItemID);
	void UpdateItemStock(int32 ShopID, int32 ItemID, int32 NewStock);
	void UpdateItemPrice(int32 ShopID, int32 ItemID, float NewPrice);
	void RestockShop(int32 ShopID);
	void SetShopStatus(int32 ShopID, bool bIsOpen);
	void GetShopInfo(int32 ShopID);

private:
	TScriptInterface<IShopRepositoryInterface> Repository;
	bool ValidatePurchaseRules(const FShopDomain& ShopData, int32 ItemID, int32 Quantity, float PlayerCurrency) const;
	bool ValidateItemAdditionRules(const FShopDomain& ShopData, const FShopItemDTO& ItemDTO) const;
	template<typename Func>
	void PublishDomainEvent(Func&& EventFunction);
};
