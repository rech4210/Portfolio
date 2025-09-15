#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../../GameSharedModule/Public/Interface/IShopRepositoryInterface.h"
#include "ShopSubsystem.generated.h"

class UShopRepository;
class UShopDomainService;
class APlayerState;
class UShopComponent;

UCLASS()
class SHOPMODULE_API UShopSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	TScriptInterface<IShopRepositoryInterface> GetShopRepository() const;
	UShopDomainService* GetDomainService() const;

	// ============================================================================
	// DDD Entry Points
	// ============================================================================

	void RequestPurchaseItem(APlayerState* PlayerState, int32 ShopID, int32 ItemID, int32 Quantity, float PlayerCurrency);
	void RequestLoadShopData(APlayerState* PlayerState, int32 ShopID);
	void RequestAddItemToShop(int32 ShopID, int32 ItemID, int32 Stock, float Price);
	void RequestRemoveItemFromShop(int32 ShopID, int32 ItemID);
	void RequestRestockShop(int32 ShopID);

private:
	UPROPERTY()
	UShopRepository* ShopRepository;

	UPROPERTY()
	UShopDomainService* ShopDomainService;
};
