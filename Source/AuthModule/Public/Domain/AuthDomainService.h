#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Repository/AuthRepositoryInterface.h"
#include "Domain/AuthComponent.h"
#include "Data/AuthDTO.h"
#include "Tasks/Task.h"
#include "AuthDomainService.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAuthenticationComplete, bool, bSuccess, const FAuthResponseDTO&, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRegistrationComplete, bool, bSuccess, const FString&, Message);

/**
 * Auth Domain Service
 * Contains core business logic for authentication operations
 * This is where the main authentication rules and workflows are implemented
 */
UCLASS(BlueprintType)
class AUTHMODULE_API UAuthDomainService : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TScriptInterface<IAuthRepositoryInterface> AuthRepository;

	// Security configuration
	UPROPERTY(EditAnywhere, Category = "Security")
	int32 MaxLoginAttempts = 5;

	UPROPERTY(EditAnywhere, Category = "Security")
	int32 LockoutDurationMinutes = 30;

	UPROPERTY(EditAnywhere, Category = "Security")
	bool bEnableAuditLogging = true;

public:
	UAuthDomainService();

	// Initialization
	UFUNCTION(BlueprintCallable, Category = "Auth Domain Service")
	void Initialize(TScriptInterface<IAuthRepositoryInterface> Repository);

	// Core Authentication Business Logic
	UE::Tasks::TTask<FAuthResponseDTO> RegisterUser(const FAuthRequestDTO& Request);

	UE::Tasks::TTask<FAuthResponseDTO> AuthenticateUser(const FAuthRequestDTO& Request);

	UE::Tasks::TTask<bool> ChangeUserPassword(const FString& UserId, const FString& CurrentPassword, const FString& NewPassword);

	UE::Tasks::TTask<bool> LockUserAccount(const FString& UserId, const FString& Reason);

	UE::Tasks::TTask<bool> UnlockUserAccount(const FString& UserId);

	UE::Tasks::TTask<bool> DeactivateUserAccount(const FString& UserId, const FString& Reason);

	// User Management
	UE::Tasks::TTask<TOptional<FUserAccountDTO>> GetUserInfo(const FString& UserId);

	UE::Tasks::TTask<TArray<FUserAuditLogDTO>> GetUserAuditHistory(const FString& UserId, int32 Limit = 50);

	// Maintenance Operations
	UE::Tasks::TTask<int32> UnlockExpiredAccounts();

	UFUNCTION(BlueprintCallable, Category = "Auth Domain Service")
	bool ValidatePasswordComplexity(const FString& Password, FString& OutErrorMessage);

	UFUNCTION(BlueprintCallable, Category = "Auth Domain Service")
	bool ValidateUsernameFormat(const FString& Username, FString& OutErrorMessage);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Auth Events")
	FOnAuthenticationComplete OnAuthenticationComplete;

	UPROPERTY(BlueprintAssignable, Category = "Auth Events")
	FOnRegistrationComplete OnRegistrationComplete;

private:
	// Helper methods
	FString HashPassword(const FString& Password) const;
	bool VerifyPassword(const FString& Password, const FString& Hash) const;
	FString GenerateUserId() const;
	void LogAuditEvent(const FString& UserId, const FString& Action, const FString& Detail) const;
	FAuthResponseDTO CreateErrorResponse(int32 ErrorCode, const FString& ErrorMessage) const;
	FAuthResponseDTO CreateSuccessResponse(const FString& Token, const FString& UserId) const;
};
