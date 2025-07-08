#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/FCharacterData.h"
#include "Tasks/Task.h"
#include "DatabaseManager.generated.h"

// Delegate for async operations
DECLARE_DELEGATE_OneParam(FCharacterDataLoadDelegate, const TOptional<FCharacterData>& /* CharacterData */);
DECLARE_DELEGATE_OneParam(FCharacterDataSaveDelegate, bool /* bSuccess */);
DECLARE_DELEGATE_OneParam(FInventoryDataLoadDelegate, bool /* bSuccess */);
DECLARE_DELEGATE_OneParam(FInventoryDataSaveDelegate, bool /* bSuccess */);

// Forward declaration for the implementation class (PIMPL pattern)
struct FDatabaseManagerImpl;

// DTO for inventory items
USTRUCT(BlueprintType)
struct DATABASEMODULE_API FInventoryItemDTO
{
	GENERATED_BODY()

	UPROPERTY()
	FName ItemID;

	UPROPERTY()
	int32 Quantity;

	UPROPERTY()
	FString ItemData; // JSON serialized data

	FInventoryItemDTO()
		: Quantity(0)
	{}

	FInventoryItemDTO(const FName& InItemID, int32 InQuantity, const FString& InItemData)
		: ItemID(InItemID), Quantity(InQuantity), ItemData(InItemData)
	{}
};

UCLASS()
class DATABASEMODULE_API UDatabaseManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDatabaseManager();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	~UDatabaseManager() override;

	/**
	 * Asynchronously loads character information from the database.
	 * @param UserId The ID of the user whose character to load.
	 * @param Delegate The delegate to call upon completion.
	 */
	void LoadCharacterInfo(int32 UserId, FCharacterDataLoadDelegate Delegate);

	/**
	 * Asynchronously saves character information to the database.
	 * @param CharacterData The character data to save.
	 * @param Delegate The delegate to call upon completion.
	 */
	void SaveCharacterInfo(const FCharacterData& CharacterData, FCharacterDataSaveDelegate Delegate);

	/**
	 * Transaction wrapper for multiple database operations.
	 * @param Function The function to execute within a transaction
	 * @return Task that completes when transaction finishes
	 */
	template<typename F>
	UE::Tasks::TTask<bool> WithTransaction(F&& Function, const TCHAR* TaskLabel = TEXT("DB/Transaction"));

	/**
	 * Load inventory items for a specific player
	 * @param PlayerId The player's unique ID
	 * @param Delegate Callback with loaded inventory data
	 */
	UE::Tasks::TTask<TArray<FInventoryItemDTO>> LoadInventoryForPlayer(int32 PlayerId);

	/**
	 * Save inventory items for a specific player
	 * @param PlayerId The player's unique ID
	 * @param Items The inventory items to save
	 * @return Task that completes when save finishes
	 */
	UE::Tasks::TTask<bool> SaveInventoryForPlayer(int32 PlayerId, const TArray<FInventoryItemDTO>& Items);

	/**
	 * Add a single item to player's inventory
	 * @param PlayerId The player's unique ID
	 * @param Item The item to add
	 * @return Task that completes when item is added
	 */
	UE::Tasks::TTask<bool> AddInventoryItem(int32 PlayerId, const FInventoryItemDTO& Item);

	/**
	 * Remove a single item from player's inventory
	 * @param PlayerId The player's unique ID
	 * @param ItemID The item ID to remove
	 * @param Quantity How many to remove
	 * @return Task that completes when item is removed
	 */
	UE::Tasks::TTask<bool> RemoveInventoryItem(int32 PlayerId, const FName& ItemID, int32 Quantity);

private:
	// Pointer to the implementation
	FDatabaseManagerImpl* Impl;
	
	void LogToExternalServer(const FString& Message);
}; 