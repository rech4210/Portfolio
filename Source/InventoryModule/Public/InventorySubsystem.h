
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

	TScriptInterface<IInventoryRepositoryInterface> GetInventoryRepository() const;
	void SetInventoryRepository(TScriptInterface<IInventoryRepositoryInterface> Repository);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UInventoryDomainService* GetDomainService();

	// ============================================================================
	// Use Case Orchestration - App Layer Responsibilities Only
	// ============================================================================

	void RequestAddItemToInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FInventoryItemDTO& Item);
	void RequestRemoveItemFromInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FName& ItemID, int32 Quantity);
	void RequestLoadPlayerInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity);
	void RequestSavePlayerInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FInventoryDomain& InventoryData);

private:
	UPROPERTY()
	TScriptInterface<IInventoryRepositoryInterface> InventoryRepositoryInterface;

	UPROPERTY()
	UInventoryRepository* DefaultInventoryRepository;

	UPROPERTY()
	UInventoryDomainService* DomainService;
};