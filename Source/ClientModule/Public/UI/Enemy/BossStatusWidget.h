// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "Shared/AI/EnemySystemCore/FEnemyWidgetData.h"
#include "BossStatusWidget.generated.h"

class UEnemyAttributeSet;
class UEnemyAbilitySystemComponent;
class UProgressBar;
class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class CLIENTMODULE_API UBossStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UImage>      Symbol_Image;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UProgressBar> HP_Bar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UTextBlock>   HP_Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UTextBlock>   Name_Text;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UTextBlock>   LimitTime_Text;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UTextBlock>   EnemyType_Text;
	
	float RemainingTime = 0.f;

	/**  
	 * 보스 심볼·이름 ·초기 체력·타이머 동시 설정  
	 */
	void SetWidget(const FEnemyWidgetData& WidgetData, const FBossDataStruct& Data);

	/**  
	 * 서버에서 내려준 FBossDataStruct 에 담긴 Current/Max HP로만 체력 업데이트  
	 */
	void UpdateHealthBar(const FBossDataStruct& Data);

	void UpdateWidget(const FBossDataStruct& Data);

	/**  
	 * DisplayTime > 0 인 경우에만 LimitTime UI를 설정  
	 */
	void SetBossTimer(float DisplayTime);
	
private:
	FTimerHandle TimerHandle_BossDisplay;
};