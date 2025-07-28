
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InventoryRepositoryInterface.h"
#include "Interface/PlayerIdentityInterface.h"
#include "InventorySubsystem.generated.h"

class UInventoryComponent;
class UInventoryRepository;
class UInventoryDomainService;
struct FInventoryItemDTO;
struct FInventoryDomain;

/**
 * Inventory Subsystem - Pure Repository Management
 * Responsibility: Only manages repository instances and provides DI for domain services
 * Does NOT contain business logic, validation, or event handling
 */
UCLASS()
class INVENTORYMODULE_API UInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Get the current repository interface (for dependency injection)
	 * @return Current repository implementation
	 */
	TScriptInterface<IInventoryRepositoryInterface> GetInventoryRepository() const;

	/**
	 * Set repository implementation (Dependency Injection)
	 * @param Repository The repository implementation to use (MySQL/NoSQL/etc)
	 */
	void SetInventoryRepository(TScriptInterface<IInventoryRepositoryInterface> Repository);

	/**
	 * Create and configure a Domain Service instance
	 * @return Configured domain service with repository dependency injected
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryDomainService* GetDomainService();

	// ============================================================================
	// Use Case Orchestration - App Layer Responsibilities Only
	// ============================================================================

	/**
	 * Request item addition to inventory with full validation pipeline
	 * Handles network authority, logging, and transaction boundaries
	 * @param PlayerState Target player state containing InventoryComponent
	 * @param Item Item data to add
	 */
	void RequestAddItemToInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FInventoryItemDTO& Item);

	/**
	 * Request item removal from inventory with full validation pipeline
	 * Handles network authority, logging, and transaction boundaries
	 * @param PlayerState Target player state containing InventoryComponent
	 * @param ItemID Item ID to remove
	 * @param Quantity Quantity to remove
	 */
	void RequestRemoveItemFromInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FName& ItemID, int32 Quantity);

	/**
	 * Request player inventory loading with full validation pipeline
	 * Handles network authority, logging, and transaction boundaries
	 * @param PlayerState Target player state containing InventoryComponent
	 */
	void RequestLoadPlayerInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity);

	/**
	 * Request player inventory saving with full validation pipeline
	 * Handles network authority, logging, and transaction boundaries
	 * @param PlayerState Target player state containing InventoryComponent
	 * @param InventoryData The inventory domain data to save
	 */
	void RequestSavePlayerInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FInventoryDomain& InventoryData);

private:
	// Repository interface for Dependency Injection
	UPROPERTY()
	TScriptInterface<IInventoryRepositoryInterface> InventoryRepositoryInterface;

	// Default concrete implementation
	UPROPERTY()
	UInventoryRepository* DefaultInventoryRepository;

	UPROPERTY()
	UInventoryDomainService* DomainService;
};