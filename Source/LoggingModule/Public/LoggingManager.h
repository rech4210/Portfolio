#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LoggingManager.generated.h"

UCLASS()
class LOGGINGMODULE_API ULoggingManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void LogItemPurchase(const FString& PlayerID, const FString& ItemID, int32 Quantity, int32 Cost);

	// TODO: Add more logging functions as needed (e.g., LogSkillUsed, LogPlayerLogin, etc.)
}; 