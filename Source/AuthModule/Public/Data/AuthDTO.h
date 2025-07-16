#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AuthDTO.generated.h"

/**
 * Data Transfer Object for User Account Information
 * Used for transferring user data between layers
 */
USTRUCT(BlueprintType)
struct AUTHMODULE_API FUserAccountDTO : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User")
	FString UserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User")
	FString Username;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User")
	FString PasswordHash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User")
	FDateTime CreatedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User")
	FDateTime LastLoginAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User")
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User")
	FDateTime LockExpiresAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User")
	bool bIsDeleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User")
	FDateTime DeletedAt;

	FUserAccountDTO()
	{
		UserId = TEXT("");
		Username = TEXT("");
		PasswordHash = TEXT("");
		CreatedAt = FDateTime::Now();
		LastLoginAt = FDateTime::MinValue();
		bIsLocked = false;
		LockExpiresAt = FDateTime::MinValue();
		bIsDeleted = false;
		DeletedAt = FDateTime::MinValue();
	}
};

/**
 * Data Transfer Object for Authentication Request
 */
USTRUCT(BlueprintType)
struct AUTHMODULE_API FAuthRequestDTO
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auth")
	FString Username;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auth")
	FString Password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auth")
	FString ClientIP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auth")
	FString UserAgent;

	FAuthRequestDTO()
	{
		Username = TEXT("");
		Password = TEXT("");
		ClientIP = TEXT("");
		UserAgent = TEXT("");
	}
};

/**
 * Data Transfer Object for Authentication Response
 */
USTRUCT(BlueprintType)
struct AUTHMODULE_API FAuthResponseDTO
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auth")
	bool bIsSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auth")
	FString Token;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auth")
	FString UserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auth")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auth")
	int32 ErrorCode = 0;

	FAuthResponseDTO()
	{
		bIsSuccess = false;
		Token = TEXT("");
		UserId = TEXT("");
		ErrorMessage = TEXT("");
		ErrorCode = 0;
	}
};

/**
 * Data Transfer Object for User Audit Log
 */
USTRUCT(BlueprintType)
struct AUTHMODULE_API FUserAuditLogDTO
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audit")
	int64 LogId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audit")
	FString UserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audit")
	FString Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audit")
	FString Detail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audit")
	FDateTime CreatedAt;

	FUserAuditLogDTO()
	{
		LogId = 0;
		UserId = TEXT("");
		Action = TEXT("");
		Detail = TEXT("");
		CreatedAt = FDateTime::Now();
	}
};
