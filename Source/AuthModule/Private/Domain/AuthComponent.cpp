#include "Domain/AuthComponent.h"
#include "Misc/Guid.h"
#include "Misc/DateTime.h"

UAuthComponent::UAuthComponent()
{
	// Initialize with default values
}

UAuthComponent* UAuthComponent::CreateNewUser(const FString& Username, const FString& PasswordHash)
{
	UAuthComponent* NewUser = NewObject<UAuthComponent>();
	
	NewUser->UserData.UserId = FGuid::NewGuid().ToString();
	NewUser->UserData.Username = Username;
	NewUser->UserData.PasswordHash = PasswordHash;
	NewUser->UserData.CreatedAt = FDateTime::Now();
	NewUser->UserData.LastLoginAt = FDateTime::MinValue();
	NewUser->UserData.bIsLocked = false;
	NewUser->UserData.LockExpiresAt = FDateTime::MinValue();
	NewUser->UserData.bIsDeleted = false;
	NewUser->UserData.DeletedAt = FDateTime::MinValue();

	// Add creation audit log
	NewUser->AddAuditLog(TEXT("registration"), TEXT("User account created"));

	return NewUser;
}

UAuthComponent* UAuthComponent::CreateFromDTO(const FUserAccountDTO& UserDTO)
{
	UAuthComponent* User = NewObject<UAuthComponent>();
	User->UserData = UserDTO;
	return User;
}

bool UAuthComponent::CanLogin() const
{
	// Cannot login if account is deleted
	if (IsAccountDeleted())
	{
		return false;
	}

	// Cannot login if account is locked and lock hasn't expired
	if (IsAccountLocked())
	{
		return false;
	}

	return true;
}

bool UAuthComponent::IsAccountLocked() const
{
	if (!UserData.bIsLocked)
	{
		return false;
	}

	// Check if lock has expired
	if (UserData.LockExpiresAt.GetTicks() > 0 && FDateTime::Now() >= UserData.LockExpiresAt)
	{
		// Lock has expired, but we don't modify state here (that's repository's job)
		return false;
	}

	return true;
}

bool UAuthComponent::IsAccountDeleted() const
{
	return UserData.bIsDeleted;
}

void UAuthComponent::LockAccount(const FDateTime& ExpiresAt)
{
	UserData.bIsLocked = true;
	UserData.LockExpiresAt = ExpiresAt;
	
	AddAuditLog(TEXT("account_locked"), 
		FString::Printf(TEXT("Account locked until %s"), *ExpiresAt.ToString()));
}

void UAuthComponent::UnlockAccount()
{
	UserData.bIsLocked = false;
	UserData.LockExpiresAt = FDateTime::MinValue();
	
	AddAuditLog(TEXT("account_unlocked"), TEXT("Account manually unlocked"));
}

void UAuthComponent::UpdateLastLogin()
{
	UserData.LastLoginAt = FDateTime::Now();
	
	// If account was locked and lock expired, automatically unlock
	if (UserData.bIsLocked && UserData.LockExpiresAt.GetTicks() > 0 && 
		FDateTime::Now() >= UserData.LockExpiresAt)
	{
		UnlockAccount();
	}
	
	AddAuditLog(TEXT("login_success"), TEXT("User login successful"));
}

void UAuthComponent::ChangePassword(const FString& NewPasswordHash)
{
	UserData.PasswordHash = NewPasswordHash;
	AddAuditLog(TEXT("password_change"), TEXT("Password changed"));
}

void UAuthComponent::SoftDelete()
{
	UserData.bIsDeleted = true;
	UserData.DeletedAt = FDateTime::Now();
	AddAuditLog(TEXT("account_deleted"), TEXT("Account soft deleted"));
}

void UAuthComponent::AddAuditLog(const FString& Action, const FString& Detail)
{
	FUserAuditLogDTO AuditLog;
	AuditLog.UserId = UserData.UserId;
	AuditLog.Action = Action;
	AuditLog.Detail = Detail;
	AuditLog.CreatedAt = FDateTime::Now();
	
	AuditLogs.Add(AuditLog);
}

bool UAuthComponent::IsValid() const
{
	return !UserData.UserId.IsEmpty() && !UserData.Username.IsEmpty() && !UserData.PasswordHash.IsEmpty();
}

bool UAuthComponent::IsValidUsername(const FString& Username)
{
	// Username validation rules
	if (Username.IsEmpty() || Username.Len() < 3 || Username.Len() > 30)
	{
		return false;
	}

	// Check for valid characters (alphanumeric and underscore only)
	for (const TCHAR& Char : Username)
	{
		if (!FChar::IsAlnum(Char) && Char != TEXT('_'))
		{
			return false;
		}
	}

	return true;
}

bool UAuthComponent::IsValidPassword(const FString& Password)
{
	// Password validation rules
	if (Password.IsEmpty() || Password.Len() < 8 || Password.Len() > 128)
	{
		return false;
	}

	bool bHasUpper = false;
	bool bHasLower = false;
	bool bHasDigit = false;
	bool bHasSpecial = false;

	for (const TCHAR& Char : Password)
	{
		if (FChar::IsUpper(Char)) bHasUpper = true;
		else if (FChar::IsLower(Char)) bHasLower = true;
		else if (FChar::IsDigit(Char)) bHasDigit = true;
		else if (!FChar::IsAlnum(Char)) bHasSpecial = true;
	}

	// Require at least 3 of the 4 character types
	int32 TypeCount = (bHasUpper ? 1 : 0) + (bHasLower ? 1 : 0) + (bHasDigit ? 1 : 0) + (bHasSpecial ? 1 : 0);
	return TypeCount >= 3;
}
