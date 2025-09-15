#pragma once

#include "CoreMinimal.h"
#include "AuthDTOs.generated.h"

USTRUCT(BlueprintType)
struct GAMESHAREDMODULE_API FDatabaseUserData {
	GENERATED_BODY()
	UPROPERTY() FString UserId;
	UPROPERTY() FString Username;
	UPROPERTY() FString PasswordHash;
	UPROPERTY() FDateTime CreatedAt;
	UPROPERTY() TOptional<FDateTime> LastLoginAt;
	UPROPERTY() bool bIsLocked;
	UPROPERTY() TOptional<FDateTime> LockExpiresAt;
	UPROPERTY() bool bIsDeleted;
	UPROPERTY() TOptional<FDateTime> DeletedAt;
	FDatabaseUserData() : UserId(TEXT("")), Username(TEXT("")), PasswordHash(TEXT("")), CreatedAt(FDateTime::Now()), bIsLocked(false), bIsDeleted(false) {}
};

USTRUCT(BlueprintType)
struct GAMESHAREDMODULE_API FDatabaseAuditLogData {
	GENERATED_BODY()
	UPROPERTY() int64 LogId;
	UPROPERTY() int32 UserId;
	UPROPERTY() FString Action;
	UPROPERTY() FString Details;
	UPROPERTY() FString IpAddress;
	UPROPERTY() FDateTime CreatedAt;
	FDatabaseAuditLogData() : LogId(0), UserId(0) {}
};
