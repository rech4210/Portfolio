#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/AuthDTO.h"
#include "Tasks/Task.h"
#include "AuthRepositoryInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UAuthRepositoryInterface : public UInterface
{
	GENERATED_BODY()
};

class AUTHMODULE_API IAuthRepositoryInterface
{
	GENERATED_BODY()

public:
	// User Account Operations
	virtual UE::Tasks::TTask<bool> CreateUser(const FUserAccountDTO& UserData) = 0;
	virtual UE::Tasks::TTask<TOptional<FUserAccountDTO>> GetUserByUsername(const FString& Username) = 0;
	virtual UE::Tasks::TTask<TOptional<FUserAccountDTO>> GetUserById(const FString& UserId) = 0;
	virtual UE::Tasks::TTask<bool> UpdateUser(const FUserAccountDTO& UserData) = 0;
	virtual UE::Tasks::TTask<bool> DeleteUser(const FString& UserId) = 0;

	// Authentication Operations
	virtual UE::Tasks::TTask<bool> ValidateCredentials(const FString& Username, const FString& PasswordHash) = 0;
	virtual UE::Tasks::TTask<bool> UpdateLastLogin(const FString& UserId) = 0;
	virtual UE::Tasks::TTask<bool> LockUser(const FString& UserId, const FDateTime& ExpiresAt) = 0;
	virtual UE::Tasks::TTask<bool> UnlockUser(const FString& UserId) = 0;

	// Audit Operations
	virtual UE::Tasks::TTask<bool> AddAuditLog(const FUserAuditLogDTO& AuditLog) = 0;
	virtual UE::Tasks::TTask<TArray<FUserAuditLogDTO>> GetAuditLogs(const FString& UserId, int32 Limit = 100) = 0;

	// Utility Operations
	virtual UE::Tasks::TTask<bool> CheckUsernameExists(const FString& Username) = 0;
	virtual UE::Tasks::TTask<TArray<FUserAccountDTO>> GetExpiredLockedUsers() = 0;
	virtual UE::Tasks::TTask<bool> UnlockExpiredUsers() = 0;
};
