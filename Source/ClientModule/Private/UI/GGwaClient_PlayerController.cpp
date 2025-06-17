// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GGwaClient_PlayerController.h"

#include "Shared/AI/BossCharacter.h"
#include "Shared/AI/EnemyAbilitySystemComponent.h"
#include "Shared/AI/EnemyAttributeSet.h"
#include "UI/Widget/GGwaWidget.h"
#include "UI/GGwaHUD.h"
#include "Shared/GAS/GGwaAttributeSet.h"
#include "Shared/GAS/GGwaAbilitySystemComponent.h"
#include "UI/Enemy/BossStatusWidget.h"
#include "Shared/Player/GGwaPlayerState.h"


void AGGwaClient_PlayerController::BeginPlay() {
	Super::BeginPlay();
	bEnableMouseOverEvents  = true;
}

void AGGwaClient_PlayerController::InitClientWidget() {
	if (HasAuthority()) {
		UE_LOG(LogTemp, Warning, TEXT("server Controller"));
		return;
	}
	else if (IsLocalController()) {
		UE_LOG(LogTemp, Warning, TEXT("Client has been initialized"));
	}
	if (WidgetClass) {
		UGGwaWidget * Widget = CreateWidget<UGGwaWidget>(this, WidgetClass);
		UBossStatusWidget* BossWidget = CreateWidget<UBossStatusWidget>(this, BossStatusWidgetClass);
		Widget->AddToViewport();
		BossWidget->AddToViewport();
		BossWidget->SetVisibility(ESlateVisibility::Hidden);
		if (Widget) {
			GGwaHUD = Cast<AGGwaHUD>(GetHUD());
			GGwaHUD->SetBaseWidget(Widget);
			GGwaHUD->SetBossWidget(BossWidget);
			
			if (AGGwaPlayerState * PS = GetPlayerState<AGGwaPlayerState>()) {
				auto ASC = PS->GetAbilitySystemComponent();
				UGGwaAbilitySystemComponent* GGawASC = CastChecked<UGGwaAbilitySystemComponent>(ASC);
				const UGGwaAttributeSet* GGwaAttributeSet = Cast<UGGwaAttributeSet>(GGawASC->GetAttributeSet(UGGwaAttributeSet::StaticClass()));
				Widget->InitWidget(GGawASC, GGwaAttributeSet);
			}
		}
	}
}

void AGGwaClient_PlayerController::Client_ApplyAbilityDataAsset_Implementation(UBaseDataAsset* Data) {
	if (GGwaHUD && IsLocalController()) {
		// Is it call twice?
		GGwaHUD->GetBaseWidget()->BindWidgetWithTooltip(Data);
	}
}

void AGGwaClient_PlayerController::Client_ReceiveBossData_Implementation(const FBossDataStruct& Data) {
	if (GGwaHUD && IsLocalController()) {
		GGwaHUD->GetBossWidget()->UpdateWidget(Data);
	}
}

void AGGwaClient_PlayerController::PlayerTick(float DeltaTime) {
	{
		Super::PlayerTick(DeltaTime);

		FHitResult Hit;
		// Visibility 채널로 마우스 밑 Actor 판별
		if (GetHitResultUnderCursorByChannel(
				UEngineTypes::ConvertToTraceType(ECC_Visibility),
				true, Hit))
		{
			if (ABossCharacter* Enemy = Cast<ABossCharacter>(Hit.GetActor()))
			{
				GGwaHUD->GetBossWidget()->SetVisibility(ESlateVisibility::Visible);
				if (Enemy != LastHoveredEnemy){
					LastHoveredEnemy = Enemy;

					// ASC와 AttributeSet 가져오기
					UEnemyAbilitySystemComponent* ASC = Cast<UEnemyAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
					const UEnemyAttributeSet* AttrSet = ASC ? ASC->GetSet<UEnemyAttributeSet>() : nullptr;
					if (!ASC || !AttrSet) return;

					FBossDataStruct BossData;
					BossData.Health = AttrSet->GetHealth();
					BossData.MaxHealth = AttrSet->GetMaxHealth();
					BossData.Damage = AttrSet->GetDamage();
					// BossData.Phase  = Enemy->GetPhase(); // 적의 현재 페이즈
					// BossData.MaxHealth     = AttrSet->GetMaxHealth();

					// FEnemyWidgetData 구성
					FEnemyWidgetData WidgetData = Enemy->GetWidgetData();
					if (GGwaHUD && IsLocalController()){
						GGwaHUD->GetBossWidget()->SetWidget(WidgetData, BossData);
					}
				}
				return;
			}
		}

		// 커서가 적 이외 영역에 있을 때: 클리어
		if (LastHoveredEnemy.IsValid()){
			LastHoveredEnemy = nullptr;
			if (GGwaHUD && IsLocalController()){
				GGwaHUD->GetBossWidget()->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

// void AGGwaClient_PlayerController::Client_ApplyAbilityDataAsset(UBaseDataAsset* Data) {
// 	if (GGwaHUD && IsLocalController()) {
// 		GGwaHUD->GetBaseWidget()->BindWidgetWithTooltip(Data);
// 	}
// }
//
// void AGGwaClient_PlayerController::Client_ReceiveBossData(const FBossDataStruct& Data) {
// 	if (GGwaHUD && IsLocalController()) {
// 		GGwaHUD->GetBossWidget()->UpdateBossWidget(Data);
// 	}
// }

