#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shared/Data/BuffDataAsset.h"
#include "UI/ToolTip/BuffToolTip.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UI/SetWidgetDataInterface.h"
#include "BuffSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class CLIENTMODULE_API UBuffSlotWidget : public UUserWidget, public ISetWidgetDataInterface
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category= "Widget")
	TObjectPtr<UBuffToolTip> BuffToolTipWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category= "Data")
	TObjectPtr<UBuffDataAsset> BuffDataAsset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UImage> SkillImage;
	/** 쿨타임 표시용 프로그레스바 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UProgressBar> RemainTimeBar;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UTextBlock> RemainTimeText;
	
	UFUNCTION(BlueprintCallable)
	void ConsumeDuration(float CooldownTime);

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void SetWidgetData(UPrimaryDataAsset* Data) override;

private:
	FTimerHandle CooldownTimerHandle;
	float TotalCooldownTime;
	float ElapsedCooldownTime;

	void UpdateDuration();
};
