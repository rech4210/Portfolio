#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Tasks/Task.h"

#include "IAuthDBProvider.generated.h"

struct FDatabaseUserData; // forward
struct FDatabaseAuditLogData; // forward

UINTERFACE()
class UAuthDBProvider : public UInterface {
 	GENERATED_BODY()
};

class GAMESHAREDMODULE_API IAuthDBProvider {
	GENERATED_BODY()
public:
	// Account state operations
	virtual UE::Tasks::TTask<bool> LockUserAccount(int32 UserId, const FDateTime& ExpiresAt) = 0;
	virtual UE::Tasks::TTask<bool> UnlockUserAccount(const FString& UserId) = 0;
	virtual UE::Tasks::TTask<bool> UpdateLastLogin(const FString& UserId) = 0;
	virtual UE::Tasks::TTask<TArray<FDatabaseUserData>> GetExpiredLockedUsers() = 0;
	virtual UE::Tasks::TTask<int32> UnlockExpiredUsers() = 0;

	// Audit logs
	virtual UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> GetUserAuditLogs(int32 UserId, int32 Limit) = 0;
	virtual UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> GetAuditLogsByAction(const FString& Action, int32 Limit) = 0;
	virtual UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> GetRecentAuditLogs(int32 Limit) = 0;
};
