#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryItem.h"
#include "InventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryItemAdded, UFInventoryItem* /* AddedItem */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryItemRemoved, const FName& /* ItemID */, int32 /* Quantity */);
DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INVENTORYMODULE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Server_SetInventoryItems(const TArray<UFInventoryItem*>& InItems);
	const TArray<UFInventoryItem*>& GetItems() const { return Items; }
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasItem(const FName& ItemID) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasEnoughSpace(int32 RequiredSpace = 1) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UFInventoryItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(const FName& ItemID);

	bool CanAddItem(const struct FInventoryItemDTO& Item) const;
	bool CanRemoveItem(const FName& ItemID, int32 Quantity = 1) const;
	bool AddItemDirect(const struct FInventoryItemDTO& Item);
	bool RemoveItemDirect(const FName& ItemID, int32 Quantity = 1);

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;
	FOnInventoryItemAdded OnInventoryItemAdded;
	FOnInventoryItemRemoved OnInventoryItemRemoved;
	FOnInventoryChanged OnInventoryChanged;

	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<TObjectPtr<UFInventoryItem>> Items;

	UFUNCTION()
	void OnRep_Items();
};