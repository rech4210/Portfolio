// ServerModule/GameMode/ServerGameMode.cpp

#include "GGwaGameMode.h"

#include "HttpModule.h"
#include "GameMode/BattleFlowController.h"

AGGwaGameMode::AGGwaGameMode() {
	
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
