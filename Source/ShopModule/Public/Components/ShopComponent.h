// Fill out your copyright notice in the Description page of Project Settings.

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

/**
 * 상점 아이템들을 관리하는 컴포넌트입니다.
 * 실질적으로 ItemDataAsset이 아닌, 표면적인 데이터를 다룬다. ItemDataAsset은 부가 기능이 많기 때문.
 * 
 * Enhanced as aggregate root for shop domain with business rule validation and domain events.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopStateChanged, const TArray<FShopItemState>&, UpdatedItems);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemPurchaseAttempted, int32, ItemID, int32, RequestedQuantity, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemStockUpdated, int32, ItemID, int32, NewStock);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPriceUpdated, int32, ItemID, float, NewPrice);

// UI and Client Event Delegates for Subsystem integration
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemPurchasedEvent, int32, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopDataLoadedEvent, const struct FShopDomain&, ShopData);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHOPMODULE_API UShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UShopComponent();

	// Domain events
	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnShopStateChanged OnShopStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnItemPurchaseAttempted OnItemPurchaseAttempted;

	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnItemStockUpdated OnItemStockUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Shop|Events")
	FOnItemPriceUpdated OnItemPriceUpdated;

	// UI and Client Events for Subsystem integration
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

	/**
	 * Purchase an item with business rule validation
	 * @param ItemID The item to purchase
	 * @param Quantity The quantity to purchase
	 * @param PlayerCurrency The player's available currency
	 * @return True if purchase was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Operations")
	bool PurchaseItemWithValidation(int32 ItemID, int32 Quantity, float PlayerCurrency);

	/**
	 * Update item stock with business rule validation
	 * @param ItemID The item ID to update
	 * @param NewStock The new stock amount
	 * @return True if update was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Operations")
	bool UpdateItemStock(int32 ItemID, int32 NewStock);

	/**
	 * Update item price with business rule validation
	 * @param ItemID The item ID to update
	 * @param NewPrice The new price
	 * @return True if update was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Operations")
	bool UpdateItemPrice(int32 ItemID, float NewPrice);

	// ============================================================================
	// Essential Query Methods
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	const TArray<FShopItemState>& GetAllShopItems() const { return ShopItems; }

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	bool GetShopItem(int32 ItemID, FShopItemState& OutItem) const;

	// Non-UFUNCTION versions for C++ usage
	const FShopItemState* GetShopItemPtr(int32 ItemID) const;
	FShopItemState* GetShopItemPtr(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	int32 GetShopID() const { return ShopID; }

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	bool IsShopOpen() const { return bIsShopOpen; }

	/**
	 * Check if an item can be purchased
	 * @param ItemID The item to check
	 * @param Quantity The desired quantity
	 * @param PlayerCurrency The player's available currency
	 * @return True if item can be purchased
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	bool CanPurchaseItem(int32 ItemID, int32 Quantity, float PlayerCurrency) const;

	// ============================================================================
	// Essential Configuration
	// ============================================================================

	/**
	 * Set shop ID (server authority required)
	 * @param NewShopID The new shop ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Configuration")
	void SetShopID(int32 NewShopID);

	/**
	 * Set shop open/closed status (server authority required)
	 * @param bNewIsOpen Whether the shop should be open
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Configuration")
	void SetShopOpenStatus(bool bNewIsOpen);

	// ============================================================================
	// Domain Integration (Essential for DDD)
	// ============================================================================

	/**
	 * Synchronize component state with domain object
	 * @param ShopData The domain object to sync with
	 */
	void SyncWithDomain(const FShopDomain& ShopData);

	/**
	 * Extract domain object from component state
	 * @return The domain object representation
	 */
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
	/**
	 * Validate server authority for operations
	 * @return True if server has authority
	 */
	bool ValidateServerAuthority() const;

	/**
	 * Notify state change and trigger replication
	 */
	void NotifyStateChanged();

	/**
	 * Validate purchase transaction rules (internal)
	 * @param ItemID The item to purchase
	 * @param Quantity The quantity to purchase
	 * @param PlayerCurrency The player's available currency
	 * @return True if purchase is valid
	 */
	bool ValidatePurchaseRules(int32 ItemID, int32 Quantity, float PlayerCurrency) const;

	/**
	 * Publish domain event safely on GameThread
	 * @param EventFunction The event function to execute
	 */
	template<typename Func>
	void PublishDomainEvent(Func&& EventFunction);
};
