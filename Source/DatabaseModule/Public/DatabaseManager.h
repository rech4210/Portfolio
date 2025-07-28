// @Needmodifi
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

// Database-related delegates
DECLARE_DELEGATE_TwoParams(FDatabaseUserLoadDelegate, bool /* bSuccess */, const TOptional<FDatabaseUserData>& /* UserData */);
DECLARE_DELEGATE_OneParam(FDatabaseUserSaveDelegate, bool /* bSuccess */);
DECLARE_DELEGATE_OneParam(FDatabaseAuditLogDelegate, bool /* bSuccess */);

// Forward declaration for the implementation class (PIMPL pattern)
struct FDatabaseManagerImpl;

// ============================================================================
// DATABASE DATA TRANSFER OBJECTS
// ============================================================================

/**
 * DTO for User Account Data in Database
 * Maps to 'users' table structure
 * Note: Email field is optional for future features (password recovery, etc.)
 */
USTRUCT(BlueprintType)
struct DATABASEMODULE_API FDatabaseUserData
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserId; // CHAR(36) PRIMARY KEY - GUID format

	UPROPERTY()
	FString Username; // VARCHAR(30) UNIQUE

	UPROPERTY()
	FString PasswordHash; // VARCHAR(255)

	UPROPERTY()
	FDateTime CreatedAt; // DATETIME(3) DEFAULT CURRENT_TIMESTAMP(3)

	UPROPERTY()
	TOptional<FDateTime> LastLoginAt; // DATETIME(3) NULL

	UPROPERTY()
	bool bIsLocked; // TINYINT(1) DEFAULT 0

	UPROPERTY()
	TOptional<FDateTime> LockExpiresAt; // DATETIME(3) NULL

	UPROPERTY()
	bool bIsDeleted; // TINYINT(1) DEFAULT 0

	UPROPERTY()
	TOptional<FDateTime> DeletedAt; // DATETIME(3) NULL

	FDatabaseUserData()
		: UserId(TEXT(""))
		, Username(TEXT(""))
		, PasswordHash(TEXT(""))
		, CreatedAt(FDateTime::Now())
		, bIsLocked(false)
		, bIsDeleted(false)
	{}
};

/**
 * DTO for User Audit Log Data in Database
 * Maps to 'user_audit_logs' table structure
 */
USTRUCT(BlueprintType)
struct DATABASEMODULE_API FDatabaseAuditLogData
{
	GENERATED_BODY()

	UPROPERTY()
	int64 LogId; // BIGINT AUTO_INCREMENT

	UPROPERTY()
	int32 UserId; // INT - Foreign Key to users.user_id

	UPROPERTY()
	FString Action; // VARCHAR(50) - LOGIN_SUCCESS, LOGIN_FAILED, etc.

	UPROPERTY()
	FString Details; // TEXT - JSON details about the action

	UPROPERTY()
	FString IpAddress; // VARCHAR(45) - IPv4/IPv6 address

	UPROPERTY()
	FDateTime CreatedAt; // DATETIME DEFAULT CURRENT_TIMESTAMP

	FDatabaseAuditLogData()
		: LogId(0)
		, UserId(0)
	{}
};

// ============================================================================
// EXISTING INVENTORY DTOs
// ============================================================================


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
	int32 SlotIndex; // ?∏Î≤§?†Î¶¨ ?¨Î°Ø ?ÑÏπò

	UPROPERTY()
	FString ItemData; // JSON serialized data

	FInventoryItemDTO()
		: Quantity(0), SlotIndex(-1)
	{}

	FInventoryItemDTO(const FName& InItemID, int32 InQuantity, int32 InSlotIndex, const FString& InItemData)
		: ItemID(InItemID), Quantity(InQuantity), SlotIndex(InSlotIndex), ItemData(InItemData)
	{}
};


USTRUCT(BlueprintType)
struct DATABASEMODULE_API FCharacterData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	FString UserId; // VARCHAR(255) in database - supports UUID, external auth IDs

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	FString CharacterId; // VARCHAR(255) in database - matches schema

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	FString CharacterName; // VARCHAR(100) in database - character display name

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	int32 Level;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	int64 Exp; // BIGINT in database - supports large experience values

	// JSONB data from the database, can be parsed into another USTRUCT if needed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	FString JsonData; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
	TArray<FInventoryItemDTO> Inventory;

	FCharacterData()
		: UserId(TEXT("")), CharacterId(TEXT("")), CharacterName(TEXT("")), Level(1), Exp(0)
	{}
};

// ===============================================================================
// DTO LAYER - ?úÏàò ?∞Ïù¥??Íµ¨Ï°∞ (SQL ?§ÌÇ§Îß?Î∞òÏòÅ)
// ===============================================================================

// SQL Í∏∞Î∞ò Skill Slot DTO (user_skill_slots ?åÏù¥Î∏?Íµ¨Ï°∞)
USTRUCT(BlueprintType)
struct DATABASEMODULE_API FSkillSlotDatabaseDTO
{
	GENERATED_BODY()

	// Primary Keys (SQL user_skill_slots ?åÏù¥Î∏?Í∏∞Ï?)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FString UserId;                    // user_skill_slots.user_id (CHAR(36) UUID)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FString SlotKey;                   // user_skill_slots.slot_key (Q, W, E, R ??

	// Skill Binding
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	int32 SkillId = 0;                 // user_skill_slots.skill_id

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	int32 SlotIndex = -1;              // user_skill_slots.slot_index

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	int32 SkillLevel = 1;              // user_skills.skill_level (?ïÍ∑ú?îÎêú ?åÏù¥Î∏îÏóê??

	// Cooldown Data (ÏµúÏ†Å?îÎêú Íµ¨Ï°∞ - last_used_timeÎß??Ä??
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FDateTime LastUsedTime;            // user_skill_slots.last_used_time

	// Metadata
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FDateTime CreatedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FDateTime UpdatedAt;

	FSkillSlotDatabaseDTO()
		: UserId(TEXT("")), SkillId(0), SlotIndex(-1), SkillLevel(1)
	{
		LastUsedTime = FDateTime::MinValue();
		CreatedAt = FDateTime::Now();
		UpdatedAt = FDateTime::Now();
	}

	bool IsValid() const
	{
		return !UserId.IsEmpty() && !SlotKey.IsEmpty() && SlotIndex >= 0;
	}
};

// SQL Í∏∞Î∞ò Skill Master DTO (skills ?åÏù¥Î∏?Íµ¨Ï°∞)
USTRUCT(BlueprintType)
struct DATABASEMODULE_API FSkillMasterDatabaseDTO
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	int32 SkillId;                     // skills.skill_id

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FString DisplayName;               // skills.display_name

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	FString Description;               // skills.description

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	float BaseCooltime = 0.0f;         // skills.base_cooltime

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	float BaseCost = 0.0f;             // skills.base_cost

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	int32 MaxLevel = 1;                // skills.max_level

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillDTO")
	bool bEnabled = true;              // skills.enabled

	FSkillMasterDatabaseDTO()
		: SkillId(0), BaseCooltime(0.0f), BaseCost(0.0f), MaxLevel(1), bEnabled(true)
	{
	}
};

// Í∏∞Ï°¥ DTO (?òÏúÑ ?∏Ìôò???†Ï?)
USTRUCT(BlueprintType)
struct DATABASEMODULE_API FSkillSlotDTO
{
	GENERATED_BODY()

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
		LastUsedTime = FDateTime::Now();
	}

	FSkillSlotDTO(int32 InSkillID, int32 InSlotIndex)
		: SkillID(InSkillID), SlotIndex(InSlotIndex), RemainingCooldown(0.0f), bIsActive(true)
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

// ID Î≥Ä???ÑÎûµ ?¨Ìçº ?¥Îûò??- ?ÑÎ°ú?†Ì??ÖÏö© Í∞ÑÎã® Î≥Ä??
UCLASS()
class DATABASEMODULE_API UPlayerIdHelper : public UObject
{
	GENERATED_BODY()

public:
	// int32 PlayerIdÎ•?VARCHAR UserIdÎ°?Î≥Ä??(?ÑÎ°ú?†Ì??ÖÏö© ?®Ïàú Î≥Ä??
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static FString ConvertPlayerIdToUserId(int32 PlayerId);
	
	// VARCHAR UserIdÎ•?int32 PlayerIdÎ°?Î≥Ä??(Í∞Ä?•Ìïú Í≤ΩÏö∞)
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static int32 ConvertUserIdToPlayerId(const FString& UserId);
	
	// PlayerId Í∏∞Î∞ò UserId ?ùÏÑ± (?ëÎëê??+ ?úÌÄÄ??Ï°∞Ìï©)
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static FString GenerateUserIdFromPlayerId(int32 PlayerId, const FString& Prefix = TEXT("player"));
	
	// ?†Ìö®??UserId ?ïÏãù?∏Ï? Í≤ÄÏ¶?
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static bool IsValidUserId(const FString& UserId);
};

// JSON ?†Ìã∏Î¶¨Ìã∞ ?¥Îûò??- DatabaseManager?êÏÑú JSON Ï≤òÎ¶¨Î•??ÑÌïú ?¨Ìçº
UCLASS()
class DATABASEMODULE_API UDatabaseJsonHelper : public UObject
{
	GENERATED_BODY()

public:
	// ?∏Î≤§?†Î¶¨ ?ÑÏù¥???∞Ïù¥?∞Î? JSON?ºÎ°ú ÏßÅÎ†¨??
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeInventoryItemData(const TMap<FString, FString>& ItemProperties);
	
	// JSON?êÏÑú ?∏Î≤§?†Î¶¨ ?ÑÏù¥???∞Ïù¥?∞Î? ??ßÅ?¨Ìôî
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static TMap<FString, FString> DeserializeInventoryItemData(const FString& JsonData);
	
	// Ï∫êÎ¶≠???ïÏû• ?∞Ïù¥?∞Î? JSON?ºÎ°ú ÏßÅÎ†¨??(Position, Health, Mana ??
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeCharacterExtendedData(const FVector& Position, float Health, float Mana, const TMap<FString, FString>& AdditionalData);
	
	// JSON?êÏÑú Ï∫êÎ¶≠???ïÏû• ?∞Ïù¥?∞Î? ??ßÅ?¨Ìôî
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static bool DeserializeCharacterExtendedData(const FString& JsonData, FVector& OutPosition, float& OutHealth, float& OutMana, TMap<FString, FString>& OutAdditionalData);
	
	// ?§ÌÇ¨ ?∞Ïù¥?∞Î? JSON?ºÎ°ú ÏßÅÎ†¨??
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeSkillData(const TMap<FString, FString>& SkillProperties);
	
	// JSON?êÏÑú ?§ÌÇ¨ ?∞Ïù¥?∞Î? ??ßÅ?¨Ìôî
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static TMap<FString, FString> DeserializeSkillData(const FString& JsonData);
	
	// ?•ÎπÑ Í∞ïÌôî ?∞Ïù¥?∞Î? JSON?ºÎ°ú ÏßÅÎ†¨??
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeEquipmentEnhancement(int32 EnhancementLevel, const TArray<FString>& EnhancementEffects);
	
	// JSON?êÏÑú ?•ÎπÑ Í∞ïÌôî ?∞Ïù¥?∞Î? ??ßÅ?¨Ìôî
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

	// ========================================================================
	// SKILL MANAGEMENT METHODS
	// ========================================================================

	/**
	 * Load skill slots for a specific player
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @return Task that returns loaded skill slots
	 */
	UE::Tasks::TTask<TArray<FSkillSlotDTO>> LoadSkillsForPlayer(const FString& UserId);

	/**
	 * Save skill slots for a specific player
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param SkillSlots The skill slots to save
	 * @return Task that completes when save finishes
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead")
	UE::Tasks::TTask<bool> SaveSkillsForPlayer(const FString& UserId, const TArray<FSkillSlotDTO>& SkillSlots);

	/**
	 * Register a single skill to player's skill slots
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param SkillSlot The skill slot to register
	 * @return Task that completes when skill is registered
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: SaveUserSkillSlots() instead")
	UE::Tasks::TTask<bool> RegisterSkill(const FString& UserId, const FSkillSlotDTO& SkillSlot);

	/**
	 * Unregister a skill from player's skill slots (legacy GUID version)
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param SlotId The slot ID to unregister
	 * @return Task that completes when skill is unregistered
	 */
	UE_DEPRECATED(5.0, "Legacy GUID-based method. Use 3-Layer Mapping Architecture: ClearUserSkillSlots() instead")
	UE::Tasks::TTask<bool> UnregisterSkill(const FString& UserId, const FGuid& SlotId);

	/**
	 * Unregister a skill from player's skill slots (SlotIndex version)
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param SlotIndex The slot index to unregister
	 * @return Task that completes when skill is unregistered
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: ClearUserSkillSlots() instead")
	UE::Tasks::TTask<bool> UnregisterSkill(const FString& UserId, int32 SlotIndex);

	/**
	 * Update skill cooldown state (legacy GUID version)
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param SlotId The slot ID to update
	 * @param LastUsedTime When the skill was last used
	 * @param RemainingCooldown Remaining cooldown time
	 * @return Task that completes when cooldown is updated
	 */
	UE_DEPRECATED(5.0, "Legacy GUID-based method. Use 3-Layer Mapping Architecture: UpdateSkillSlotCooldown() instead")
	UE::Tasks::TTask<bool> UpdateSkillCooldown(const FString& UserId, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown);

	/**
	 * Update skill cooldown state (SlotIndex version)
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param SlotIndex The slot index to update
	 * @param LastUsedTime When the skill was last used
	 * @param RemainingCooldown Remaining cooldown time
	 * @return Task that completes when cooldown is updated
	 */
	UE_DEPRECATED(5.0, "Use 3-Layer Mapping Architecture: UpdateSkillSlotCooldown() instead")
	UE::Tasks::TTask<bool> UpdateSkillCooldown(const FString& UserId, int32 SlotIndex, const FDateTime& LastUsedTime, float RemainingCooldown);

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

	/**
	 * DEPRECATED: Create a new user account in the database
	 * @deprecated Use external auth service (Node.js) for user account creation
	 * This method violates separation of concerns in microservice architecture
	 */
	UE_DEPRECATED(5.0, "User account creation should be handled by external auth service")
	UE::Tasks::TTask<bool> CreateUserAccount(const FString& Username, const FString& PasswordHash, const FString& Email, FString& OutUserId);

	/**
	 * DEPRECATED: Get user account by username
	 * @deprecated Use external auth service for user account queries
	 * Game server should only query by verified user ID from JWT token
	 */
	UE_DEPRECATED(5.0, "User account queries should be handled by external auth service")
	UE::Tasks::TTask<TOptional<FDatabaseUserData>> GetUserByUsername(const FString& Username);

	/**
	 * DEPRECATED: Get user account by user ID
	 * @deprecated Use external auth service for user account queries
	 * Game server should only work with verified user IDs from JWT tokens
	 */
	UE_DEPRECATED(5.0, "User account queries should be handled by external auth service")
	UE::Tasks::TTask<TOptional<FDatabaseUserData>> GetUserById(const FString& UserId);

	/**
	 * DEPRECATED: Update user account information
	 * @deprecated Use external auth service for user account updates
	 */
	UE_DEPRECATED(5.0, "User account updates should be handled by external auth service")
	UE::Tasks::TTask<bool> UpdateUserAccount(const FDatabaseUserData& UserData);

	/**
	 * @param ExpiresAt When the lock should expire
	 * @return Task that completes when user is locked
	 */
	
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
	UE::Tasks::TTask<int32> UnlockExpiredUsers();

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
	// Pointer to the implementation
	FDatabaseManagerImpl* Impl;
	
	void LogToExternalServer(const FString& Message);
};