
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShopComponent.generated.h"

class UShopItemData;
class UShopDomainService;
struct FShopDomain;

USTRUCT(BlueprintType)
struct FShopItemState
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ItemID = 0;

	UPROPERTY()
	int32 Stock = 0;

	UPROPERTY()
	float Price = 0.0f;

	UPROPERTY()
	bool bIsAvailable = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopStateChanged, const TArray<FShopItemState>&, UpdatedItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemPurchaseAttempted, int32, ItemID, int32, RequestedQuantity, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemStockUpdated, int32, ItemID, int32, NewStock);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPriceUpdated, int32, ItemID, float, NewPrice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPurchasedEvent, int32, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopDataLoadedEvent, const struct FShopDomain&, ShopData);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHOPMODULE_API UShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UShopComponent();

	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnShopStateChanged OnShopStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnItemPurchaseAttempted OnItemPurchaseAttempted;

	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnItemStockUpdated OnItemStockUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnItemPriceUpdated OnItemPriceUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Shop|UI Events")
	FOnItemPurchasedEvent OnItemPurchasedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Shop|UI Events")
	FOnShopDataLoadedEvent OnShopDataLoadedEvent;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_ShopItems, VisibleAnywhere, BlueprintReadOnly, Category = "Shop|Component")
	TArray<FShopItemState> ShopItems;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Shop|Component")
	int32 ShopID = 0;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Shop|Component")
	bool bIsShopOpen = true;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Shop|Component")
	float GlobalPriceModifier = 1.0f;

public:
	// ============================================================================
	// Core Shop Operations (Essential DDD Operations)
	// ============================================================================
	
	UFUNCTION(BlueprintCallable, Category = "Shop|Operations")
	bool PurchaseItemWithValidation(int32 ItemID, int32 Quantity, float PlayerCurrency);

	UFUNCTION(BlueprintCallable, Category = "Shop|Operations")
	bool UpdateItemStock(int32 ItemID, int32 NewStock);

	UFUNCTION(BlueprintCallable, Category = "Shop|Operations")
	bool UpdateItemPrice(int32 ItemID, float NewPrice);

	// ============================================================================
	// Essential Query Methods
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	const TArray<FShopItemState>& GetAllShopItems() const { return ShopItems; }

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	bool GetShopItem(int32 ItemID, FShopItemState& OutItem) const;

	const FShopItemState* GetShopItemPtr(int32 ItemID) const;
	FShopItemState* GetShopItemPtr(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	int32 GetShopID() const { return ShopID; }

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	bool IsShopOpen() const { return bIsShopOpen; }

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	bool CanPurchaseItem(int32 ItemID, int32 Quantity, float PlayerCurrency) const;

	// ============================================================================
	// Essential Configuration
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Shop|Configuration")
	void SetShopID(int32 NewShopID);

	UFUNCTION(BlueprintCallable, Category = "Shop|Configuration")
	void SetShopOpenStatus(bool bNewIsOpen);

	// ============================================================================
	// Domain Integration (Essential for DDD)
	// ============================================================================

	void SyncWithDomain(const FShopDomain& ShopData);

	FShopDomain ExtractDomain() const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_ShopItems();

	UFUNCTION()
	virtual void OnRep_ShopID();

	UFUNCTION()
	virtual void OnRep_ShopOpenStatus();

private:
	
	bool ValidateServerAuthority() const;
	void NotifyStateChanged();
	bool ValidatePurchaseRules(int32 ItemID, int32 Quantity, float PlayerCurrency) const;
	
	template<typename Func>
	void PublishDomainEvent(Func&& EventFunction);
};
