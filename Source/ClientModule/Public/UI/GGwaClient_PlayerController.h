// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "GGwaClient_PlayerController.generated.h"

class UBossStatusWidget;
class UBaseDataAsset;
class AGGwaHUD;
class UGGwaWidget;
class UGGwaAttributeSet;
UCLASS(Blueprintable)
class CLIENTMODULE_API AGGwaClient_PlayerController : public AGGwaPlayerController
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<UGGwaWidget> WidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<UBossStatusWidget> BossStatusWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGGwaHUD> GGwaHUD;
	
	virtual void BeginPlay() override;
	virtual void InitClientWidget() override;
	virtual void Client_ApplyAbilityDataAsset_Implementation(UBaseDataAsset* Data) override;
	virtual void Client_ReceiveBossData_Implementation(const FBossDataStruct& Data) override;
};
