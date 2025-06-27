// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AuthClientModule/Public/AuthService.h"
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
	// This delegate is no longer needed.
	// UPROPERTY(BlueprintAssignable, Cegory = "UI")
	// FOnAbilityDataAssetApplied OnAbilityDataAssetApplied;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<UGGwaWidget> WidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<UBossStatusWidget> BossStatusWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AGGwaHUD> GGwaHUD;
	
	virtual void BeginPlay() override;
	virtual void InitClientWidget() override;
	virtual void Client_ReceiveBossData_Implementation(const FBossDataStruct& Data) override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void NotifyClientStateChanged() override;

private:
	// hover 대상을 적으로 제어하기 위해, enemy base character 제공할것.
	TWeakObjectPtr<ABossCharacter> LastHoveredEnemy;

	UPROPERTY()
	TObjectPtr<UAuthService> AuthService;

	void OnLoginSuccess(const FString& Token);
	void OnLoginFailure(const FString& ErrorReason);
};
