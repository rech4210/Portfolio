// ServerModule/GameMode/ServerGameMode.cpp

#include "GGwaGameMode.h"

#include "HttpModule.h"
#include "GameMode/BattleFlowController.h"
#include "AuthVerificationService.h" // 서비스 헤더 Include
#include "GameFramework/PlayerState.h" // APlayerState 사용을 위해
#include "Kismet/GameplayStatics.h"
// #include "DatabaseModule/Public/DatabaseManager.h"
// #include "DatabaseModule/Public/Data/FCharacterData.h"
#include "AuthSubsystem.h"
#include "InventoryDomainService.h"
#include "InventorySubsystem.h"
#include "ShopSubsystem.h"
#include "SkillSubsystem.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerState.h"


#include "Utill/LocalDataBaseLoader.h"

// TODO: The concrete implementation class headers would be here.
// For example: #include "DatabaseModule/Public/MySQLInventoryRepository.h"

constexpr static float GAME_MODE_FREQUENCY = 3.0f;

AGGwaGameMode::AGGwaGameMode()
{
	// 서비스 인스턴스 생성
	AuthVerificationService = NewObject<UAuthVerificationService>(this, TEXT("AuthVerificationService"));


	// TODO: EquipmentRepository의 구체적인 구현 클래스로 초기화해야 합니다.
	// EquipmentRepository = NewObject<UEquipmentRepositoryImpl>(this, TEXT("EquipmentRepository")); 
	
	// TODO: DatabaseManager는 별도의 초기화 방식(예: 싱글톤 또는 서비스 로케이터)을 사용할 수 있습니다.
	// DatabaseManager = ...;
}

void AGGwaGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	UE_LOG(LogTemp, Warning, TEXT("[Game Server] PreLogin attempt"));
	UE_LOG(LogTemp, Warning, TEXT("[Game Server] Connection from address: %s"), *Address);
	UE_LOG(LogTemp, Warning, TEXT("[Game Server] Options: %s"), *Options);
	// UE_LOG(LogTemp, Warning, TEXT("[Game Server] UniqueId: %s"), *UniqueId);

	// Options 문자열에서 토큰 파싱
	// 클라이언트가 OpenLevel 시 "?token=...“ 형태로 보냈다고 가정
	const FString Token = UGameplayStatics::ParseOption(Options, TEXT("token"));
	UE_LOG(LogTemp, Warning, TEXT("Token: %s"), *Token);

	if (Token.IsEmpty())
	{
		ErrorMessage = TEXT("No authentication token provided.");
		UE_LOG(LogTemp, Warning, TEXT("PreLogin failed: %s"), *ErrorMessage);
		return;
	}
	
	FString UserId;
	if (AuthVerificationService->VerifyToken(Token, UserId))
	{
		// 인증 성공!
		UE_LOG(LogTemp, Log, TEXT("PreLogin successful for user: %s"), *UserId);
		// PostLogin에서 맵 이동을 처리하기 위해 플레이어 정보를 임시 저장합니다.
		PendingPlayers.Add(UniqueId, UserId);
	}
	else
	{
		// 인증 실패
		ErrorMessage = TEXT("Invalid authentication token.");
		UE_LOG(LogTemp, Warning, TEXT("PreLogin failed: %s"), *ErrorMessage);
	}
	

}

void AGGwaGameMode::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	static float LogTimer = 0.0f;
	LogTimer += DeltaSeconds;
	if (LogTimer >= GAME_MODE_FREQUENCY)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server is ALIVE. Current GameMode is [ %s ] running in map [ %s ]"), *GetName(), *GetWorld()->GetName());
		LogTimer = 0.0f;
	}
}

void AGGwaGameMode::InitializeServerManagers()
{
	Super::InitializeServerManagers();
	ULocalDataBaseLoader::Initialize();
	if (HasAuthority())
	{
		// BattleFlowController creation
		BattleFlowController = NewObject<UBattleFlowController>(this, TEXT("BattleFlowController"));

		// 레포지토리 초기화는 생성자에서 수행됩니다.
		// 이곳에서는 필요 시 추가적인 초기화 로직을 수행할 수 있습니다.
		
		// After initialization, load all shop data.
		if (TScriptInterface<IShopRepositoryInterface> Repo = GetGameInstance()->GetSubsystem<UShopSubsystem>()->GetShopRepository())
		{
			Repo->LoadShopByID(0);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGGwaGameMode: ShopRepository is not initialized!"));
		}
	}
}

void AGGwaGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	/*로그인 성공 이후, player의 GUID 가져오기*/
	FString PlayerId;
	//TODO: 임시코드, 어떤 필드로 데이터를 인덱싱할건지?
	GetGameInstance()->GetSubsystem<UAuthSubsystem>()->GetDomainService()->GetUserInfo(PlayerId);
	AGGwaPlayerState* State = NewPlayer->GetPlayerState<AGGwaPlayerState>();
	State->SetPlayerGuid(PlayerId);

	

	/*
* 1. 플레이어 초기 설정
*  - GetUserInfo
*  - Skill
*  - Equipment
*  - Inventory
*  - Character
*
*/
	
	UE_LOG(LogTemp, Warning, TEXT("=== PostLogin Start ==="));
	UE_LOG(LogTemp, Warning, TEXT("Checking player validity..."));

	if (!NewPlayer || !NewPlayer->PlayerState){
		UE_LOG(LogTemp, Error, TEXT("[FAIL] NewPlayer or PlayerState is not available on PostLogin!"));
		return;
	}

	if (FString* UserId = PendingPlayers.Find(NewPlayer->PlayerState->GetUniqueId())){
		PendingPlayers.Remove(NewPlayer->PlayerState->GetUniqueId());
	}
	else{
		UE_LOG(LogTemp, Error, TEXT("[FAIL] Player %s is not authenticated. Authentication not found in PendingPlayers."), 
			*NewPlayer->PlayerState->GetPlayerName());
		UE_LOG(LogTemp, Warning, TEXT("[ACTION] Redirecting unauthorized player to login screen"));
		
		NewPlayer->ClientTravel(TEXT("/Game/Login/LoginLevel"), ETravelType::TRAVEL_Absolute);
		// UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), FString::Printf(TEXT("kick %s"), *NewPlayer->PlayerState->GetPlayerName()));
		UE_LOG(LogTemp, Warning, TEXT("=== PostLogin End (Failed) ==="));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Loading inventory..."));
	if (auto InventorySubsystem = GetGameInstance()->GetSubsystem<UInventorySubsystem>())
	{
		UE_LOG(LogTemp, Warning, TEXT("[START] Loading inventory for player %s"), *NewPlayer->PlayerState->GetPlayerName());
		InventorySubsystem->RequestLoadPlayerInventory(NewPlayer->PlayerState);
		UE_LOG(LogTemp, Warning, TEXT("[SUCCESS] Inventory loaded"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[FAIL] InventoryRepository is not available on PostLogin!"));
	}
	
	// 캐릭터 및 스킬 정보 로드 (안전한 접근 방식으로 변경)
	UE_LOG(LogTemp, Warning, TEXT("Loading skills..."));
	if (auto SkillSubsystem = GetGameInstance()->GetSubsystem<USkillSubsystem>())
	{
		SkillSubsystem->RequestLoadPlayerSkills(NewPlayer->PlayerState);
		UE_LOG(LogTemp, Warning, TEXT("[SUCCESS] Loaded %d skill definitions"), LoadedSkillDefinitions.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[FAIL] SkillConfigRepository is not available!"));
	}
	//플레이어가 설정한 스킬에 대한 정보 로드.
	// TArray<int32> SkillList ={100,101,102,103,104,105,106,107};
}

void AGGwaGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	/*	FObjectPtr(const FObjectPtr& InOther)
		: Handle(InOther.Handle)
	{
	null 뜬다. 아마 로그아웃이라?
		ConditionallyMarkAsReachable(*this);
	}*/
	// auto Inventoryrepo = GetGameInstance()->GetSubsystem<UInventorySubsystem>()->GetInventoryRepository();
	// if (Exiting && Inventoryrepo)
	// {
	// 	APlayerController* PC = Cast<APlayerController>(Exiting);
	//
	// 	
	// 	if(PC && PC->PlayerState)
	// 	{
	// 		// Use the repository to save the player's inventory.
	// 		UE_LOG(LogTemp, Log, TEXT("Player logging out. Saving inventory data for %s."), *PC->PlayerState->GetPlayerName());
	// 		if (auto InventorySubsystem = GetGameInstance()->GetSubsystem<UInventorySubsystem>())
	// 		{
	// 			UE_LOG(LogTemp, Warning, TEXT("[START] Save inventory for player %s"), *PC->PlayerState->GetPlayerName());
	// 			// UInventoryDomainService* DomainService = InventorySubsystem->CreateDomainService();
	// 			// PC->PlayerState->GetComponentByClass<UInventoryComponent>()->GetItems();
	// 			// DomainService->SaveInventory(PC->PlayerState, DomainService->LoadInventory(PC->PlayerState).InventoryData);
	// 			// TODO: 저장 로직이 의도된 것인지 확인이 필요합니다. (예: 신규 유저의 기본 상태 저장)
	// 			// SkillStateRepository->SaveSkillState(PC->PlayerState->GetPlayerId(), PlayerState->GetSkillComponent(), TODO);
	// 			
	// 			UE_LOG(LogTemp, Warning, TEXT("[SUCCESS] Inventory Saved"));
	// 		}
	// 		// Unload Repo, Clear Component.
	// 	}
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("AGGwaGameMode: InventoryRepository is not available on Logout!"));
	// }
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

void AGGwaGameMode::RequestFlowControllerInit(EModeType ModeType)
{
	if (HasAuthority() && BattleFlowController)
	{
		BattleFlowController->InitBattleMode(ModeType);
	}
}
