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
			// This follows the master_schema.sql users table structure
			int32 OutUserId;
			auto CreateUserTask = DatabaseManager->CreateUserAccount(
				UserData.Username, 
				UserData.PasswordHash, 
				UserData.Email, 
				OutUserId
			);
			
			bool bUserCreated = CreateUserTask.GetResult();
			
			if (bUserCreated)
			{
				UE_LOG(LogTemp, Log, TEXT("AuthRepository::CreateUser: User created successfully with database ID %d"), OutUserId);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("AuthRepository::CreateUser: Failed to create user in database"));
			}
			
			return bUserCreated;
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
			// This follows the master_schema.sql users table structure
			auto GetUserTask = DatabaseManager->GetUserByUsername(Username);
			TOptional<FDatabaseUserData> DatabaseUserOpt = GetUserTask.GetResult();
			
			if (DatabaseUserOpt.IsSet())
			{
				// Convert DatabaseUserData to UserAccountDTO
				FUserAccountDTO UserData = ConvertDatabaseUserToDTO(DatabaseUserOpt.GetValue());
				return TOptional<FUserAccountDTO>(UserData);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("AuthRepository::GetUserByUsername: User not found - %s"), *Username);
				return TOptional<FUserAccountDTO>();
			}
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
			int32 DatabaseUserId = FCString::Atoi(*UserId);
			if (DatabaseUserId == 0 && UserId != TEXT("0"))
			{
				UE_LOG(LogTemp, Error, TEXT("AuthRepository::GetUserById: Invalid UserId format - %s"), *UserId);
				return TOptional<FUserAccountDTO>();
			}

			// Use DatabaseManager's GetUserById method
			auto GetUserTask = DatabaseManager->GetUserById(DatabaseUserId);
			TOptional<FDatabaseUserData> DatabaseUserOpt = GetUserTask.GetResult();
			
			if (DatabaseUserOpt.IsSet())
			{
				// Convert DatabaseUserData to UserAccountDTO
				FUserAccountDTO UserData = ConvertDatabaseUserToDTO(DatabaseUserOpt.GetValue());
				return TOptional<FUserAccountDTO>(UserData);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("AuthRepository::GetUserById: User not found - %s"), *UserId);
				return TOptional<FUserAccountDTO>();
			}
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
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId]() -> bool
	{
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::DeleteUser: DatabaseManager is null"));
			return false;
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::DeleteUser: Soft deleting user %s"), *UserId);

			// Note: DatabaseManager doesn't have a direct soft delete method for users
			// This operation would be handled by the external auth server (Node.js)
			// Auth repository in UE should not handle user deletion directly
			
			UE_LOG(LogTemp, Warning, TEXT("AuthRepository::DeleteUser: User deletion should be handled by external auth server"));
			return false; // Delegate to external auth server
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::DeleteUser: Database error - %hs"), 
				e.what());
			return false;
		}
	});
}

UE::Tasks::TTask<bool> UAuthRepository::ValidateCredentials(const FString& Username, const FString& PasswordHash)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, Username, PasswordHash]() -> bool
	{
		if (!DatabaseManager)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::ValidateCredentials: DatabaseManager is null"));
			return false;
		}

		try
		{
			UE_LOG(LogTemp, Log, TEXT("AuthRepository::ValidateCredentials: Validating credentials for %s"), *Username);

			// Note: Credential validation is handled by external auth server (Node.js)
			// UE AuthRepository should not directly validate passwords
			// This method should not be used in the current architecture
			
			UE_LOG(LogTemp, Warning, TEXT("AuthRepository::ValidateCredentials: Credential validation should be handled by external auth server"));
			return false; // Always fail - delegate to external auth server
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthRepository::ValidateCredentials: Database error - %hsp"), 
				e.what());
			return false;
		}
	});
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

// Helper methods
FUserAccountDTO UAuthRepository::ConvertDatabaseUserToDTO(const FDatabaseUserData& DatabaseUser) const
{
	FUserAccountDTO UserData;
	
	// Convert from DatabaseManager's FDatabaseUserData to Auth DTO
	UserData.UserId = FString::FromInt(DatabaseUser.UserId); // Convert int32 to string
	UserData.Username = DatabaseUser.Username;
	UserData.Email = DatabaseUser.Email;
	UserData.CreatedAt = DatabaseUser.CreatedAt;
	UserData.LastLoginAt = DatabaseUser.LastLogin.GetValue();
	
	// Note: Password hash and auth-specific fields might not be in FDatabaseUserData
	// These would need to be fetched separately or FDatabaseUserData extended
	UserData.PasswordHash = TEXT(""); // Not included in game database user data
	UserData.bIsLocked = false; // Default values - should be extended in DatabaseManager
	UserData.LockExpiresAt = FDateTime::MinValue();
	UserData.bIsDeleted = false;
	UserData.DeletedAt = FDateTime::MinValue();
	// UserData.FailedLoginAttempts = 0;
	
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
