
#pragma once

#include "CoreMinimal.h"
#include "Data/BaseDataAsset.h"
#include "ItemDataAsset.generated.h"
UENUM(BlueprintType)
enum class EItemType : uint8
{
	Consumable,
	Equipment,
	Quest,
	Misc
};

class UGameplayEffect;
UCLASS()
class GAMESHAREDMODULE_API UItemDataAsset : public UBaseDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 ItemID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 CurrentCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 MaxStackCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 Price;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> UseEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> CostGE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Buff")
	TArray<FPrimaryAssetId> AppliedBuffs;

	UFUNCTION(BlueprintCallable, Category = "Item")
	FName GetItemID() const
	{
		return FName(*FString::FromInt(ItemID));
	}
}; 