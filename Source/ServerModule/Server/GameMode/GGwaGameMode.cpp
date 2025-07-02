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
#include "InventoryRepository.h"
#include "IShopRepositoryInterface.h"
#include "ShopRepository.h"
#include "Components/SkillComponent.h"
#include "Data/FCharacterData.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerState.h"
#include "MyGame/Public/Shared/Player/GGwaCharacter.h"
#include "InventoryModule/Public/InventoryRepositoryInterface.h" // Add repository interface
#include "Repositories/SkillConfigRepository.h"
#include "Repositories/SkillStateRepository.h"
#include "ShopModule/Public/IShopRepositoryInterface.h" // Add shop repository interface

// TODO: The concrete implementation class headers would be here.
// For example: #include "DatabaseModule/Public/MySQLInventoryRepository.h"

AGGwaGameMode::AGGwaGameMode()
{
	// 서비스 인스턴스 생성
	AuthVerificationService = NewObject<UAuthVerificationService>(this, TEXT("AuthVerificationService"));
	InventoryRepository = NewObject<UInventoryRepository>(this, TEXT("InventoryRepository"));
	SkillStateRepository = NewObject<USkillStateRepository>(this, TEXT("SkillStateRepository"));
	SkillConfigRepository = NewObject<USkillConfigRepository>(this, TEXT("SkillConfigRepository"));
	ShopRepository = NewObject<UShopRepository>(this, TEXT("ShopRepository"));
	// EquipmentRepository = NewObject<>()
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

void AGGwaGameMode::InitializeServerManagers()
{
	Super::InitializeServerManagers();
	
	if (HasAuthority())
	{
		// BattleFlowController creation
		BattleFlowController = NewObject<UBattleFlowController>(this, TEXT("BattleFlowController"));

		// Initialize Repositories
		// Here, we would create concrete instances of our repository implementations.
		// Since we don't have them yet, this is where they would be instantiated.
		// For example:
		// InventoryRepository = NewObject<UMySQLInventoryRepository>(this);
		// ShopRepository = NewObject<UMySQLShopRepository>(this);
		// SkillConfigRepository = NewObject<UMySkillConfigRepository>(this);
		// SkillStateRepository = NewObject<UMySkillStateRepository>(this);
		
		// After initialization, load all shop data.
		if (ShopRepository)
		{
			ShopRepository->LoadAllShops(this);
		}
	}
}

void AGGwaGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (NewPlayer && InventoryRepository)
	{
		// Use the repository to load the player's inventory.
		// The repository will handle finding the PlayerState and its InventoryComponent.
		
		InventoryRepository->LoadInventoryForPlayer(NewPlayer->PlayerState);
		// EquipmentRepository->LoadPlayerEquipment(NewPlayer);
		auto SkillComponent = Cast<AGGwaCharacter>(NewPlayer->GetCharacter())->GetSkillComponent();
		auto PlayerID = NewPlayer->PlayerState->GetPlayerId();
		// 전체 스킬을 로딩.
		SkillConfigRepository->LoadSkillDefinitions(LoadedSkillDefinitions);
		//8개의 스킬 슬롯에 대한 정보 로드. 정의되지 않은 슬롯의 경우 default, 내부에서 RegisterSkill 호출
		SkillStateRepository->LoadSkillState(PlayerID, SkillComponent);
		SkillStateRepository->SaveSkillState(PlayerID, SkillComponent);
		// Skill 구성의 초기화를 담당. by job type
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGGwaGameMode: InventoryRepository is not available on PostLogin!"));
	}
}

void AGGwaGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (Exiting && InventoryRepository)
	{
		APlayerController* PC = Cast<APlayerController>(Exiting);
		if(PC && PC->PlayerState)
		{
			// Use the repository to save the player's inventory.
			UE_LOG(LogTemp, Log, TEXT("Player logging out. Saving inventory data for %s."), *PC->PlayerState->GetPlayerName());
			if (auto PlayerCharacter = Cast<AGGwaCharacter>(PC->GetCharacter())) {
				InventoryRepository->SaveInventoryForPlayer(PC->PlayerState);
				// EquipmentRepository->SavePlayerEquipment(PC);

				SkillStateRepository->SaveSkillState(PC->PlayerState->GetPlayerId(), PlayerCharacter->GetSkillComponent());
			}
			// Unload Repo, Clear Component.
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGGwaGameMode: InventoryRepository is not available on Logout!"));
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

void AGGwaGameMode::RequestFlowControllerInit(EModeType ModeType)
{
	if (HasAuthority() && BattleFlowController)
	{
		BattleFlowController->InitBattleMode(ModeType);
	}
}
