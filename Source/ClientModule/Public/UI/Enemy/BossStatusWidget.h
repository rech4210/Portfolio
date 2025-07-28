// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyGame/Public/Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "MyGame/Public/Shared/AI/EnemySystemCore/FEnemyWidgetData.h"
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
	 * Î≥¥Ïä§ ?¨Î≥º¬∑?¥Î¶Ñ ¬∑Ï¥àÍ∏∞ Ï≤¥Î†•¬∑?Ä?¥Î®∏ ?ôÏãú ?§Ï†ï  
	 */
	void SetWidget(const FEnemyWidgetData& WidgetData, const FBossDataStruct& Data);

	/**  
	 * ?úÎ≤Ñ?êÏÑú ?¥Î†§Ï§Ä FBossDataStruct ???¥Í∏¥ Current/Max HPÎ°úÎßå Ï≤¥Î†• ?ÖÎç∞?¥Ìä∏  
	 */
	void UpdateHealthBar(const FBossDataStruct& Data);

	void UpdateWidget(const FBossDataStruct& Data);

	/**  
	 * DisplayTime > 0 ??Í≤ΩÏö∞?êÎßå LimitTime UIÎ•??§Ï†ï  
	 */
	void SetBossTimer(float DisplayTime);
	
private:
	FTimerHandle TimerHandle_BossDisplay;
};