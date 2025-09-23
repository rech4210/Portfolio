#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AuthTokenTypes.h"
#include "AuthExternalService.h"
#include "GameSharedModule/Public/Interface/AuthRequestRouter.h"
#include "AuthSessionSubsystem.generated.h"

class IAuthRPCInterface; // forward (interface from GameSharedModule)

class AGGwaPlayerController; // retained forward declare (legacy, will be removed after migration)

DECLARE_LOG_CATEGORY_EXTERN(LogServerAuth, Log, All);

/*
 * AuthSessionSubsystem (Server-only)
 * Responsibilities:
 *  - Coordinate external auth service (login/register/verify)
 *  - Manage in-memory token cache (expiry, purge, JWT exp decoding)
 *  - Emit client RPC results (pure transport, no game rules)
 * Non-Responsibilities:
 *  - Password hashing / policy (external service)
 *  - Storing credentials locally (no DB persistence of secrets)
 *  - Token transport via travel URL (Deprecated: avoid ?token= leaks)
 * Migration Notes:
 *  - Legacy UAuthSubsystem to be removed once all references replaced.
 *  - UAuthVerificationService is deprecated—use UAuthExternalService.
 */

UCLASS()
class AUTHSERVERMODULE_API UAuthSessionSubsystem : public UGameInstanceSubsystem, public IAuthRequestRouter
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// IAuthRequestRouter
	virtual void RequestLogin(const FString& Username, const FString& Password, const FString& IP, UObject* CallerContext) override { LoginUser(Username, Password, IP, CallerContext); }
	virtual void RequestRegistration(const FString& Username, const FString& Password, const FString& IP, UObject* CallerContext) override { RegisterUser(Username, Password, IP, CallerContext); }

	bool TryGetTokenByUser(const FString& UserId, FString& OutToken) const;
	void PurgeExpired();

private:
	
	// Legacy direct API (will be internalized)
	void LoginUser(const FString& Username, const FString& Password, const FString& ClientIP, UObject* CallerContext);
	void RegisterUser(const FString& Username, const FString& Password, const FString& ClientIP, UObject* CallerContext);
	void VerifyToken(const FString& Token, TFunction<void(bool bValid, const FString& UserId)> Callback);

	
	UPROPERTY()
	TObjectPtr<UAuthExternalService> ExternalService = nullptr;
	TMap<FString, FAuthTokenRecord> TokenCache; // key = UserId
	FTimerHandle ExpiryTimerHandle;
	float CacheSweepIntervalSeconds = 60.f;

	void ScheduleCacheSweep();
	void CacheSweep();
	void StoreToken(const FString& UserId, const FString& Token);
};
