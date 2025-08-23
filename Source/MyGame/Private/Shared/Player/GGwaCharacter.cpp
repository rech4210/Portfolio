#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Components/SkillComponent.h"
#include "GameSharedModule/Public/Utill/UEnumTagMatchHelper.h"
#include "GameSharedModule/Public/Enum/ESkillType.h"
#include "Services/SkillCastingService.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/Player/Component/PlayerReactionComponent.h"
#include "InputMappingContext.h"
#include "Net/UnrealNetwork.h"
#include "SkillModule/Public/Data/AbilityInputID.h"

AGGwaCharacter::AGGwaCharacter() {
	ReactionComponent = CreateDefaultSubobject<UPlayerReactionComponent>("ReactionComponent");
	SkillCastingService = CreateDefaultSubobject<USkillCastingService>("SkillCastingService");
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

UAbilitySystemComponent* AGGwaCharacter::GetAbilitySystemComponent() const {
	return ASC.Get();
}

void AGGwaCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	InitASC();
	FGameplayAbilitySpec MoveSpec(MoveAbility, 1, static_cast<int32>(EAbilityInputID::Move), this);
	ASC->GiveAbility(MoveSpec);
	if (ASC && HasAuthority()) {
		for (int32 i = 0; i < SkillAbilities.Num(); ++i){
			if (SkillAbilities[i]){
				FGameplayAbilitySpec Spec(SkillAbilities[i], 1, static_cast<int32>(EAbilityInputID::Skill1) + i, this);
				ASC->GiveAbility(Spec);
			}
		}
		ASC->RefreshAbilityActorInfo();
	}
	Cast<AGGwaPlayerState>(GetPlayerState())->InitPlayerState();

}

void AGGwaCharacter::OnRep_PlayerState() {
	Super::OnRep_PlayerState();
	InitASC();
	
	Cast<AGGwaPlayerState>(GetPlayerState())->InitPlayerState();
}

void AGGwaCharacter::InitASC() {
	if (AGGwaPlayerState * State = GetPlayerState<AGGwaPlayerState>(); nullptr != State) {
		ASC = Cast<UGGwaAbilitySystemComponent>(State->GetAbilitySystemComponent());
		if (ASC) {
			ASC->InitAbilityActorInfo(State, this);
		}
	}
}

void AGGwaCharacter::PlayerMove() {
	if (SkillAbilities.IsEmpty()) {
		UE_LOG(LogTemp, Warning, TEXT("PlayerMove: No Skill Abilities Assigned"));
		return;
	}
	const FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromClass(MoveAbility);
	if (AbilitySpec)
	{
		bool bIsActivated = ASC->TryActivateAbility(AbilitySpec->Handle,true);
	}
}

void AGGwaCharacter::PostInitializeComponents() {
	Super::PostInitializeComponents();
}

void AGGwaCharacter::BeginPlay() {
	Super::BeginPlay();
	if (APlayerController * PC = Cast<APlayerController>(GetController()); nullptr != PC) {
		UEnhancedInputLocalPlayerSubsystem * Subsystem = PC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (MappingContext) {
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}
//
// void AGGwaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent){
// 	Super::SetupPlayerInputComponent(PlayerInputComponent);
// 	
// 	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)){
//
// 		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGGwaCharacter::PlayerMove);
// 		for (int32 i = 0; i < SkillActions.Num(); ++i){
// 			if (SkillActions[i]) {
// 				EIC->BindAction(SkillActions[i], ETriggerEvent::Triggered, this, &AGGwaCharacter::OnLocalSkillInput, i);
// 			}
// 		}
// 	}
// }

void AGGwaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent){
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)){
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGGwaCharacter::PlayerMove);

		for (int32 i = 0; i < SkillActions.Num(); ++i){
			EIC->BindAction(SkillActions[i], ETriggerEvent::Triggered, this, &AGGwaCharacter::OnLocalSkillInput, i);
			for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()){
				UE_LOG(LogTemp, Warning, TEXT("Ability: %s"), *GetNameSafe(Spec.Ability));
				if (IsLocallyControlled()) UE_LOG(LogTemp, Warning, TEXT("Client %s: Ability: %s"),*GetNameSafe(GetController()),*GetNameSafe(Spec.Ability));
			}
		}
	}
}


void AGGwaCharacter::OnLocalSkillInput(const FInputActionInstance& Instance, int32 Index)
{
	auto State = GetPlayerState<AGGwaPlayerState>();
	if (!State) {
		UE_LOG(LogTemp, Error, TEXT("OnLocalSkillInput: PlayerState is null"));
		return;
	}
	
	if (!State->GetSkillComponent()) {
		UE_LOG(LogTemp, Error, TEXT("OnLocalSkillInput: SkillComponent is null"));
		return;
	}

	if (bIsTest) {
		auto bSuccess = ASC->TryActivateAbilityByClass(SkillAbilities[0], true);
		if (bSuccess) {
			UE_LOG(LogTemp, Log, TEXT("OnLocalSkillInput: Test skill cast successful"));
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("OnLocalSkillInput: Test skill cast failed"));
		}
		return;
	}
	
	auto bisSucces  = SkillCastingService->TryCastSkill(this, Index);
	if (bisSucces) {
		UE_LOG(LogTemp, Log, TEXT("OnLocalSkillInput: Skill cast successful for index %d"), Index);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("OnLocalSkillInput: Skill cast failed for index %d"), Index);
	}
}

UPlayerReactionComponent* AGGwaCharacter::GetReactionComponent() const {
	return ReactionComponent.Get();
}


void AGGwaCharacter::SetMoveData_Implementation(const TArray<FVector>& Path, int32 PathIndex, bool bIsFollowing) {
	this->CurrentPath = Path;
	this->CurrentPathIndex = PathIndex;
	this->bIsFollowingPath = bIsFollowing;
}


void AGGwaCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (false == bIsFollowingPath) return;

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
	if (ASC->HasMatchingGameplayTag(UEnumTagMatchHelper::GetTagFromEnum(ESkillType::None))) {
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
		Direction.Normalize();
		AddMovementInput(Direction, 1.0,true);
	}
}
