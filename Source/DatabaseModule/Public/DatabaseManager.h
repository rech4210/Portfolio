#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tasks/Task.h"
#include "GameSharedModule/Public/DTO/SkillDTOs.h" // Extracted skill DTOs
#include "GameSharedModule/Public/DTO/ShopDTOs.h" // Extracted shop DTOs
#include "GameSharedModule/Public/DTO/AuthDTOs.h" // Extracted auth DTOs
#include "GameSharedModule/Public/DTO/InventoryDTOs.h" // Extracted inventory DTO
#include "DatabaseManager.generated.h"

DECLARE_DELEGATE_OneParam(FCharacterDataLoadDelegate, const TOptional<FCharacterData>& /* CharacterData */);
DECLARE_DELEGATE_OneParam(FCharacterDataSaveDelegate, bool /* bSuccess */);
DECLARE_DELEGATE_OneParam(FInventoryDataLoadDelegate, bool /* bSuccess */);
DECLARE_DELEGATE_OneParam(FInventoryDataSaveDelegate, bool /* bSuccess */);

DECLARE_DELEGATE_TwoParams(FDatabaseUserLoadDelegate, bool /* bSuccess */, const TOptional<FDatabaseUserData>& /* UserData */);
DECLARE_DELEGATE_OneParam(FDatabaseUserSaveDelegate, bool /* bSuccess */);
DECLARE_DELEGATE_OneParam(FDatabaseAuditLogDelegate, bool /* bSuccess */);

// Forward declaration for the implementation class (PIMPL pattern)
struct FDatabaseManagerImpl;

// ============================================================================
// DATABASE DATA TRANSFER OBJECTS
// ============================================================================




USTRUCT(BlueprintType)
struct DATABASEMODULE_API FCharacterData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	FString UserId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	FString CharacterId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	FString CharacterName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	int32 Level;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	int64 Exp;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	FString JsonData; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	TArray<FInventoryItemDTO> Inventory;

	FCharacterData()
		: UserId(TEXT("")), CharacterId(TEXT("")), CharacterName(TEXT("")), Level(1), Exp(0)
	{}
};

// ===============================================================================
// DTO LAYER - 
// ===============================================================================




UCLASS()
class DATABASEMODULE_API UPlayerIdHelper : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static FString ConvertPlayerIdToUserId(int32 PlayerId);
	
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static int32 ConvertUserIdToPlayerId(const FString& UserId);
	
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static FString GenerateUserIdFromPlayerId(int32 PlayerId, const FString& Prefix = TEXT("player"));
	
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static bool IsValidUserId(const FString& UserId);
};

UCLASS()
class DATABASEMODULE_API UDatabaseJsonHelper : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeInventoryItemData(const TMap<FString, FString>& ItemProperties);
	
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static TMap<FString, FString> DeserializeInventoryItemData(const FString& JsonData);
	
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeCharacterExtendedData(const FVector& Position, float Health, float Mana, const TMap<FString, FString>& AdditionalData);
	
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static bool DeserializeCharacterExtendedData(const FString& JsonData, FVector& OutPosition, float& OutHealth, float& OutMana, TMap<FString, FString>& OutAdditionalData);
	
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeSkillData(const TMap<FString, FString>& SkillProperties);
	
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static TMap<FString, FString> DeserializeSkillData(const FString& JsonData);
	
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeEquipmentEnhancement(int32 EnhancementLevel, const TArray<FString>& EnhancementEffects);
	
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static bool DeserializeEquipmentEnhancement(const FString& JsonData, int32& OutEnhancementLevel, TArray<FString>& OutEnhancementEffects);
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
	 * @param UserId The ID of the user whose character to load (VARCHAR(255) in database).
	 * @param Delegate The delegate to call upon completion.
	 */
	void LoadCharacterInfo(const FString& UserId, FCharacterDataLoadDelegate Delegate);

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
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param Delegate Callback with loaded inventory data
	 */
	UE::Tasks::TTask<TArray<FInventoryItemDTO>> LoadInventoryForPlayer(const FString& UserId);

	/**
	 * Save inventory items for a specific player
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param Items The inventory items to save
	 * @return Task that completes when save finishes
	 */
	UE::Tasks::TTask<bool> SaveInventoryForPlayer(const FString& UserId, const TArray<FInventoryItemDTO>& Items);

	/**
	 * Add a single item to player's inventory
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param Item The item to add
	 * @return Task that completes when item is added
	 */
	UE::Tasks::TTask<bool> AddInventoryItem(const FString& UserId, const FInventoryItemDTO& Item);

	/**
	 * Remove a single item from player's inventory
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param ItemID The item ID to remove
	 * @param Quantity How many to remove
	 * @return Task that completes when item is removed
	 */
	UE::Tasks::TTask<bool> RemoveInventoryItem(const FString& UserId, const FName& ItemID, int32 Quantity);

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

	// ============================================================================
	// USER AUTHENTICATION METHODS - DEPRECATED
	// ============================================================================
	// NOTE: These methods are DEPRECATED and should not be used in production
	// Reason: User authentication should be handled by external auth service (Node.js)
	// The game server should only handle game-related data, not user account management
	// Use external auth service for: registration, login, password management, account locks
	// Game server responsibility: character data, inventory, skills, shop data only
	
	UE::Tasks::TTask<bool> LockUserAccount(int32 UserId, const FDateTime& ExpiresAt);

	/**
	 * Unlock user account manually
	 * @param UserId The user ID to unlock
	 * @return Task that completes when user is unlocked
	 */
	UE::Tasks::TTask<bool> UnlockUserAccount(const FString& UserId);

	/**
	 * Update user's last login timestamp
	 * @param UserId The user ID to update
	 * @return Task that completes when timestamp is updated
	 */
	UE::Tasks::TTask<bool> UpdateLastLogin(const FString& UserId);

	/**
	 * Get users whose account locks have expired
	 * @return Task that returns array of expired locked users
	 */
	UE::Tasks::TTask<TArray<FDatabaseUserData>> GetExpiredLockedUsers();

	/**
	 * Unlock all users whose lock has expired (batch operation)
	 * @return Task that returns number of users unlocked
	 */
	// UE::Tasks::TTask<int32> UnlockExpiredUsers();

	// ============================================================================
	// USER AUDIT LOG METHODS
	// ============================================================================

	/**
	 * Add an audit log entry for user action
	 * @param UserId The user ID performing the action
	 * @param Action The action type (LOGIN_SUCCESS, LOGIN_FAILED, etc.)
	 * @param Details JSON details about the action
	 * @param IpAddress The IP address of the user
	 * @return Task that completes when log is added
	 */
	/**
	 * Get audit logs for a specific user
	 * @param UserId The user ID to get logs for
	 * @param Limit Maximum number of logs to return (default 100)
	 * @return Task that returns array of audit logs
	 */
	UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> GetUserAuditLogs(int32 UserId, int32 Limit = 100);

	/**
	 * Get audit logs by action type
	 * @param Action The action type to filter by (e.g., "LOGIN_SUCCESS", "PASSWORD_CHANGE")
	 * @param Limit Maximum number of logs to return (default 100)
	 * @return Task that returns array of audit logs
	 */
	UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> GetAuditLogsByAction(const FString& Action, int32 Limit = 100);

	/**
	 * Get recent audit logs across all users (for admin monitoring)
	 * @param Limit Maximum number of logs to return (default 50)
	 * @return Task that returns array of audit logs
	 */
	UE::Tasks::TTask<TArray<FDatabaseAuditLogData>> GetRecentAuditLogs(int32 Limit = 50);

	// ============================================================================
	// SKILL DATABASE OPERATIONS - 3-Layer Mapping Architecture
	// ============================================================================

	/**
	 * Load user skill slots using 3-layer mapping architecture
	 * @param UserId The user ID to load skills for
	 * @param SlotKey The slot key (e.g., "ActionBar", "QuickSlot")
	 * @return Task that returns array of skill slot DTOs
	 */
	UE::Tasks::TTask<TArray<FSkillSlotDatabaseDTO>> LoadUserSkillSlots(const FString& UserId, const FString& SlotKey);

	/**
	 * Save user skill slots using 3-layer mapping architecture
	 * @param SkillSlotDTOs Array of skill slot DTOs to save
	 * @return Task that completes when all slots are saved
	 */
	UE::Tasks::TTask<bool> SaveUserSkillSlots(const TArray<FSkillSlotDatabaseDTO>& SkillSlotDTOs);

	/**
	 * Load skill master data using 3-layer mapping architecture
	 * @param SkillIds Array of skill IDs to load (empty for all skills)
	 * @return Task that returns array of skill master DTOs
	 */
	UE::Tasks::TTask<TArray<FSkillMasterDatabaseDTO>> LoadSkillMasterData(const TArray<int32>& SkillIds = TArray<int32>());

	/**
	 * Save or update skill master data
	 * @param SkillMasterDTOs Array of skill master DTOs to save
	 * @return Task that completes when all skills are saved
	 */
	UE::Tasks::TTask<bool> SaveSkillMasterData(const TArray<FSkillMasterDatabaseDTO>& SkillMasterDTOs);

	/**
	 * Update skill slot's last used time (for cooldown tracking)
	 * @param UserId The user ID
	 * @param SlotKey The slot key
	 * @param SlotIndex The slot index
	 * @param LastUsedTime The last used timestamp
	 * @return Task that completes when update is done
	 */
	UE::Tasks::TTask<bool> UpdateSkillSlotCooldown(
		const FString& UserId, 
		const FString& SlotKey, 
		int32 SlotIndex, 
		const FDateTime& LastUsedTime
	);

	/**
	 * Clear all skill slots for a user (for skill reset)
	 * @param UserId The user ID
	 * @param SlotKey The slot key to clear (empty for all slot keys)
	 * @return Task that completes when slots are cleared
	 */
	UE::Tasks::TTask<bool> ClearUserSkillSlots(const FString& UserId, const FString& SlotKey = TEXT(""));

	/**
	 * Get skill usage statistics for analytics
	 * @param UserId The user ID (0 for all users)
	 * @param SkillId The skill ID (0 for all skills)
	 * @param StartDate Start date for the query
	 * @param EndDate End date for the query
	 * @return Task that returns usage statistics
	 */
	UE::Tasks::TTask<TMap<int32, int32>> GetSkillUsageStatistics(
		const FString& UserId = TEXT(""),
		int32 SkillId = 0,
		const FDateTime& StartDate = FDateTime::MinValue(),
		const FDateTime& EndDate = FDateTime::MaxValue()
	);

private:
	FDatabaseManagerImpl* Impl;
	
	void LogToExternalServer(const FString& Message);
};