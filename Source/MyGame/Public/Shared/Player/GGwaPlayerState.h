#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "InventoryComponent.h"
#include "Components/SkillComponent.h"
#include "ShopModule/Public/Components/ShopComponent.h"
#include "EquipmentModule/Public/Components/EquipmentComponent.h"
#include "GameFramework/PlayerState.h"
#include "Interface/PlayerIdentityInterface.h"
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
class MYGAME_API AGGwaPlayerState : public APlayerState, public IAbilitySystemInterface, public IPlayerIdentityInterface
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
	void SetStateComponent(UPlayerStateComponent* NewComponent);
	
	UFUNCTION(BlueprintCallable)
	void SetSkillComponent(USkillComponent* NewComponent);
	
	UFUNCTION(BlueprintCallable)
	void SetShopComponent(UShopComponent* NewComponent);
	
	UFUNCTION(BlueprintCallable)
	void SetEquipmentComponent(UEquipmentComponent* NewComponent);
	
	UFUNCTION(BlueprintCallable)
	void SetInventoryComponent(UInventoryComponent* NewComponent);

	void BroadcastAttributeChange(const FGameplayAttribute& Attribute, float NewValue) const;
	void InitPlayerState();

	UPROPERTY(BlueprintAssignable, Category = "GAS")
	FOnAttributeChanged OnAttributeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Skills")
	FOnSkillsUpdated OnSkillsUpdated;

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnSkillLoadCompleted(const FGuid& PlayerGuid);

	UFUNCTION()
	void OnSkillOperationFailed(const FGuid& PlayerGuid, const FString& Reason);

public:
	UFUNCTION(BlueprintCallable)
	virtual FGuid GetPlayerGuid() const override {
		if (UserKey.IsValid()) {
			return UserKey;
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("PlayerState: UserKey is not valid, returning empty GUID"));
		}
		return FGuid();
	} 
	UFUNCTION()
	virtual void SetPlayerGuid(const FString& UserGuid) override {
		FGuid::Parse(UserGuid, UserKey);
	}

protected:
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

	UPROPERTY()
	FGuid UserKey;
};
