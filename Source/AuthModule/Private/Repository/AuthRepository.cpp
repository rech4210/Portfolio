#include "Repository/AuthRepository.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Engine/Engine.h"
#include "Tasks/Task.h"
#include "Misc/DateTime.h"

UAuthRepository::UAuthRepository()
{
	DatabaseManager = nullptr;
}

void UAuthRepository::Initialize()
{
	if (GetWorld())
	{
		DatabaseManager = GetWorld()->GetGameInstance()->GetSubsystem<UDatabaseManager>();
		if (DatabaseManager)
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository: DatabaseManager initialized successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository: Failed to get DatabaseManager subsystem"));
		}
	}
}

UE::Tasks::TTask<bool> UAuthRepository::CreateUser(const FUserAccountDTO& UserData)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserData]() -> bool
	{
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::CreateUser: DatabaseManager is null"));
			return false;
		}

		try
		{
			// Delegate user creation to DatabaseManager
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::CreateUser: Creating user %s with ID %s"), 
				*UserData.Username, *UserData.UserId);

			// Use DatabaseManager's CreateUserAccount method
			// Note: DatabaseManager auth methods are deprecated - this should use external auth service
			UE_LOG(LogTemp, Warning, TEXT("AuthRepository::CreateUser: Using deprecated DatabaseManager method"));
			
			// Since DatabaseManager auth methods are deprecated, return false to indicate 
			// that user creation should be handled by external auth service
			return false;
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::CreateUser: Database error - %hs"), 
				e.what());
			return false;
		}
	});
}

UE::Tasks::TTask<TOptional<FUserAccountDTO>> UAuthRepository::GetUserByUsername(const FString& Username)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, Username]() -> TOptional<FUserAccountDTO>
	{
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetUserByUsername: DatabaseManager is null"));
			return TOptional<FUserAccountDTO>();
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::GetUserByUsername: Searching for user %s"), *Username);

			// Use DatabaseManager's GetUserByUsername method
			// Note: DatabaseManager auth methods are deprecated - this should use external auth service
			UE_LOG(LogTemp, Warning, TEXT("AuthRepository::GetUserByUsername: Using deprecated DatabaseManager method"));
			
			// Since DatabaseManager auth methods are deprecated, return empty result to indicate 
			// that user queries should be handled by external auth service
			return TOptional<FUserAccountDTO>();
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetUserByUsername: Database error - %hs"), 
				e.what());
			return TOptional<FUserAccountDTO>();
		}
	});
}

UE::Tasks::TTask<TOptional<FUserAccountDTO>> UAuthRepository::GetUserById(const FString& UserId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId]() -> TOptional<FUserAccountDTO>
	{
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetUserById: DatabaseManager is null"));
			return TOptional<FUserAccountDTO>();
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::GetUserById: Searching for user ID %s"), *UserId);

			// Convert string UserId to int32 for DatabaseManager
			// Since we now use string UserIds, we need to pass the string directly
			// but DatabaseManager deprecated methods expect different parameter types
			UE_LOG(LogTemp, Warning, TEXT("AuthRepository::GetUserById: Using deprecated DatabaseManager method"));
			
			// For now, since DatabaseManager auth methods are deprecated, return empty result
			return TOptional<FUserAccountDTO>();
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetUserById: Database error - %hsp"), 
				e.what());
			return TOptional<FUserAccountDTO>();
		}
	});
}

UE::Tasks::TTask<bool> UAuthRepository::UpdateUser(const FUserAccountDTO& UserData)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserData]() -> bool
	{
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UpdateUser: DatabaseManager is null"));
			return false;
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::UpdateUser: Updating user %s"), *UserData.UserId);

			// Note: DatabaseManager doesn't have a direct UpdateUser method for auth-specific fields
			// This would need to be implemented with custom SQL through DatabaseManager
			// For now, log the operation and return true as placeholder
			
			UE_LOG(LogTemp, Warning, TEXT("AuthRepository::UpdateUser: Auth-specific user updates not yet implemented"));
			return true; // Placeholder - needs custom implementation
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UpdateUser: Database error - %hsp"), 
				e.what());
			return false;
		}
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
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UpdateLastLogin: DatabaseManager is null"));
			return false;
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::UpdateLastLogin: Updating last login for user %s"), *UserId);

			// TODO: Implement last login update
			// Example SQL: UPDATE users SET last_login_at=NOW() WHERE user_id=?
			
			return true;
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UpdateLastLogin: Database error - %hsp"), 
				e.what());
			return false;
		}
	});
}

UE::Tasks::TTask<bool> UAuthRepository::LockUser(const FString& UserId, const FDateTime& ExpiresAt)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, ExpiresAt]() -> bool
	{
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::LockUser: DatabaseManager is null"));
			return false;
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::LockUser: Locking user %s until %s"), 
				*UserId, *ExpiresAt.ToString());

			// TODO: Implement user locking
			// Example SQL: UPDATE users SET is_locked=1, lock_expires_at=? WHERE user_id=?
			
			return true;
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::LockUser: Database error - %hsp"), 
				e.what());
			return false;
		}
	});
}

UE::Tasks::TTask<bool> UAuthRepository::UnlockUser(const FString& UserId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId]() -> bool
	{
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UnlockUser: DatabaseManager is null"));
			return false;
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::UnlockUser: Unlocking user %s"), *UserId);

			// TODO: Implement user unlocking
			// Example SQL: UPDATE users SET is_locked=0, lock_expires_at=NULL WHERE user_id=?
			
			return true;
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UnlockUser: Database error - %hsp"), 
				e.what());
			return false;
		}
	});
}

UE::Tasks::TTask<bool> UAuthRepository::AddAuditLog(const FUserAuditLogDTO& AuditLog)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, AuditLog]() -> bool
	{
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::AddAuditLog: DatabaseManager is null"));
			return false;
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::AddAuditLog: Adding audit log for user %s, action %s"), 
				*AuditLog.UserId, *AuditLog.Action);

			// Note: Audit logs should be handled by external auth server according to master_schema.sql
			// The user_audit_logs table is designed for auth-specific events
			// UE game server should not directly insert into auth audit logs
			
			UE_LOG(LogTemp, Warning, TEXT("AuthRepository::AddAuditLog: Audit logging should be handled by external auth server"));
			return true; // Placeholder - delegate to external auth server
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::AddAuditLog: Database error - %hsp"), 
				e.what());
			return false;
		}
	});
}

UE::Tasks::TTask<TArray<FUserAuditLogDTO>> UAuthRepository::GetAuditLogs(const FString& UserId, int32 Limit)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, Limit]() -> TArray<FUserAuditLogDTO>
	{
		TArray<FUserAuditLogDTO> AuditLogs;

		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetAuditLogs: DatabaseManager is null"));
			return AuditLogs;
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::GetAuditLogs: Getting audit logs for user %s (limit: %d)"), 
				*UserId, Limit);

			// TODO: Implement audit log retrieval
			// Example SQL: SELECT * FROM user_audit_logs WHERE user_id=? ORDER BY created_at DESC LIMIT ?
			
			return AuditLogs;
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetAuditLogs: Database error - %hsp"), 
				e.what());
			return AuditLogs;
		}
	});
}

UE::Tasks::TTask<bool> UAuthRepository::CheckUsernameExists(const FString& Username)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, Username]() -> bool
	{
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::CheckUsernameExists: DatabaseManager is null"));
			return false;
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::CheckUsernameExists: Checking if username %s exists"), *Username);

			// Use DatabaseManager's GetUserByUsername to check existence
			auto GetUserTask = DatabaseManager->GetUserByUsername(Username);
			TOptional<FDatabaseUserData> DatabaseUserOpt = GetUserTask.GetResult();
			
			bool bExists = DatabaseUserOpt.IsSet();
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::CheckUsernameExists: Username %s exists = %s"), 
				*Username, bExists ? TEXT("true") : TEXT("false"));
			
			return bExists;
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::CheckUsernameExists: Database error - %hsp"), 
				e.what());
			return false;
		}
	});
}

UE::Tasks::TTask<TArray<FUserAccountDTO>> UAuthRepository::GetExpiredLockedUsers()
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]() -> TArray<FUserAccountDTO>
	{
		TArray<FUserAccountDTO> ExpiredUsers;

		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetExpiredLockedUsers: DatabaseManager is null"));
			return ExpiredUsers;
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::GetExpiredLockedUsers: Getting expired locked users"));

			// TODO: Implement expired locked users retrieval
			// Example SQL: SELECT * FROM users WHERE is_locked=1 AND lock_expires_at <= NOW()
			
			return ExpiredUsers;
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetExpiredLockedUsers: Database error - %hsp"), 
				e.what());
			return ExpiredUsers;
		}
	});
}

UE::Tasks::TTask<bool> UAuthRepository::UnlockExpiredUsers()
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]() -> bool
	{
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UnlockExpiredUsers: DatabaseManager is null"));
			return false;
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::UnlockExpiredUsers: Unlocking expired users"));

			// TODO: Implement batch unlock of expired users
			// Example SQL: UPDATE users SET is_locked=0, lock_expires_at=NULL WHERE is_locked=1 AND lock_expires_at <= NOW()
			
			return true;
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::UnlockExpiredUsers: Database error - %hsp"), 
				e.what());
			return false;
		}
	});
}

// Helper methods - DEPRECATED
FUserAccountDTO UAuthRepository::ConvertDatabaseUserToDTO(const FDatabaseUserData& DatabaseUser) const
{
	// DEPRECATED: This conversion should not be needed as auth data should come from external service
	// Game server should only work with verified user IDs from JWT tokens, not raw auth data
	UE_LOG(LogTemp, Warning, TEXT("DEPRECATED: AuthRepository::ConvertDatabaseUserToDTO should not be used"));
	
	FUserAccountDTO UserData;
	// Return empty data since this conversion violates architectural boundaries
	return UserData;
}

FUserAuditLogDTO UAuthRepository::ConvertAuditLogFromDatabaseResult(const TMap<FString, FString>& DatabaseRow) const
{
	FUserAuditLogDTO AuditLog;
	
	// TODO: Implement conversion from database row to audit log DTO
	
	return AuditLog;
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
