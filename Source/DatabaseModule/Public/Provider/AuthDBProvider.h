#pragma once
#include "Interface/Provider/IAuthDBProvider.h"
#include "DatabaseManager.h"

/*DEPRECATED
 * 현재 Auth 시스템은 Node.js 기반의 외부 서버로 연동되어있습니다.
 */
class AuthDBProvider : public IAuthDBProvider {
public:
	AuthDBProvider(UDatabaseManager* InDB): DBManager(InDB) {}

	virtual UE::Tasks::TTask<bool> LockUserAccount(int32 UserId, const FDateTime& ExpiresAt) override;
	virtual UE::Tasks::TTask<bool> UnlockUserAccount(const FString& UserId) override;
	virtual UE::Tasks::TTask<bool> UpdateLastLogin(const FString& UserId) override;
	virtual UE::Tasks::TTask<TArray<FDatabaseUserData>> GetExpiredLockedUsers() override;
	virtual UE::Tasks::TTask<int32> UnlockExpiredUsers() override;
	virtual UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> GetUserAuditLogs(int32 UserId, int32 Limit) override;
	virtual UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> GetAuditLogsByAction(const FString& Action, int32 Limit) override;
	virtual UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> GetRecentAuditLogs(int32 Limit) override;
private:
	TWeakObjectPtr<UDatabaseManager> DBManager;
};
