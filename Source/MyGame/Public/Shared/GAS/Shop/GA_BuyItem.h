#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GA_BuyItem.generated.h"

class UItemDataAsset;
class AShopSeller;

/**
 * Handles the logic for a player to buy an item from a shop.
 * This ability should be activated on the server.
 * It coordinates with the ShopManager, InventoryComponent, and the ASC.
 */
UCLASS()
class MYGAME_API UGA_BuyItem : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_BuyItem();

	/** 
	 * The main execution entry point for the ability.
	 * Expects the ItemDataAsset to buy in TriggerEventData->OptionalObject and the ShopManager actor in TriggerEventData->TargetData.
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Checks if the ability can be activated */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag ErrorTag_ItemNotFound;

	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag ErrorTag_NotEnoughGold;

	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag ErrorTag_InventoryFull;
}; 