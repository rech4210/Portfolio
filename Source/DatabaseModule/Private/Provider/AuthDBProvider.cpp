#include "Provider/AuthDBProvider.h"
#include "DatabaseManager.h"

UE::Tasks::TTask<bool> AuthDBProvider::LockUserAccount(int32 UserId, const FDateTime& ExpiresAt) {
	if (!DBManager.IsValid()) return UE::Tasks::MakeCompletedTask<bool>(false);
	return DBManager->LockUserAccount(UserId, ExpiresAt);
}

UE::Tasks::TTask<bool> AuthDBProvider::UnlockUserAccount(const FString& UserId) {
	if (!DBManager.IsValid()) return UE::Tasks::MakeCompletedTask<bool>(false);
	return DBManager->UnlockUserAccount(UserId);
}

UE::Tasks::TTask<bool> AuthDBProvider::UpdateLastLogin(const FString& UserId) {
	if (!DBManager.IsValid()) return UE::Tasks::MakeCompletedTask<bool>(false);
	return DBManager->UpdateLastLogin(UserId);
}

UE::Tasks::TTask<TArray<FDatabaseUserData>> AuthDBProvider::GetExpiredLockedUsers() {
	if (!DBManager.IsValid()) return UE::Tasks::MakeCompletedTask<TArray<FDatabaseUserData>>();
	return DBManager->GetExpiredLockedUsers();
}

UE::Tasks::TTask<int32> AuthDBProvider::UnlockExpiredUsers() {
	return UE::Tasks::MakeCompletedTask<int32>(0);
}


UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> AuthDBProvider::GetUserAuditLogs(int32 UserId, int32 Limit) {
	if (!DBManager.IsValid()) return UE::Tasks::MakeCompletedTask<TArray<FDatabaseAuditLogData>>();
	return DBManager->GetUserAuditLogs(UserId, Limit);
}

UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> AuthDBProvider::GetAuditLogsByAction(const FString& Action, int32 Limit) {
	if (!DBManager.IsValid()) return UE::Tasks::MakeCompletedTask<TArray<FDatabaseAuditLogData>>();
	return DBManager->GetAuditLogsByAction(Action, Limit);
}

UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> AuthDBProvider::GetRecentAuditLogs(int32 Limit) {
	if (!DBManager.IsValid()) return UE::Tasks::MakeCompletedTask<TArray<FDatabaseAuditLogData>>();
	return DBManager->GetRecentAuditLogs(Limit);
}
