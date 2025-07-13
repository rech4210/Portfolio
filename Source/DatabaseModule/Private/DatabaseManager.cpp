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

void UDatabaseManager::LoadCharacterInfo(int32 UserId, FCharacterDataLoadDelegate Delegate)
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
				TUniquePtr<sql::PreparedStatement> Pstmt(Con->prepareStatement("SELECT character_id, level, exp, json_data FROM characters WHERE user_id = ?"));
				Pstmt->setInt(1, UserId);

				TUniquePtr<sql::ResultSet> Res(Pstmt->executeQuery());
				if (Res->next())
				{
					FCharacterData LoadedData;
					LoadedData.UserId = UserId;
					LoadedData.CharacterId = Res->getInt("character_id");
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
		{
			try
			{
				TUniquePtr<sql::PreparedStatement> Pstmt(Con->prepareStatement(
					"INSERT INTO characters (user_id, character_name, level, exp, json_data) "
					"VALUES (?, ?, ?, ?, ?) "
					"ON DUPLICATE KEY UPDATE level=VALUES(level), exp=VALUES(exp), json_data=VALUES(json_data), character_name=VALUES(character_name)"
				));

				// Use a temporary variable for the character name for now.
				std::string CharacterName = "DefaultCharacter";

				Pstmt->setInt(1, CharacterData.UserId);
				Pstmt->setString(2, CharacterName);
				Pstmt->setInt(3, CharacterData.Level);
				Pstmt->setInt64(4, CharacterData.Exp);
				Pstmt->setString(5, TCHAR_TO_UTF8(*CharacterData.JsonData));
				
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

UE::Tasks::TTask<TArray<FInventoryItemDTO>> UDatabaseManager::LoadInventoryForPlayer(int32 PlayerId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId]() -> TArray<FInventoryItemDTO>
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
				"SELECT item_id, quantity, item_data FROM inventory WHERE player_id = ?"
			));
			Pstmt->setInt(1, PlayerId);

			TUniquePtr<sql::ResultSet> Res(Pstmt->executeQuery());
			while (Res->next())
			{
				FInventoryItemDTO Item;
				Item.ItemID = FName(UTF8_TO_TCHAR(Res->getString("item_id").c_str()));
				Item.Quantity = Res->getInt("quantity");
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

UE::Tasks::TTask<bool> UDatabaseManager::SaveInventoryForPlayer(int32 PlayerId, const TArray<FInventoryItemDTO>& Items)
{
	return WithTransaction([PlayerId, Items](sql::Connection* Con) -> bool
	{
		try
		{
			// Clear existing inventory
			TUniquePtr<sql::PreparedStatement> DeleteStmt(Con->prepareStatement(
				"DELETE FROM inventory WHERE player_id = ?"
			));
			DeleteStmt->setInt(1, PlayerId);
			DeleteStmt->executeUpdate();

			// Insert new items
			TUniquePtr<sql::PreparedStatement> InsertStmt(Con->prepareStatement(
				"INSERT INTO inventory (player_id, item_id, quantity, item_data) VALUES (?, ?, ?, ?)"
			));

			for (const FInventoryItemDTO& Item : Items)
			{
				InsertStmt->setInt(1, PlayerId);
				InsertStmt->setString(2, TCHAR_TO_UTF8(*Item.ItemID.ToString()));
				InsertStmt->setInt(3, Item.Quantity);
				InsertStmt->setString(4, TCHAR_TO_UTF8(*Item.ItemData));
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

UE::Tasks::TTask<bool> UDatabaseManager::AddInventoryItem(int32 PlayerId, const FInventoryItemDTO& Item)
{
	return WithTransaction([PlayerId, Item](sql::Connection* Con) -> bool
	{
		try
		{
			// Try to update existing item first
			TUniquePtr<sql::PreparedStatement> UpdateStmt(Con->prepareStatement(
				"UPDATE inventory SET quantity = quantity + ? WHERE player_id = ? AND item_id = ?"
			));
			UpdateStmt->setInt(1, Item.Quantity);
			UpdateStmt->setInt(2, PlayerId);
			UpdateStmt->setString(3, TCHAR_TO_UTF8(*Item.ItemID.ToString()));
			
			int32 UpdatedRows = UpdateStmt->executeUpdate();
			
			// If no rows were updated, insert new item
			if (UpdatedRows == 0)
			{
				TUniquePtr<sql::PreparedStatement> InsertStmt(Con->prepareStatement(
					"INSERT INTO inventory (player_id, item_id, quantity, item_data) VALUES (?, ?, ?, ?)"
				));
				InsertStmt->setInt(1, PlayerId);
				InsertStmt->setString(2, TCHAR_TO_UTF8(*Item.ItemID.ToString()));
				InsertStmt->setInt(3, Item.Quantity);
				InsertStmt->setString(4, TCHAR_TO_UTF8(*Item.ItemData));
				InsertStmt->executeUpdate();
			}

			return true;
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("AddInventoryItem failed: %hs"), e.what());
			return false;
		}
	}, TEXT("Inventory/AddItem"));
}

UE::Tasks::TTask<bool> UDatabaseManager::RemoveInventoryItem(int32 PlayerId, const FName& ItemID, int32 Quantity)
{
	return WithTransaction([PlayerId, ItemID, Quantity](sql::Connection* Con) -> bool
	{
		try
		{
			// Update quantity, but don't let it go below 0
			TUniquePtr<sql::PreparedStatement> UpdateStmt(Con->prepareStatement(
				"UPDATE inventory SET quantity = GREATEST(0, quantity - ?) WHERE player_id = ? AND item_id = ?"
			));
			UpdateStmt->setInt(1, Quantity);
			UpdateStmt->setInt(2, PlayerId);
			UpdateStmt->setString(3, TCHAR_TO_UTF8(*ItemID.ToString()));
			
			int32 UpdatedRows = UpdateStmt->executeUpdate();
			
			// Remove items with 0 quantity
			TUniquePtr<sql::PreparedStatement> DeleteStmt(Con->prepareStatement(
				"DELETE FROM inventory WHERE player_id = ? AND item_id = ? AND quantity <= 0"
			));
			DeleteStmt->setInt(1, PlayerId);
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

UE::Tasks::TTask<TArray<FSkillSlotDTO>> UDatabaseManager::LoadSkillsForPlayer(int32 PlayerId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId]() -> TArray<FSkillSlotDTO>
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
				"SELECT slot_id, skill_id, last_used_time, remaining_cooldown FROM skill_slots WHERE player_id = ?"
			));
			Stmt->setInt(1, PlayerId);
			
			TUniquePtr<sql::ResultSet> Res(Stmt->executeQuery());
			
			while (Res->next())
			{
				FSkillSlotDTO Skill;
				FString SlotIdString = UTF8_TO_TCHAR(Res->getString("slot_id").c_str());
				FGuid::ParseExact(SlotIdString, EGuidFormats::DigitsWithHyphens, Skill.SlotId);
				Skill.SkillID = Res->getInt("skill_id");
				
				// Parse last used time
				FString LastUsedString = UTF8_TO_TCHAR(Res->getString("last_used_time").c_str());
				FDateTime::ParseIso8601(*LastUsedString, Skill.LastUsedTime);
				
				ResultSkills.Add(Skill);
			}
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("LoadSkillsForPlayer failed: %hs"), e.what());
		}

		return ResultSkills;
	});
}

UE::Tasks::TTask<bool> UDatabaseManager::SaveSkillsForPlayer(int32 PlayerId, const TArray<FSkillSlotDTO>& SkillSlots)
{
	return WithTransaction([PlayerId, SkillSlots](sql::Connection* Con) -> bool
	{
		try
		{
			// Clear existing skills
			TUniquePtr<sql::PreparedStatement> DeleteStmt(Con->prepareStatement(
				"DELETE FROM skill_slots WHERE player_id = ?"
			));
			DeleteStmt->setInt(1, PlayerId);
			DeleteStmt->executeUpdate();

			// Insert new skills
			TUniquePtr<sql::PreparedStatement> InsertStmt(Con->prepareStatement(
				"INSERT INTO skill_slots (player_id, slot_id, skill_id, last_used_time, remaining_cooldown) VALUES (?, ?, ?, ?, ?)"
			));

			for (const FSkillSlotDTO& Skill : SkillSlots)
			{
				InsertStmt->setInt(1, PlayerId);
				InsertStmt->setString(2, TCHAR_TO_UTF8(*Skill.SlotId.ToString(EGuidFormats::DigitsWithHyphens)));
				InsertStmt->setInt(3, Skill.SkillID);
				InsertStmt->setString(4, TCHAR_TO_UTF8(*Skill.LastUsedTime.ToIso8601()));
				InsertStmt->setDouble(5, 0.0); // RemainingCooldown placeholder
				
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

UE::Tasks::TTask<bool> UDatabaseManager::RegisterSkill(int32 PlayerId, const FSkillSlotDTO& SkillSlot)
{
	return WithTransaction([PlayerId, SkillSlot](sql::Connection* Con) -> bool
	{
		try
		{
			TUniquePtr<sql::PreparedStatement> InsertStmt(Con->prepareStatement(
				"INSERT INTO skill_slots (player_id, slot_id, skill_id, last_used_time, remaining_cooldown) VALUES (?, ?, ?, ?, ?) "
				"ON DUPLICATE KEY UPDATE skill_id = VALUES(skill_id), last_used_time = VALUES(last_used_time)"
			));
			
			InsertStmt->setInt(1, PlayerId);
			InsertStmt->setString(2, TCHAR_TO_UTF8(*SkillSlot.SlotId.ToString(EGuidFormats::DigitsWithHyphens)));
			InsertStmt->setInt(3, SkillSlot.SkillID);
			InsertStmt->setString(4, TCHAR_TO_UTF8(*SkillSlot.LastUsedTime.ToIso8601()));
			InsertStmt->setDouble(5, 0.0); // RemainingCooldown placeholder
			
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

UE::Tasks::TTask<bool> UDatabaseManager::UnregisterSkill(int32 PlayerId, const FGuid& SlotId)
{
	return WithTransaction([PlayerId, SlotId](sql::Connection* Con) -> bool
	{
		try
		{
			TUniquePtr<sql::PreparedStatement> DeleteStmt(Con->prepareStatement(
				"DELETE FROM skill_slots WHERE player_id = ? AND slot_id = ?"
			));
			DeleteStmt->setInt(1, PlayerId);
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

UE::Tasks::TTask<bool> UDatabaseManager::UpdateSkillCooldown(int32 PlayerId, const FGuid& SlotId, const FDateTime& LastUsedTime, float RemainingCooldown)
{
	return WithTransaction([PlayerId, SlotId, LastUsedTime, RemainingCooldown](sql::Connection* Con) -> bool
	{
		try
		{
			TUniquePtr<sql::PreparedStatement> UpdateStmt(Con->prepareStatement(
				"UPDATE skill_slots SET last_used_time = ?, remaining_cooldown = ? WHERE player_id = ? AND slot_id = ?"
			));
			UpdateStmt->setString(1, TCHAR_TO_UTF8(*LastUsedTime.ToIso8601()));
			UpdateStmt->setDouble(2, RemainingCooldown);
			UpdateStmt->setInt(3, PlayerId);
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
				"INSERT INTO shops (shop_id, shop_name, is_open, global_price_modifier, shop_owner_name) "
				"VALUES (?, ?, ?, ?, ?) "
				"ON DUPLICATE KEY UPDATE "
				"shop_name = VALUES(shop_name), is_open = VALUES(is_open), "
				"global_price_modifier = VALUES(global_price_modifier), shop_owner_name = VALUES(shop_owner_name)"
			));
			
			ShopStmt->setInt(1, ShopData.ShopID);
			ShopStmt->setString(2, TCHAR_TO_UTF8(*ShopData.ShopName));
			ShopStmt->setBoolean(3, ShopData.bIsOpen);
			ShopStmt->setDouble(4, ShopData.GlobalPriceModifier);
			ShopStmt->setString(5, TCHAR_TO_UTF8(*ShopData.ShopOwnerName));
			
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