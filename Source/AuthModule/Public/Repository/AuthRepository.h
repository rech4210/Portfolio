#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Repository/AuthRepositoryInterface.h"
#include "AuthRepository.generated.h"

UCLASS()
class AUTHMODULE_API UAuthRepository : public UObject, public IAuthRepositoryInterface
{
	GENERATED_BODY()
	UPROPERTY()
	UDatabaseManager* DatabaseManager;

public:
	UAuthRepository();

	UFUNCTION(BlueprintCallable, Category = "Auth Repository")
	void Initialize();

	virtual UE::Tasks::TTask<bool> CreateUser(const FUserAccountDTO& UserData) override;
	virtual UE::Tasks::TTask<TOptional<FUserAccountDTO>> GetUserByUsername(const FString& Username) override;
	virtual UE::Tasks::TTask<TOptional<FUserAccountDTO>> GetUserById(const FString& UserId) override;
	virtual UE::Tasks::TTask<bool> UpdateUser(const FUserAccountDTO& UserData) override;
	virtual UE::Tasks::TTask<bool> DeleteUser(const FString& UserId) override;

	virtual UE::Tasks::TTask<bool> ValidateCredentials(const FString& Username, const FString& PasswordHash) override;
	virtual UE::Tasks::TTask<bool> UpdateLastLogin(const FString& UserId) override;
	virtual UE::Tasks::TTask<bool> LockUser(const FString& UserId, const FDateTime& ExpiresAt) override;
	virtual UE::Tasks::TTask<bool> UnlockUser(const FString& UserId) override;

	virtual UE::Tasks::TTask<bool> AddAuditLog(const FUserAuditLogDTO& AuditLog) override;
	virtual UE::Tasks::TTask<TArray<FUserAuditLogDTO>> GetAuditLogs(const FString& UserId, int32 Limit = 100) override;

	virtual UE::Tasks::TTask<bool> CheckUsernameExists(const FString& Username) override;
	virtual UE::Tasks::TTask<TArray<FUserAccountDTO>> GetExpiredLockedUsers() override;
	virtual UE::Tasks::TTask<bool> UnlockExpiredUsers() override;

private:
	FUserAccountDTO ConvertDatabaseUserToDTO(const FDatabaseUserData& DatabaseUser) const;
	FUserAuditLogDTO ConvertAuditLogFromDatabaseResult(const TMap<FString, FString>& DatabaseRow) const;
	FString ConvertDateTimeToString(const FDateTime& DateTime) const;
	FDateTime ConvertStringToDateTime(const FString& DateTimeString) const;
};
