#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AuthRPCInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UAuthRPCInterface : public UInterface
{
	GENERATED_BODY()
};

class GAMESHAREDMODULE_API IAuthRPCInterface
{
	GENERATED_BODY()

public:
	virtual void RequestServerRegistration(const FString& Username, const FString& Password) = 0;

	virtual void RequestServerLogin(const FString& Username, const FString& Password) = 0;

	virtual bool IsAuthRPCAvailable() const = 0;

	virtual void Request_Client_TravelToGameWorld(const FString& MapURL) = 0;

	virtual void Request_Client_ConnectToGameServerWithToken(const FString& Token, const FString& UserId) = 0;

	// New callbacks for auth results (server -> client bridge intent)
	virtual void NotifyAuthLoginResult(bool bSuccess, const FString& UserId, const FString& Token, const FString& ErrorCode) {}
	virtual void NotifyAuthRegisterResult(bool bSuccess, const FString& UserId, const FString& Token, const FString& ErrorCode) {}
};
