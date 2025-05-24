#include "Shared/Player/GGwaCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Shared/GAS/Skill/GA_Skill1.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Shared/Player/GGwaPlayerController.h"

void AGGwaCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	InitASC();
	if (ASC && HasAuthority()) {
		for (int32 i = 0; i < SkillAbilities.Num(); ++i){
			if (SkillAbilities[i]){
				FGameplayAbilitySpec Spec(SkillAbilities[i], 1, static_cast<int32>(EAbilityInputID::Move) + i, this);
				ASC->GiveAbility(Spec);
			}
		}

		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()){
			UE_LOG(LogTemp, Warning, TEXT("Ability: %s | InputID: %d"),
				*GetNameSafe(Spec.Ability), Spec.InputID);
		}
		ASC->RefreshAbilityActorInfo();
	}
}

void AGGwaCharacter::OnRep_PlayerState() {
	Super::OnRep_PlayerState();
	InitASC();
	if (auto PC = GetController()) {
		if (auto GGwaPC = Cast<AGGwaPlayerController>(PC))	{
			GGwaPC->InitClientWidget();
		}
	}
}

void AGGwaCharacter::InitASC() {
	if (AGGwaPlayerState * State = GetPlayerState<AGGwaPlayerState>(); nullptr != State) {
		ASC = Cast<UGGwaAbilitySystemComponent>(State->GetAbilitySystemComponent());
		if (ASC) {
			// ASC의 연결 정보를 부여, ASC의 owner, Replicated 객체를 지정
			ASC->InitAbilityActorInfo(State, this);
		}
	}
}


void AGGwaCharacter::BeginPlay() {
	Super::BeginPlay();
	if (APlayerController * PC = Cast<APlayerController>(GetController()); nullptr != PC) {
		UEnhancedInputLocalPlayerSubsystem * Subsystem = PC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		Subsystem->AddMappingContext(MappingContext, 0);
	}
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

}

void AGGwaCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (false == bIsFollowingPath && !IsLocallyControlled()) return;

	if (!CurrentPath.IsValidIndex(CurrentPathIndex)) {
		bIsFollowingPath = false;
		CurrentPath.Empty();
		CurrentPathIndex = 0;
		GetCharacterMovement()->StopMovementImmediately();
		return;
	}
	
	FVector CurrentLocation = GetActorLocation();
	FVector TargetPoint = CurrentPath[CurrentPathIndex];
	TargetPoint.Z = CurrentLocation.Z;

	FVector Direction = TargetPoint - CurrentLocation;
	float Distance = Direction.Size();
	auto Dir = Direction.GetSafeNormal();
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Skill"))) {
		bIsFollowingPath = false;
		CurrentPath.Empty();
		CurrentPathIndex = 0;
	}
	if (Distance < AcceptanceRadius) {
		CurrentPathIndex++;
		if (!CurrentPath.IsValidIndex(CurrentPathIndex)) {
			bIsFollowingPath = false;
		}
	}
	else {

		bool bCanMove = GetCharacterMovement()->MovementMode != MOVE_None;
		UE_LOG(LogTemp, Warning, TEXT("Can Move (MovementMode != MOVE_None): %d"), bCanMove);

		UE_LOG(LogTemp, Warning, TEXT("Movement Mode: %d"), (int32)GetCharacterMovement()->MovementMode);
		UE_LOG(LogTemp, Warning, TEXT("Velocity: %s"), *GetCharacterMovement()->Velocity.ToString());
		UE_LOG(LogTemp, Warning, TEXT("IsMovingOnGround: %d"), GetCharacterMovement()->IsMovingOnGround());
		UE_LOG(LogTemp, Warning, TEXT("Controller: %s"), *GetController()->GetName());

		Direction.Normalize();
		AddMovementInput(Direction, 1.0,true);
		UE_LOG(LogTemp, Log, TEXT("Current Actor Location : %s"), *CurrentLocation.ToString());
	}
}

void AGGwaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent){
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)){
		for (int32 i = 0; i < SkillActions.Num(); ++i){
			EIC->BindAction(SkillActions[i], ETriggerEvent::Triggered, this, &AGGwaCharacter::OnLocalSkillInput, i);
			for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()){
				UE_LOG(LogTemp, Warning, TEXT("Ability: %s"), *GetNameSafe(Spec.Ability));
				if (IsLocallyControlled()) UE_LOG(LogTemp, Warning, TEXT("Client %s: Ability: %s"),*GetNameSafe(GetController()),*GetNameSafe(Spec.Ability));
			}
		}
	}
}


bool AGGwaCharacter::OnSkillTriggered_Validate(const FGameplayEventData& EventData, int32 Index) {
	if (GetLocalRole() != ROLE_Authority){
		UE_LOG(LogTemp, Warning, TEXT("Validate: Not Authority"));
		return false;
	}

	if (!SkillAbilities.IsValidIndex(Index) || !SkillAbilities[Index]){
		UE_LOG(LogTemp, Warning, TEXT("Validate: Invalid Index %d"), Index);
		return false;
	}

	if (!AbilityTags.IsValidIndex(Index) ||EventData.EventTag != AbilityTags[Index]){
		UE_LOG(LogTemp, Warning, TEXT("Validate: EventTag mismatch (got %s, expected %s)"),
			*EventData.EventTag.ToString(), *AbilityTags[Index].ToString());
		return false;
	}

	if (EventData.Instigator != this){
		UE_LOG(LogTemp, Warning, TEXT("Validate: Instigator mismatch"));
		return false;
	}

	return true;
}

// Local Prediction
void AGGwaCharacter::OnLocalSkillInput(const FInputActionInstance& Instance, int32 Index)
{
	if (!IsLocallyControlled() || !ASC || !SkillAbilities.IsValidIndex(Index)) return;

	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(SkillAbilities[Index]);
	if (!Spec) return;

	// 클라이언트 예측 발동 (PredictionKey 생성됨)
	bool bActivated = ASC->TryActivateAbility(Spec->Handle, true);

	if (!bActivated){
		UE_LOG(LogTemp, Warning, TEXT("TryActivateAbility failed for Index %d"), Index);
	}
}

// OnSkillTriggered_Implementation()	서버 (RPC)	클라이언트 요청 받아 실행
void AGGwaCharacter::OnSkillTriggered_Implementation(const FGameplayEventData& EventData, int32 Index) {
	ExecuteAbility(EventData, Index);
}




// ExecuteAbility()	공통	실제 GA 발동 처리
void AGGwaCharacter::ExecuteAbility(const FGameplayEventData& EventData, int32 Index) {
	if (ASC && SkillAbilities.IsValidIndex(Index) && SkillAbilities[Index]){
		AGGwaPlayerController * PC = Cast<AGGwaPlayerController>(GetController());
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(SkillAbilities[Index]);
		if (Spec == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("ExecuteAbility: Spec is nullptr"));
			return;
		}
		ASC->TryActivateAbility(Spec->Handle, true);
	}
}

void AGGwaCharacter::SetMoveData_Implementation(const TArray<FVector>& Path, int32 PathIndex, bool bIsFollowing) {
	this->CurrentPath = Path;
	this->CurrentPathIndex = PathIndex;
	this->bIsFollowingPath = bIsFollowing;
}


// OnLocalSkillInput()	클라이언트	입력 수신 → 서버 호출
// 클라이언트에서 마우스 위치 기반 RotationEventData 전송
// void AGGwaCharacter::OnLocalSkillInput(const FInputActionInstance& Instance, int32 Index)
// {
// 	if (IsLocallyControlled())
// 	{
// 		AGGwaPlayerController* PC = Cast<AGGwaPlayerController>(GetController());
// 		if (PC)
// 		{
// 			FHitResult Hit;
// 			if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
// 			{
// 				FGameplayEventData EventData;
// 				EventData.Instigator = this;
// 				EventData.EventTag = AbilityTags[Index];
// 				EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(Hit);
// 				
// 				// if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance()) {
// 				// 	if (SkillMontages.IsValidIndex(Index) && SkillMontages[Index]) {
// 				// 		AnimInstance->Montage_Play(SkillMontages[Index], 1.0f, EMontagePlayReturnType::MontageLength, 0.0f);
// 				// 	}
// 				// }
//
// 				//해당 부분의 RPC
// 				OnSkillTriggered(EventData, Index);  // 클라 -> 서버 RPC 호출
// 			}
// 		}
// 	}
// }

