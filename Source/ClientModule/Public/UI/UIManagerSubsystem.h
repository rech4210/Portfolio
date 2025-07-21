#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Interface/UISubsystemInterface.h"
#include "MyGame/Public/Shared/Interface/IClientComponentProvider.h"
#include "UIManagerSubsystem.generated.h"

class AUIConfigCacheActor;
class UClientAuthComponent;
class UClientUIComponent;

UCLASS(BlueprintType, Blueprintable)
class CLIENTMODULE_API UUIManagerSubsystem : public UGameInstanceSubsystem, public IUISubsystemInterface, public IClientManagerInterface
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// ============================================================================
	// UI
	// ============================================================================

	UFUNCTION()
	void OnMapChanged(UWorld* LoadedWorld);

	// UFUNCTION(BlueprintCallable, Category = "UI Manager")
	virtual void SetCacheActor(AUIConfigCacheActor* NewCacheActor) override;

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void RequestSetWidgetForMap(FName MapName, TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI Manager")
	UUserWidget* GetCurrentWidget() const { return CurrentWidget; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI Manager")
	AUIConfigCacheActor* GetCacheActor() const { return CacheActor; }

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void RefreshCurrentWidget();
	void OnWorldInit(UWorld* NewWorld, const UWorld::InitializationValues IVS);


	// ============================================================================
	// ClientComponent IMPLEMENTATION
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void RemoveCurrentWidget();
	// void RegisterAuthService(UClientAuthComponent* AuthService);
	// void RegisterUIService(UClientUIComponent* UIService);
	bool IsServiceReady() const;

	// ============================================================================
	// IClientUIManagerInterface IMPLEMENTATION
	// ============================================================================
	virtual void RegistClientComponent(UActorComponent* Component) override;
	
	virtual void InitializeUI(const USkillComponent* SkillComponent) override;
	virtual void ProcessRegistration(const FString& Username, const FString& Password) override;
	virtual void ProcessLogin(const FString& Username, const FString& Password) override;
	virtual void HandleRegistrationResult(bool bSuccess, const FString& Message) override;
	virtual void HandleLoginResult(bool bSuccess, const FString& Token, const FString& UserId) override;

	virtual void ProcessMouseOverDetection() override;
	virtual void NotifyStateChanged() override;
	virtual void ProcessBossData(const FBossDataStruct& BossData) override;
	virtual void ProcessSkillData(const USkillComponent* SkillComponent) override;

protected:

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> CurrentWidget;

	UPROPERTY(Transient)
	TObjectPtr<AUIConfigCacheActor> CacheActor;

	// ============================================================================
	// IOC SERVICE MANAGEMENT
	// ============================================================================

	UPROPERTY(Transient)
	TObjectPtr<UClientAuthComponent> ClientAuthService;
	
	UPROPERTY(Transient)
	TObjectPtr<UClientUIComponent> ClientUIService;
};
