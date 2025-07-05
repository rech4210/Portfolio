// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkillComponent.h"
#include "GameFramework/PlayerController.h"
#include "Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "Shared/Utill/FRewardRequest.h"
#include "GGwaPlayerController.generated.h"

class ABossCharacter;
class UBaseDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityDataAssetApplied, UBaseDataAsset*, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossDataReceived, const FBossDataStruct&, BossData);


// class FRewardData;
/**
 * 
 */
UCLASS()
class MYGAME_API AGGwaPlayerController : public APlayerController {
	GENERATED_BODY()
public:
	AGGwaPlayerController();
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnAbilityDataAssetApplied OnAbilityDataAssetApplied;

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnBossDataReceived OnBossDataReceived;

	/* 클라이언트 모듈 UI 설정을 위한 함수*/
	virtual void InitClientWidget(const USkillComponent* SkillCompoent){}

	// UFUNCTION(Client ,Reliable)
	// virtual void Client_ApplyAbilityDataAsset(UBaseDataAsset* Data);
	UFUNCTION(Client ,Reliable)
	virtual void Client_ReceiveBossData(const FBossDataStruct& BossCharacter);
	UFUNCTION(Client ,Reliable)
	virtual void Client_ReceiveSkillData(const USkillComponent* SkillComponent);

	virtual void NotifyClientStateChanged(){}

	//클라이언트가 자신에게 Possess한 Pawn을 인식(승인)하도록 알려주는 함수
	virtual void AcknowledgePossession(APawn* PossessedPawn) override;

	UFUNCTION(Server, Reliable)
	void Server_InitiateReward(const FString& PlayerId, const FRewardRequest& Payload);

	// UFUNCTION(Client, Reliable)
	// void Client_OnRewardResult(bool bOK, const FRewardData& Data, const FString& Error);
	// virtual void Client_NotifySkillActivated(int32 SkillId);
};


