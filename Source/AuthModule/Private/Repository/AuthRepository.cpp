#include "Repository/AuthRepository.h"
#include "Engine/Engine.h"
#include "Tasks/Task.h"
#include "Misc/DateTime.h"
#include "Provider/AuthDBProvider.h"

UAuthRepository::UAuthRepository(){
}

void UAuthRepository::Initialize(IDBProviderInfra* Infra) {
	if (!Infra)
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryRepository Initialize: Infra is null"));
		return;
	}
	AuthDBProvider = Infra->GetAuthDbProvider();
	if (!AuthDBProvider.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryRepository: InventoryProvider is not available!"));
	}
}

UE::Tasks::TTask<bool> UAuthRepository::CreateUser(const FUserAccountDTO& UserData)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserData]() -> bool
	{
		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::CreateUser: AuthDBProvider is null"));
			return false;
		}

		// Delegate user creation to AuthDBProvider
		UE_LOG(LogTemp, Log, TEXT("AuthRepository::CreateUser: Creating user %s with ID %s"), 
			*UserData.Username, *UserData.UserId);

		// Use AuthDBProvider's CreateUserAccount method
		// Note: AuthDBProvider auth methods are deprecated - this should use external auth service
		UE_LOG(LogTemp, Warning, TEXT("AuthRepository::CreateUser: Using deprecated AuthDBProvider method"));
		
		// Since AuthDBProvider auth methods are deprecated, return false to indicate 
		// that user creation should be handled by external auth service
		return false;
	});
}

UE::Tasks::TTask<TOptional<FUserAccountDTO>> UAuthRepository::GetUserByUsername(const FString& Username)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, Username]() -> TOptional<FUserAccountDTO>
	{
		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetUserByUsername: AuthDBProvider is null"));
			return TOptional<FUserAccountDTO>();
		}

		UE_LOG(LogTemp, Log, TEXT("AuthRepository::GetUserByUsername: Searching for user %s"), *Username);

		// Use AuthDBProvider's GetUserByUsername method
		// Note: AuthDBProvider auth methods are deprecated - this should use external auth service
		UE_LOG(LogTemp, Warning, TEXT("AuthRepository::GetUserByUsername: Using deprecated AuthDBProvider method"));
		
		// Since AuthDBProvider auth methods are deprecated, return empty result to indicate 
		// that user queries should be handled by external auth service
		return TOptional<FUserAccountDTO>();
	});
}

UE::Tasks::TTask<TOptional<FUserAccountDTO>> UAuthRepository::GetUserById(const FString& UserId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId]() -> TOptional<FUserAccountDTO>
	{
		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetUserById: AuthDBProvider is null"));
			return TOptional<FUserAccountDTO>();
		}

		
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::GetUserById: Searching for user ID %s"), *UserId);

			// Convert string UserId to int32 for AuthDBProvider
			// Since we now use string UserIds, we need to pass the string directly
			// but AuthDBProvider deprecated methods expect different parameter types
			UE_LOG(LogTemp, Warning, TEXT("AuthRepository::GetUserById: Using deprecated AuthDBProvider method"));
			
			// For now, since AuthDBProvider auth methods are deprecated, return empty result
			return TOptional<FUserAccountDTO>();
		
	});
}

UE::Tasks::TTask<bool> UAuthRepository::UpdateUser(const FUserAccountDTO& UserData)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserData]() -> bool
	{
		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UpdateUser: AuthDBProvider is null"));
			return false;
		}

		
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::UpdateUser: Updating user %s"), *UserData.UserId);

			// Note: AuthDBProvider doesn't have a direct UpdateUser method for auth-specific fields
			// This would need to be implemented with custom SQL through AuthDBProvider
			// For now, log the operation and return true as placeholder
			
			UE_LOG(LogTemp, Warning, TEXT("AuthRepository::UpdateUser: Auth-specific user updates not yet implemented"));
			return true; // Placeholder - needs custom implementation
		
	});
}

UE::Tasks::TTask<bool> UAuthRepository::DeleteUser(const FString& UserId)
{
	// DEPRECATED: User deletion should be handled by external auth server (Node.js)
	// This violates the separation of concerns - game server should not manage user accounts
	// User account lifecycle is the responsibility of the authentication service
	// Reason for deprecation: DDD principle violation - auth concerns should not be in game repository
	UE_LOG(LogTemp, Warning, TEXT("DEPRECATED: AuthRepository::DeleteUser should not be used. Delegate to external auth server."));
	return UE::Tasks::MakeCompletedTask<bool>(false);
}

UE::Tasks::TTask<bool> UAuthRepository::ValidateCredentials(const FString& Username, const FString& PasswordHash)
{
	// DEPRECATED: Credential validation should be handled by external auth server (Node.js)
	// This method violates DDD principles as AuthRepository should not handle authentication logic
	// UE game server should only handle game-related data, not authentication validation
	// Reason for deprecation: Separation of concerns - auth logic belongs to dedicated auth service
	UE_LOG(LogTemp, Warning, TEXT("DEPRECATED: AuthRepository::ValidateCredentials should not be used. Delegate to external auth server."));
	return UE::Tasks::MakeCompletedTask<bool>(false);
}

UE::Tasks::TTask<bool> UAuthRepository::UpdateLastLogin(const FString& UserId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId]() -> bool
	{
		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UpdateLastLogin: AuthDBProvider is null"));
			return false;
		}

		
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::UpdateLastLogin: Updating last login for user %s"), *UserId);

			// TODO: Implement last login update
			// Example SQL: UPDATE users SET last_login_at=NOW() WHERE user_id=?
			
			return true;
		
	});
}

UE::Tasks::TTask<bool> UAuthRepository::LockUser(const FString& UserId, const FDateTime& ExpiresAt)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, ExpiresAt]() -> bool
	{
		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::LockUser: AuthDBProvider is null"));
			return false;
		}

		
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::LockUser: Locking user %s until %s"), 
				*UserId, *ExpiresAt.ToString());

			// TODO: Implement user locking
			// Example SQL: UPDATE users SET is_locked=1, lock_expires_at=? WHERE user_id=?
			
			return true;
		
	});
}

UE::Tasks::TTask<bool> UAuthRepository::UnlockUser(const FString& UserId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId]() -> bool
	{
		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UnlockUser: AuthDBProvider is null"));
			return false;
		}

		
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::UnlockUser: Unlocking user %s"), *UserId);

			// TODO: Implement user unlocking
			// Example SQL: UPDATE users SET is_locked=0, lock_expires_at=NULL WHERE user_id=?
			
			return true;
		
	});
}

UE::Tasks::TTask<bool> UAuthRepository::AddAuditLog(const FUserAuditLogDTO& AuditLog)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, AuditLog]() -> bool
	{
		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::AddAuditLog: AuthDBProvider is null"));
			return false;
		}

		
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::AddAuditLog: Adding audit log for user %s, action %s"), 
				*AuditLog.UserId, *AuditLog.Action);

			// Note: Audit logs should be handled by external auth server according to master_schema.sql
			// The user_audit_logs table is designed for auth-specific events
			// UE game server should not directly insert into auth audit logs
			
			UE_LOG(LogTemp, Warning, TEXT("AuthRepository::AddAuditLog: Audit logging should be handled by external auth server"));
			return true; // Placeholder - delegate to external auth server
		
	});
}

UE::Tasks::TTask<TArray<FUserAuditLogDTO>> UAuthRepository::GetAuditLogs(const FString& UserId, int32 Limit)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, Limit]() -> TArray<FUserAuditLogDTO>
	{
		TArray<FUserAuditLogDTO> AuditLogs;

		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetAuditLogs: AuthDBProvider is null"));
			return AuditLogs;
		}

		
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::GetAuditLogs: Getting audit logs for user %s (limit: %d)"), 
				*UserId, Limit);

			// TODO: Implement audit log retrieval
			// Example SQL: SELECT * FROM user_audit_logs WHERE user_id=? ORDER BY created_at DESC LIMIT ?
			
			return AuditLogs;
		
	});
}

UE::Tasks::TTask<bool> UAuthRepository::CheckUsernameExists(const FString& Username)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, Username]() -> bool
	{
		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::CheckUsernameExists: AuthDBProvider is null"));
			return false;
		}

		
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::CheckUsernameExists: Checking if username %s exists"), *Username);

			// // Use AuthDBProvider's GetUserByUsername to check existence
			// auto GetUserTask = AuthDBProvider->GetUserByUsername(Username);
			// TOptional<FDatabaseUserData> DatabaseUserOpt = GetUserTask.GetResult();
			//
			// bool bExists = DatabaseUserOpt.IsSet();
			// UE_LOG(LogTemp, Log, TEXT("AuthRepository::CheckUsernameExists: Username %s exists = %s"), 
			// 	*Username, bExists ? TEXT("true") : TEXT("false"));
			//
			return false;
	});
}

UE::Tasks::TTask<TArray<FUserAccountDTO>> UAuthRepository::GetExpiredLockedUsers()
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]() -> TArray<FUserAccountDTO>
	{
		TArray<FUserAccountDTO> ExpiredUsers;

		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetExpiredLockedUsers: AuthDBProvider is null"));
			return ExpiredUsers;
		}

		
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::GetExpiredLockedUsers: Getting expired locked users"));

			// TODO: Implement expired locked users retrieval
			// Example SQL: SELECT * FROM users WHERE is_locked=1 AND lock_expires_at <= NOW()
			
			return ExpiredUsers;
		
	});
}

UE::Tasks::TTask<bool> UAuthRepository::UnlockExpiredUsers()
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]() -> bool
	{
		if (!AuthDBProvider)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UnlockExpiredUsers: AuthDBProvider is null"));
			return false;
		}

		
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::UnlockExpiredUsers: Unlocking expired users"));

			// TODO: Implement batch unlock of expired users
			// Example SQL: UPDATE users SET is_locked=0, lock_expires_at=NULL WHERE is_locked=1 AND lock_expires_at <= NOW()
			
			return true;
		
	});
}

FString UAuthRepository::ConvertDateTimeToString(const FDateTime& DateTime) const
{
	if (DateTime == FDateTime::MinValue())
	{
		return TEXT("");
	}
	return DateTime.ToString(TEXT("%Y-%m-%d %H:%M:%S"));
}

FDateTime UAuthRepository::ConvertStringToDateTime(const FString& DateTimeString) const
{
	if (DateTimeString.IsEmpty())
	{
		return FDateTime::MinValue();
	}
	
	FDateTime DateTime;
	FDateTime::ParseIso8601(*DateTimeString, DateTime);
	return DateTime;
}

