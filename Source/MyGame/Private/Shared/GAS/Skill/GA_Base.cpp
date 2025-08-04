

#include "Shared/GAS/Skill/GA_Base.h"
#include "AbilitySystemInterface.h"
#include "MyGame/Public/Shared/Player/GGwaCharacter.h"
#include "MyGame/Public/Shared/GAS/GGwaAbilitySystemComponent.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MyGame/Public/Shared/AI/BossCharacter.h"
#include "MyGame/Public/Shared/AI/EnemyAbilitySystemComponent.h"
#include "MyGame/Public/Shared/AI/EnemyAttributeSet.h"
#include "GameSharedModule/Public/Data/EGasDataType.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerState.h"
#include "GameSharedModule/Public/Utill/UEnumTagMatchHelper.h"

const FGameplayTag UGA_Base::SkillAssetTypeTag = UEnumTagMatchHelper::GetTagFromEnum(EGasDataType::SkillID);

void UGA_Base::PreProcessSkillStart(const FGameplayAbilityActorInfo* ActorInfo) {
	AGGwaCharacter * AvatarActor = Cast<AGGwaCharacter>(ActorInfo->AvatarActor.Get());
	if (IsLocallyControlled()) {
		UE_LOG(LogTemp, Warning, TEXT("??Client PreProcess Activated"));
		UGGwaAbilitySystemComponent* ASC = Cast<UGGwaAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
		
		if (AGGwaPlayerController * PC = Cast<AGGwaPlayerController>(ActorInfo->PlayerController.Get())) {
			// FHitResult Hit;
			// if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit)) {
			// 	CacheHitLocation = Hit.ImpactPoint;
			// 	FVector Dir = Hit.ImpactPoint - AvatarActor->GetActorLocation();
			// 	Dir.Z = 0;
			// 	
			// 	AvatarActor->SetActorRotation(Dir.Rotation());
			// 	FGameplayEventData RotationEventData;
			// 	RotationEventData.Instigator = AvatarActor;
			// 	RotationEventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);
			// 	ActorInfo->AbilitySystemComponent->HandleGameplayEvent(RotationEventData.EventTag ,&RotationEventData);
			// }
		}
	}
}

UAbilitySystemComponent* UGA_Base::GetTargetASC(AActor* Actor) const {
	if (AGGwaCharacter* Character = Cast<AGGwaCharacter>(Actor)){
		if (AGGwaPlayerState* PS = Cast<AGGwaPlayerState>(Character->GetPlayerState())) {
			return PS->GetAbilitySystemComponent();
		}
		return nullptr;
	}
	if (ABossCharacter* Boss = Cast<ABossCharacter>(Actor)) {
		if (UAbilitySystemComponent* ASC = Boss->GetAbilitySystemComponent()) {
			return ASC;
		}
		return nullptr;
	}
	return nullptr;
}

FSkillContext UGA_Base::BuildSkillContext(const FGameplayAbilityActorInfo* ActorInfo) {
	FSkillContext Context;
	Context.SourceASC = ActorInfo->AbilitySystemComponent.Get();
	Context.SourceActor = ActorInfo->AvatarActor.Get();
	Context.TargetLocation = CacheHitLocation;
	return Context;
}
