
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/SetWidgetDataInterface.h"
#include "Shared/GAS/GGwaAttributeSet.h"
#include "GGwaWidget.generated.h"

class UItemToolTip;
class USkillToolTip;
class UBuffToolTip;
class UBaseToolTip;
class UPrimaryDataAsset;

class UPlayerStatusWidget;
class USkillSetWidget;
class UItemSetWidget;

class UAbilitySystemComponent;

UCLASS()
class CLIENTMODULE_API UGGwaWidget : public UUserWidget, public ISetWidgetDataInterface
{
	GENERATED_BODY()
public:

	virtual void NativeConstruct() override;
	/** Initialize the widget with the AbilitySystemComponent and GGwaAttributeSet */

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="AbilitySystem")
	TObjectPtr<UAbilitySystemComponent> ASC;
	UPROPERTY(Transient, BlueprintReadWrite, Category="AbilitySystem")
	const UGGwaAttributeSet* GGwaAttributeSet;
	// UPROPERTY(EditAnywhere, BlueprintReadWrite,Category= "Widget")
	// TObjectPtr<UBaseToolTip> BaseToolTipWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category= "Widget")
	TSubclassOf<UBuffToolTip> BuffToolTipClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category= "Widget")
	TSubclassOf<USkillToolTip> SkillToolTipClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category= "Widget")
	TSubclassOf<UItemToolTip> ItemToolTipClass;

	/*Player Status widget에서 buff Widget 구조 확인하기, depth 깊음*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UPlayerStatusWidget> BP_PlayerStatusWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<USkillSetWidget> BP_SkillBarWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UItemSetWidget> BP_ItemBarWidget;

	virtual void SetWidgetData(UPrimaryDataAsset* Data) override;
	void InitWidget(UAbilitySystemComponent * AbilitySystemComponent, const UGGwaAttributeSet * AttributeSet);
	void OnHealthChanged(const FOnAttributeChangeData& Data) const;
	void OnManaChanged(const FOnAttributeChangeData& Data) const;
};
