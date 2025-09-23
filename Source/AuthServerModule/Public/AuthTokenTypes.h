#pragma once

#include "CoreMinimal.h"
#include "AuthTokenTypes.generated.h"

USTRUCT()
struct AUTHSERVERMODULE_API FAuthTokenRecord
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserId;

	UPROPERTY()
	FString Token;

	UPROPERTY()
	FDateTime IssuedAtUtc = FDateTime::UtcNow();

	UPROPERTY()
	FDateTime ExpiresAtUtc = FDateTime::MinValue();

	// Optional: last time client activity validated with this token
	UPROPERTY()
	FDateTime LastAccessUtc = FDateTime::UtcNow();

	bool IsExpired(const FDateTime& NowUtc = FDateTime::UtcNow()) const
	{
		return ExpiresAtUtc != FDateTime::MinValue() && NowUtc >= ExpiresAtUtc;
	}
};
