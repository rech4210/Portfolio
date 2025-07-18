#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IClientComponentProvider.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UClientComponentProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for providing access to client-specific components
 * This avoids circular dependencies between MyGame and ClientModule
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
