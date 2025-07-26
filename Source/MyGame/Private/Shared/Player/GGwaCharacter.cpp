#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"
// #include "Shared/GAS/Skill/GA_Skill1.h"

// #include "AbilitySystemComponent.h"
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
				//skillrepo 기반으로 초기화 수행해야 함. 스킬 등록 변경시 동적으로 Ability의 등록 내용을 변경시켜줘야한다.
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
	 * 현재 구조:
	 * - 각 입력 액션(SkillActions[i])은 슬롯 인덱스(i)와 연결됨
	 * - 슬롯에는 고정된 GA가 할당되어 있으며, 해당 슬롯의 GA를 실행
	 * 
	 * 개선 아이디어:
	 * - 입력 키마다 슬롯 인덱스가 아닌 "GA를 소유한 오브젝트"를 직접 연결
	 * - 키 입력 시, 현재 장착된 무기/장비/오브젝트에서 GA를 추출하여 실행
	 * - 장비 변경 시, 해당 키에 연결된 오브젝트만 교체하면 자동으로 능력이 바뀌도록 설계 가능
*/


// TODO: 스킬 Input 동적 바인딩 고려, 스킬 변경시, Repo로 부터의 스킬 수정 -> 플레이어의 상태 동기화 EIC, GiveAbility
void AGGwaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent){
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent)){
		for (int32 i = 0; i < SkillActions.Num(); ++i){
			// 동적으로 키를 변경시 무효화 됨. 수정대상
			EIC->BindAction(SkillActions[i], ETriggerEvent::Triggered, this, &AGGwaCharacter::OnLocalSkillInput, i);
			for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities()){
				UE_LOG(LogTemp, Warning, TEXT("Ability: %s"), *GetNameSafe(Spec.Ability));
				if (IsLocallyControlled()) UE_LOG(LogTemp, Warning, TEXT("Client %s: Ability: %s"),*GetNameSafe(GetController()),*GetNameSafe(Spec.Ability));
			}
		}
	}
}


// 플레이어가 스킬 UI에 드래그하여 스킬을 등록 ->  UI의 인덱스 및 키를 가져옴 -> UI<->SkillSlot 양방향 매핑 -> 스킬 사용 (UI RPC -> TrySkill[SkillSlot]) -> 결과 반환
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
	//SlotIndex 기반으로 변경 - 직접 Index 사용
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

