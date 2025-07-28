#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IClientComponentProvider.generated.h"

// Forward declarations
class USkillComponent;
struct FBossDataStruct;


/**
 * Interface for UI Subsystem (Client Module)
 * This interface allows PlayerController to interact with UIManagerSubsystem
 */
UINTERFACE(MinimalAPI)
class UClientManagerInterface : public UInterface
{
	GENERATED_BODY()
};

class MYGAME_API IClientManagerInterface
{
	GENERATED_BODY()

public:
	virtual void RegistClientComponent(UActorComponent* Component) = 0;
	virtual void InitializeUI(const USkillComponent* SkillComponent) = 0;
	// Auth service delegation
	virtual void ProcessRegistration(const FString& Username, const FString& Password) = 0;
	virtual void ProcessLogin(const FString& Username, const FString& Password) = 0;
	virtual void HandleRegistrationResult(bool bSuccess, const FString& Message) = 0;
	virtual void HandleLoginResult(bool bSuccess, const FString& Token, const FString& UserId) = 0;
	
	// UI service delegation
	virtual void ProcessMouseOverDetection() = 0;
	virtual void NotifyStateChanged() = 0;
	virtual void ProcessBossData(const FBossDataStruct& BossData) = 0;
	virtual void ProcessSkillData(const USkillComponent* SkillComponent) = 0;
};


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
	virtual TScriptInterface<IClientManagerInterface> GetClientSubSystem() = 0;
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
	virtual void SetOwnerController(class AGGwaPlayerController* Controller) = 0;
};
