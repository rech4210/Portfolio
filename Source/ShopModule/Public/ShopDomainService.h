// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/World.h"
#include "Async/TaskGraphInterfaces.h"
#include "Tasks/Task.h"
#include "DatabaseModule/Public/DatabaseManager.h" // Import DTO types from DatabaseModule
#include "ShopDomain.h" // Keep for FShopPurchaseResult only
#include "ShopDomainService.generated.h"

class APlayerState;
class UShopComponent;
class IShopRepositoryInterface;
struct FShopItemDTO;

// Domain events for shop operations
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemPurchased, APlayerState* /* PlayerState */, int32 /* ItemID */, int32 /* Quantity */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnShopLoaded, APlayerState* /* PlayerState */, const FShopDomain& /* ShopData */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopSaved, const FShopDomain& /* ShopData */);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnShopItemAdded, int32 /* ShopID */, int32 /* ItemID */, const FShopItemDTO& /* Item */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnShopItemRemoved, int32 /* ShopID */, int32 /* ItemID */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopRestocked, int32 /* ShopID */);

/**
 * Domain service responsible for orchestrating shop business logic operations.
 * Handles thread coordination between Worker threads (database) and GameThread (UE objects).
 */
UCLASS()
class SHOPMODULE_API UShopDomainService : public UObject
{
	GENERATED_BODY()

public:
	UShopDomainService();

	// Domain events
	FOnItemPurchased OnItemPurchased;
	FOnShopLoaded OnShopLoaded;
	FOnShopSaved OnShopSaved;
	FOnShopItemAdded OnShopItemAdded;
	FOnShopItemRemoved OnShopItemRemoved;
	FOnShopRestocked OnShopRestocked;

	/**
	 * Initialize the domain service with repository dependency
	 * @param InRepository The shop repository interface for data access
	 */
	void Initialize(TScriptInterface<IShopRepositoryInterface> InRepository);

	/**
	 * Purchase an item from a shop
	 * @param PlayerState The player making the purchase
	 * @param ShopID The shop identifier
	 * @param ItemID The item to purchase
	 * @param Quantity The quantity to purchase
	 * @param PlayerCurrency The player's available currency
	 */
	void PurchaseItem(
		APlayerState* PlayerState, 
		int32 ShopID, 
		int32 ItemID, 
		int32 Quantity, 
		float PlayerCurrency
	);

	/**
	 * Load shop data for a player's current shop
	 * @param PlayerState The player requesting shop data
	 * @param ShopID The shop identifier to load
	 */
	void LoadShop(APlayerState* PlayerState, int32 ShopID);

	/**
	 * Save shop data changes
	 * @param ShopData The shop domain data to save
	 */
	void SaveShop(const FShopDomain& ShopData);

	/**
	 * Add a new item to a shop
	 * @param ShopID The shop identifier
	 * @param ItemDTO The item data to add
	 */
	void AddItemToShop(int32 ShopID, const FShopItemDTO& ItemDTO);

	/**
	 * Remove an item from a shop
	 * @param ShopID The shop identifier
	 * @param ItemID The item to remove
	 */
	void RemoveItemFromShop(int32 ShopID, int32 ItemID);

	/**
	 * Update item stock in a shop
	 * @param ShopID The shop identifier
	 * @param ItemID The item to update
	 * @param NewStock The new stock amount
	 */
	void UpdateItemStock(int32 ShopID, int32 ItemID, int32 NewStock);

	/**
	 * Update item price in a shop
	 * @param ShopID The shop identifier
	 * @param ItemID The item to update
	 * @param NewPrice The new price
	 */
	void UpdateItemPrice(int32 ShopID, int32 ItemID, float NewPrice);

	/**
	 * Restock all items in a shop
	 * @param ShopID The shop identifier
	 */
	void RestockShop(int32 ShopID);

	/**
	 * Set shop open/closed status
	 * @param ShopID The shop identifier
	 * @param bIsOpen Whether the shop should be open
	 */
	void SetShopStatus(int32 ShopID, bool bIsOpen);

	/**
	 * Get shop information without modifying anything
	 * @param ShopID The shop identifier
	 */
	void GetShopInfo(int32 ShopID);

private:
	// Repository interface for data access (Worker thread)
	TScriptInterface<IShopRepositoryInterface> Repository;

	/**
	 * Validates business rules for item purchase
	 * @param ShopData The shop domain object
	 * @param ItemID The item to purchase
	 * @param Quantity The quantity to purchase
	 * @param PlayerCurrency The player's available currency
	 * @return True if purchase is valid
	 */
	bool ValidatePurchaseRules(const FShopDomain& ShopData, int32 ItemID, int32 Quantity, float PlayerCurrency) const;

	/**
	 * Validates business rules for item addition
	 * @param ShopData The shop domain object
	 * @param ItemDTO The item to add
	 * @return True if addition is valid
	 */
	bool ValidateItemAdditionRules(const FShopDomain& ShopData, const FShopItemDTO& ItemDTO) const;

	/**
	 * Publishes domain events on the GameThread
	 * @param EventFunction The event function to execute
	 */
	template<typename Func>
	void PublishDomainEvent(Func&& EventFunction);
};
