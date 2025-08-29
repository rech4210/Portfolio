#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/SetWidgetDataInterface.h"
#include "MyGame/Public/Shared/GAS/GGwaAttributeSet.h"
#include "UI/ToolTip/BaseToolTip.h"
#include "UObject/Object.h"
#include "GGwaWidget.generated.h"

class USkillDataAsset;
class UBaseDataAsset;
class UItemToolTip;
class USkillToolTip;
class UBuffToolTip;
class UBaseToolTip;
class UPrimaryDataAsset;

class USkillComponent;
class UPlayerStatusWidget;
class USkillSetWidget;
class UItemSetWidget;
class UGGwaAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplayTagEvent, FGameplayTag, Tag);

UCLASS()
class CLIENTMODULE_API UGGwaWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	FOnGameplayTagEvent OnGameplayTagEvent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="AbilitySystem")
	TObjectPtr<UGGwaAbilitySystemComponent> ASC;
	UPROPERTY(Transient, BlueprintReadWrite, Category="AbilitySystem")
	const UGGwaAttributeSet* GGwaAttributeSet;
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category= "Widget")
	TSubclassOf<UBuffToolTip> BuffToolTipClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category= "Widget")
	TSubclassOf<USkillToolTip> SkillToolTipClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category= "Widget")
	TSubclassOf<UItemToolTip> ItemToolTipClass;

	UPROPERTY()
	TMap<UBaseDataAsset*, UBaseToolTip*> SlotTooltipCachePair;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UPlayerStatusWidget> BP_PlayerStatusWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<USkillSetWidget> BP_SkillBarWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UItemSetWidget> BP_ItemBarWidget;

	UFUNCTION()
	void BindWidgetWithTooltip(UBaseDataAsset* Data);
	void InitWidget(UGGwaAbilitySystemComponent * AbilitySystemComponent, const UGGwaAttributeSet * AttributeSet);
	void OnHealthChanged(const FOnAttributeChangeData& Data) const;
	void OnManaChanged(const FOnAttributeChangeData& Data) const;
	void UpdateSkillWidgetFromServer(TArray<USkillDataAsset*> Datas);
	UFUNCTION()
	void DoWidgetWork();
private:
	TArray<USkillDataAsset*> InitSkillDataAssets;
};
