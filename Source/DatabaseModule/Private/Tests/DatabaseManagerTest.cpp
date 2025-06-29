#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "DatabaseModule/Public/Data/FCharacterData.h"
#include "Tests/AutomationCommon.h"

// Latent command for waiting on an async task
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitForDatabaseDelegate, FEvent*, EventToTrigger);
bool FWaitForDatabaseDelegate::Update()
{
	return EventToTrigger->Wait(0);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDatabaseManagerTest, "MyGame.Database.Manager.SaveAndLoad",EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FDatabaseManagerTest::RunTest(const FString& Parameters)
{
	// Find the Game Instance, which is needed to get the subsystem

	UGameInstance* GameInstance = GEngine->GetWorld()->GetGameInstance();
	if (!TestNotNull(TEXT("GameInstance should not be null"), GameInstance)) return false;

	// Get the DatabaseManager subsystem
	UDatabaseManager* DBManager = GameInstance->GetSubsystem<UDatabaseManager>();
	if (!TestNotNull(TEXT("DatabaseManager should not be null"), DBManager)) return false;
	
	// --- Test Data ---
	const int32 TestUserID = 9999; // Use a dedicated ID for testing
	FCharacterData DataToSave;
	DataToSave.UserId = TestUserID;
	DataToSave.CharacterId = 1;
	DataToSave.Level = 10;
	DataToSave.Exp = 12345;
	DataToSave.JsonData = TEXT("{\"weapon\":\"sword\",\"armor\":\"plate\"}");

	// --- Latent Test Logic ---
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));

	// 1. Save Character Info
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, DBManager, DataToSave]()
	{
		FEvent* SaveEvent = FPlatformProcess::GetSynchEventFromPool();
		FCharacterDataSaveDelegate SaveDelegate = FCharacterDataSaveDelegate::CreateLambda([this, SaveEvent](bool bSuccess)
		{
			TestTrue(TEXT("Save operation should be successful"), bSuccess);
			SaveEvent->Trigger();
		});

		DBManager->SaveCharacterInfo(DataToSave, SaveDelegate);
		
		// This command will block until the delegate triggers the event.
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForDatabaseDelegate(SaveEvent));
		return true;
	}));
	
	// 2. Load Character Info
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, DBManager, TestUserID, DataToSave]()
	{
		FEvent* LoadEvent = FPlatformProcess::GetSynchEventFromPool();
		FCharacterDataLoadDelegate LoadDelegate = FCharacterDataLoadDelegate::CreateLambda([this, LoadEvent, DataToSave](const TOptional<FCharacterData>& LoadedDataOpt)
		{
			if (TestTrue(TEXT("Loaded data should be present"), LoadedDataOpt.IsSet()))
			{
				const FCharacterData& LoadedData = LoadedDataOpt.GetValue();
				TestEqual(TEXT("UserID should match"), LoadedData.UserId, DataToSave.UserId);
				TestEqual(TEXT("Level should match"), LoadedData.Level, DataToSave.Level);
				TestEqual(TEXT("Exp should match"), LoadedData.Exp, DataToSave.Exp);
				TestEqual(TEXT("JsonData should match"), LoadedData.JsonData, DataToSave.JsonData);
			}
			LoadEvent->Trigger();
		});

		DBManager->LoadCharacterInfo(TestUserID, LoadDelegate);

		// This command will block until the delegate triggers the event.
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForDatabaseDelegate(LoadEvent));
		return true;
	}));

	return true;
} 