
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillModule/Public/Data/SkillInputSlot.h"
#include "SkillSlotWidget.generated.h"

class USkillDataAsset;
class UTextBlock;
class USkillToolTip;
class UMaterialInstanceDynamic;
class UImage;

UCLASS()
class CLIENTMODULE_API USkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite ,Category= "ToolTip")
	TObjectPtr<USkillToolTip> SkillToolTipWidget;
	UPROPERTY(VisibleAnywhere ,Category= "Data")
	TObjectPtr<USkillDataAsset> SkillDataAsset = nullptr;
	
	UPROPERTY(VisibleAnywhere,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UImage> SkillImage;
	UPROPERTY(VisibleAnywhere,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UImage> CooldownImage;
	UPROPERTY(VisibleAnywhere,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UTextBlock> CoolTimeText;
	float CoolTime = 999.f;
	float CurrentCoolTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta = (BindWidget) ,Category= "Widget")
	TObjectPtr<UTextBlock> SkillKeyText;
	

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	void UseSkillSlot(USkillDataAsset *Data);
	void ApplySkillData(USkillDataAsset* NewData);
	//
	// virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	// virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	// virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	// virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkillCooldownMaterial;
	FTimerHandle CooldownTimerHandle;
	virtual void NativeConstruct() override;
	void TickCoolDown();
};
