#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DatabaseSettings.generated.h"

UCLASS(Config = Engine, defaultconfig)
class DATABASEMODULE_API UDatabaseSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Database Settings")
	FString DBHost;

	UPROPERTY(Config, EditAnywhere, Category = "Database Settings")
	int32 DBPort;

	UPROPERTY(Config, EditAnywhere, Category = "Database Settings")
	FString DBUser;

	UPROPERTY(Config, EditAnywhere, Category = "Database Settings")
	FString DBPassword;

	UPROPERTY(Config, EditAnywhere, Category = "Database Settings")
	FString DBSchema;
}; 