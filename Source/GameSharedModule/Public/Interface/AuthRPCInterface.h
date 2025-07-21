#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AuthRPCInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UAuthRPCInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for Authentication RPC communication
 * Abstracts auth communication to avoid module dependencies
 */
class GAMESHAREDMODULE_API IAuthRPCInterface
{
	GENERATED_BODY()

public:
	/**
	 * Register user through server RPC
	 * @param Username The username to register
	 * @param Password The password to register
	 */
	virtual void RequestServerRegistration(const FString& Username, const FString& Password) = 0;

	/**
	 * Login user through server RPC
	 * @param Username The username to login
	 * @param Password The password to login
	 */
	virtual void RequestServerLogin(const FString& Username, const FString& Password) = 0;

	/**
	 * Get if this auth RPC provider is available
	 */
	virtual bool IsAuthRPCAvailable() const = 0;

	/**
	 * Travel to game world
	 * @param MapURL The URL to travel to
	 */
	virtual void Request_Client_TravelToGameWorld(const FString& MapURL) = 0;

	/**
	 * Connect to game server with JWT token for authentication
	 * @param Token JWT authentication token
	 * @param UserId User ID for verification
	 */
	virtual void Request_Client_ConnectToGameServerWithToken(const FString& Token, const FString& UserId) = 0;
};
