#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Shared/GAS/Skill/GA_Skill1.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

// Sets default values
AGGwaCharacter::AGGwaCharacter()
{
}

void AGGwaCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	InitASC();
}

void AGGwaCharacter::OnRep_PlayerState() {
	Super::OnRep_PlayerState();
	InitASC();
}

void AGGwaCharacter::BeginPlay() {
	Super::BeginPlay();
	if (APlayerController * PC = Cast<APlayerController>(GetController()); nullptr != PC) {
		UEnhancedInputLocalPlayerSubsystem * Subsystem = PC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		Subsystem->AddMappingContext(MappingContext, 0);
	}
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	if (ASC) {
		
		// 이 부분 스킬 연동과 통합하여 수정
		// 스킬 변경시 재 바인딩 요구.
		for (int32 i = 0; i < SkillAbilities.Num(); ++i)
		{
			if (SkillAbilities[i])
			{
				FGameplayAbilitySpec Spec(SkillAbilities[i], 1, static_cast<int32>(EAbilityInputID::Move) + i, this);
				ASC->GiveAbility(Spec);
			}
		}
	}

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		UE_LOG(LogTemp, Warning, TEXT("Ability: %s | InputID: %d"),
			*GetNameSafe(Spec.Ability), Spec.InputID);
	}
}

void AGGwaCharacter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	if (false == bIsFollowingPath) return;

	if (!CurrentPath.IsValidIndex(CurrentPathIndex)||ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Skill"))) {
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

	if (Distance < AcceptanceRadius) {
		CurrentPathIndex++;
		if (!CurrentPath.IsValidIndex(CurrentPathIndex)) {
			bIsFollowingPath = false;
		}
	}
	else {
		Direction.Normalize();
		AddMovementInput(Direction, 1.0);
	}
}

void AGGwaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		for (int32 i = 0; i < SkillActions.Num(); ++i)
		{
			EIC->BindAction(SkillActions[i], ETriggerEvent::Triggered, this, &AGGwaCharacter::OnSkillTriggered, i);
		}
	}
}

void AGGwaCharacter::OnSkillTriggered(const FInputActionInstance& Instance, int32 Index)
{
	
	if (ASC && SkillAbilities.IsValidIndex(Index) && SkillAbilities[Index])
	{
		// if (USkillDataAsset->skillCooldown <= 0.f) {
			ASC->TryActivateAbilityByClass(SkillAbilities[Index]);
		// }
	}
}

void AGGwaCharacter::InitASC() {
	if (AGGwaPlayerState * State = GetPlayerState<AGGwaPlayerState>(); nullptr != State) {
		ASC = State->GetAbilitySystemComponent();
		if (ASC) {
			// the structure that holds information about who we are acting on and who controls us.
			// ASC의 연결 정보를 부여, ASC의 owner, Replicated 객체를 지정
			ASC->InitAbilityActorInfo(State, this);
		}
	}
}

void AGGwaCharacter::SetMoveData(TArray<FVector> Path, int32 Idx, bool bIsFollwing) {
	this->CurrentPath = Path;
	this->CurrentPathIndex = Idx;
	this->bIsFollowingPath = bIsFollwing;
}
