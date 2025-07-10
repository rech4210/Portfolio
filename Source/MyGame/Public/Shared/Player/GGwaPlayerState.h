// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameSharedModule/Public/Enum/EPlayerState.h"
#include "GameplayEffectTypes.h"
#include "InventoryComponent.h"
#include "Components/SkillComponent.h"
#include "ShopModule/Public/Components/ShopComponent.h"
#include "EquipmentModule/Public/Components/EquipmentComponent.h"
#include "GameFramework/PlayerState.h"
#include "GGwaPlayerState.generated.h"

class USkillDataAsset;
class AGGwaCharacter;
class UPlayerStateComponent;
class UGGwaAttributeSet;
class UGGwaAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged, FGameplayAttribute, Attribute, float, NewValue, USkillDataAsset*, SkillData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillsUpdated);

/**
 * 
 */
UCLASS()
class MYGAME_API AGGwaPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGGwaPlayerState();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(BlueprintCallable)
	UPlayerStateComponent* GetStateComponent() const;
	
	UFUNCTION(BlueprintCallable)
	USkillComponent* GetSkillComponent() const;
	
	UFUNCTION(BlueprintCallable)
	UShopComponent* GetShopComponent() const;
	
	UFUNCTION(BlueprintCallable)
	UEquipmentComponent* GetEquipmentComponent() const;

	UFUNCTION(BlueprintCallable)
	UInventoryComponent* GetInventoryComponent() const;
	
	UFUNCTION(BlueprintCallable)
	void SetSkillComponent(USkillComponent* NewComponent);

	UFUNCTION()
	void OnSkillSlotsUpdated() const;

	void BroadcastAttributeChange(const FGameplayAttribute& Attribute, float NewValue) const;
	void InitPlayerState();

	UPROPERTY(BlueprintAssignable, Category = "GAS")
	FOnAttributeChanged OnAttributeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Skills")
	FOnSkillsUpdated OnSkillsUpdated;

protected:
	virtual void BeginPlay() override;

	// Domain Service Event Handlers
	UFUNCTION()
	void OnSkillLoadCompleted(APlayerState* PlayerState);

	UFUNCTION()
	void OnSkillOperationFailed(APlayerState* PlayerState, const FString& Reason);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UGGwaAbilitySystemComponent> ASC;
	
	UPROPERTY()
	TObjectPtr<UGGwaAttributeSet> AttributeSet;
	
	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UPlayerStateComponent> StateComponent;
	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UInventoryComponent> InventoryComponent;
	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<USkillComponent> SkillComponent;
	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UShopComponent> ShopComponent;
	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UEquipmentComponent> EquipmentComponent;
private:
	UPROPERTY()
	TObjectPtr<AGGwaCharacter> Character;
};
