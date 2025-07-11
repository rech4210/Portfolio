// #pragma once
//
// #include "CoreMinimal.h"
// #include "Abilities/GameplayAbility.h"
// #include "GameplayTagContainer.h"
// #include "GA_SellItem.generated.h"
//
// class UItemDataAsset;
// class AShopSeller;
//
// /**
//  * Handles the logic for a player to sell an item to a shop.
//  * This ability should be activated on the server.
//  */
// UCLASS()
// class MYGAME_API UGA_SellItem : public UGameplayAbility
// {
// 	GENERATED_BODY()
//
// public:
// 	UGA_SellItem();
//
// 	/** The main execution entry point for the ability. */
// 	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
//
// 	/** Checks if the ability can be activated */
// 	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
//
// 	UPROPERTY(EditAnywhere)
// 	TSubclassOf<UGameplayEffect> GE_SellItem;
// protected:
// 	UPROPERTY(EditDefaultsOnly, Category = "Tags")
// 	FGameplayTag ErrorTag_ItemNotOwned;
//
// 	UPROPERTY(EditDefaultsOnly, Category = "Tags")
// 	FGameplayTag ErrorTag_CannotBeSold;
// }; 