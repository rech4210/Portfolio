// @Needmodifi
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/AuthDTO.h"
#include "AuthComponent.generated.h"

/**
 * Auth Component (formerly Domain Aggregate)
 * Contains core business logic for user authentication and account management
 */
UCLASS(BlueprintType)
class AUTHMODULE_API UAuthComponent : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FUserAccountDTO UserData;

	UPROPERTY()
	TArray<FUserAuditLogDTO> AuditLogs;

public:
	UAuthComponent();

	// Factory Methods
	UFUNCTION(BlueprintCallable, Category = "Auth Component")
	static UAuthComponent* CreateNewUser(const FString& Username, const FString& PasswordHash);

	UFUNCTION(BlueprintCallable, Category = "Auth Component")
	static UAuthComponent* CreateFromDTO(const FUserAccountDTO& UserDTO);

	// Core Business Logic
	UFUNCTION(BlueprintCallable, Category = "Auth Component")
	bool CanLogin() const;

	UFUNCTION(BlueprintCallable, Category = "Auth Component")
	bool IsAccountLocked() const;

	UFUNCTION(BlueprintCallable, Category = "Auth Component")
	bool IsAccountDeleted() const;

	UFUNCTION(BlueprintCallable, Category = "Auth Component")
	void LockAccount(const FDateTime& ExpiresAt);

	UFUNCTION(BlueprintCallable, Category = "Auth Component")
	void UnlockAccount();

	UFUNCTION(BlueprintCallable, Category = "Auth Component")
	void UpdateLastLogin();

	UFUNCTION(BlueprintCallable, Category = "Auth Component")
	void ChangePassword(const FString& NewPasswordHash);

	UFUNCTION(BlueprintCallable, Category = "Auth Component")
	void SoftDelete();

	UFUNCTION(BlueprintCallable, Category = "Auth Component")
	void AddAuditLog(const FString& Action, const FString& Detail);

	// Getters
	UFUNCTION(BlueprintPure, Category = "Auth Component")
	const FUserAccountDTO& GetUserData() const { return UserData; }

	UFUNCTION(BlueprintPure, Category = "Auth Component")
	FString GetUserId() const { return UserData.UserId; }

	UFUNCTION(BlueprintPure, Category = "Auth Component")
	FString GetUsername() const { return UserData.Username; }

	UFUNCTION(BlueprintPure, Category = "Auth Component")
	const TArray<FUserAuditLogDTO>& GetAuditLogs() const { return AuditLogs; }

	// Validation
	UFUNCTION(BlueprintPure, Category = "Auth Component")
	bool IsValid() const;

	UFUNCTION(BlueprintPure, Category = "Auth Component")
	static bool IsValidUsername(const FString& Username);

	UFUNCTION(BlueprintPure, Category = "Auth Component")
	static bool IsValidPassword(const FString& Password);
};
