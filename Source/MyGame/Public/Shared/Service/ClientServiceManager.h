#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shared/Interface/IClientComponentProvider.h"
#include "Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "ClientServiceManager.generated.h"

class USkillComponent;
class IClientUIInterface;
class IClientAuthInterface;

/**
 * UClientServiceManager
 * 
 * ActorComponent that implements service locator pattern for client functionality.
 * Similar to UISubsystemInterface pattern used in GGwaGameState.
 * 
 * Architecture Benefits:
 * - Service Locator pattern (single point of service access)
 * - Interface segregation (Auth and UI services separated)
 * - Dependency injection (services injected from ClientModule)
 * - GGwaGameState-style IoC implementation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class MYGAME_API UClientServiceManager : public UActorComponent, public IClientComponentProvider, public IClientServiceManagerInterface
{
	GENERATED_BODY()

public:
	UClientServiceManager();

protected:
	virtual void BeginPlay() override;

public:
	// ============================================================================
	// IClientServiceManagerInterface INTERFACE IMPLEMENTATION
	// Service location and registration (GGwaGameState pattern)
	// ============================================================================

	virtual IClientAuthInterface* GetAuthService() override;
	virtual IClientUIInterface* GetUIService() override;
	virtual void SetAuthService(TScriptInterface<IClientAuthInterface> AuthService) override;
	virtual void SetUIService(TScriptInterface<IClientUIInterface> UIService) override;
	virtual bool IsServiceReady() const override;

	// ============================================================================
	// IClientComponentProvider INTERFACE IMPLEMENTATION
	// Legacy compatibility - delegates to service locator
	// ============================================================================

	// Authentication delegation
	virtual void InitializeClientAuth() override;
	virtual void RequestClientRegistration(const FString& Username, const FString& Password) override;
	virtual void RequestClientLogin(const FString& Username, const FString& Password) override;
	virtual void OnServerRegistrationResult(bool bSuccess, const FString& Message) override;
	virtual void OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId) override;

	// UI delegation
	virtual void InitializeClientUI(const USkillComponent* SkillComponent) override;
	virtual void HandleClientMouseOverDetection() override;
	virtual void NotifyClientStateChanged() override;
	virtual void ReceiveBossDataFromServer(const FBossDataStruct& BossData) override;
	virtual void ReceiveSkillDataFromServer(const USkillComponent* SkillComponent) override;

	// ============================================================================
	// BLUEPRINT ACCESSIBLE SERVICE ACCESS
	// ============================================================================

	UFUNCTION(BlueprintCallable, Category = "Client Service")
	bool HasAuthService() const { return AuthServiceInterface.GetInterface() != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Client Service")
	bool HasUIService() const { return UIServiceInterface.GetInterface() != nullptr; }

private:
	// ============================================================================
	// SERVICE REFERENCES (GGwaGameState pattern)
	// ============================================================================

	/**
	 * Authentication service interface - injected from ClientModule
	 * Similar to UISubsystemInterface in GGwaGameState
	 */
	UPROPERTY()
	TScriptInterface<IClientAuthInterface> AuthServiceInterface;

	/**
	 * UI service interface - injected from ClientModule
	 * Similar to UISubsystemInterface in GGwaGameState
	 */
	UPROPERTY()
	TScriptInterface<IClientUIInterface> UIServiceInterface;
};
