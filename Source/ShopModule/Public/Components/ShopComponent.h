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
	// Aggregate Root Methods with Business Rule Validation
	// ============================================================================

	/**
	 * Purchase an item with business rule validation
	 * @param ItemID The item to purchase
	 * @param Quantity The quantity to purchase
	 * @param PlayerCurrency The player's available currency
	 * @return True if purchase was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Business Logic")
	bool PurchaseItemWithValidation(int32 ItemID, int32 Quantity, float PlayerCurrency);

	/**
	 * Add shop item with business rule validation
	 * @param ItemState The item state to add
	 * @return True if addition was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Management")
	bool AddShopItem(const FShopItemState& ItemState);

	/**
	 * Remove shop item with business rule validation
	 * @param ItemID The item ID to remove
	 * @return True if removal was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Management")
	bool RemoveShopItem(int32 ItemID);

	/**
	 * Update shop item with business rule validation
	 * @param ItemID The item ID to update
	 * @param NewStock The new stock amount
	 * @param NewPrice The new price
	 * @return True if update was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Management")
	bool UpdateShopItem(int32 ItemID, int32 NewStock, float NewPrice);

	/**
	 * Update item stock with business rule validation
	 * @param ItemID The item ID to update
	 * @param NewStock The new stock amount
	 * @return True if update was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Management")
	bool UpdateItemStock(int32 ItemID, int32 NewStock);

	/**
	 * Update item price with business rule validation
	 * @param ItemID The item ID to update
	 * @param NewPrice The new price
	 * @return True if update was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Management")
	bool UpdateItemPrice(int32 ItemID, float NewPrice);

	// ============================================================================
	// Getters and Query Methods
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	const TArray<FShopItemState>& GetAllShopItems() const { return ShopItems; }

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	FShopItemState GetShopItem(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	int32 GetShopID() const { return ShopID; }

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	bool IsShopOpen() const { return bIsShopOpen; }

	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	float GetGlobalPriceModifier() const { return GlobalPriceModifier; }

	/**
	 * Check if an item can be purchased
	 * @param ItemID The item to check
	 * @param Quantity The desired quantity
	 * @param PlayerCurrency The player's available currency
	 * @return True if item can be purchased
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	bool CanPurchaseItem(int32 ItemID, int32 Quantity, float PlayerCurrency) const;

	/**
	 * Get the effective price of an item (including global modifier)
	 * @param ItemID The item ID
	 * @return The effective price, or 0.0 if item not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	float GetItemEffectivePrice(int32 ItemID) const;

	/**
	 * Get available items (in stock and available)
	 * @return Array of available items
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Queries")
	TArray<FShopItemState> GetAvailableItems() const;

	// ============================================================================
	// Business Rule Validation
	// ============================================================================

	/**
	 * Validate if a shop item follows business rules
	 * @param Item The item to validate
	 * @param Quantity The quantity for validation
	 * @return True if item passes validation
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Validation")
	bool ShopItemRuleCheck(FShopItemState Item, int32 Quantity) const;

	/**
	 * Validate purchase transaction rules
	 * @param ItemID The item to purchase
	 * @param Quantity The quantity to purchase
	 * @param PlayerCurrency The player's available currency
	 * @return True if purchase is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Validation")
	bool ValidatePurchaseRules(int32 ItemID, int32 Quantity, float PlayerCurrency) const;

	/**
	 * Validate item addition rules
	 * @param ItemState The item to add
	 * @return True if addition is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Validation")
	bool ValidateItemAdditionRules(const FShopItemState& ItemState) const;

	// ============================================================================
	// Shop Configuration
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

	/**
	 * Set global price modifier (server authority required)
	 * @param NewModifier The new price modifier (0.1 to 10.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Shop|Configuration")
	void SetGlobalPriceModifier(float NewModifier);

	// ============================================================================
	// Domain Integration
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

	UFUNCTION()
	virtual void OnRep_GlobalPriceModifier();

private:
	// Maximum number of items this shop can hold
	UPROPERTY(EditDefaultsOnly, Category = "Shop|Configuration")
	int32 MaxShopItems = 50;

	// Minimum price for any item
	UPROPERTY(EditDefaultsOnly, Category = "Shop|Configuration")
	float MinItemPrice = 1.0f;

	// Maximum price for any item
	UPROPERTY(EditDefaultsOnly, Category = "Shop|Configuration")
	float MaxItemPrice = 999999.0f;

	/**
	 * Publish domain event safely on GameThread
	 * @param EventFunction The event function to execute
	 */
	template<typename Func>
	void PublishDomainEvent(Func&& EventFunction);

	/**
	 * Validate server authority for operations
	 * @return True if server has authority
	 */
	bool ValidateServerAuthority() const;

	/**
	 * Notify state change and trigger replication
	 */
	void NotifyStateChanged();
};
