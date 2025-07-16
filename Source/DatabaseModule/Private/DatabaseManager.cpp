// 1) Windows 타입 래퍼 & 매크로 충돌 방지
/* 
 * 문제 원인:
 * - Unreal Engine은 전처리 단계에서 'check(...)' 매크로를 정의하고 사용함.
 * - MySQL Connector/C++는 전역 함수 'check(const std::string&)' 등을 정의함.
 * - 두 정의가 이름을 공유하며, include 순서에 따라 서로 다른 방식으로 해석됨.
 *
 * include 순서에 따른 차이:
 * 1) UE 헤더 먼저 포함 → 매크로 정의 → #undef check → MySQL 헤더 포함
 *    - UE 매크로가 이미 사용된 이후이므로 MySQL 함수 정의만 잠깐 가려져 충돌 없음.
 * 2) MySQL 헤더 먼저 포함 → 전역 함수 정의 → #undef check → UE 헤더 포함
 *    - UE 매크로가 아직 정의되지 않은 상태이므로 전역 함수가 남아 UE 코드의 check 호출과 충돌.
 *
 * 해결책:
 * - MySQL 헤더는 매크로가 제거된 상태에서만 포함하고,
 * - Unreal 헤더들은 MySQL 헤더 이후 'CoreMinimal.h' 등에서 매크로가 복원된 뒤에 포함해야 함.
 */
#include "DatabaseManager.h"

// 3) 언리얼 코어 헤더로 매크로 복원
#include "CoreMinimal.h"
#include "Tasks/Task.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ScopeExit.h"

#include "Data/DatabaseSettings.h"
// 4) 그 뒤에 나머지 UE4 헤더들
#include "Async/Async.h"
#include "Misc/Optional.h"
#include "HAL/PlatformProcess.h"
#include "Containers/Queue.h"
#include "HAL/CriticalSection.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#undef check
#include <mysql/jdbc.h>

#include "Windows/HideWindowsPlatformTypes.h"

// …PIMPL 구현체 정의…
// C4150: 불완전한 형식 'FDatabaseManagerImpl'에 대한 포인터를 삭제했습니다.
// -> PIMPL 구조 사용시 unique_ptr에서 소멸자 delete를 하는데, 컴파일 시점에 FDatabaseManagerImpl의 정의가 불완전하기에 해당 에러가 출력됨
// 해결 : FDatabaseManagerImpl 구조체를 완전하게 정의하거나, UniquePtr를 사용하지 않고 raw 포인터를 사용하여 소멸자를 직접 구현합니다.
struct FDatabaseManagerImpl
{
	sql::Driver* Driver = nullptr;
	sql::ConnectOptionsMap ConnectionProperties;
	
	TQueue<sql::Connection*> ConnectionPool;
	FCriticalSection PoolMutex;
	const int32 PoolSize = 5;

	~FDatabaseManagerImpl()
	{
		FScopeLock Lock(&PoolMutex);
		sql::Connection* Con;
		while (ConnectionPool.Dequeue(Con))
		{
			try
			{
				if (Con && !Con->isClosed())
				{
					Con->close();
				}
			}
			catch (const sql::SQLException& e)
			{
				UE_LOG(LogTemp, Error, TEXT("SQLException during connection close: %hs"), e.what());
			}
			delete Con;
		}
	}

	sql::Connection* GetConnection()
	{
		FScopeLock Lock(&PoolMutex);
		sql::Connection* Con = nullptr;
		if (ConnectionPool.Dequeue(Con))
		{
			try
			{
				if (Con && !Con->isClosed() && Con->isValid())
				{
					return Con;
				}
				delete Con; // old connection is invalid
			}
			catch(const sql::SQLException& e)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to validate pooled connection, creating new one. Error: %hs"), e.what());
				delete Con;
			}
		}
		
		try
		{
			return Driver->connect(ConnectionProperties);
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create new connection: %hs"), e.what());
			return nullptr;
		}
	}

	// Helper to return a connection to the pool
	void ReturnConnection(sql::Connection* Con)
	{
		if (!Con) return;

		FScopeLock Lock(&PoolMutex);
		ConnectionPool.Enqueue(Con);
	}

	// Transaction wrapper with RAII
	class FTransactionGuard
	{
	public:
		sql::Connection* Connection;
		bool bCommitted;

		FTransactionGuard(sql::Connection* Con) : Connection(Con), bCommitted(false)
		{
			if (Connection)
			{
				try
				{
					Connection->setAutoCommit(false);
				}
				catch (const sql::SQLException& e)
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to begin transaction: %hs"), e.what());
					Connection = nullptr;
				}
			}
		}

		~FTransactionGuard()
		{
			if (Connection && !bCommitted)
			{
				try
				{
					Connection->rollback();
					Connection->setAutoCommit(true);
				}
				catch (const sql::SQLException& e)
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to rollback in destructor: %hs"), e.what());
				}
			}
		}

		bool Commit()
		{
			if (Connection && !bCommitted)
			{
				try
				{
					Connection->commit();
					Connection->setAutoCommit(true);
					bCommitted = true;
					return true;
				}
				catch (const sql::SQLException& e)
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to commit transaction: %hs"), e.what());
					return false;
				}
			}
			return false;
		}
	};

	sql::Connection* BeginTransaction()
	{
		sql::Connection* Con = GetConnection();
		return Con; // FTransactionGuard will handle transaction setup
	}

	void CommitTransaction(sql::Connection* Con)
	{
		// Deprecated: Use FTransactionGuard instead
		if (Con)
		{
			try
			{
				Con->commit();
				Con->setAutoCommit(true);
			}
			catch (const sql::SQLException& e)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to commit transaction: %hs"), e.what());
				throw;
			}
		}
	}

	void RollbackTransaction(sql::Connection* Con)
	{
		// Deprecated: Use FTransactionGuard instead
		if (Con)
		{
			try
			{
				Con->rollback();
				Con->setAutoCommit(true);
			}
			catch (const sql::SQLException& e)
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to rollback transaction: %hs"), e.what());
			}
		}
	}
};

UDatabaseManager::UDatabaseManager(): Impl(new FDatabaseManagerImpl()){}
UDatabaseManager::~UDatabaseManager() {
	delete Impl;
};

void UDatabaseManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UDatabaseSettings* Settings = GetDefault<UDatabaseSettings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Fatal, TEXT("Database settings not found!"));
		return;
	}

	// [디버그 로그 추가] 어떤 설정으로 접속을 시도하는지 확인합니다.
	UE_LOG(LogTemp, Log, TEXT("Attempting DB connection with the following settings:"));
	UE_LOG(LogTemp, Log, TEXT(" - Host: %s"), *Settings->DBHost);
	UE_LOG(LogTemp, Log, TEXT(" - Port: %d"), Settings->DBPort);
	UE_LOG(LogTemp, Log, TEXT(" - User: %s"), *Settings->DBUser);
	UE_LOG(LogTemp, Log, TEXT(" - Schema: %s"), *Settings->DBSchema);
	// 경고: 보안을 위해 비밀번호는 절대 로그로 출력하지 마세요.

	try
	{
		Impl->Driver = sql::mysql::get_driver_instance();
		
		Impl->ConnectionProperties["hostName"] = TCHAR_TO_UTF8(*Settings->DBHost);
		Impl->ConnectionProperties["port"] = Settings->DBPort;
		Impl->ConnectionProperties["userName"] = TCHAR_TO_UTF8(*Settings->DBUser);
		Impl->ConnectionProperties["password"] = TCHAR_TO_UTF8(*Settings->DBPassword);
		Impl->ConnectionProperties["schema"] = TCHAR_TO_UTF8(*Settings->DBSchema);
		Impl->ConnectionProperties["OPT_RECONNECT"] = true;

		for (int32 i = 0; i < Impl->PoolSize; ++i)
		{
			//connectionPool의 초기화가 이루어졌는가?
			Impl->ConnectionPool.Enqueue(Impl->Driver->connect(Impl->ConnectionProperties));
		}
		UE_LOG(LogTemp, Log, TEXT("Database connection pool initialized with %d connections."), Impl->PoolSize);
	}
	
	catch (const sql::SQLException &e)
	{
		UE_LOG(LogTemp, Fatal, TEXT("Failed to initialize database connection: %hs"), e.what());
		delete Impl;
		Impl = nullptr;
	}
}

void UDatabaseManager::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("UDatabaseManager Deinitializing."));
	delete Impl;
	Impl = nullptr;
	Super::Deinitialize();
}

void UDatabaseManager::LoadCharacterInfo(const FString& UserId, FCharacterDataLoadDelegate Delegate)
{
	if (!Impl)
	{
		Delegate.ExecuteIfBound(TOptional<FCharacterData>());
		return;
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, UserId, Delegate]()
	{
		TOptional<FCharacterData> ResultData;
		sql::Connection* Con = Impl->GetConnection();

		if (Con)
		{
			try
			{
				TUniquePtr<sql::PreparedStatement> Pstmt(Con->prepareStatement("SELECT character_id, character_name, level, exp, json_data FROM characters WHERE user_id = ?"));
				Pstmt->setString(1, TCHAR_TO_UTF8(*UserId));

				TUniquePtr<sql::ResultSet> Res(Pstmt->executeQuery());
				if (Res->next())
				{
					FCharacterData LoadedData;
					LoadedData.UserId = UserId;
					LoadedData.CharacterId = UTF8_TO_TCHAR(Res->getString("character_id").c_str());
					LoadedData.CharacterName = UTF8_TO_TCHAR(Res->getString("character_name").c_str());
					LoadedData.Level = Res->getInt("level");
					LoadedData.Exp = Res->getInt64("exp");
					LoadedData.JsonData = UTF8_TO_TCHAR(Res->getString("json_data").c_str());
					
					ResultData = LoadedData;
				}
			}
			catch (const sql::SQLException &e)
			{
				UE_LOG(LogTemp, Error, TEXT("LoadCharacterInfo failed: %hs"), e.what());
			}
			Impl->ReturnConnection(Con);
		}

		AsyncTask(ENamedThreads::GameThread, [Delegate, ResultData]()
		{
			Delegate.ExecuteIfBound(ResultData);
		});
	});
}

void UDatabaseManager::SaveCharacterInfo(const FCharacterData& CharacterData, FCharacterDataSaveDelegate Delegate)
{
	if (!Impl)
	{
		Delegate.ExecuteIfBound(false);
		return;
	}
	
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, CharacterData, Delegate]()
	{
		bool bSuccess = false;
		sql::Connection* Con = Impl->GetConnection();

		if (Con)
		{		try
		{
			TUniquePtr<sql::PreparedStatement> Pstmt(Con->prepareStatement(
				"INSERT INTO characters (user_id, character_id, character_name, level, exp, json_data) "
				"VALUES (?, ?, ?, ?, ?, ?) "
				"ON DUPLICATE KEY UPDATE level=VALUES(level), exp=VALUES(exp), json_data=VALUES(json_data), character_name=VALUES(character_name)"
			));

			Pstmt->setString(1, TCHAR_TO_UTF8(*CharacterData.UserId));
			Pstmt->setString(2, TCHAR_TO_UTF8(*CharacterData.CharacterId));
			Pstmt->setString(3, TCHAR_TO_UTF8(*CharacterData.CharacterName));
			Pstmt->setInt(4, CharacterData.Level);
			Pstmt->setInt64(5, CharacterData.Exp);
			Pstmt->setString(6, TCHAR_TO_UTF8(*CharacterData.JsonData));
			
			Pstmt->executeUpdate();
			bSuccess = true;
		}
			catch (const sql::SQLException &e)
			{
				UE_LOG(LogTemp, Error, TEXT("SaveCharacterInfo failed: %hs"), e.what());
			}

			Impl->ReturnConnection(Con);
		}

		AsyncTask(ENamedThreads::GameThread, [Delegate, bSuccess]()
		{
			Delegate.ExecuteIfBound(bSuccess);
		});
	});
}

void UDatabaseManager::LogToExternalServer(const FString& Message)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField(TEXT("log"), Message);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule.CreateRequest();
	
	HttpRequest->SetURL(TEXT("http://127.0.0.1:5000/api/skill-log")); 
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetContentAsString(RequestBody);
	
	HttpRequest->ProcessRequest();
}

template<typename F>
UE::Tasks::TTask<bool> UDatabaseManager::WithTransaction(F&& Function, const TCHAR* TaskLabel)
{
	return UE::Tasks::Launch(TaskLabel, [this, Function = Forward<F>(Function)]() mutable -> bool
	{
		if (!Impl)
		{
			UE_LOG(LogTemp, Error, TEXT("DatabaseManager not initialized"));
			return false;
		}

		sql::Connection* Con = Impl->GetConnection();
		if (!Con)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get database connection"));
			return false;
		}

		// RAII connection management with SCOPE_EXIT
		ON_SCOPE_EXIT
		{
			if (Con)
			{
				Impl->ReturnConnection(Con);
			}
		};

		// RAII transaction management
		FDatabaseManagerImpl::FTransactionGuard TransactionGuard(Con);
		if (!TransactionGuard.Connection)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to begin transaction"));
			return false;
		}

		bool bSuccess = false;
		try
		{
			bSuccess = Function(Con);
			if (bSuccess)
			{
				if (!TransactionGuard.Commit())
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to commit transaction"));
					return false;
				}
			}
			//Try가 성공하면 TransactionGuard의 종료자가 호출되지 않는건가?
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("Transaction failed with SQL exception: %hs"), e.what());
			// Destructor will handle rollback
		}
		catch (...)
		{
			UE_LOG(LogTemp, Error, TEXT("Transaction failed with unknown exception"));
			// Destructor will handle rollback
		}

		return bSuccess;
	});
}

UE::Tasks::TTask<TArray<FInventoryItemDTO>> UDatabaseManager::LoadInventoryForPlayer(const FString& UserId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId]() -> TArray<FInventoryItemDTO>
	{
		TArray<FInventoryItemDTO> ResultItems;
		
		if (!Impl)
		{
			UE_LOG(LogTemp, Error, TEXT("DatabaseManager not initialized"));
			return ResultItems;
		}

		sql::Connection* Con = Impl->GetConnection();
		if (!Con)
		{
			return ResultItems;
		}

		// RAII connection management with SCOPE_EXIT
		ON_SCOPE_EXIT
		{
			if (Con)
			{
				Impl->ReturnConnection(Con);
			}
		};

		try
		{
			TUniquePtr<sql::PreparedStatement> Pstmt(Con->prepareStatement(
				"SELECT item_id, quantity, slot_index, item_data FROM inventory WHERE user_id = ? ORDER BY slot_index"
			));
			Pstmt->setString(1, TCHAR_TO_UTF8(*UserId));

			TUniquePtr<sql::ResultSet> Res(Pstmt->executeQuery());
			while (Res->next())
			{
				FInventoryItemDTO Item;
				Item.ItemID = FName(UTF8_TO_TCHAR(Res->getString("item_id").c_str()));
				Item.Quantity = Res->getInt("quantity");
				Item.SlotIndex = Res->getInt("slot_index");
				Item.ItemData = UTF8_TO_TCHAR(Res->getString("item_data").c_str());
				ResultItems.Add(Item);
			}
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("LoadInventoryForPlayer failed: %hs"), e.what());
		}

		return ResultItems;
	});
}

UE::Tasks::TTask<bool> UDatabaseManager::SaveInventoryForPlayer(const FString& UserId, const TArray<FInventoryItemDTO>& Items)
{
	return WithTransaction([UserId, Items](sql::Connection* Con) -> bool
	{
		try
		{
			// Clear existing inventory
			TUniquePtr<sql::PreparedStatement> DeleteStmt(Con->prepareStatement(
				"DELETE FROM inventory WHERE user_id = ?"
			));
			DeleteStmt->setString(1, TCHAR_TO_UTF8(*UserId));
			DeleteStmt->executeUpdate();

			// Insert new items with slot index
			TUniquePtr<sql::PreparedStatement> InsertStmt(Con->prepareStatement(
				"INSERT INTO inventory (user_id, item_id, quantity, slot_index, item_data) VALUES (?, ?, ?, ?, ?)"
			));

			for (const FInventoryItemDTO& Item : Items)
			{
				InsertStmt->setString(1, TCHAR_TO_UTF8(*UserId));
				InsertStmt->setString(2, TCHAR_TO_UTF8(*Item.ItemID.ToString()));
				InsertStmt->setInt(3, Item.Quantity);
				InsertStmt->setInt(4, Item.SlotIndex);
				InsertStmt->setString(5, TCHAR_TO_UTF8(*Item.ItemData));
				InsertStmt->executeUpdate();
			}

			return true;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("SaveInventoryForPlayer failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Inventory/SaveItems"));
}

UE::Tasks::TTask<bool> UDatabaseManager::AddInventoryItem(const FString& UserId, const FInventoryItemDTO& Item)
{
	return WithTransaction([UserId, Item](sql::Connection* Con) -> bool
	{
		try
		{
			// Insert or update item with slot management
			TUniquePtr<sql::PreparedStatement> InsertStmt(Con->prepareStatement(
				"INSERT INTO inventory (user_id, item_id, quantity, slot_index, item_data) VALUES (?, ?, ?, ?, ?) "
				"ON DUPLICATE KEY UPDATE quantity = VALUES(quantity), item_data = VALUES(item_data)"
			));
			InsertStmt->setString(1, TCHAR_TO_UTF8(*UserId));
			InsertStmt->setString(2, TCHAR_TO_UTF8(*Item.ItemID.ToString()));
			InsertStmt->setInt(3, Item.Quantity);
			InsertStmt->setInt(4, Item.SlotIndex);
			InsertStmt->setString(5, TCHAR_TO_UTF8(*Item.ItemData));
			InsertStmt->executeUpdate();

			return true;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AddInventoryItem failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Inventory/AddItem"));
}

UE::Tasks::TTask<bool> UDatabaseManager::RemoveInventoryItem(const FString& UserId, const FName& ItemID, int32 Quantity)
{
	return WithTransaction([UserId, ItemID, Quantity](sql::Connection* Con) -> bool
	{
		try
		{
			// Update quantity, but don't let it go below 0
			TUniquePtr<sql::PreparedStatement> UpdateStmt(Con->prepareStatement(
				"UPDATE inventory SET quantity = GREATEST(0, quantity - ?) WHERE user_id = ? AND item_id = ?"
			));
			UpdateStmt->setInt(1, Quantity);
			UpdateStmt->setString(2, TCHAR_TO_UTF8(*UserId));
			UpdateStmt->setString(3, TCHAR_TO_UTF8(*ItemID.ToString()));
			
			int32 UpdatedRows = UpdateStmt->executeUpdate();
			
			// Remove items with 0 quantity
			TUniquePtr<sql::PreparedStatement> DeleteStmt(Con->prepareStatement(
				"DELETE FROM inventory WHERE user_id = ? AND item_id = ? AND quantity <= 0"
			));
			DeleteStmt->setString(1, TCHAR_TO_UTF8(*UserId));
			DeleteStmt->setString(2, TCHAR_TO_UTF8(*ItemID.ToString()));
			DeleteStmt->executeUpdate();

			return UpdatedRows > 0;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("RemoveInventoryItem failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Inventory/RemoveItem"));
}

// ============================================================================
// SKILL REPOSITORY METHODS
// ============================================================================

UE::Tasks::TTask<TArray<FSkillSlotDTO>> UDatabaseManager::LoadSkillsForPlayer(const FString& UserId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId]() -> TArray<FSkillSlotDTO>
	{
		TArray<FSkillSlotDTO> ResultSkills;

		if (!Impl)
		{
			UE_LOG(LogTemp, Error, TEXT("DatabaseManager not initialized"));
			return ResultSkills;
		}

		sql::Connection* Con = Impl->GetConnection();
		if (!Con)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get database connection"));
			return ResultSkills;
		}

		try
		{
			TUniquePtr<sql::PreparedStatement> Stmt(Con->prepareStatement(
				"SELECT slot_id, skill_id, slot_index, last_used_time, remaining_cooldown, is_active, skill_data FROM skills WHERE user_id = ? ORDER BY slot_index"
			));
			Stmt->setString(1, TCHAR_TO_UTF8(*UserId));
			UE_LOG(LogTemp, Log, TEXT("Executing skill query for user: %s"), *UserId);

			TUniquePtr<sql::ResultSet> Res(Stmt->executeQuery());

			while (Res->next())
			{
				FSkillSlotDTO Skill;

				FString SlotIdString = UTF8_TO_TCHAR(Res->getString("slot_id").c_str());
				UE_LOG(LogTemp, Verbose, TEXT("Raw Slot ID: %s"), *SlotIdString);

				if (!FGuid::ParseExact(SlotIdString, EGuidFormats::DigitsWithHyphens, Skill.SlotId))
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid GUID format: %s"), *SlotIdString);
				}

				Skill.SkillID = Res->getInt("skill_id");
				Skill.SlotIndex = Res->getInt("slot_index");

				FString LastUsedString = UTF8_TO_TCHAR(Res->getString("last_used_time").c_str());
				if (!FDateTime::ParseIso8601(*LastUsedString, Skill.LastUsedTime))
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to parse last used time: %s"), *LastUsedString);
				}

				Skill.RemainingCooldown = static_cast<float>(Res->getDouble("remaining_cooldown"));
				Skill.bIsActive = Res->getBoolean("is_active");

				// Skill.SkillData = UTF8_TO_TCHAR(Res->getString("skill_data").c_str());
				UE_LOG(LogTemp, Verbose, TEXT("Skill loaded: ID=%d, SlotIndex=%d, Active=%s, CD=%.2f"),
					Skill.SkillID,
					Skill.SlotIndex,
					Skill.bIsActive ? TEXT("true") : TEXT("false"),
					Skill.RemainingCooldown
				);

				ResultSkills.Add(Skill);
			}

			UE_LOG(LogTemp, Log, TEXT("Finished loading skills. Total loaded: %d"), ResultSkills.Num());
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("LoadSkillsForPlayer failed: %hs"), e.what());
		}

		return ResultSkills;
	});
}

UE::Tasks::TTask<bool> UDatabaseManager::SaveSkillsForPlayer(const FString& UserId, const TArray<FSkillSlotDTO>& SkillSlots)
{
	return WithTransaction([UserId, SkillSlots](sql::Connection* Con) -> bool
	{
		try
		{
			// Clear existing skills
			TUniquePtr<sql::PreparedStatement> DeleteStmt(Con->prepareStatement(
				"DELETE FROM skills WHERE user_id = ?"
			));
			DeleteStmt->setString(1, TCHAR_TO_UTF8(*UserId));
			DeleteStmt->executeUpdate();

			// Insert new skills
			TUniquePtr<sql::PreparedStatement> InsertStmt(Con->prepareStatement(
				"INSERT INTO skills (user_id, slot_id, skill_id, slot_index, last_used_time, remaining_cooldown, is_active, skill_data) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
			));

			for (const FSkillSlotDTO& Skill : SkillSlots)
			{
				InsertStmt->setString(1, TCHAR_TO_UTF8(*UserId));
				InsertStmt->setString(2, TCHAR_TO_UTF8(*Skill.SlotId.ToString(EGuidFormats::DigitsWithHyphens)));
				InsertStmt->setInt(3, Skill.SkillID);
				InsertStmt->setInt(4, Skill.SlotIndex);
				InsertStmt->setString(5, TCHAR_TO_UTF8(*Skill.LastUsedTime.ToIso8601()));
				InsertStmt->setDouble(6, Skill.RemainingCooldown);
				InsertStmt->setBoolean(7, Skill.bIsActive);
				InsertStmt->setString(8, TCHAR_TO_UTF8(*Skill.SkillData));
				
				InsertStmt->executeUpdate();
			}

			return true;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("SaveSkillsForPlayer failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Skill/SaveSkills"));
}

UE::Tasks::TTask<bool> UDatabaseManager::RegisterSkill(const FString& UserId, const FSkillSlotDTO& SkillSlot)
{
	return WithTransaction([UserId, SkillSlot](sql::Connection* Con) -> bool
	{
		try
		{
			TUniquePtr<sql::PreparedStatement> InsertStmt(Con->prepareStatement(
				"INSERT INTO skills (user_id, slot_id, skill_id, slot_index, last_used_time, remaining_cooldown, is_active, skill_data) VALUES (?, ?, ?, ?, ?, ?, ?, ?) "
				"ON DUPLICATE KEY UPDATE skill_id = VALUES(skill_id), slot_index = VALUES(slot_index), last_used_time = VALUES(last_used_time), remaining_cooldown = VALUES(remaining_cooldown), is_active = VALUES(is_active), skill_data = VALUES(skill_data)"
			));
			
			InsertStmt->setString(1, TCHAR_TO_UTF8(*UserId));
			InsertStmt->setString(2, TCHAR_TO_UTF8(*SkillSlot.SlotId.ToString(EGuidFormats::DigitsWithHyphens)));
			InsertStmt->setInt(3, SkillSlot.SkillID);
			InsertStmt->setInt(4, SkillSlot.SlotIndex);
			InsertStmt->setString(5, TCHAR_TO_UTF8(*SkillSlot.LastUsedTime.ToIso8601()));
			InsertStmt->setDouble(6, SkillSlot.RemainingCooldown);
			InsertStmt->setBoolean(7, SkillSlot.bIsActive);
			InsertStmt->setString(8, TCHAR_TO_UTF8(*SkillSlot.SkillData));
			
			int32 AffectedRows = InsertStmt->executeUpdate();
			return AffectedRows > 0;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("RegisterSkill failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Skill/RegisterSkill"));
}

UE::Tasks::TTask<bool> UDatabaseManager::UnregisterSkill(const FString& UserId, const FGuid& SlotId)
{
	return WithTransaction([UserId, SlotId](sql::Connection* Con) -> bool
	{
		try
		{
			TUniquePtr<sql::PreparedStatement> DeleteStmt(Con->prepareStatement(
				"DELETE FROM skills WHERE user_id = ? AND slot_id = ?"
			));
			DeleteStmt->setString(1, TCHAR_TO_UTF8(*UserId));
			DeleteStmt->setString(2, TCHAR_TO_UTF8(*SlotId.ToString(EGuidFormats::DigitsWithHyphens)));
			
			int32 AffectedRows = DeleteStmt->executeUpdate();
			return AffectedRows > 0;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("UnregisterSkill failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Skill/UnregisterSkill"));
}

UE::Tasks::TTask<bool> UDatabaseManager::UpdateSkillCooldown(const FString& UserId, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown)
{
	return WithTransaction([UserId, SlotId, LastUsedTime, RemainingCooldown](sql::Connection* Con) -> bool
	{
		try
		{
			TUniquePtr<sql::PreparedStatement> UpdateStmt(Con->prepareStatement(
				"UPDATE skills SET last_used_time = ?, remaining_cooldown = ? WHERE user_id = ? AND slot_id = ?"
			));
			UpdateStmt->setString(1, TCHAR_TO_UTF8(*LastUsedTime.ToIso8601()));
			UpdateStmt->setDouble(2, RemainingCooldown);
			UpdateStmt->setString(3, TCHAR_TO_UTF8(*UserId));
			UpdateStmt->setString(4, TCHAR_TO_UTF8(*SlotId.ToString(EGuidFormats::DigitsWithHyphens)));
			
			int32 AffectedRows = UpdateStmt->executeUpdate();
			return AffectedRows > 0;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("UpdateSkillCooldown failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Skill/UpdateCooldown"));
}

// ============================================================================
// SHOP REPOSITORY METHODS
// ============================================================================

UE::Tasks::TTask<FShopRepositoryResult> UDatabaseManager::LoadShopByID(int32 ShopID)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID]() -> FShopRepositoryResult
	{
		FShopRepositoryResult Result;
		
		if (!Impl)
		{
			UE_LOG(LogTemp, Error, TEXT("DatabaseManager not initialized"));
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("DatabaseManager not initialized");
			return Result;
		}

		sql::Connection* Con = Impl->GetConnection();
		if (!Con)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get database connection"));
			Result.bSuccess = false;
			Result.ErrorMessage = TEXT("Failed to get database connection");
			return Result;
		}

		// RAII connection management with SCOPE_EXIT
		ON_SCOPE_EXIT
		{
			if (Con)
			{
				Impl->ReturnConnection(Con);
			}
		};

		try
		{
			// Load shop basic info
			TUniquePtr<sql::PreparedStatement> ShopStmt(Con->prepareStatement(
				"SELECT shop_id, shop_name, shop_description, is_open, area_id, shop_location_x, shop_location_y, shop_location_z, last_restock_time FROM shops WHERE shop_id = ?"
			));
			ShopStmt->setInt(1, ShopID);
			
			TUniquePtr<sql::ResultSet> ShopRes(ShopStmt->executeQuery());
			
			if (ShopRes->next())
			{
				Result.ShopData.ShopID = ShopRes->getInt("shop_id");
				Result.ShopData.ShopName = UTF8_TO_TCHAR(ShopRes->getString("shop_name").c_str());
				Result.ShopData.ShopDescription = UTF8_TO_TCHAR(ShopRes->getString("shop_description").c_str());
				Result.ShopData.bIsOpen = ShopRes->getBoolean("is_open");
				Result.ShopData.AreaID = ShopRes->getInt("area_id");
				
				// Parse location
				float X = static_cast<float>(ShopRes->getDouble("shop_location_x"));
				float Y = static_cast<float>(ShopRes->getDouble("shop_location_y"));
				float Z = static_cast<float>(ShopRes->getDouble("shop_location_z"));
				Result.ShopData.ShopLocation = FVector(X, Y, Z);
				
				// Parse last restock time
				std::string TimeStr = ShopRes->getString("last_restock_time");
				// Convert SQL timestamp to FDateTime if needed
				Result.ShopData.LastRestockTime = FDateTime::Now();

				// Load shop items
				TUniquePtr<sql::PreparedStatement> ItemsStmt(Con->prepareStatement(
					"SELECT item_id, item_name, item_description, price, stock, is_available, category, max_stock FROM shop_items WHERE shop_id = ?"
				));
				ItemsStmt->setInt(1, ShopID);
				
				TUniquePtr<sql::ResultSet> ItemsRes(ItemsStmt->executeQuery());
				
				while (ItemsRes->next())
				{
					FShopItemDTO Item;
					Item.ItemID = ItemsRes->getInt("item_id");
					Item.ItemName = UTF8_TO_TCHAR(ItemsRes->getString("item_name").c_str());
					Item.ItemDescription = UTF8_TO_TCHAR(ItemsRes->getString("item_description").c_str());
					Item.Price = static_cast<float>(ItemsRes->getDouble("price"));
					Item.Stock = ItemsRes->getInt("stock");
					Item.bIsAvailable = ItemsRes->getBoolean("is_available");
					Item.Category = UTF8_TO_TCHAR(ItemsRes->getString("category").c_str());
					Item.MaxStock = ItemsRes->getInt("max_stock");
					
					Result.ShopData.ShopItems.Add(Item);
				}
				
				Result.bSuccess = true;
			}
			else
			{
				Result.bSuccess = false;
				Result.ErrorMessage = FString::Printf(TEXT("Shop with ID %d not found"), ShopID);
			}
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("LoadShopByID failed: %hs"), e.what());
			Result.bSuccess = false;
			Result.ErrorMessage = FString(e.what());
		}

		return Result;
	});
}

UE::Tasks::TTask<bool> UDatabaseManager::SaveShop(const FShopDomain& ShopData)
{
	return WithTransaction([ShopData](sql::Connection* Con) -> bool
	{
		try
		{
			// Update/Insert shop basic info
			TUniquePtr<sql::PreparedStatement> ShopStmt(Con->prepareStatement(
				"INSERT INTO shops (shop_id, shop_name, shop_description, is_open, area_id, shop_location_x, shop_location_y, shop_location_z, last_restock_time, global_price_modifier, shop_owner_name) "
				"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
				"ON DUPLICATE KEY UPDATE "
				"shop_name = VALUES(shop_name), shop_description = VALUES(shop_description), is_open = VALUES(is_open), "
				"area_id = VALUES(area_id), shop_location_x = VALUES(shop_location_x), shop_location_y = VALUES(shop_location_y), shop_location_z = VALUES(shop_location_z), "
				"last_restock_time = VALUES(last_restock_time), global_price_modifier = VALUES(global_price_modifier), shop_owner_name = VALUES(shop_owner_name)"
			));
			
			ShopStmt->setInt(1, ShopData.ShopID);
			ShopStmt->setString(2, TCHAR_TO_UTF8(*ShopData.ShopName));
			ShopStmt->setString(3, TCHAR_TO_UTF8(*ShopData.ShopDescription));
			ShopStmt->setBoolean(4, ShopData.bIsOpen);
			ShopStmt->setInt(5, ShopData.AreaID);
			ShopStmt->setDouble(6, ShopData.ShopLocation.X);
			ShopStmt->setDouble(7, ShopData.ShopLocation.Y);
			ShopStmt->setDouble(8, ShopData.ShopLocation.Z);
			ShopStmt->setString(9, TCHAR_TO_UTF8(*ShopData.LastRestockTime.ToIso8601()));
			ShopStmt->setDouble(10, ShopData.GlobalPriceModifier);
			ShopStmt->setString(11, TCHAR_TO_UTF8(*ShopData.ShopOwnerName));
			
			ShopStmt->executeUpdate();

			// Clear existing items
			TUniquePtr<sql::PreparedStatement> DeleteStmt(Con->prepareStatement(
				"DELETE FROM shop_items WHERE shop_id = ?"
			));
			DeleteStmt->setInt(1, ShopData.ShopID);
			DeleteStmt->executeUpdate();

			// Insert new items
			TUniquePtr<sql::PreparedStatement> ItemsStmt(Con->prepareStatement(
				"INSERT INTO shop_items (shop_id, item_id, stock, price, is_available, max_stock, restock_interval_hours) "
				"VALUES (?, ?, ?, ?, ?, ?, ?)"
			));

			for (const FShopItemDTO& Item : ShopData.ShopItems)
			{
				ItemsStmt->setInt(1, ShopData.ShopID);
				ItemsStmt->setInt(2, Item.ItemID);
				ItemsStmt->setInt(3, Item.Stock);
				ItemsStmt->setDouble(4, Item.Price);
				ItemsStmt->setBoolean(5, Item.bIsAvailable);
				ItemsStmt->setInt(6, Item.MaxStock);
				ItemsStmt->setDouble(7, Item.RestockIntervalHours);
				
				ItemsStmt->executeUpdate();
			}

			return true;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("SaveShop failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Shop/SaveShop"));
}

UE::Tasks::TTask<TArray<FShopRepositoryResult>> UDatabaseManager::LoadShopsByIDs(const TArray<int32>& ShopIDs)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopIDs]() -> TArray<FShopRepositoryResult>
	{
		TArray<FShopRepositoryResult> Results;
		
		if (!Impl)
		{
			UE_LOG(LogTemp, Error, TEXT("DatabaseManager not initialized"));
			return Results;
		}

		sql::Connection* Con = Impl->GetConnection();
		if (!Con)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get database connection"));
			return Results;
		}

		// RAII connection management
		ON_SCOPE_EXIT
		{
			if (Con)
			{
				Impl->ReturnConnection(Con);
			}
		};

		try
		{
			for (int32 ShopID : ShopIDs)
			{
				auto LoadTask = LoadShopByID(ShopID);
				FShopRepositoryResult ShopResult = LoadTask.GetResult();
				
				Results.Add(ShopResult); // Add all results, even failures
			}
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("LoadShopsByIDs failed: %s"), UTF8_TO_TCHAR(e.what()));
			Results.Add(FShopRepositoryResult::Failure(UTF8_TO_TCHAR(e.what())));
		}

		return Results;
	});
}

UE::Tasks::TTask<TArray<FShopRepositoryResult>> UDatabaseManager::LoadShopsForArea(int32 AreaID)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, AreaID]() -> TArray<FShopRepositoryResult>
	{
		TArray<FShopRepositoryResult> Results;
		
		if (!Impl)
		{
			UE_LOG(LogTemp, Error, TEXT("DatabaseManager not initialized"));
			return Results;
		}

		sql::Connection* Con = Impl->GetConnection();
		if (!Con)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get database connection"));
			return Results;
		}

		// RAII connection management
		ON_SCOPE_EXIT
		{
			if (Con)
			{
				Impl->ReturnConnection(Con);
			}
		};

		try
		{
			TUniquePtr<sql::PreparedStatement> Stmt(Con->prepareStatement(
				"SELECT shop_id FROM shops WHERE area_id = ?"
			));
			Stmt->setInt(1, AreaID);
			
			TUniquePtr<sql::ResultSet> Res(Stmt->executeQuery());
			TArray<int32> ShopIDs;
			
			while (Res->next())
			{
				ShopIDs.Add(Res->getInt("shop_id"));
			}

			// Load full shop data for each shop ID
			for (int32 ShopID : ShopIDs)
			{
				auto LoadTask = LoadShopByID(ShopID);
				FShopRepositoryResult ShopResult = LoadTask.GetResult();
				
				Results.Add(ShopResult); // Add all results, even failures
			}
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("LoadShopsForArea failed: %s"), UTF8_TO_TCHAR(e.what()));
			Results.Add(FShopRepositoryResult::Failure(UTF8_TO_TCHAR(e.what())));
		}

		return Results;
	});
}

UE::Tasks::TTask<bool> UDatabaseManager::DeleteShop(int32 ShopID)
{
	return WithTransaction([ShopID](sql::Connection* Con) -> bool
	{
		try
		{
			// Delete shop items first (foreign key constraint)
			TUniquePtr<sql::PreparedStatement> DeleteItemsStmt(Con->prepareStatement(
				"DELETE FROM shop_items WHERE shop_id = ?"
			));
			DeleteItemsStmt->setInt(1, ShopID);
			DeleteItemsStmt->executeUpdate();

			// Delete shop
			TUniquePtr<sql::PreparedStatement> DeleteShopStmt(Con->prepareStatement(
				"DELETE FROM shops WHERE shop_id = ?"
			));
			DeleteShopStmt->setInt(1, ShopID);
			
			int32 AffectedRows = DeleteShopStmt->executeUpdate();
			return AffectedRows > 0;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("DeleteShop failed: %s"), UTF8_TO_TCHAR(e.what()));
			return false;
		}
	}, TEXT("Shop/DeleteShop"));
}

UE::Tasks::TTask<bool> UDatabaseManager::CheckShopExists(int32 ShopID)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID]() -> bool
	{
		if (!Impl)
		{
			UE_LOG(LogTemp, Error, TEXT("DatabaseManager not initialized"));
			return false;
		}

		sql::Connection* Con = Impl->GetConnection();
		if (!Con)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get database connection"));
			return false;
		}

		// RAII connection management
		ON_SCOPE_EXIT
		{
			if (Con)
			{
				Impl->ReturnConnection(Con);
			}
		};

		try
		{
			TUniquePtr<sql::PreparedStatement> Stmt(Con->prepareStatement(
				"SELECT COUNT(*) as count FROM shops WHERE shop_id = ?"
			));
			Stmt->setInt(1, ShopID);
			
			TUniquePtr<sql::ResultSet> Res(Stmt->executeQuery());
			
			if (Res->next())
			{
				return Res->getInt("count") > 0;
			}
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("CheckShopExists failed: %s"), UTF8_TO_TCHAR(e.what()));
		}

		return false;
	});
}

UE::Tasks::TTask<bool> UDatabaseManager::AddItemToShop(int32 ShopID, const FShopItemDTO& ItemData)
{
	return WithTransaction([ShopID, ItemData](sql::Connection* Con) -> bool
	{
		try
		{
			TUniquePtr<sql::PreparedStatement> InsertStmt(Con->prepareStatement(
				"INSERT INTO shop_items (shop_id, item_id, item_name, item_description, price, stock, is_available, category, max_stock, restock_interval_hours) "
				"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
				"ON DUPLICATE KEY UPDATE "
				"item_name = VALUES(item_name), item_description = VALUES(item_description), "
				"price = VALUES(price), stock = VALUES(stock), is_available = VALUES(is_available), "
				"category = VALUES(category), max_stock = VALUES(max_stock), restock_interval_hours = VALUES(restock_interval_hours)"
			));
			
			InsertStmt->setInt(1, ShopID);
			InsertStmt->setInt(2, ItemData.ItemID);
			InsertStmt->setString(3, TCHAR_TO_UTF8(*ItemData.ItemName));
			InsertStmt->setString(4, TCHAR_TO_UTF8(*ItemData.ItemDescription));
			InsertStmt->setDouble(5, ItemData.Price);
			InsertStmt->setInt(6, ItemData.Stock);
			InsertStmt->setBoolean(7, ItemData.bIsAvailable);
			InsertStmt->setString(8, TCHAR_TO_UTF8(*ItemData.Category));
			InsertStmt->setInt(9, ItemData.MaxStock);
			InsertStmt->setDouble(10, ItemData.RestockIntervalHours);
			
			int32 AffectedRows = InsertStmt->executeUpdate();
			return AffectedRows > 0;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AddItemToShop failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Shop/AddItem"));
}

UE::Tasks::TTask<bool> UDatabaseManager::RemoveItemFromShop(int32 ShopID, int32 ItemID)
{
	return WithTransaction([ShopID, ItemID](sql::Connection* Con) -> bool
	{
		try
		{
			TUniquePtr<sql::PreparedStatement> DeleteStmt(Con->prepareStatement(
				"DELETE FROM shop_items WHERE shop_id = ? AND item_id = ?"
			));
			DeleteStmt->setInt(1, ShopID);
			DeleteStmt->setInt(2, ItemID);
			
			int32 AffectedRows = DeleteStmt->executeUpdate();
			return AffectedRows > 0;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("RemoveItemFromShop failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Shop/RemoveItem"));
}

UE::Tasks::TTask<bool> UDatabaseManager::UpdateShopItemStock(int32 ShopID, int32 ItemID, int32 NewStock)
{
	return WithTransaction([ShopID, ItemID, NewStock](sql::Connection* Con) -> bool
	{
		try
		{
			TUniquePtr<sql::PreparedStatement> UpdateStmt(Con->prepareStatement(
				"UPDATE shop_items SET stock = ?, is_available = ? WHERE shop_id = ? AND item_id = ?"
			));
			UpdateStmt->setInt(1, NewStock);
			UpdateStmt->setBoolean(2, NewStock > 0);
			UpdateStmt->setInt(3, ShopID);
			UpdateStmt->setInt(4, ItemID);
			
			int32 AffectedRows = UpdateStmt->executeUpdate();
			return AffectedRows > 0;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("UpdateShopItemStock failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Shop/UpdateStock"));
}

UE::Tasks::TTask<bool> UDatabaseManager::UpdateShopItemPrice(int32 ShopID, int32 ItemID, float NewPrice)
{
	return WithTransaction([ShopID, ItemID, NewPrice](sql::Connection* Con) -> bool
	{
		try
		{
			TUniquePtr<sql::PreparedStatement> UpdateStmt(Con->prepareStatement(
				"UPDATE shop_items SET price = ? WHERE shop_id = ? AND item_id = ?"
			));
			UpdateStmt->setDouble(1, NewPrice);
			UpdateStmt->setInt(2, ShopID);
			UpdateStmt->setInt(3, ItemID);
			
			int32 AffectedRows = UpdateStmt->executeUpdate();
			return AffectedRows > 0;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("UpdateShopItemPrice failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Shop/UpdatePrice"));
}

// ============================================================================
// PLAYER ID HELPER IMPLEMENTATIONS
// ============================================================================

FString UPlayerIdHelper::ConvertPlayerIdToUserId(int32 PlayerId)
{
	// 프로토타입용 간단 변환: player_[PlayerId] 형식
	return FString::Printf(TEXT("player_%d"), PlayerId);
}

int32 UPlayerIdHelper::ConvertUserIdToPlayerId(const FString& UserId)
{
	// player_[number] 형식에서 number 추출
	if (UserId.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("UPlayerIdHelper::ConvertUserIdToPlayerId: Empty UserId"));
		return -1;
	}
	
	FString PlayerPrefix = TEXT("player_");
	if (!UserId.StartsWith(PlayerPrefix))
	{
		// 숫자만 있는 경우 직접 변환 시도
		if (UserId.IsNumeric())
		{
			return FCString::Atoi(*UserId);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("UPlayerIdHelper::ConvertUserIdToPlayerId: UserId '%s' does not follow player_[id] format"), *UserId);
		return -1;
	}
	
	FString NumberPart = UserId.RightChop(PlayerPrefix.Len());
	if (NumberPart.IsNumeric())
	{
		return FCString::Atoi(*NumberPart);
	}
	
	UE_LOG(LogTemp, Error, TEXT("UPlayerIdHelper::ConvertUserIdToPlayerId: Invalid numeric part in UserId '%s'"), *UserId);
	return -1;
}

FString UPlayerIdHelper::GenerateUserIdFromPlayerId(int32 PlayerId, const FString& Prefix)
{
	
	return FString::Printf(TEXT("%s_%d"), *Prefix, PlayerId);
}

bool UPlayerIdHelper::IsValidUserId(const FString& UserId)
{
	if (UserId.IsEmpty())
	{
		return false;
	}
	
	// VARCHAR(255) 제한 검사
	if (UserId.Len() > 255)
	{
		return false;
	}
	
	// 기본적인 문자열 유효성 검사 (공백, 특수문자 등)
	for (const TCHAR& Char : UserId)
	{
		if (FChar::IsWhitespace(Char) || Char == TEXT('\0'))
		{
			return false;
		}
	}
	
	return true;
}

// ============================================================================
// JSON HELPER IMPLEMENTATIONS
// ============================================================================

FString UDatabaseJsonHelper::SerializeInventoryItemData(const TMap<FString, FString>& ItemProperties)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	for (const auto& Property : ItemProperties)
	{
		JsonObject->SetStringField(Property.Key, Property.Value);
	}
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	return OutputString;
}

TMap<FString, FString> UDatabaseJsonHelper::DeserializeInventoryItemData(const FString& JsonData)
{
	TMap<FString, FString> Result;
	
	if (JsonData.IsEmpty())
	{
		return Result;
	}
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonData);
	
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		for (const auto& JsonPair : JsonObject->Values)
		{
			FString StringValue;
			if (JsonPair.Value->TryGetString(StringValue))
			{
				Result.Add(JsonPair.Key, StringValue);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to deserialize inventory item JSON data: %s"), *JsonData);
	}
	
	return Result;
}

FString UDatabaseJsonHelper::SerializeCharacterExtendedData(const FVector& Position, float Health, float Mana, const TMap<FString, FString>& AdditionalData)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	// Position data
	TSharedPtr<FJsonObject> PositionObject = MakeShareable(new FJsonObject);
	PositionObject->SetNumberField(TEXT("X"), Position.X);
	PositionObject->SetNumberField(TEXT("Y"), Position.Y);
	PositionObject->SetNumberField(TEXT("Z"), Position.Z);
	JsonObject->SetObjectField(TEXT("Position"), PositionObject);
	
	// Health and Mana
	JsonObject->SetNumberField(TEXT("Health"), Health);
	JsonObject->SetNumberField(TEXT("Mana"), Mana);
	
	// Additional data
	if (AdditionalData.Num() > 0)
	{
		TSharedPtr<FJsonObject> AdditionalObject = MakeShareable(new FJsonObject);
		for (const auto& Data : AdditionalData)
		{
			AdditionalObject->SetStringField(Data.Key, Data.Value);
		}
		JsonObject->SetObjectField(TEXT("Additional"), AdditionalObject);
	}
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	return OutputString;
}

bool UDatabaseJsonHelper::DeserializeCharacterExtendedData(const FString& JsonData, FVector& OutPosition, float& OutHealth, float& OutMana, TMap<FString, FString>& OutAdditionalData)
{
	if (JsonData.IsEmpty())
	{
		return false;
	}
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonData);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to deserialize character extended JSON data: %s"), *JsonData);
		return false;
	}
	
	// Parse Position
	const TSharedPtr<FJsonObject>* PositionObject;
	if (JsonObject->TryGetObjectField(TEXT("Position"), PositionObject))
	{
		double X, Y, Z;
		if ((*PositionObject)->TryGetNumberField(TEXT("X"), X) &&
			(*PositionObject)->TryGetNumberField(TEXT("Y"), Y) &&
			(*PositionObject)->TryGetNumberField(TEXT("Z"), Z))
		{
			OutPosition = FVector(X, Y, Z);
		}
	}
	
	// Parse Health and Mana
	double HealthValue, ManaValue;
	if (JsonObject->TryGetNumberField(TEXT("Health"), HealthValue))
	{
		OutHealth = static_cast<float>(HealthValue);
	}
	if (JsonObject->TryGetNumberField(TEXT("Mana"), ManaValue))
	{
		OutMana = static_cast<float>(ManaValue);
	}
	
	// Parse Additional data
	const TSharedPtr<FJsonObject>* AdditionalObject;
	if (JsonObject->TryGetObjectField(TEXT("Additional"), AdditionalObject))
	{
		for (const auto& JsonPair : (*AdditionalObject)->Values)
		{
			FString StringValue;
			if (JsonPair.Value->TryGetString(StringValue))
			{
				OutAdditionalData.Add(JsonPair.Key, StringValue);
			}
		}
	}
	
	return true;
}

// ============================================================================
// DATABASE USER AUTHENTICATION METHODS
// ============================================================================

UE::Tasks::TTask<bool> UDatabaseManager::CreateUserAccount(const FString& Username, const FString& PasswordHash, const FString& Email, int32& OutUserId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, Username, PasswordHash, Email, &OutUserId]() -> bool
	{
		if (!Impl)
		{
			UE_LOG(LogTemp, Error, TEXT("DatabaseManager not initialized"));
			return false;
		}

		sql::Connection* Con = Impl->GetConnection();
		if (!Con)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get database connection"));
			return false;
		}

		// RAII connection management
		ON_SCOPE_EXIT
		{
			if (Con)
			{
				Impl->ReturnConnection(Con);
			}
		};

		try
		{
			TUniquePtr<sql::PreparedStatement> Stmt(Con->prepareStatement(
				"INSERT INTO users (username, password_hash, email, created_at, is_active, failed_login_attempts) "
				"VALUES (?, ?, ?, NOW(), 1, 0)"
			));
			Stmt->setString(1, TCHAR_TO_UTF8(*Username));
			Stmt->setString(2, TCHAR_TO_UTF8(*PasswordHash));
			
			// Handle empty email as NULL
			if (Email.IsEmpty())
			{
				Stmt->setNull(3, sql::DataType::VARCHAR);
			}
			else
			{
				Stmt->setString(3, TCHAR_TO_UTF8(*Email));
			}

			int32 AffectedRows = Stmt->executeUpdate();
			if (AffectedRows > 0)
			{
				// Get the generated user ID
				TUniquePtr<sql::Statement> LastIdStmt(Con->createStatement());
				TUniquePtr<sql::ResultSet> Res(LastIdStmt->executeQuery("SELECT LAST_INSERT_ID() as user_id"));
				if (Res->next())
				{
					OutUserId = Res->getInt("user_id");
				}
				return true;
			}
			return false;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("CreateUserAccount failed: %hs"), e.what());
			return false;
		}
	});
}

UE::Tasks::TTask<TOptional<FDatabaseUserData>> UDatabaseManager::GetUserByUsername(const FString& Username)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, Username]() -> TOptional<FDatabaseUserData>
	{
		TOptional<FDatabaseUserData> Result;

		if (!Impl)
		{
			UE_LOG(LogTemp, Error, TEXT("DatabaseManager not initialized"));
			return Result;
		}

		sql::Connection* Con = Impl->GetConnection();
		if (!Con)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get database connection"));
			return Result;
		}

		// RAII connection management
		ON_SCOPE_EXIT
		{
			if (Con)
			{
				Impl->ReturnConnection(Con);
			}
		};

		try
		{
			TUniquePtr<sql::PreparedStatement> Stmt(Con->prepareStatement(
				"SELECT user_id, username, password_hash, email, created_at, last_login, is_active, failed_login_attempts, account_locked_until "
				"FROM users WHERE username = ?"
			));
			Stmt->setString(1, TCHAR_TO_UTF8(*Username));

			TUniquePtr<sql::ResultSet> Res(Stmt->executeQuery());
			if (Res->next())
			{
				FDatabaseUserData UserData;
				UserData.UserId = Res->getInt("user_id");
				UserData.Username = UTF8_TO_TCHAR(Res->getString("username").c_str());
				UserData.PasswordHash = UTF8_TO_TCHAR(Res->getString("password_hash").c_str());
				
				// Handle nullable email field
				if (!Res->isNull("email"))
				{
					UserData.Email = UTF8_TO_TCHAR(Res->getString("email").c_str());
				}
				
				// Convert SQL timestamps to FDateTime (placeholder implementation)
				UserData.CreatedAt = FDateTime::Now();
				
				if (!Res->isNull("last_login"))
				{
					UserData.LastLogin = FDateTime::Now();
				}

				if (!Res->isNull("account_locked_until"))
				{
					UserData.AccountLockedUntil = FDateTime::Now();
				}

				UserData.bIsActive = Res->getBoolean("is_active");
				UserData.FailedLoginAttempts = Res->getInt("failed_login_attempts");

				Result = UserData;
			}
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("GetUserByUsername failed: %hs"), e.what());
		}

		return Result;
	});
}

UE::Tasks::TTask<TOptional<FDatabaseUserData>> UDatabaseManager::GetUserById(int32 UserId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId]() -> TOptional<FDatabaseUserData>
	{
		TOptional<FDatabaseUserData> Result;

		if (!Impl)
		{
			UE_LOG(LogTemp, Error, TEXT("DatabaseManager not initialized"));
			return Result;
		}

		sql::Connection* Con = Impl->GetConnection();
		if (!Con)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get database connection"));
			return Result;
		}

		// RAII connection management
		ON_SCOPE_EXIT
		{
			if (Con)
			{
				Impl->ReturnConnection(Con);
			}
		};

		try
		{
			TUniquePtr<sql::PreparedStatement> Stmt(Con->prepareStatement(
				"SELECT user_id, username, password_hash, email, created_at, last_login, is_active, failed_login_attempts, account_locked_until "
				"FROM users WHERE user_id = ?"
			));
			Stmt->setInt(1, UserId);

			TUniquePtr<sql::ResultSet> Res(Stmt->executeQuery());
			if (Res->next())
			{
				FDatabaseUserData UserData;
				UserData.UserId = Res->getInt("user_id");
				UserData.Username = UTF8_TO_TCHAR(Res->getString("username").c_str());
				UserData.PasswordHash = UTF8_TO_TCHAR(Res->getString("password_hash").c_str());
				
				// Handle nullable email field
				if (!Res->isNull("email"))
				{
					UserData.Email = UTF8_TO_TCHAR(Res->getString("email").c_str());
				}
				
				// Convert SQL timestamps to FDateTime (placeholder implementation)
				UserData.CreatedAt = FDateTime::Now();

				if (!Res->isNull("last_login"))
				{
					UserData.LastLogin = FDateTime::Now();
				}

				if (!Res->isNull("account_locked_until"))
				{
					UserData.AccountLockedUntil = FDateTime::Now();
				}

				UserData.bIsActive = Res->getBoolean("is_active");
				UserData.FailedLoginAttempts = Res->getInt("failed_login_attempts");

				Result = UserData;
			}
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("GetUserById failed: %hs"), e.what());
		}

		return Result;
	});
}

UE::Tasks::TTask<bool> UDatabaseManager::UpdateUserAccount(const FDatabaseUserData& UserData)
{
	return WithTransaction([UserData](sql::Connection* Con) -> bool
	{
		try
		{
			TUniquePtr<sql::PreparedStatement> Stmt(Con->prepareStatement(
				"UPDATE users SET password_hash = ?, email = ?, last_login = ?, is_active = ?, "
				"failed_login_attempts = ?, account_locked_until = ? WHERE user_id = ?"
			));

			Stmt->setString(1, TCHAR_TO_UTF8(*UserData.PasswordHash));
			
			// Handle nullable email field
			if (UserData.Email.IsEmpty())
			{
				Stmt->setNull(2, sql::DataType::VARCHAR);
			}
			else
			{
				Stmt->setString(2, TCHAR_TO_UTF8(*UserData.Email));
			}
			
			if (UserData.LastLogin.IsSet())
			{
				Stmt->setString(3, TCHAR_TO_UTF8(*UserData.LastLogin.GetValue().ToIso8601()));
			}
			else
			{
				Stmt->setNull(3, sql::DataType::TIMESTAMP);
			}

			Stmt->setBoolean(4, UserData.bIsActive);
			Stmt->setInt(5, UserData.FailedLoginAttempts);

			if (UserData.AccountLockedUntil.IsSet())
			{
				Stmt->setString(6, TCHAR_TO_UTF8(*UserData.AccountLockedUntil.GetValue().ToIso8601()));
			}
			else
			{
				Stmt->setNull(6, sql::DataType::TIMESTAMP);
			}

			Stmt->setInt(7, UserData.UserId);

			int32 AffectedRows = Stmt->executeUpdate();
			return AffectedRows > 0;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("UpdateUserAccount failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Database/UpdateUser"));
}

UE::Tasks::TTask<bool> UDatabaseManager::AddUserAuditLog(int32 UserId, const FString& Action, const FString& Details, const FString& IpAddress)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, UserId, Action, Details, IpAddress]() -> bool
	{
		if (!Impl)
		{
			UE_LOG(LogTemp, Error, TEXT("DatabaseManager not initialized"));
			return false;
		}

		sql::Connection* Con = Impl->GetConnection();
		if (!Con)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get database connection"));
			return false;
		}

		// RAII connection management
		ON_SCOPE_EXIT
		{
			if (Con)
			{
				Impl->ReturnConnection(Con);
			}
		};

		try
		{
			TUniquePtr<sql::PreparedStatement> Stmt(Con->prepareStatement(
				"INSERT INTO user_audit_logs (user_id, action, details, ip_address, created_at) "
				"VALUES (?, ?, ?, ?, NOW())"
			));
			Stmt->setInt(1, UserId);
			Stmt->setString(2, TCHAR_TO_UTF8(*Action));
			Stmt->setString(3, TCHAR_TO_UTF8(*Details));
			Stmt->setString(4, TCHAR_TO_UTF8(*IpAddress));

			int32 AffectedRows = Stmt->executeUpdate();
			return AffectedRows > 0;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AddUserAuditLog failed: %hs"), e.what());
			return false;
		}
	});
}

FString UDatabaseJsonHelper::SerializeSkillData(const TMap<FString, FString>& SkillProperties)
{
	return SerializeInventoryItemData(SkillProperties); // 같은 구조 재사용
}

TMap<FString, FString> UDatabaseJsonHelper::DeserializeSkillData(const FString& JsonData)
{
	return DeserializeInventoryItemData(JsonData); // 같은 구조 재사용
}

FString UDatabaseJsonHelper::SerializeEquipmentEnhancement(int32 EnhancementLevel, const TArray<FString>& EnhancementEffects)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	JsonObject->SetNumberField(TEXT("Level"), EnhancementLevel);
	
	TArray<TSharedPtr<FJsonValue>> EffectsArray;
	for (const FString& Effect : EnhancementEffects)
	{
		EffectsArray.Add(MakeShareable(new FJsonValueString(Effect)));
	}
	JsonObject->SetArrayField(TEXT("Effects"), EffectsArray);
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	return OutputString;
}

bool UDatabaseJsonHelper::DeserializeEquipmentEnhancement(const FString& JsonData, int32& OutEnhancementLevel, TArray<FString>& OutEnhancementEffects)
{
	if (JsonData.IsEmpty())
	{
		return false;
	}
	
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonData);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to deserialize equipment enhancement JSON data: %s"), *JsonData);
		return false;
	}
	
	// Parse enhancement level
	double Level;
	if (JsonObject->TryGetNumberField(TEXT("Level"), Level))
	{
		OutEnhancementLevel = static_cast<int32>(Level);
	}
	
	// Parse effects array
	const TArray<TSharedPtr<FJsonValue>>* EffectsArray;
	if (JsonObject->TryGetArrayField(TEXT("Effects"), EffectsArray))
	{
		OutEnhancementEffects.Empty();
		for (const auto& Effect : *EffectsArray)
		{
			FString EffectString;
			if (Effect->TryGetString(EffectString))
			{
				OutEnhancementEffects.Add(EffectString);
			}
		}
	}
	
	return true;
}