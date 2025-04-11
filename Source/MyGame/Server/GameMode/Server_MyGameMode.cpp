// Fill out your copyright notice in the Description page of Project Settings.


#include "Server_MyGameMode.h"

#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"


AServer_MyGameMode::AServer_MyGameMode() {
	PlayerController = AGGwaPlayerController::StaticClass();
	PlayerState = AGGwaPlayerState::StaticClass();
	Character = AGGwaCharacter::StaticClass();
	Test_Ability = UGA_Skill1::StaticClass();
	Test_Effect = UGA_Skill1::StaticClass();
}

// 서버 접속시 플레이어 정보 설정
void AServer_MyGameMode::PostLogin(APlayerController* NewPlayer) {
	Super::PostLogin(NewPlayer);
	AGGwaPlayerState* State = NewPlayer->GetPlayerState<AGGwaPlayerState>();
	// if (State) {
	// 	UAbilitySystemComponent* ASC = State->GetAbilitySystemComponent();
	// 	FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(FGameplayAbilitySpec(Test_Ability, 1, 0));
	// 	ASC->TryActivateAbility(AbilitySpecHandle);
	// 	
	// 	//Attribute 초기화용 GE 적용
	// 	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	// 	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Test_Effect,1,EffectContextHandle);
	// 	if (SpecHandle.IsValid()) {
	// 		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	// 		// ? ASC->ApplyGameplayEffectToSelf()
	// 	}
	// }
}
