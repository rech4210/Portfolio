// Fill out your copyright notice in the Description page of Project Settings.

#include "Shared/Player/GGwaPlayerState.h"

#include "Data/SkillDataAsset.h"
#include "Entities/SkillSlot.h"
#include "MyGame/Public/Shared/GAS/GGwaAbilitySystemComponent.h"
#include "MyGame/Public/Shared/GAS/GGwaAttributeSet.h"
#include "InventoryModule/Public/InventoryComponent.h"
#include "MyGame/Public/Shared/Player/GGwaCharacter.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerController.h"
#include "MyGame/Public/Shared/Player/Component/UPlayerStateComponent.h"
#include "SkillModule/Public/Components/SkillComponent.h"
#include "ShopModule/Public/Components/ShopComponent.h"
#include "EquipmentModule/Public/Components/EquipmentComponent.h"
#include "GameSharedModule/Public/Utill/UEnumTagMatchHelper.h"
#include "InventorySubsystem.h"
#include "SkillModule/Public/SkillSubsystem.h"
#include "ShopModule/Public/ShopSubsystem.h"
#include "EquipmentModule/Public/EquipmentSubsystem.h"


AGGwaPlayerState::AGGwaPlayerState() {
	ASC = CreateDefaultSubobject<UGGwaAbilitySystemComponent>("ASC");
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UGGwaAttributeSet>("AttributeSet");
	StateComponent = CreateDefaultSubobject<UPlayerStateComponent>("PlayerStateComponent");
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");
	SkillComponent = CreateDefaultSubobject<USkillComponent>("SkillComponent");
	ShopComponent = CreateDefaultSubobject<UShopComponent>("ShopComponent");
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>("EquipmentComponent");
}

void AGGwaPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (UInventorySubsystem* InventorySubsystem = GetGameInstance()->GetSubsystem<UInventorySubsystem>())
		{
			InventorySubsystem->RequestLoadInventory(this);
		}

		if (USkillSubsystem* SkillSubsystem = GetGameInstance()->GetSubsystem<USkillSubsystem>())
		{
			SkillSubsystem->RequestLoadSkillData(this);
		}

		if (UShopSubsystem* ShopSubsystem = GetGameInstance()->GetSubsystem<UShopSubsystem>())
		{
			ShopSubsystem->RequestLoadShopData(this);
		}

		if (UEquipmentSubsystem* EquipmentSubsystem = GetGameInstance()->GetSubsystem<UEquipmentSubsystem>())
		{
			EquipmentSubsystem->RequestLoadEquipmentData(this);
		}
	}
}

void AGGwaPlayerState::InitPlayerState() {
	Character = Cast<AGGwaCharacter>(ASC->GetAvatarActor());
	StateComponent->InitComponent(ASC);
}

UAbilitySystemComponent* AGGwaPlayerState::GetAbilitySystemComponent() const {
	return ASC.Get() ? ASC.Get() : nullptr;
}

UPlayerStateComponent* AGGwaPlayerState::GetStateComponent() const {
	return StateComponent.Get();
}

USkillComponent* AGGwaPlayerState::GetSkillComponent() const {
	return SkillComponent.Get();
}

UShopComponent* AGGwaPlayerState::GetShopComponent() const {
	return ShopComponent.Get();
}

UEquipmentComponent* AGGwaPlayerState::GetEquipmentComponent() const {
	return EquipmentComponent.Get();
}

void AGGwaPlayerState::OnSkillSlotsUpdated() const{
	auto* Controller = Cast<AGGwaPlayerController>(GetPlayerController());
	if (!Controller || !SkillComponent)
	{
		return;
	}
	Controller->Client_ReceiveSkillData(SkillComponent);
}

void AGGwaPlayerState::SetSkillComponent(USkillComponent* NewComponent) {
    SkillComponent = NewComponent;
	if (!SkillComponent) {
		return;
	}
    for (auto Element : SkillComponent->GetAllSkillSlots()) {
    	UE_LOG(LogTemp, Warning, TEXT("===SkillComponent SetSkillComponent: SkillSlot %d, Id %s ==="), Element->SkillData->InputSlot, *Element->SkillData->Description.ToString());
    }
}

void AGGwaPlayerState::BroadcastAttributeChange(const FGameplayAttribute& Attribute, float NewValue) const{
	// OnAttributeChanged.Broadcast(Attribute, NewValue, SkillData);
	// value 판정을 helper로
	// FObservedAttributeHelper<> Player의 status enum 필요
	if (HasAuthority()) {
		if (Attribute == AttributeSet->GetHealthAttribute()) {
			if (NewValue <= 0.0f) {
				// Component에 요청할것.
				// SetPlayerDeathState();
				ASC->AddLooseGameplayTag(UEnumTagMatchHelper::GetTagFromEnum(EPlayerState::Dead));
			
			}
		}
	}
}


//server? client? Deprecated -> tag 개념으로 ASC에 위임하여 상태 처리중.
// void AGGwaPlayerState::SetPlayerDeathState() const{
	// StateComponent->OnDeath();
	// if (Character && Character->GetReactionComponent())
	// {
	// 	Character->GetReactionComponent()->ExecuteDeadReaction();
	// }
// }