// Fill out your copyright notice in the Description page of Project Settings.

#include "Shared/Player/GGwaPlayerController.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Shared/AI/Interface/ServerLogicBridge.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Shared/Player/GGwaPlayerState.h"

// Client-only includes
#if !UE_SERVER
#include "MyGame/Public/Shared/AI/BossCharacter.h"
#include "MyGame/Public/Shared/AI/EnemyAbilitySystemComponent.h"
#include "MyGame/Public/Shared/AI/EnemyAttributeSet.h"
#include "MyGame/Public/Shared/GAS/GGwaAttributeSet.h"

// These headers are from ClientModule - only include if available
PRAGMA_DISABLE_DEPRECATION_WARNINGS
// Conditionally include client module headers
#ifdef CLIENTMODULE_API
#include "UI/Widget/GGwaWidget.h"
#include "UI/GGwaHUD.h"
#include "UI/Enemy/BossStatusWidget.h"
#include "AuthClientModule/Public/AuthService.h"
#endif
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#endif

AGGwaPlayerController::AGGwaPlayerController() {
}

void AGGwaPlayerController::BeginPlay() {
	Super::BeginPlay();
	
	// Client-only UI setup
	if (IsLocalController()) {
		bShowMouseCursor = true;
		FInputModeGameAndUI inputMode;
		inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		inputMode.SetHideCursorDuringCapture(false);
		SetInputMode(inputMode);

#if !UE_SERVER
		bEnableMouseOverEvents = true;
#endif
	}
	
	UE_LOG(LogTemp, Warning, TEXT("IsServer: %d | IsLocallyControlled: %d"), HasAuthority(), IsLocalController());
}


// 해당 아래 RPC들이 OnReplicatedUsing에 비해 안정적인지 검토해봐야한다.
// void AGGwaPlayerController::Client_ApplyAbilityDataAsset_Implementation(UBaseDataAsset* Data) {
// 	UE_LOG(LogTemp, Log, TEXT("ApplyAbility Data Called From Server"));
// }

void AGGwaPlayerController::Client_ReceiveBossData_Implementation(const FBossDataStruct& BossCharacter) {
#if !UE_SERVER && defined(CLIENTMODULE_API)
	if (IsLocalController()) {
		OnBossDataReceived.Broadcast(BossCharacter);
	}
#endif
	UE_LOG(LogTemp, Log, TEXT("BossData Called From Server"));
}

void AGGwaPlayerController::Client_ReceiveSkillData_Implementation(const USkillComponent* SkillComponent) {
#if !UE_SERVER && defined(CLIENTMODULE_API)
	if (IsLocalController() && GGwaHUD && GGwaHUD->GetBaseWidget()) {
		GGwaHUD->GetBaseWidget()->UpdateSkillWidgetFromServer(SkillComponent);
	}
#endif
}

void AGGwaPlayerController::AcknowledgePossession(class APawn* PossessedPawn) {
	Super::AcknowledgePossession(PossessedPawn);
	AGGwaCharacter * MyCharacter = Cast<AGGwaCharacter>(PossessedPawn);
	if (nullptr != MyCharacter) {
		UE_LOG(LogTemp,Warning,TEXT("AGGwaPlayerController::AcknowledgePossession : Pawn Possessed"));
		UGGwaAbilitySystemComponent * ASC = Cast<UGGwaAbilitySystemComponent>( GetPlayerState<AGGwaPlayerState>()->GetAbilitySystemComponent());
		if (ASC) {
			ASC->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds("Confirm", "Cancel", FTopLevelAssetPath(TEXT("/Script/SkillModule"), TEXT("EAbilityInputID"))));
		}
	}
}


void AGGwaPlayerController::Server_InitiateReward_Implementation(const FString& PlayerId, const FRewardRequest& Payload) {
	IServerLogicBridge* Bridge = Cast<IServerLogicBridge>(GetWorld()->GetSubsystem<UWorldSubsystem>());
	// Bridge->InitiateRewardFlow(PlayerId, Payload, FOnFlowComplete::CreateUObject(this, &AGGwaPlayerController::Client_OnRewardResult));
}
// void AGGwaPlayerController::Client_OnRewardResult_Implementation(bool bOK, const FRewardData& Data,const FString& Error) {
// }

// --- Client-only functions ---
#if !UE_SERVER

void AGGwaPlayerController::InitClientWidget(const USkillComponent* SkillComponent) {
#if !UE_SERVER && defined(CLIENTMODULE_API)
	if (HasAuthority()) {
		UE_LOG(LogTemp, Warning, TEXT("server Controller"));
		return;
	}
	else if (IsLocalController()) {
		UE_LOG(LogTemp, Warning, TEXT("Client has been initialized"));
	}
	
	if (WidgetClass) {
		UGGwaWidget* Widget = CreateWidget<UGGwaWidget>(this, WidgetClass);
		UBossStatusWidget* BossWidget = CreateWidget<UBossStatusWidget>(this, BossStatusWidgetClass);
		Widget->AddToViewport();
		BossWidget->AddToViewport();
		BossWidget->SetVisibility(ESlateVisibility::Hidden);
		
		if (Widget) {
			GGwaHUD = Cast<AGGwaHUD>(GetHUD());
			GGwaHUD->SetBaseWidget(Widget);
			GGwaHUD->SetBossWidget(BossWidget);

			// Bind to the controller's delegates
			OnBossDataReceived.AddDynamic(GGwaHUD, &AGGwaHUD::HandleBossDataReceived);
			
			if (AGGwaPlayerState* PS = GetPlayerState<AGGwaPlayerState>()) {
				auto ASC = PS->GetAbilitySystemComponent();
				UGGwaAbilitySystemComponent* GGawASC = CastChecked<UGGwaAbilitySystemComponent>(ASC);
				const UGGwaAttributeSet* GGwaAttributeSet = Cast<UGGwaAttributeSet>(GGawASC->GetAttributeSet(UGGwaAttributeSet::StaticClass()));
				Widget->UpdateSkillWidgetFromServer(SkillComponent);
				Widget->InitWidget(GGawASC, GGwaAttributeSet);
			}
		}
	}
#else
	// Server does not need UI initialization
#endif
}

void AGGwaPlayerController::PlayerTick(float DeltaTime) {
	Super::PlayerTick(DeltaTime);

	// Client-only mouse over logic
#if !UE_SERVER && defined(CLIENTMODULE_API)
	if (IsLocalController()) {
		FHitResult Hit;
		// Visibility 채널로 마우스 밑 Actor 판별
		if (GetHitResultUnderCursorByChannel(
				UEngineTypes::ConvertToTraceType(ECC_Visibility),
				true, Hit))
		{
			if (ABossCharacter* Enemy = Cast<ABossCharacter>(Hit.GetActor()))
			{
				if (GGwaHUD) {
					GGwaHUD->GetBossWidget()->SetVisibility(ESlateVisibility::Visible);
				}
				
				if (Enemy != LastHoveredEnemy.Get()) {
					LastHoveredEnemy = Enemy;

					// ASC와 AttributeSet 가져오기
					UEnemyAbilitySystemComponent* ASC = Cast<UEnemyAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
					const UEnemyAttributeSet* AttrSet = ASC ? ASC->GetSet<UEnemyAttributeSet>() : nullptr;
					if (!ASC || !AttrSet) return;

					FBossDataStruct BossData;
					BossData.Health = AttrSet->GetHealth();
					BossData.MaxHealth = AttrSet->GetMaxHealth();
					BossData.Damage = AttrSet->GetDamage();

					// FEnemyWidgetData 구성
					FEnemyWidgetData WidgetData = Enemy->GetWidgetData();
					if (GGwaHUD && IsLocalController()) {
						GGwaHUD->GetBossWidget()->SetWidget(WidgetData, BossData);
					}
				}
				return;
			}
		}

		// 커서가 적 이외 영역에 있을 때: 클리어
		if (LastHoveredEnemy.IsValid()) {
			LastHoveredEnemy = nullptr;
			if (GGwaHUD && IsLocalController()) {
				GGwaHUD->GetBossWidget()->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
#endif
}

void AGGwaPlayerController::NotifyClientStateChanged() {
#if !UE_SERVER && defined(CLIENTMODULE_API)
	if (IsLocalController() && GGwaHUD && GGwaHUD->GetBaseWidget()) {
		GGwaHUD->GetBaseWidget()->OnPlayerStateChanged.Broadcast();
	}
#endif
}

void AGGwaPlayerController::OnLoginSuccess(const FString& Token) {
	// Implementation for login success
}

void AGGwaPlayerController::OnLoginFailure(const FString& ErrorReason) {
	// Implementation for login failure
}

#else

// Stub implementations for server builds
void AGGwaPlayerController::InitClientWidget(const USkillComponent* SkillComponent) {
	// Server does not need UI initialization
}

void AGGwaPlayerController::PlayerTick(float DeltaTime) {
	Super::PlayerTick(DeltaTime);
	// Server-only tick logic if needed
}

void AGGwaPlayerController::NotifyClientStateChanged() {
	// Server does not need UI notifications
}

#endif // !UE_SERVER

