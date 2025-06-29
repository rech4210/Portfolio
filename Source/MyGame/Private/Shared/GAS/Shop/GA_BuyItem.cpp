#include "Shared/GAS/Shop/GA_BuyItem.h"
#include "GameSharedModule/Public/Data/ItemDataAsset.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "InventoryModule/Public/InventoryComponent.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "ShopModule/Public/ShopManager.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "LoggingModule/Public/LoggingManager.h"

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

	const AGGwaPlayerState* PlayerState = Cast<AGGwaPlayerState>(ActorInfo->OwnerActor.Get());
	const UInventoryComponent* InventoryComponent = PlayerState ? PlayerState->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!PlayerState || !InventoryComponent)
	{
		return false;
	}

	return true;
}

void UGA_BuyItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const UItemDataAsset* ItemToBuy = Cast<UItemDataAsset>(TriggerEventData->OptionalObject);
	const AShopManager* ShopManager = Cast<AShopManager>(TriggerEventData->Target);

	if (!ItemToBuy || !ShopManager)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* BuyerCharacter = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	UInventoryComponent* InventoryComponent = ActorInfo->OwnerActor->FindComponentByClass<UInventoryComponent>();
	UGGwaAbilitySystemComponent* ASC = Cast<UGGwaAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	if (!BuyerCharacter || !InventoryComponent || !ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!ShopManager->CanBuyItem(BuyerCharacter, ItemToBuy))
	{
		// ASC->ClientSendGameplayEventToActor(ActorInfo->AvatarActor.Get(), ErrorTag_ItemNotFound, FGameplayEventData());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!InventoryComponent->HasEnoughSpace(ItemToBuy, 1))
	{
		// ASC->ClientSendGameplayEventToActor(ActorInfo->AvatarActor.Get(), ErrorTag_InventoryFull, FGameplayEventData());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayEffectSpecHandle CostSpecHandle = ASC->MakeOutgoingSpec(ItemToBuy->CostGE, GetAbilityLevel(), MakeEffectContext(Handle, ActorInfo));
	if (!CostSpecHandle.IsValid() || !ASC->CheckCost(CostSpecHandle))
	{
		// ASC->ClientSendGameplayEventToActor(ActorInfo->AvatarActor.Get(), ErrorTag_NotEnoughGold, FGameplayEventData());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ASC->ApplyGameplayEffectSpecToSelf(*CostSpecHandle.Data.Get());
	InventoryComponent->AddItem(ItemToBuy->GetClass(), 1);

	// if (ULoggingManager* LoggingManager = ActorInfo->GameInstance->GetSubsystem<ULoggingManager>())
	// {
		// const FString PlayerID = BuyerCharacter->GetName(); 
		// const FString Message = FString::Printf(TEXT("Player '%s' bought item '%s'."), *PlayerID, *ItemToBuy->GetName());
		// LoggingManager->LogInfo(Message);
	// }

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
} 