
#pragma once

#include "CoreMinimal.h"
#include "InventoryDomain.h"
#include "UObject/Object.h"
#include "Tasks/Task.h"
#include "Interface/PlayerIdentityInterface.h"
#include "InventoryDomainService.generated.h"

class UInventoryComponent;
class IInventoryRepositoryInterface;
struct FInventoryItemDTO;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryOperationSucceeded, TScriptInterface<IPlayerIdentityInterface> /* PlayerIdentity */, const FString& /* Operation */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryOperationFailed, TScriptInterface<IPlayerIdentityInterface> /* PlayerIdentity */, const FString& /* Reason */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryLoadCompleted, TScriptInterface<IPlayerIdentityInterface> /* PlayerIdentity */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventorySaveCompleted, TScriptInterface<IPlayerIdentityInterface> /* PlayerIdentity */);

UCLASS(BlueprintType)
class INVENTORYMODULE_API UInventoryDomainService : public UObject
{
	GENERATED_BODY()

public:
	UInventoryDomainService();

	void Initialize(TScriptInterface<IInventoryRepositoryInterface> Repository);

	UE::Tasks::TTask<void> AddItemToInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FInventoryItemDTO& Item);
	UE::Tasks::TTask<void> RemoveItemFromInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FName& ItemID, int32 Quantity);
	UE::Tasks::TTask<void> LoadInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity);
	UE::Tasks::TTask<void> SaveInventory(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, const FInventoryDomain& InventoryData);

	FOnInventoryOperationSucceeded OnInventoryOperationSucceeded;
	FOnInventoryOperationFailed OnInventoryOperationFailed;
	FOnInventoryLoadCompleted OnInventoryLoadCompleted;
	FOnInventorySaveCompleted OnInventorySaveCompleted;

private:
	UPROPERTY()
	TScriptInterface<IInventoryRepositoryInterface> InventoryRepository;

	void SubscribeToDomainEvents(UInventoryComponent* InventoryComponent);
	void UnsubscribeFromDomainEvents(UInventoryComponent* InventoryComponent);
	
	UFUNCTION()
	void OnDomainItemAdded(UFInventoryItem* AddedItem);

	UFUNCTION()
	void OnDomainItemRemoved(const FName& ItemID, int32 Quantity);

	UFUNCTION()
	void OnDomainInventoryChanged();
};
