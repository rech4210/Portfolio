#include "Shared/GAS/Shop/GA_SellItem.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "InventoryModule/Public/InventoryComponent.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "ShopModule/Public/ShopManager.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"

UGA_SellItem::UGA_SellItem()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	// TODO: Add Tag Data ini.
	// ErrorTag_ItemNotOwned = FGameplayTag::RequestGameplayTag(FName("Shop.Error.ItemNotOwned"));
	// ErrorTag_CannotBeSold = FGameplayTag::RequestGameplayTag(FName("Shop.Error.CannotBeSold"));
}

bool UGA_SellItem::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	const AGGwaPlayerState* PlayerState = Cast<AGGwaPlayerState>(ActorInfo->OwnerActor.Get());
	const UInventoryComponent* InventoryComponent = PlayerState ? PlayerState->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!PlayerState || !InventoryComponent)
	{
		return false;
	}

	return true;
}

void UGA_SellItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	const UItemDataAsset* ItemToSell = Cast<UItemDataAsset>(TriggerEventData->OptionalObject);
	const AShopManager* ShopManager = Cast<AShopManager>(TriggerEventData->Target);

	if (!ItemToSell || !ShopManager)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* SellerCharacter = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	UInventoryComponent* InventoryComponent = ActorInfo->OwnerActor->FindComponentByClass<UInventoryComponent>();
	UGGwaAbilitySystemComponent* ASC = Cast<UGGwaAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	if (!SellerCharacter || !InventoryComponent || !ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 1. Check if player owns the item
	if (!InventoryComponent->HasItem(ItemToSell, 1))
	{
		// ASC->ClientSendGameplayEventToActor(ActorInfo->AvatarActor.Get(), ErrorTag_ItemNotOwned, FGameplayEventData());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2. Check with ShopManager if the item can be sold
	// if (!ShopManager->CanSellItem(SellerCharacter, ItemToSell))
	// {
	// 	// ASC->ClientSendGameplayEventToActor(ActorInfo->AvatarActor.Get(), ErrorTag_CannotBeSold, FGameplayEventData());
	// 	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	// 	return;
	// }

	// 3. Apply the gold effect to the player
	if (GE_SellItem)
	{
		FGameplayEffectSpecHandle ValueSpecHandle = ASC->MakeOutgoingSpec(GE_SellItem, GetAbilityLevel(), MakeEffectContext(Handle, ActorInfo));
		if (ValueSpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*ValueSpecHandle.Data.Get());
		}
	}
	
	// 4. Remove the item from inventory
	InventoryComponent->RemoveItem(ItemToSell->GetClass(), 1);

	// TODO: Add logging for sale

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
} 