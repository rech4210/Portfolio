// ServerModule/GameMode/ServerGameMode.cpp

#include "GGwaGameMode.h"

#include "HttpModule.h"
#include "GameMode/BattleFlowController.h"
#include "AuthVerificationService.h" // 서비스 헤더 Include
#include "GameFramework/PlayerState.h" // APlayerState 사용을 위해
#include "Kismet/GameplayStatics.h"
// #include "DatabaseModule/Public/DatabaseManager.h"
// #include "DatabaseModule/Public/Data/FCharacterData.h"
#include "DatabaseManager.h"
#include "Data/FCharacterData.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerState.h"
#include "MyGame/Public/Shared/Player/GGwaCharacter.h"
#include "InventoryModule/Public/InventoryComponent.h" // 인벤토리 컴포넌트 헤더 추가

AGGwaGameMode::AGGwaGameMode()
{
	// 서비스 인스턴스 생성
	AuthVerificationService = CreateDefaultSubobject<UAuthVerificationService>(TEXT("AuthVerificationService"));
}

void AGGwaGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	// Options 문자열에서 토큰 파싱
	// 클라이언트가 OpenLevel 시 "?token=...“ 형태로 보냈다고 가정
	const FString Token = UGameplayStatics::ParseOption(Options, TEXT("token"));

	if (Token.IsEmpty())
	{
		ErrorMessage = TEXT("No authentication token provided.");
		UE_LOG(LogTemp, Warning, TEXT("PreLogin failed: %s"), *ErrorMessage);
		return;
	}
	
	FString UserId;
	if (AuthVerificationService && AuthVerificationService->VerifyToken(Token, UserId))
	{
		// 인증 성공!
		UE_LOG(LogTemp, Log, TEXT("PreLogin successful for user: %s"), *UserId);
		// 필요하다면, 여기서 플레이어의 UniqueNetId를 UserId와 매핑하여 관리할 수 있습니다.
	}
	else
	{
		// 인증 실패
		ErrorMessage = TEXT("Invalid authentication token.");
		UE_LOG(LogTemp, Warning, TEXT("PreLogin failed: %s"), *ErrorMessage);
	}
}

void AGGwaGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (!DatabaseManager)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			DatabaseManager = GameInstance->GetSubsystem<UDatabaseManager>();
		}
	}
	
	if (DatabaseManager)
	{
		// For now, hardcode a user ID. Later, this should come from the verified JWT.
		const int32 UserIdToLoad = 1; 
	
		FCharacterDataLoadDelegate Delegate;
		Delegate.BindUObject(this, &AGGwaGameMode::OnCharacterDataLoaded, NewPlayer);
		DatabaseManager->LoadCharacterInfo(UserIdToLoad, Delegate);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGGwaGameMode: DatabaseManager is not available!"));
	}
}

void AGGwaGameMode::OnCharacterDataLoaded(const TOptional<FCharacterData>& CharacterData, APlayerController* NewPlayer)
{
	if (CharacterData.IsSet() && NewPlayer)
	{
		const FCharacterData& Data = CharacterData.GetValue();
		AGGwaPlayerState* MyPlayerState = NewPlayer->GetPlayerState<AGGwaPlayerState>();
		if (MyPlayerState)
		{
			// 1. Initialize PlayerState with loaded data.
			MyPlayerState->SetPlayerName(FString::FromInt(Data.CharacterId)); 
	
			// 2. Add and initialize InventoryComponent
			UInventoryComponent* InventoryComponent = MyPlayerState->FindComponentByClass<UInventoryComponent>();
			if (!InventoryComponent)
			{
				InventoryComponent = NewObject<UInventoryComponent>(MyPlayerState, "InventoryComponent");
				InventoryComponent->RegisterComponent();
			}
			
			if (InventoryComponent)
			{
				// TODO: DB 로직이 구현되면 이 부분을 FCharacterData에서 읽어온 데이터로 채워야 합니다.
				// For now, we are just adding items based on the data we have.
				// This assumes the DB loading part will populate Data.Inventory.
				// for(const UFInventoryItem& Item : Data.Inventory)
				// {
				// 	InventoryComponent->AddItem(Item.ItemData, Item.Quantity);
				// }
			}
	
			// 3. Initialize attributes and grant abilities based on the loaded FCharacterData.
			UE_LOG(LogTemp, Log, TEXT("Character data loaded for UserId %d. Level: %d, Exp: %d"), Data.UserId, Data.Level, Data.Exp);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load character data for player. Kicking player."));
		// Handle failure, e.g., kick the player.
		if(NewPlayer)
		{
			NewPlayer->ClientTravel(TEXT("/Game/ThirdPerson/Maps/ThirdPersonMap"), ETravelType::TRAVEL_Absolute);
		}
	}
}

void AGGwaGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	UE_LOG(LogTemp, Log, TEXT("Player logging out. Attempting to save data."));
	
	if (DatabaseManager)
	{
		// This is a placeholder. In a real scenario, you would retrieve the
		// FCharacterData from the player's state or a component.
		FCharacterData DataToSave;
		DataToSave.UserId = 1; // Hardcoded to match the PostLogin UserId for now.
		DataToSave.Level = 10; // Example data
		DataToSave.Exp = 5000; // Example data
		DataToSave.JsonData = TEXT("{\"message\":\"Player Logged Out\"}");
	
		FCharacterDataSaveDelegate Delegate;
		Delegate.BindUObject(this, &AGGwaGameMode::OnCharacterDataSaved);
		DatabaseManager->SaveCharacterInfo(DataToSave, Delegate);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGGwaGameMode: DatabaseManager is not available on Logout!"));
	}
}

void AGGwaGameMode::OnCharacterDataSaved(bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Character data saved successfully."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save character data."));
	}
}

void AGGwaGameMode::Server_SkillLog(FString Name, const FString& SkillName, FVector SkillLocation) {
	FHttpModule& Http = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http.CreateRequest();

	Request->SetURL(TEXT("http://localhost:8000/api/skill-log"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	FString Payload = FString::Printf(TEXT(
		"{\"player_id\":\"%s\", \"skill\":\"%s\", \"location\":{\"x\":%.2f, \"y\":%.2f, \"z\":%.2f}}"),
		*Name,
		*SkillName,
		SkillLocation.X, SkillLocation.Y, SkillLocation.Z);

	Request->SetContentAsString(Payload);
	Request->ProcessRequest();
}

void AGGwaGameMode::InitializeServerManagers()
{
	if (HasAuthority())
	{
		// BattleFlowController 생성
		BattleFlowController = NewObject<UBattleFlowController>(this,
																 TEXT("BattleFlowController"));
		// 추가 서버 매니저 초기화 (RoomInit, PVPManager 등)을 여기에…
	}
}


void AGGwaGameMode::RequestFlowControllerInit(EModeType ModeType)
{
	if (HasAuthority() && BattleFlowController)
	{
		BattleFlowController->InitBattleMode(ModeType);
	}
}
