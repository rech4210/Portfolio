#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LoggingManager.generated.h"

/**
 * Manages logging game events to an external service or file.
 */
UCLASS()
class LOGGINGMODULE_API ULoggingManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// UGameInstanceSubsystem interface begin
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// UGameInstanceSubsystem interface end

	/**
	 * Logs an item purchase event.
	 * @param PlayerID The ID of the player who made the purchase.
	 * @param ItemID The ID of the item purchased.
	 * @param Quantity The quantity of the item purchased.
	 * @param Cost The cost of the purchase.
	 */
	UFUNCTION(BlueprintCallable, Category = "Logging")
	void LogItemPurchase(const FString& PlayerID, const FString& ItemID, int32 Quantity, int32 Cost);

	// TODO: Add more logging functions as needed (e.g., LogSkillUsed, LogPlayerLogin, etc.)
}; 