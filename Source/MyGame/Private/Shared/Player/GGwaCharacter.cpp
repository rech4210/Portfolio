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
	
	if (ASC && HasAuthority()) {
		FGameplayAbilitySpec MoveSpec(MoveAbility, 1, static_cast<int32>(EAbilityInputID::Move), this);
		ASC->GiveAbility(MoveSpec);
		for (int32 i = 0; i < SkillAbilities.Num(); ++i){
			if (SkillAbilities[i]){
				FGameplayAbilitySpec Spec(SkillAbilities[i], 1, static_cast<int32>(EAbilityInputID::Skill1) + i, this);
				ASC->GiveAbility(Spec);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("=== AGGwaCharacter::PossessedBy DEBUG ==="));
		ASC->RefreshAbilityActorInfo();
	}
	Cast<AGGwaPlayerState>(GetPlayerState())->InitPlayerState();

}

void AGGwaCharacter::OnRep_PlayerState() {
	Super::OnRep_PlayerState();
	InitASC();
	
	UE_LOG(LogTemp, Warning, TEXT("=== AGGwaCharacter::OnRep_PlayerState DEBUG ==="));
	UE_LOG(LogTemp, Warning, TEXT("Character: %p | PlayerState: %p"), this, GetPlayerState());
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
	const FGameplayAbilitySpecHandle& AbilitySpec = ASC->FindAbilitySpecFromClass(MoveAbility)->Handle;
	if (AbilitySpec.IsValid())
	{
		bool bIsActivated = ASC->TryActivateAbility(AbilitySpec,true);
	}
}

void AGGwaCharacter::PostInitializeComponents() {
	Super::PostInitializeComponents();
}

// 1. Server beginplay 시점에 PC가 null이라 아래 로직이 수행되지 않음.
// 2. Client beginplay 가 호출되지 않음.
void AGGwaCharacter::BeginPlay() {
	Super::BeginPlay();
}

void AGGwaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent){
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController * PC = Cast<APlayerController>(GetController()); nullptr != PC) {
		UEnhancedInputLocalPlayerSubsystem * Subsystem = PC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (MappingContext) {
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("=== SetupPlayerInputComponent DEBUG ==="));
	UE_LOG(LogTemp, Warning, TEXT("PlayerInputComponent: %p | IsLocallyControlled: %s"), 
		PlayerInputComponent, IsLocallyControlled() ? TEXT("YES") : TEXT("NO"));
	
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)){
		UE_LOG(LogTemp, Warning, TEXT("Enhanced Input Component found"));
		UE_LOG(LogTemp, Warning, TEXT("SkillActions.Num(): %d | ASC: %p"), SkillActions.Num(), ASC.Get());

		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGGwaCharacter::PlayerMove);
		for (int32 i = 0; i < SkillActions.Num(); ++i){
			if (SkillActions[i]) {
				// Enhanced Input Action 바인딩
				EIC->BindAction(SkillActions[i], ETriggerEvent::Triggered, this, &AGGwaCharacter::OnLocalSkillInput, i);
				UE_LOG(LogTemp, Warning, TEXT("Bound SkillAction[%d]: %s"), i, *SkillActions[i]->GetName());
			} else {
				UE_LOG(LogTemp, Error, TEXT("SkillActions[%d] is null!"), i);
			}
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to cast to UEnhancedInputComponent"));
	}
}

void AGGwaCharacter::CustomKeySet(UInputAction* Action, FKey CustomKey) {

	// 0. Check base logic ex) exist...
	// 1. Gey Old Key
	FKey OldKey = "q";
	MappingContext->UnmapKey(Action, OldKey);
	MappingContext->MapKey(Action, CustomKey);
	// SkillComponent->
}

void AGGwaCharacter::OnLocalSkillInput(const FInputActionInstance& Instance, int32 Index)
{
	UE_LOG(LogTemp, Warning, TEXT("=== OnLocalSkillInput CALLED ==="));
	UE_LOG(LogTemp, Warning, TEXT("Index: %d | IsLocallyControlled: %s"), 
		Index, IsLocallyControlled() ? TEXT("YES") : TEXT("NO"));
	
	//HOW Get SlotIndex For Find Getskillslot..?,
	auto State = GetPlayerState<AGGwaPlayerState>();
	if (!State) {
		UE_LOG(LogTemp, Error, TEXT("OnLocalSkillInput: PlayerState is null"));
		return;
	}
	
	if (!State->GetSkillComponent()) {
		UE_LOG(LogTemp, Error, TEXT("OnLocalSkillInput: SkillComponent is null"));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("OnLocalSkillInput: Attempting to cast skill at index %d"), Index);
	
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


void AGGwaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGGwaCharacter, CurrentPath);
	DOREPLIFETIME(AGGwaCharacter, CurrentPathIndex);
	DOREPLIFETIME(AGGwaCharacter, bIsFollowingPath);
}


// OnLocalSkillInput()	?�라?�언???�력 ?�신 ???�버 ?�출
// ?�라?�언?�에??마우???�치 기반 RotationEventData ?�송
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
// 				//?�당 부분의 RPC
// 				OnSkillTriggered(EventData, Index);  // ?�라 -> ?�버 RPC ?�출
// 			}
// 		}
// 	}
// }

