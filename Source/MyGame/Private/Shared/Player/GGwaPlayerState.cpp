#include "Shared/Player/GGwaPlayerState.h"

#include "Data/SkillDataAsset.h"
#include "Entities/SkillSlot.h"
#include "MyGame/Public/Shared/GAS/GGwaAbilitySystemComponent.h"
#include "MyGame/Public/Shared/GAS/GGwaAttributeSet.h"
#include "InventoryModule/Public/InventoryComponent.h"
#include "InventoryModule/Public/InventoryDomainService.h"
#include "MyGame/Public/Shared/Player/GGwaCharacter.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerController.h"
#include "MyGame/Public/Shared/Player/Component/UPlayerStateComponent.h"
#include "SkillModule/Public/Components/SkillComponent.h"
#include "SkillModule/Public/SkillDomainService.h"
#include "ShopModule/Public/Components/ShopComponent.h"
#include "EquipmentModule/Public/Components/EquipmentComponent.h"
#include "GameSharedModule/Public/Utill/UEnumTagMatchHelper.h"
#include "InventorySubsystem.h"
#include "SkillModule/Public/SkillSubsystem.h"
#include "ShopModule/Public/ShopSubsystem.h"
#include "EquipmentModule/Public/EquipmentSubsystem.h"
#include "Interface/PlayerIdentityInterface.h"


AGGwaPlayerState::AGGwaPlayerState() {
	/*?�질?�인 ?�레?�어가 ?�용?�게 ??컴포?�트 게임 로직 체크?� 메모�?변경을 ?�당*/
	ASC = CreateDefaultSubobject<UGGwaAbilitySystemComponent>("ASC");
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UGGwaAttributeSet>("AttributeSet");
	StateComponent = CreateDefaultSubobject<UPlayerStateComponent>("PlayerStateComponent");
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");
	SkillComponent = CreateDefaultSubobject<USkillComponent>("SkillComponent");
	ShopComponent = CreateDefaultSubobject<UShopComponent>("ShopComponent");
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>("EquipmentComponent");
	/*�????�속?? DB 추상 계층?� SubSystem - Repository (var DBmanager) �?처리??�?*/
}

void AGGwaPlayerState::BeginPlay() {
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		// Use DDD-style domain services instead of direct subsystem calls
		if (UInventorySubsystem* InventorySubsystem = GetGameInstance()->GetSubsystem<UInventorySubsystem>())
		{
			if (UInventoryDomainService* InventoryDomainService = InventorySubsystem->GetDomainService())
			{
				// Use domain service for inventory operations
				// auto LoadTask = InventoryDomainService->LoadInventory(this);
				// Task will handle completion callbacks automatically
			}
		}

		if (USkillSubsystem* SkillSubsystem = GetGameInstance()->GetSubsystem<USkillSubsystem>())
		{
			if (USkillDomainService* SkillDomainService = SkillSubsystem->GetDomainService())
			{
				// Use domain service for skill operations
				// SkillDomainService->LoadSkills(this);
				// Task will handle completion callbacks automatically
				
				// Subscribe to domain service events
				SkillDomainService->OnSkillLoadCompleted.AddUObject(this, &AGGwaPlayerState::OnSkillLoadCompleted);
				SkillDomainService->OnSkillOperationFailed.AddUObject(this, &AGGwaPlayerState::OnSkillOperationFailed);
			}
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

UInventoryComponent* AGGwaPlayerState::GetInventoryComponent() const {
	return InventoryComponent.Get();
}

void AGGwaPlayerState::OnSkillSlotsUpdated() const{
	auto* Controller = Cast<AGGwaPlayerController>(GetPlayerController());
	if (!Controller || !SkillComponent)
	{
		return;
	}
	Controller->ProcessSkillData(SkillComponent);
}

// void AGGwaPlayerState::SetSkillComponent(USkillComponent* NewComponent) {
//     SkillComponent = NewComponent;
// 	if (!SkillComponent) {
// 		return;
// 	}
//     for (auto Element : SkillComponent->GetAllSkillSlots()) {
//     	UE_LOG(LogTemp, Warning, TEXT("===SkillComponent SetSkillComponent: SkillSlot %d, Id %s ==="), Element->SkillData->InputSlot, *Element->SkillData->Description.ToString());
//     }
// }

void AGGwaPlayerState::BroadcastAttributeChange(const FGameplayAttribute& Attribute, float NewValue) const{
	// OnAttributeChanged.Broadcast(Attribute, NewValue, SkillData);
	// value ?�정??helper�?
	// FObservedAttributeHelper<> Player??status enum ?�요
	if (HasAuthority()) {
		if (Attribute == AttributeSet->GetHealthAttribute()) {
			if (NewValue <= 0.0f) {
				// Component???�청?�것.
				// SetPlayerDeathState();
				ASC->AddLooseGameplayTag(UEnumTagMatchHelper::GetTagFromEnum(EPlayerState::Dead));
			
			}
		}
	}
}


//server? client? Deprecated -> tag 개념?�로 ASC???�임?�여 ?�태 처리�?
// void AGGwaPlayerState::SetPlayerDeathState() const{
	// StateComponent->OnDeath();
	// if (Character && Character->GetReactionComponent())
	// {
	// 	Character->GetReactionComponent()->ExecuteDeadReaction();
	// }
// }

void AGGwaPlayerState::OnSkillLoadCompleted(const FGuid& PlayerGuid)
{
	if (PlayerGuid == GetPlayerGuid())
	{
		UE_LOG(LogTemp, Log, TEXT("Skills loaded successfully for player %s"), *GetPlayerName());
		// Notify UI or other systems that skills are ready
		OnSkillsUpdated.Broadcast();
	}
}

void AGGwaPlayerState::OnSkillOperationFailed(const FGuid& PlayerGuid, const FString& Reason)
{
	if (PlayerGuid == GetPlayerGuid())
	{
		UE_LOG(LogTemp, Error, TEXT("Skill operation failed for player %s: %s"), *GetPlayerName(), *Reason);
		// Handle skill operation failure (show UI notification, etc.)
	}
}