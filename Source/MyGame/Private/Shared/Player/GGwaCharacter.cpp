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
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/Component/PlayerReactionComponent.h"
#include "InputMappingContext.h"
#include "SkillModule/Public/Data/AbilityInputID.h"
#include "Shared/Player/Component/UPlayerStateComponent.h"

AGGwaCharacter::AGGwaCharacter() {
	ReactionComponent = CreateDefaultSubobject<UPlayerReactionComponent>("ReactionComponent");
	SkillCastingService = NewObject<USkillCastingService>(this, "SkillCastingService");
}

UAbilitySystemComponent* AGGwaCharacter::GetAbilitySystemComponent() const {
	return ASC.Get();
}

void AGGwaCharacter::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	InitASC();
	if (ASC && HasAuthority()) {
		for (int32 i = 0; i < SkillAbilities.Num(); ++i){
			if (SkillAbilities[i]){
				//skillrepo 기반?�로 초기???�행?�야 ?? ?�킬 ?�록 변경시 ?�적?�로 Ability???�록 ?�용??변경시켜줘?�한??
				FGameplayAbilitySpec Spec(SkillAbilities[i], 1, static_cast<int32>(EAbilityInputID::Skill1) + i, this);
				ASC->GiveAbility(Spec);
			}
		}

		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()){
			UE_LOG(LogTemp, Warning, TEXT("Ability: %s | InputID: %d"),
				*GetNameSafe(Spec.Ability), Spec.InputID);
		}
		ASC->RefreshAbilityActorInfo();
	}
	Cast<AGGwaPlayerState>(GetPlayerState())->InitPlayerState();
}

void AGGwaCharacter::OnRep_PlayerState() {
	Super::OnRep_PlayerState();
	InitASC();
	
	UE_LOG(LogTemp, Warning, TEXT("=== AGGwaCharacter::OnRep_PlayerState DEBUG ==="));
	UE_LOG(LogTemp, Warning, TEXT("Character: %p | PlayerState: %p"), this, GetPlayerState());
	
	// UI initialization moved to GameMode::PreLogin
	// Server will handle UI initialization through PlayerController interface
	UE_LOG(LogTemp, Log, TEXT("AGGwaCharacter::OnRep_PlayerState - UI initialization handled by server"));
	
	Cast<AGGwaPlayerState>(GetPlayerState())->InitPlayerState();
	// Cast<AGGwaPlayerController>(GetController())->InitializeClientComponent();
}


void AGGwaCharacter::InitASC() {
	if (AGGwaPlayerState * State = GetPlayerState<AGGwaPlayerState>(); nullptr != State) {
		ASC = Cast<UGGwaAbilitySystemComponent>(State->GetAbilitySystemComponent());
		if (ASC) {
			// ASC???�결 ?�보�?부?? ASC??owner, Replicated 객체�?지??
			ASC->InitAbilityActorInfo(State, this);
		}
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


/**
	 * ?�재 구조:
	 * - �??�력 ?�션(SkillActions[i])?� ?�롯 ?�덱??i)?� ?�결??
	 * - ?�롯?�는 고정??GA가 ?�당?�어 ?�으�? ?�당 ?�롯??GA�??�행
	 * 
	 * 개선 ?�이?�어:
	 * - ?�력 ?�마???�롯 ?�덱?��? ?�닌 "GA�??�유???�브?�트"�?직접 ?�결
	 * - ???�력 ?? ?�재 ?�착??무기/?�비/?�브?�트?�서 GA�?추출?�여 ?�행
	 * - ?�비 변�??? ?�당 ?�에 ?�결???�브?�트�?교체?�면 ?�동?�로 ?�력??바뀌도�??�계 가??
*/


// TODO: ?�킬 Input ?�적 바인??고려, ?�킬 변경시, Repo�?부?�의 ?�킬 ?�정 -> ?�레?�어???�태 ?�기??EIC, GiveAbility
void AGGwaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent){
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)){
		for (int32 i = 0; i < SkillActions.Num(); ++i){
			// ?�적?�로 ?��? 변경시 무효???? ?�정?�??
			EIC->BindAction(SkillActions[i], ETriggerEvent::Triggered, this, &AGGwaCharacter::OnLocalSkillInput, i);
			for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()){
				UE_LOG(LogTemp, Warning, TEXT("Ability: %s"), *GetNameSafe(Spec.Ability));
				if (IsLocallyControlled()) UE_LOG(LogTemp, Warning, TEXT("Client %s: Ability: %s"),*GetNameSafe(GetController()),*GetNameSafe(Spec.Ability));
			}
		}
	}
}


// ?�레?�어가 ?�킬 UI???�래그하???�킬???�록 ->  UI???�덱??�??��? 가?�옴 -> UI<->SkillSlot ?�방??매핑 -> ?�킬 ?�용 (UI RPC -> TrySkill[SkillSlot]) -> 결과 반환
void AGGwaCharacter::CustomKeySet(UInputAction* Action, FKey CustomKey) {

	// 0. Check base logic ex) exist...
	// 1. Gey Old Key
	FKey OldKey = "q";
	MappingContext->UnmapKey(Action, OldKey);
	MappingContext->MapKey(Action, CustomKey);
	// SkillComponent->
}

/* Change Custom Skill Input System
 */

void AGGwaCharacter::OnLocalSkillInput(const FInputActionInstance& Instance, int32 Index)
{
	//HOW Get SlotIndex For Find Getskillslot..?,
	auto State = GetPlayerState<AGGwaPlayerState>();
	if (!State->GetSkillComponent()) {
		return;
	}
	//SlotIndex 기반?�로 변�?- 직접 Index ?�용
	auto bisSucces  = SkillCastingService->TryCastSkill(this, Index);
	if (bisSucces) {
		UE_LOG(LogTemp, Log, TEXT("OnLocalSkillInput: Skill cast successful for index %d"), Index);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("OnLocalSkillInput: Skill cast failed for index %d"), Index);
	}
}


void AGGwaCharacter::SetMoveData_Implementation(const TArray<FVector>& Path, int32 PathIndex, bool bIsFollowing) {
	this->CurrentPath = Path;
	this->CurrentPathIndex = PathIndex;
	this->bIsFollowingPath = bIsFollowing;
}


UPlayerReactionComponent* AGGwaCharacter::GetReactionComponent() const {
	return ReactionComponent.Get();
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

