// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Shared/AI/EnemySystemCore/BossStateFlags.h"
#include "Shared/AI/EnemySystemCore/BossPhaseTypes.h"
#include "Shared/AI/Interface/EnemyDataReceiver.h"
#include "BossAIController.generated.h"

class UBossUIComponent;
class UBossStateComponent;
class UBossBehaviorTree;
class UEnemyAbilitySystemComponent;

UCLASS()
class SERVERMODULE_API ABossAIController : public AAIController, public IEnemyDataReceiver
{
	GENERATED_BODY()

public:
	ABossAIController();
	virtual void OnPossess(APawn* InPawn) override;

	//Never Used... maybe used in the future.
	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<UBossBehaviorTree> BossBehaviorTree;
	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<UBehaviorTree> TempBehavior;
	UPROPERTY(EditAnywhere, Category = AI)
	TObjectPtr<UBlackboardComponent> BB;

	void RequestBossStateChange(EBossState BossState) const;
	virtual void ReceiveEnemyData(FBossDataStruct& Data) override;
private:
	void RequestPhaseChange() const;
	TObjectPtr<UBossStateComponent> StateComponent;
	TObjectPtr<UBossUIComponent> UIComponent;
	FBossDataStruct CachedBossData = {};
};


/**
 * God ë°©ì? ?¤ê³„

 * 1. ì»´í¬?ŒíŠ¸ ?ìœ¨??ê°•í™” (?„ì„ ë°©ì‹)
 * - ì»´í¬?ŒíŠ¸ê°€ ë¡œì§???´ë??ìœ¼ë¡?ì²˜ë¦¬?˜ê²Œ ?„ì„
 * - ì»¨íŠ¸ë¡¤ëŸ¬??"ë¬´ì—‡??? ì?"ë§?ì§€?? * - ì±…ì„ ë¶„ë¦¬, ?¬ì‚¬?©ì„± ?? ê²°í•©???? */

/**
 * 2. ì¤‘ì¬??Mediator) ?ëŠ” ?Œì‚¬??Facade) ?¨í„´
 * - ?¬ëŸ¬ ì»´í¬?ŒíŠ¸???‘ì—…??ì¡°ìœ¨?˜ëŠ” ì¤‘ê°„ ê³„ì¸µ
 * - ì»¨íŠ¸ë¡¤ëŸ¬???¨ì¼ ?¸í„°?˜ì´?¤ë¡œ ê´€ë¦?ê°€?? * - ë³µì¡???í˜¸?‘ìš© ê°„ì†Œ?? ?•ì¥??? ë¦¬
 */

/**
 * 3. Command/Event ?¨í„´
 * - ?‰ë™??ëª…ë ¹ ê°ì²´???´ë²¤?¸ë¡œ ì¶”ìƒ?? * - ?¤í–‰, ì§€?? ì·¨ì†Œ ???œì–´ ? ì—°???? * - ?‰ë™ ë¡œê¹…/ë¦¬í”Œ?ˆì´/Undo ê¸°ëŠ¥ ?•ì¥ ê°€?? */

/**
 * - ê¸°ë³¸?€ 1ë²??„ì„ êµ¬ì¡°ë¡??œì‘
 * - ?‘ì—… ë§ì•„ì§€ë©?2ë²?Mediator ?„ì…
 * - ë³µì¡???¡ì…˜ ?ë‚˜ ?íƒœ ê¸°ë¡ ?„ìš”?˜ë©´ 3ë²?Commandë¡??•ì¥
 */
