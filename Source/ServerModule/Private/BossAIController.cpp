// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#include "BossAIController.h"
#include "AI/UI/BossUIComponent.h"
#include "Shared/AI/BossComponent/BossStateComponent.h"

/*
 * 1. ë³´ìŠ¤??BT ?ì„±
 * 2. Detectê¸°ëŠ¥ ?œê³µ
 * 3. Phase ëª¨ë“ˆ ?‘ì„±? ê²ƒ.
 * 4. Boss State, Boss Skill Component ?‘ì„±? ê²ƒ. (AI controller??sence? ê¸°ëŠ¥ ?¬ìš© ê°€?¥í•œê°€?(
 * 5. ë³´ìŠ¤??AI Controller(HUD ?°ê²°), Character, ASC, AttributeSet ê¸°ë³¸ êµ¬ì¡° ?‘ì„± -> Game Mode ?ì„œ ?ì„ ?ì„±?œì¼œì£¼ë„ë¡?
 * 6. ë³´ìŠ¤???„ì‹œ ?ì„± ?„ì¹˜ë¥?PostLogin?¼ë¡œ ?¡ìŒ.
 * 7. ?¨ì¼ì±…ì„ ?ì¹™ ê¸°ì????°ë¼ ëª¨ë“ˆ ë¶„ë¦¬, Character???¤í–‰ ì£¼ì²´ / Controller??ë¡œì§ FSM ?„ì£¼ë¡??¤ê³„? ê²ƒ.
 */

ABossAIController::ABossAIController() {
	StateComponent = CreateDefaultSubobject<UBossStateComponent>(TEXT("StateComponent"));
	UIComponent = CreateDefaultSubobject<UBossUIComponent>(TEXT("UIComponent"));
}

// ?¤ì œ ì´ˆê¸°??ë¡œì§
void ABossAIController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);
	RunBehaviorTree( TempBehavior);
	BB = GetBlackboardComponent();
}
// Phase ë³€ê²½ì— ?€??ì±…ì„??ê°€ì§€ê³??ˆë‹¤. ì¶”í›„ ?„ìš”?˜ë‹¤ë©?SRP ?ì¹™???°ë¼ ë¶„ë¦¬? ê²ƒ.
void ABossAIController::RequestPhaseChange() const{
	StateComponent->AdvanceBossPhase(BB);
}

void ABossAIController::RequestBossStateChange(EBossState BossState) const{
	StateComponent->UpdateBossState(BB, BossState);
}

void ABossAIController::ReceiveEnemyData(FBossDataStruct& Data){
	if (HasAuthority()) {
		if (Data.Health <70.f) {
			RequestPhaseChange();
		}
	}
}

// void ABossAIController::RequestHUDUpdate(FBossDataStruct& Data) {
// 	UIComponent->UpdateWidgetData(Data);
// }
//

