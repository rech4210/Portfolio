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
				UE_LOG(LogTemp, Error, TEXT("SQLException during connection close: %s"), UTF8_TO_TCHAR(e.what()));
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
				UE_LOG(LogTemp, Warning, TEXT("Failed to validate pooled connection, creating new one. Error: %s"), UTF8_TO_TCHAR(e.what()));
				delete Con;
			}
		}
		
		try
		{
			return Driver->connect(ConnectionProperties);
		}
		catch (const sql::SQLException& e)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create new connection: %s"), UTF8_TO_TCHAR(e.what()));
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
			Impl->ConnectionPool.Enqueue(Impl->Driver->connect(Impl->ConnectionProperties));
		}
		UE_LOG(LogTemp, Log, TEXT("Database connection pool initialized with %d connections."), Impl->PoolSize);
	}
	catch (const sql::SQLException &e)
	{
		UE_LOG(LogTemp, Fatal, TEXT("Failed to initialize database connection: %s"), UTF8_TO_TCHAR(e.what()));
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
				UE_LOG(LogTemp, Error, TEXT("LoadCharacterInfo failed: %s"), UTF8_TO_TCHAR(e.what()));
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
				UE_LOG(LogTemp, Error, TEXT("SaveCharacterInfo failed: %s"), UTF8_TO_TCHAR(e.what()));
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