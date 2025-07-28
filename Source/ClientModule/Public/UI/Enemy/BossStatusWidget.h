
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
	 * 보스 ?�볼·?�름 ·초기 체력·?�?�머 ?�시 ?�정  
	 */
	void SetWidget(const FEnemyWidgetData& WidgetData, const FBossDataStruct& Data);

	/**  
	 * ?�버?�서 ?�려준 FBossDataStruct ???�긴 Current/Max HP로만 체력 ?�데?�트  
	 */
	void UpdateHealthBar(const FBossDataStruct& Data);

	void UpdateWidget(const FBossDataStruct& Data);

	/**  
	 * DisplayTime > 0 ??경우?�만 LimitTime UI�??�정  
	 */
	void SetBossTimer(float DisplayTime);
	
private:
	FTimerHandle TimerHandle_BossDisplay;
};