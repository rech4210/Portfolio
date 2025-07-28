#include "Domain/AuthDomainService.h"
#include "Misc/Guid.h"
#include "Misc/DateTime.h"
#include "Misc/Base64.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/SecureHash.h"

UAuthDomainService::UAuthDomainService()
{
	MaxLoginAttempts = 5;
	LockoutDurationMinutes = 30;
	bEnableAuditLogging = true;
}

void UAuthDomainService::Initialize(TScriptInterface<IAuthRepositoryInterface> Repository)
{
	AuthRepository = Repository;
	UE_LOG(LogTemp, Log, TEXT("AuthDomainService: Initialized with repository"));
}

UE::Tasks::TTask<FAuthResponseDTO> UAuthDomainService::RegisterUser(const FAuthRequestDTO& Request)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, Request]() -> FAuthResponseDTO
	{
		UE_LOG(LogTemp, Log, TEXT("AuthDomainService::RegisterUser: Starting registration for user %s"), *Request.Username);

		// 1. Validate input using AuthComponent static validation
		if (!UAuthComponent::IsValidUsername(Request.Username))
		{
			UE_LOG(LogTemp, Warning, TEXT("AuthDomainService::RegisterUser: Invalid username format - %s"), *Request.Username);
			return CreateErrorResponse(400, TEXT("Invalid username format"));
		}

		if (!UAuthComponent::IsValidPassword(Request.Password))
		{
			UE_LOG(LogTemp, Warning, TEXT("AuthDomainService::RegisterUser: Invalid password complexity"));
			return CreateErrorResponse(400, TEXT("Password does not meet complexity requirements"));
		}

		// 2. Check if username already exists
		if (!AuthRepository.GetInterface())
		{
			return CreateErrorResponse(500, TEXT("Repository not available"));
		}

		auto UsernameExistsTask = AuthRepository->CheckUsernameExists(Request.Username);
		bool bUsernameExists = UsernameExistsTask.GetResult();
		
		if (bUsernameExists)
		{
			UE_LOG(LogTemp, Warning, TEXT("AuthDomainService::RegisterUser: Username already exists - %s"), *Request.Username);
			return CreateErrorResponse(409, TEXT("Username already exists"));
		}

		// 3. Create new user account
		FUserAccountDTO NewUserData;
		NewUserData.UserId = GenerateUserId();
		NewUserData.Username = Request.Username;
		NewUserData.PasswordHash = HashPassword(Request.Password);
		NewUserData.CreatedAt = FDateTime::Now();
		NewUserData.LastLoginAt = FDateTime::MinValue();
		NewUserData.bIsLocked = false;
		NewUserData.LockExpiresAt = FDateTime::MinValue();
		NewUserData.bIsDeleted = false;
		NewUserData.DeletedAt = FDateTime::MinValue();

		// 4. Create domain object for business logic validation
		UAuthComponent* UserComponent = UAuthComponent::CreateFromDTO(NewUserData);
		if (!UserComponent || !UserComponent->IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("AuthDomainService::RegisterUser: Failed to create valid user component object"));
			return CreateErrorResponse(500, TEXT("Failed to create user account"));
		}

		// 5. Save to repository
		auto CreateUserTask = AuthRepository->CreateUser(NewUserData);
		bool bUserCreated = CreateUserTask.GetResult();

		if (!bUserCreated)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthDomainService::RegisterUser: Failed to save user to repository"));
			return CreateErrorResponse(500, TEXT("Failed to create user account"));
		}

		// 6. Log audit event
		if (bEnableAuditLogging)
		{
			LogAuditEvent(NewUserData.UserId, TEXT("registration"), 
				FString::Printf(TEXT("User registered from IP: %s"), *Request.ClientIP));
		}

		UE_LOG(LogTemp, Log, TEXT("AuthDomainService::RegisterUser: Successfully registered user %s with ID %s"), 
			*Request.Username, *NewUserData.UserId);

		// Broadcast event
		OnRegistrationComplete.Broadcast(true, TEXT("User registered successfully"));

		return CreateSuccessResponse(TEXT(""), NewUserData.UserId);
	});
}

UE::Tasks::TTask<FAuthResponseDTO> UAuthDomainService::AuthenticateUser(const FAuthRequestDTO& Request)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, Request]() -> FAuthResponseDTO
	{
		UE_LOG(LogTemp, Log, TEXT("AuthDomainService::AuthenticateUser: Starting authentication for user %s"), *Request.Username);

		// 1. Validate input
		if (Request.Username.IsEmpty() || Request.Password.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("AuthDomainService::AuthenticateUser: Empty username or password"));
			return CreateErrorResponse(400, TEXT("Username and password are required"));
		}

		if (!AuthRepository.GetInterface())
		{
			return CreateErrorResponse(500, TEXT("Repository not available"));
		}

		// 2. Get user from repository
		auto GetUserTask = AuthRepository->GetUserByUsername(Request.Username);
		TOptional<FUserAccountDTO> UserDataOpt = GetUserTask.GetResult();

		if (!UserDataOpt.IsSet())
		{
			UE_LOG(LogTemp, Warning, TEXT("AuthDomainService::AuthenticateUser: User not found - %s"), *Request.Username);
			// Don't reveal whether user exists or not for security
			return CreateErrorResponse(401, TEXT("Invalid credentials"));
		}

		FUserAccountDTO UserData = UserDataOpt.GetValue();

		// 3. Create domain object and check account status
		UAuthComponent* UserComponent = UAuthComponent::CreateFromDTO(UserData);
		if (!UserComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthDomainService::AuthenticateUser: Failed to create user component object"));
			return CreateErrorResponse(500, TEXT("Internal server error"));
		}

		// 4. Check if user can login (not locked, not deleted, etc.)
		if (!UserComponent->CanLogin())
		{
			FString Reason;
			if (UserComponent->IsAccountDeleted())
			{
				Reason = TEXT("Account has been deactivated");
			}
			else if (UserComponent->IsAccountLocked())
			{
				Reason = TEXT("Account is temporarily locked");
			}
			else
			{
				Reason = TEXT("Account access denied");
			}

			UE_LOG(LogTemp, Warning, TEXT("AuthDomainService::AuthenticateUser: User cannot login - %s"), *Reason);
			LogAuditEvent(UserData.UserId, TEXT("login_denied"), Reason);
			return CreateErrorResponse(403, Reason);
		}

		// 5. Verify password
		if (!VerifyPassword(Request.Password, UserData.PasswordHash))
		{
			UE_LOG(LogTemp, Warning, TEXT("AuthDomainService::AuthenticateUser: Password verification failed for user %s"), *Request.Username);
			LogAuditEvent(UserData.UserId, TEXT("login_failed"), TEXT("Invalid password"));
			
			// Implement login attempt tracking and automatic locking
			UserData.FailedLoginAttempts++;
			
			// Lock account if too many failed attempts
			if (UserData.FailedLoginAttempts >= MaxLoginAttempts)
			{
				FDateTime LockExpiresAt = FDateTime::Now() + FTimespan::FromMinutes(LockoutDurationMinutes);
				UserComponent->LockAccount(LockExpiresAt);
				
				auto UpdateLockedUserTask = AuthRepository->UpdateUser(UserComponent->GetUserData());
				UpdateLockedUserTask.GetResult(); // Fire and forget
				
				LogAuditEvent(UserData.UserId, TEXT("account_auto_locked"), 
					FString::Printf(TEXT("Account locked after %d failed login attempts"), MaxLoginAttempts));
				
				return CreateErrorResponse(423, TEXT("Account locked due to too many failed login attempts"));
			}
			else
			{
				// Update failed attempt count
				auto UpdateFailedAttemptsTask = AuthRepository->UpdateUser(UserData);
				UpdateFailedAttemptsTask.GetResult(); // Fire and forget
			}
			
			return CreateErrorResponse(401, TEXT("Invalid credentials"));
		}

		// 6. Reset failed login attempts and update last login time
		UserData.FailedLoginAttempts = 0; // Reset failed attempts on successful login
		UserComponent->UpdateLastLogin();
		auto UpdateUserTask = AuthRepository->UpdateUser(UserComponent->GetUserData());
		bool bUserUpdated = UpdateUserTask.GetResult();

		if (!bUserUpdated)
		{
			UE_LOG(LogTemp, Warning, TEXT("AuthDomainService::AuthenticateUser: Failed to update last login time"));
			// Continue with login even if update fails
		}

		// 7. Log successful authentication
		if (bEnableAuditLogging)
		{
			LogAuditEvent(UserData.UserId, TEXT("login_success"), 
				FString::Printf(TEXT("Successful login from IP: %s"), *Request.ClientIP));
		}

		UE_LOG(LogTemp, Log, TEXT("AuthDomainService::AuthenticateUser: Successfully authenticated user %s"), *Request.Username);

		// 8. Note: JWT token generation is handled by external auth server (Node.js)
		// This domain service only validates business rules and updates user state
		// The actual token will be provided by the calling AuthSubsystem from external auth server

		// Broadcast event without token (token comes from external auth server)
		FAuthResponseDTO Response = CreateSuccessResponse(TEXT(""), UserData.UserId);
		OnAuthenticationComplete.Broadcast(true, Response);

		return Response;
	});
}

UE::Tasks::TTask<bool> UAuthDomainService::ChangeUserPassword(const FString& UserId, const FString& CurrentPassword, const FString& NewPassword)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, CurrentPassword, NewPassword]() -> bool
	{
		UE_LOG(LogTemp, Log, TEXT("AuthDomainService::ChangeUserPassword: Changing password for user %s"), *UserId);

		if (!AuthRepository.GetInterface())
		{
			return false;
		}

		// 1. Validate new password
		FString ValidationError;
		if (!ValidatePasswordComplexity(NewPassword, ValidationError))
		{
			UE_LOG(LogTemp, Warning, TEXT("AuthDomainService::ChangeUserPassword: New password validation failed - %s"), *ValidationError);
			return false;
		}

		// 2. Get current user data
		auto GetUserTask = AuthRepository->GetUserById(UserId);
		TOptional<FUserAccountDTO> UserDataOpt = GetUserTask.GetResult();

		if (!UserDataOpt.IsSet())
		{
			UE_LOG(LogTemp, Warning, TEXT("AuthDomainService::ChangeUserPassword: User not found - %s"), *UserId);
			return false;
		}

		FUserAccountDTO UserData = UserDataOpt.GetValue();

		// 3. Verify current password
		if (!VerifyPassword(CurrentPassword, UserData.PasswordHash))
		{
			UE_LOG(LogTemp, Warning, TEXT("AuthDomainService::ChangeUserPassword: Current password verification failed"));
			LogAuditEvent(UserId, TEXT("password_change_failed"), TEXT("Invalid current password"));
			return false;
		}

		// 4. Update password using domain object
		UAuthComponent* UserComponent = UAuthComponent::CreateFromDTO(UserData);
		UserComponent->ChangePassword(HashPassword(NewPassword));

		// 5. Save changes
		auto UpdateUserTask = AuthRepository->UpdateUser(UserComponent->GetUserData());
		bool bSuccess = UpdateUserTask.GetResult();

		if (bSuccess)
		{
			LogAuditEvent(UserId, TEXT("password_changed"), TEXT("Password successfully changed"));
			UE_LOG(LogTemp, Log, TEXT("AuthDomainService::ChangeUserPassword: Password changed successfully for user %s"), *UserId);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AuthDomainService::ChangeUserPassword: Failed to save password change for user %s"), *UserId);
		}

		return bSuccess;
	});
}

UE::Tasks::TTask<bool> UAuthDomainService::LockUserAccount(const FString& UserId, const FString& Reason)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, Reason]() -> bool
	{
		UE_LOG(LogTemp, Log, TEXT("AuthDomainService::LockUserAccount: Locking account for user %s, reason: %s"), *UserId, *Reason);

		if (!AuthRepository.GetInterface())
		{
			return false;
		}

		// Calculate lock expiration time
		FDateTime ExpiresAt = FDateTime::Now() + FTimespan::FromMinutes(LockoutDurationMinutes);

		// Lock user in repository
		auto LockUserTask = AuthRepository->LockUser(UserId, ExpiresAt);
		bool bSuccess = LockUserTask.GetResult();

		if (bSuccess)
		{
			LogAuditEvent(UserId, TEXT("account_locked"), 
				FString::Printf(TEXT("Account locked until %s. Reason: %s"), *ExpiresAt.ToString(), *Reason));
			UE_LOG(LogTemp, Log, TEXT("AuthDomainService::LockUserAccount: Successfully locked user %s"), *UserId);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AuthDomainService::LockUserAccount: Failed to lock user %s"), *UserId);
		}

		return bSuccess;
	});
}

UE::Tasks::TTask<bool> UAuthDomainService::UnlockUserAccount(const FString& UserId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId]() -> bool
	{
		UE_LOG(LogTemp, Log, TEXT("AuthDomainService::UnlockUserAccount: Unlocking account for user %s"), *UserId);

		if (!AuthRepository.GetInterface())
		{
			return false;
		}

		auto UnlockUserTask = AuthRepository->UnlockUser(UserId);
		bool bSuccess = UnlockUserTask.GetResult();

		if (bSuccess)
		{
			LogAuditEvent(UserId, TEXT("account_unlocked"), TEXT("Account manually unlocked"));
			UE_LOG(LogTemp, Log, TEXT("AuthDomainService::UnlockUserAccount: Successfully unlocked user %s"), *UserId);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AuthDomainService::UnlockUserAccount: Failed to unlock user %s"), *UserId);
		}

		return bSuccess;
	});
}

UE::Tasks::TTask<bool> UAuthDomainService::DeactivateUserAccount(const FString& UserId, const FString& Reason)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, Reason]() -> bool
	{
		UE_LOG(LogTemp, Log, TEXT("AuthDomainService::DeactivateUserAccount: Deactivating account for user %s, reason: %s"), *UserId, *Reason);

		if (!AuthRepository.GetInterface())
		{
			return false;
		}

		auto DeleteUserTask = AuthRepository->DeleteUser(UserId);
		bool bSuccess = DeleteUserTask.GetResult();

		if (bSuccess)
		{
			LogAuditEvent(UserId, TEXT("account_deactivated"), 
				FString::Printf(TEXT("Account deactivated. Reason: %s"), *Reason));
			UE_LOG(LogTemp, Log, TEXT("AuthDomainService::DeactivateUserAccount: Successfully deactivated user %s"), *UserId);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AuthDomainService::DeactivateUserAccount: Failed to deactivate user %s"), *UserId);
		}

		return bSuccess;
	});
}

UE::Tasks::TTask<TOptional<FUserAccountDTO>> UAuthDomainService::GetUserInfo(const FString& UserId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId]() -> TOptional<FUserAccountDTO>
	{
		if (!AuthRepository.GetInterface())
		{
			return TOptional<FUserAccountDTO>();
		}

		auto GetUserTask = AuthRepository->GetUserById(UserId);
		return GetUserTask.GetResult();
	});
}

UE::Tasks::TTask<TArray<FUserAuditLogDTO>> UAuthDomainService::GetUserAuditHistory(const FString& UserId, int32 Limit)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, Limit]() -> TArray<FUserAuditLogDTO>
	{
		if (!AuthRepository.GetInterface())
		{
			return TArray<FUserAuditLogDTO>();
		}

		auto GetAuditLogsTask = AuthRepository->GetAuditLogs(UserId, Limit);
		return GetAuditLogsTask.GetResult();
	});
}

UE::Tasks::TTask<int32> UAuthDomainService::UnlockExpiredAccounts()
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this]() -> int32
	{
		UE_LOG(LogTemp, Log, TEXT("AuthDomainService::UnlockExpiredAccounts: Starting batch unlock of expired accounts"));

		if (!AuthRepository.GetInterface())
		{
			return 0;
		}

		auto UnlockExpiredTask = AuthRepository->UnlockExpiredUsers();
		bool bSuccess = UnlockExpiredTask.GetResult();

		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("AuthDomainService::UnlockExpiredAccounts: Completed batch unlock"));
			// TODO: Return actual count of unlocked users
			return 1; // Placeholder
		}

		return 0;
	});
}

bool UAuthDomainService::ValidatePasswordComplexity(const FString& Password, FString& OutErrorMessage)
{
	bool bIsValid = UAuthComponent::IsValidPassword(Password);
	if (!bIsValid)
	{
		OutErrorMessage = TEXT("Password does not meet complexity requirements");
	}
	return bIsValid;
}

bool UAuthDomainService::ValidateUsernameFormat(const FString& Username, FString& OutErrorMessage)
{
	bool bIsValid = UAuthComponent::IsValidUsername(Username);
	if (!bIsValid)
	{
		OutErrorMessage = TEXT("Invalid username format");
	}
	return bIsValid;
}

// Private helper methods
FString UAuthDomainService::HashPassword(const FString& Password) const
{
	// Simple hash for prototype - in production, use bcrypt or similar
	return FMD5::HashAnsiString(*Password);
}

bool UAuthDomainService::VerifyPassword(const FString& Password, const FString& Hash) const
{
	// Simple verification for prototype
	return HashPassword(Password) == Hash;
}

FString UAuthDomainService::GenerateUserId() const
{
	return FGuid::NewGuid().ToString();
}

void UAuthDomainService::LogAuditEvent(const FString& UserId, const FString& Action, const FString& Detail) const
{
	if (!bEnableAuditLogging || !AuthRepository.GetInterface())
	{
		return;
	}

	FUserAuditLogDTO AuditLog;
	AuditLog.UserId = UserId;
	AuditLog.Action = Action;
	AuditLog.Detail = Detail;
	AuditLog.CreatedAt = FDateTime::Now();

	// Fire and forget audit logging
	auto AddAuditTask = AuthRepository->AddAuditLog(AuditLog);
}

FAuthResponseDTO UAuthDomainService::CreateErrorResponse(int32 ErrorCode, const FString& ErrorMessage) const
{
	FAuthResponseDTO Response;
	Response.bIsSuccess = false;
	Response.ErrorCode = ErrorCode;
	Response.ErrorMessage = ErrorMessage;
	return Response;
}

FAuthResponseDTO UAuthDomainService::CreateSuccessResponse(const FString& Token, const FString& UserId) const
{
	FAuthResponseDTO Response;
	Response.bIsSuccess = true;
	Response.Token = Token;
	Response.UserId = UserId;
	Response.ErrorCode = 0;
	Response.ErrorMessage = TEXT("");
	return Response;
}
