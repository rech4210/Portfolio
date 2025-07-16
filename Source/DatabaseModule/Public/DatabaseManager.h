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
	int32 UserId; // INT AUTO_INCREMENT PRIMARY KEY

	UPROPERTY()
	FString Username; // VARCHAR(50) UNIQUE

	UPROPERTY()
	FString PasswordHash; // VARCHAR(255)

	UPROPERTY()
	FString Email; // VARCHAR(100) NULLABLE - for future email verification

	UPROPERTY()
	FDateTime CreatedAt; // DATETIME DEFAULT CURRENT_TIMESTAMP

	UPROPERTY()
	TOptional<FDateTime> LastLogin; // DATETIME NULLABLE

	UPROPERTY()
	bool bIsActive; // BOOLEAN DEFAULT TRUE

	UPROPERTY()
	int32 FailedLoginAttempts; // INT DEFAULT 0

	UPROPERTY()
	TOptional<FDateTime> AccountLockedUntil; // DATETIME NULLABLE

	FDatabaseUserData()
		: UserId(0)
		, bIsActive(true)
		, FailedLoginAttempts(0)
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
	int32 SlotIndex; // 인벤토리 슬롯 위치

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

// ID 변환 전략 헬퍼 클래스 - 프로토타입용 간단 변환
UCLASS()
class DATABASEMODULE_API UPlayerIdHelper : public UObject
{
	GENERATED_BODY()

public:
	// int32 PlayerId를 VARCHAR UserId로 변환 (프로토타입용 단순 변환)
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static FString ConvertPlayerIdToUserId(int32 PlayerId);
	
	// VARCHAR UserId를 int32 PlayerId로 변환 (가능한 경우)
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static int32 ConvertUserIdToPlayerId(const FString& UserId);
	
	// PlayerId 기반 UserId 생성 (접두사 + 시퀀스 조합)
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static FString GenerateUserIdFromPlayerId(int32 PlayerId, const FString& Prefix = TEXT("player"));
	
	// 유효한 UserId 형식인지 검증
	UFUNCTION(BlueprintCallable, Category = "Database|ID")
	static bool IsValidUserId(const FString& UserId);
};

// JSON 유틸리티 클래스 - DatabaseManager에서 JSON 처리를 위한 헬퍼
UCLASS()
class DATABASEMODULE_API UDatabaseJsonHelper : public UObject
{
	GENERATED_BODY()

public:
	// 인벤토리 아이템 데이터를 JSON으로 직렬화
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeInventoryItemData(const TMap<FString, FString>& ItemProperties);
	
	// JSON에서 인벤토리 아이템 데이터를 역직렬화
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static TMap<FString, FString> DeserializeInventoryItemData(const FString& JsonData);
	
	// 캐릭터 확장 데이터를 JSON으로 직렬화 (Position, Health, Mana 등)
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeCharacterExtendedData(const FVector& Position, float Health, float Mana, const TMap<FString, FString>& AdditionalData);
	
	// JSON에서 캐릭터 확장 데이터를 역직렬화
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static bool DeserializeCharacterExtendedData(const FString& JsonData, FVector& OutPosition, float& OutHealth, float& OutMana, TMap<FString, FString>& OutAdditionalData);
	
	// 스킬 데이터를 JSON으로 직렬화
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeSkillData(const TMap<FString, FString>& SkillProperties);
	
	// JSON에서 스킬 데이터를 역직렬화
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static TMap<FString, FString> DeserializeSkillData(const FString& JsonData);
	
	// 장비 강화 데이터를 JSON으로 직렬화
	UFUNCTION(BlueprintCallable, Category = "Database|JSON")
	static FString SerializeEquipmentEnhancement(int32 EnhancementLevel, const TArray<FString>& EnhancementEffects);
	
	// JSON에서 장비 강화 데이터를 역직렬화
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
	UE::Tasks::TTask<bool> SaveSkillsForPlayer(const FString& UserId, const TArray<FSkillSlotDTO>& SkillSlots);

	/**
	 * Register a single skill to player's skill slots
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param SkillSlot The skill slot to register
	 * @return Task that completes when skill is registered
	 */
	UE::Tasks::TTask<bool> RegisterSkill(const FString& UserId, const FSkillSlotDTO& SkillSlot);

	/**
	 * Unregister a skill from player's skill slots
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param SlotId The slot ID to unregister
	 * @return Task that completes when skill is unregistered
	 */
	UE::Tasks::TTask<bool> UnregisterSkill(const FString& UserId, const FGuid& SlotId);

	/**
	 * Update skill cooldown state
	 * @param UserId The player's unique ID (VARCHAR(255) in database)
	 * @param SlotId The slot ID to update
	 * @param LastUsedTime When the skill was last used
	 * @param RemainingCooldown Remaining cooldown time
	 * @return Task that completes when cooldown is updated
	 */
	UE::Tasks::TTask<bool> UpdateSkillCooldown(const FString& UserId, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown);

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
	// USER AUTHENTICATION METHODS
	// ============================================================================

	/**
	 * Create a new user account in the database
	 * @param Username The username for the new account
	 * @param PasswordHash The hashed password
	 * @param Email The email address (optional, can be empty)
	 * @param OutUserId Reference to store the generated user ID
	 * @return Task that completes when user is created
	 */
	UE::Tasks::TTask<bool> CreateUserAccount(const FString& Username, const FString& PasswordHash, const FString& Email, int32& OutUserId);

	/**
	 * Get user account by username
	 * @param Username The username to search for
	 * @return Task that returns user data if found
	 */
	UE::Tasks::TTask<TOptional<FDatabaseUserData>> GetUserByUsername(const FString& Username);

	/**
	 * Get user account by user ID
	 * @param UserId The user ID to search for
	 * @return Task that returns user data if found
	 */
	UE::Tasks::TTask<TOptional<FDatabaseUserData>> GetUserById(int32 UserId);

	/**
	 * Update user account information
	 * @param UserData The user data to update
	 * @return Task that completes when user is updated
	 */
	UE::Tasks::TTask<bool> UpdateUserAccount(const FDatabaseUserData& UserData);

	/**
	 * Soft delete user account (mark as inactive)
	 * @param UserId The user ID to delete
	 * @return Task that completes when user is deleted
	 */
	UE::Tasks::TTask<bool> DeleteUserAccount(int32 UserId);

	/**
	 * Check if a username already exists
	 * @param Username The username to check
	 * @return Task that returns true if username exists
	 */
	UE::Tasks::TTask<bool> CheckUsernameExists(const FString& Username);

	/**
	 * Lock user account with expiration time
	 * @param UserId The user ID to lock
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
	UE::Tasks::TTask<bool> AddUserAuditLog(int32 UserId, const FString& Action, const FString& Details, const FString& IpAddress);

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

private:
	// Pointer to the implementation
	FDatabaseManagerImpl* Impl;
	
	void LogToExternalServer(const FString& Message);
};