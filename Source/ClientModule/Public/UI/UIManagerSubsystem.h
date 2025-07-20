#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Interface/UISubsystemInterface.h"
#include "MyGame/Public/Shared/Interface/IClientComponentProvider.h"
#include "UIManagerSubsystem.generated.h"

class AUIConfigCacheActor;

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


	// ============================================================================
	// ClientComponent IMPLEMENTATION
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "UI Manager")
	void RemoveCurrentWidget();
	void RegisterAuthService(TScriptInterface<IClientAuthInterface> AuthService);
	void RegisterUIService(TScriptInterface<IClientUIInterface> UIService);
	void InitializeAuth();
	bool IsServiceReady() const;

	// ============================================================================
	// IClientUIManagerInterface IMPLEMENTATION
	// ============================================================================

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
	TScriptInterface<IClientAuthInterface> RegisteredAuthService;
	
	UPROPERTY(Transient)
	TScriptInterface<IClientUIInterface> RegisteredUIService;
};
