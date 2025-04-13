// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GGwaPlayerController.generated.h"

class UBaseDataAsset;
/**
 * 
 */
UCLASS()
class MYGAME_API AGGwaPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AGGwaPlayerController();
	virtual void BeginPlay() override;
	/* 클라이언트 모듈 UI 설정을 위한 함수*/
	virtual void InitClientWidget(){}

	virtual void GetDataFromAbility(UBaseDataAsset* Data) {}
	//클라이언트가 자신에게 Possess한 Pawn을 인식(승인)하도록 알려주는 함수
	virtual void AcknowledgePossession(APawn* PossessedPawn) override;
};
