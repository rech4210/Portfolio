#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IClientComponentProvider.generated.h"

// Forward declarations
class USkillComponent;
struct FBossDataStruct;

// ============================================================================
// INDIVIDUAL CLIENT INTERFACE DEFINITIONS
// ============================================================================

/**
 * Interface for client authentication functionality
 */
UINTERFACE(MinimalAPI)
class UClientAuthInterface : public UInterface
{
	GENERATED_BODY()
};

class MYGAME_API IClientAuthInterface
{
	GENERATED_BODY()

public:
	virtual void InitializeAuth() = 0;
	virtual void RequestRegistration(const FString& Username, const FString& Password) = 0;
	virtual void RequestLogin(const FString& Username, const FString& Password) = 0;
	virtual void OnServerRegistrationResult(bool bSuccess, const FString& Message) = 0;
	virtual void OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId) = 0;
};

/**
 * Interface for client UI functionality
 */
UINTERFACE(MinimalAPI)
class UClientUIInterface : public UInterface
{
	GENERATED_BODY()
};

class MYGAME_API IClientUIInterface
{
	GENERATED_BODY()

public:
	virtual void InitializeUI(const USkillComponent* SkillComponent) = 0;
	virtual void HandleMouseOverDetection() = 0;
	virtual void NotifyStateChanged() = 0;
	virtual void ReceiveBossData(const FBossDataStruct& BossData) = 0;
	virtual void ReceiveSkillData(const USkillComponent* SkillComponent) = 0;
};

// ============================================================================
// MAIN CLIENT SERVICE LOCATOR INTERFACE
// ============================================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UClientServiceManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Main interface for client service management and location
 * Similar to UISubsystemInterface pattern used in GGwaGameState
 * Provides access to various client-specific services through interface segregation
 */
class MYGAME_API IClientServiceManagerInterface
{
	GENERATED_BODY()

public:
	// Service location methods
	virtual IClientAuthInterface* GetAuthService() = 0;
	virtual IClientUIInterface* GetUIService() = 0;
	
	// Service registration methods (called from ClientModule)
	virtual void SetAuthService(TScriptInterface<IClientAuthInterface> AuthService) = 0;
	virtual void SetUIService(TScriptInterface<IClientUIInterface> UIService) = 0;
	
	// Service availability check
	virtual bool IsServiceReady() const = 0;
};

// ============================================================================
// LEGACY COMPATIBILITY INTERFACE
// ============================================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UClientComponentProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Legacy interface for providing access to client-specific components
 * Now delegates to IClientServiceManagerInterface for actual implementation
 * Maintained for backward compatibility
 */
class MYGAME_API IClientComponentProvider
{
	GENERATED_BODY()

public:
	// Authentication component interface
	virtual void InitializeClientAuth() = 0;
	virtual void RequestClientRegistration(const FString& Username, const FString& Password) = 0;
	virtual void RequestClientLogin(const FString& Username, const FString& Password) = 0;
	virtual void OnServerRegistrationResult(bool bSuccess, const FString& Message) = 0;
	virtual void OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId) = 0;

	// UI component interface
	virtual void InitializeClientUI(const class USkillComponent* SkillComponent) = 0;
	virtual void HandleClientMouseOverDetection() = 0;
	virtual void NotifyClientStateChanged() = 0;
	virtual void ReceiveBossDataFromServer(const struct FBossDataStruct& BossData) = 0;
	virtual void ReceiveSkillDataFromServer(const class USkillComponent* SkillComponent) = 0;
};
