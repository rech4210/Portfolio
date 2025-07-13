#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
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


USTRUCT(BlueprintType)
struct DATABASEMODULE_API FCharacterData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	int32 UserId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	int32 CharacterId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	int32 Level;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	int32 Exp;

	// JSONB data from the database, can be parsed into another USTRUCT if needed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	FString JsonData; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	TArray<FInventoryItemDTO> Inventory;

	FCharacterData()
		: UserId(0), CharacterId(0), Level(1), Exp(0)
	{}
};

// DTO for skill slots
USTRUCT(BlueprintType)
struct DATABASEMODULE_API FSkillSlotDTO
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid SlotId;

	UPROPERTY()
	int32 SkillID;

	UPROPERTY()
	int32 SlotIndex;

	UPROPERTY()
	FDateTime LastUsedTime;

	UPROPERTY()
	float RemainingCooldown;

	UPROPERTY()
	bool bIsActive;

	UPROPERTY()
	FString SkillData; // JSON serialized data

	FSkillSlotDTO()
		: SkillID(0), SlotIndex(-1), RemainingCooldown(0.0f), bIsActive(true)
	{
		SlotId = FGuid::NewGuid();
		LastUsedTime = FDateTime::Now();
	}

	FSkillSlotDTO(const FGuid& InSlotId, int32 InSkillID, int32 InSlotIndex)
		: SlotId(InSlotId), SkillID(InSkillID), SlotIndex(InSlotIndex), RemainingCooldown(0.0f), bIsActive(true)
	{
		LastUsedTime = FDateTime::Now();
	}
};

// Shop DTO structures
USTRUCT(BlueprintType)
struct DATABASEMODULE_API FShopItemDTO
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ItemID;

	UPROPERTY()
	FString ItemName;

	UPROPERTY()
	FString ItemDescription;

	UPROPERTY()
	float Price;

	UPROPERTY()
	int32 Stock;

	UPROPERTY()
	bool bIsAvailable;

	UPROPERTY()
	FString Category;

	UPROPERTY()
	int32 MaxStock;

	UPROPERTY()
	float RestockIntervalHours;

	FShopItemDTO()
		: ItemID(0), Price(0.0f), Stock(0), bIsAvailable(false), MaxStock(0), RestockIntervalHours(24.0f)
	{}

	FShopItemDTO(int32 InItemID, const FString& InItemName, const FString& InItemDescription, 
				 float InPrice, int32 InStock, bool InIsAvailable, const FString& InCategory, int32 InMaxStock)
		: ItemID(InItemID), ItemName(InItemName), ItemDescription(InItemDescription)
		, Price(InPrice), Stock(InStock), bIsAvailable(InIsAvailable)
		, Category(InCategory), MaxStock(InMaxStock), RestockIntervalHours(24.0f)
	{}

	// Legacy constructor for backward compatibility
	FShopItemDTO(int32 InItemID, int32 InStock, float InPrice, int32 InShopID)
		: ItemID(InItemID), Price(InPrice), Stock(InStock), bIsAvailable(InStock > 0), MaxStock(InStock)
	{
		ItemName = FString::Printf(TEXT("Item %d"), InItemID);
		ItemDescription = TEXT("Legacy item");
		Category = TEXT("General");
	}
};

USTRUCT(BlueprintType)
struct DATABASEMODULE_API FShopDomain
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ShopID;

	UPROPERTY()
	FString ShopName;

	UPROPERTY()
	FString ShopDescription;

	UPROPERTY()
	bool bIsOpen;

	UPROPERTY()
	int32 AreaID;

	UPROPERTY()
	FVector ShopLocation;

	UPROPERTY()
	FDateTime LastRestockTime;

	UPROPERTY()
	float GlobalPriceModifier;

	UPROPERTY()
	FString ShopOwnerName;

	UPROPERTY()
	TArray<FShopItemDTO> ShopItems;

	FShopDomain()
		: ShopID(0), bIsOpen(false), AreaID(0), ShopLocation(FVector::ZeroVector)
	{}
};

USTRUCT(BlueprintType)
struct DATABASEMODULE_API FShopRepositoryResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bSuccess;

	UPROPERTY()
	FString ErrorMessage;

	UPROPERTY()
	FShopDomain ShopData;

	FShopRepositoryResult()
		: bSuccess(false)
	{}

	FShopRepositoryResult(bool InSuccess, const FString& InErrorMessage, const FShopDomain& InShopData)
		: bSuccess(InSuccess), ErrorMessage(InErrorMessage), ShopData(InShopData)
	{}

	static FShopRepositoryResult Success(const FShopDomain& InShopData)
	{
		FShopRepositoryResult Result;
		Result.bSuccess = true;
		Result.ShopData = InShopData;
		return Result;
	}

	static FShopRepositoryResult Failure(const FString& InErrorMessage)
	{
		FShopRepositoryResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = InErrorMessage;
		return Result;
	}
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

	// ========================================================================
	// SKILL MANAGEMENT METHODS
	// ========================================================================

	/**
	 * Load skill slots for a specific player
	 * @param PlayerId The player's unique ID
	 * @return Task that returns loaded skill slots
	 */
	UE::Tasks::TTask<TArray<FSkillSlotDTO>> LoadSkillsForPlayer(int32 PlayerId);

	/**
	 * Save skill slots for a specific player
	 * @param PlayerId The player's unique ID
	 * @param SkillSlots The skill slots to save
	 * @return Task that completes when save finishes
	 */
	UE::Tasks::TTask<bool> SaveSkillsForPlayer(int32 PlayerId, const TArray<FSkillSlotDTO>& SkillSlots);

	/**
	 * Register a single skill to player's skill slots
	 * @param PlayerId The player's unique ID
	 * @param SkillSlot The skill slot to register
	 * @return Task that completes when skill is registered
	 */
	UE::Tasks::TTask<bool> RegisterSkill(int32 PlayerId, const FSkillSlotDTO& SkillSlot);

	/**
	 * Unregister a skill from player's skill slots
	 * @param PlayerId The player's unique ID
	 * @param SlotId The slot ID to unregister
	 * @return Task that completes when skill is unregistered
	 */
	UE::Tasks::TTask<bool> UnregisterSkill(int32 PlayerId, const FGuid& SlotId);

	/**
	 * Update skill cooldown state
	 * @param PlayerId The player's unique ID
	 * @param SlotId The slot ID to update
	 * @param LastUsedTime When the skill was last used
	 * @param RemainingCooldown Remaining cooldown time
	 * @return Task that completes when cooldown is updated
	 */
	UE::Tasks::TTask<bool> UpdateSkillCooldown(int32 PlayerId, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown);

	// ============================================================================
	// Shop Data Management Methods
	// ============================================================================

	/**
	 * Load shop data by shop ID
	 * @param ShopID The shop identifier
	 * @return Task that completes with shop data
	 */
	UE::Tasks::TTask<FShopRepositoryResult> LoadShopByID(int32 ShopID);

	/**
	 * Save shop data
	 * @param ShopData The shop domain object to save
	 * @return Task that completes when save operation finishes
	 */
	UE::Tasks::TTask<bool> SaveShop(const FShopDomain& ShopData);

	/**
	 * Load multiple shops by IDs
	 * @param ShopIDs Array of shop identifiers
	 * @return Task that completes with array of shop data
	 */
	UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsByIDs(const TArray<int32>& ShopIDs);

	/**
	 * Load all shops for a specific area/region
	 * @param AreaID The area identifier
	 * @return Task that completes with array of shop data
	 */
	UE::Tasks::TTask<TArray<FShopRepositoryResult>> LoadShopsForArea(int32 AreaID);

	/**
	 * Delete a shop by ID
	 * @param ShopID The shop identifier
	 * @return Task that completes when delete operation finishes
	 */
	UE::Tasks::TTask<bool> DeleteShop(int32 ShopID);

	/**
	 * Check if a shop exists
	 * @param ShopID The shop identifier
	 * @return Task that completes with existence check result
	 */
	UE::Tasks::TTask<bool> CheckShopExists(int32 ShopID);

	/**
	 * Add item to shop
	 * @param ShopID The shop identifier
	 * @param ItemData The item data to add
	 * @return Task that completes when item is added
	 */
	UE::Tasks::TTask<bool> AddItemToShop(int32 ShopID, const struct FShopItemDTO& ItemData);

	/**
	 * Remove item from shop
	 * @param ShopID The shop identifier
	 * @param ItemID The item to remove
	 * @return Task that completes when item is removed
	 */
	UE::Tasks::TTask<bool> RemoveItemFromShop(int32 ShopID, int32 ItemID);

	/**
	 * Update item stock in shop
	 * @param ShopID The shop identifier
	 * @param ItemID The item to update
	 * @param NewStock The new stock amount
	 * @return Task that completes when stock is updated
	 */
	UE::Tasks::TTask<bool> UpdateShopItemStock(int32 ShopID, int32 ItemID, int32 NewStock);

	/**
	 * Update item price in shop
	 * @param ShopID The shop identifier
	 * @param ItemID The item to update
	 * @param NewPrice The new price
	 * @return Task that completes when price is updated
	 */
	UE::Tasks::TTask<bool> UpdateShopItemPrice(int32 ShopID, int32 ItemID, float NewPrice);

private:
	// Pointer to the implementation
	FDatabaseManagerImpl* Impl;
	
	void LogToExternalServer(const FString& Message);
};