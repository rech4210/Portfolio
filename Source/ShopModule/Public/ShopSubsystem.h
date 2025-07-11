// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IShopRepositoryInterface.h"
#include "ShopSubsystem.generated.h"

class UShopRepository;
class UShopDomainService;
class APlayerState;
class UShopComponent;

/**
 * Shop subsystem managing dependency injection and coordination between repository and domain service.
 * Acts as the entry point for shop operations and manages the DDD infrastructure.
 */
UCLASS()
class SHOPMODULE_API UShopSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Get the shop repository interface for direct data access
	 * @return The shop repository interface
	 */
	TScriptInterface<IShopRepositoryInterface> GetShopRepository() const;

	/**
	 * Get the shop domain service for business logic operations
	 * @return The shop domain service
	 */
	UShopDomainService* GetShopDomainService() const;

	// ============================================================================
	// DDD Entry Points
	// ============================================================================

	/**
	 * Purchase an item from a shop (DDD operation)
	 * @param PlayerState The player making the purchase
	 * @param ShopID The shop identifier
	 * @param ItemID The item to purchase
	 * @param Quantity The quantity to purchase
	 * @param PlayerCurrency The player's available currency
	 */
	void RequestPurchaseItem(APlayerState* PlayerState, int32 ShopID, int32 ItemID, int32 Quantity, float PlayerCurrency);

	/**
	 * Load shop data for a player (DDD operation)
	 * @param PlayerState The player requesting shop data
	 * @param ShopID The shop identifier to load
	 */
	void RequestLoadShopData(APlayerState* PlayerState, int32 ShopID);

	/**
	 * Add item to shop (DDD operation)
	 * @param ShopID The shop identifier
	 * @param ItemID The item ID to add
	 * @param Stock The initial stock
	 * @param Price The item price
	 */
	void RequestAddItemToShop(int32 ShopID, int32 ItemID, int32 Stock, float Price);

	/**
	 * Remove item from shop (DDD operation)
	 * @param ShopID The shop identifier
	 * @param ItemID The item to remove
	 */
	void RequestRemoveItemFromShop(int32 ShopID, int32 ItemID);

	/**
	 * Restock all items in a shop (DDD operation)
	 * @param ShopID The shop identifier
	 */
	void RequestRestockShop(int32 ShopID);

	// ============================================================================
	// Legacy Entry Points (for backward compatibility)
	// ============================================================================

	/** Entry point for loading shop data. Can be called from PlayerState's BeginPlay. */
	void RequestLoadShopData(APlayerState* PlayerState);

	/** Called by the ShopComponent on clients when shop data is replicated. */
	void Client_OnShopStateUpdated(UShopComponent* ShopComponent);

private:
	// Repository for data access
	UPROPERTY()
	UShopRepository* ShopRepository;

	// Domain service for business logic
	UPROPERTY()
	UShopDomainService* ShopDomainService;

	/**
	 * Initialize domain events subscriptions
	 */
	void SetupDomainEventHandlers();

	/**
	 * Handle item purchased domain event
	 * @param PlayerState The player who purchased
	 * @param ItemID The purchased item ID
	 * @param Quantity The purchased quantity
	 */
	UFUNCTION()
	void OnItemPurchased(APlayerState* PlayerState, int32 ItemID, int32 Quantity);

	/**
	 * Handle shop loaded domain event
	 * @param PlayerState The player for whom shop was loaded
	 * @param ShopData The loaded shop data
	 */
	UFUNCTION()
	void OnShopLoaded(APlayerState* PlayerState, const struct FShopDomain& ShopData);

	/**
	 * Handle shop saved domain event
	 * @param ShopData The saved shop data
	 */
	UFUNCTION()
	void OnShopSaved(const struct FShopDomain& ShopData);
};
