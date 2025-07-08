#include "Shared/GAS/Shop/GA_BuyItem.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "InventoryModule/Public/InventoryComponent.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "ShopModule/Public/ShopSeller.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "LoggingModule/Public/LoggingManager.h"
#include "ShopModule/Public/TradingService.h"

UGA_BuyItem::UGA_BuyItem()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	//TODO: ADD Tag Data ini.
	// // Default error tags
	// ErrorTag_ItemNotFound = FGameplayTag::RequestGameplayTag(FName("Shop.Error.ItemNotFound"));
	// ErrorTag_NotEnoughGold = FGameplayTag::RequestGameplayTag(FName("Shop.Error.NotEnoughGold"));
	// ErrorTag_InventoryFull = FGameplayTag::RequestGameplayTag(FName("Shop.Error.InventoryFull"));
}

bool UGA_BuyItem::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Basic checks to ensure we have the necessary actors.
	if (!ActorInfo || !ActorInfo->OwnerActor.IsValid() || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	return true;
}

void UGA_BuyItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	ACharacter* BuyerCharacter = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	const AShopSeller* ShopSeller = Cast<AShopSeller>(TriggerEventData->Target);
	const UItemDataAsset* ItemToBuy = Cast<UItemDataAsset>(TriggerEventData->OptionalObject);

	if (!BuyerCharacter || !ItemToBuy)
	{
		// Cancel ability if prerequisites are not met.
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	// Create an instance of the Domain Service to handle the transaction.
	UTradingService* TradingService = NewObject<UTradingService>();
	
	// Delegate the complex logic to the domain service.
	const bool bSuccess = TradingService->AttemptToBuyItem(BuyerCharacter, ShopSeller, ItemToBuy, 1);

	if (!bSuccess)
	{
		// The TradingService is responsible for logging details.
		// The ability's job is just to end and possibly send a feedback tag to the UI.
		// For example: SendGameplayEvent(ErrorTag_...);
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	// End the ability successfully.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
} 