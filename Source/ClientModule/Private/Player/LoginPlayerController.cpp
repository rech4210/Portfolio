// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LoginPlayerController.h"
#include "Blueprint/UserWidget.h"


void ALoginPlayerController::BeginPlay() {
	Super::BeginPlay();

	// 게임 인스턴스에서 AuthService 서브시스템을 가져옵니다.
	AuthService = NewObject<UAuthService>(this, TEXT("AuthService"));

	if (!AuthService)
	{
		UE_LOG(LogTemp, Error, TEXT("LoginPlayerController: Failed to get AuthService subsystem!"));
		return;
	}

	// 로컬 컨트롤러일 경우에만 UI를 생성합니다.
	if (IsLocalController() && LoginWidgetClass)
	{
		LoginWidgetInstance = CreateWidget<UUserWidget>(this, LoginWidgetClass);
		if (LoginWidgetInstance)
		{
			LoginWidgetInstance->AddToViewport();
            
			// UI와 상호작용할 수 있도록 마우스 커서를 표시하고 입력 모드를 설정합니다.
			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}
	}
}

void ALoginPlayerController::RequestLogin(const FString& UserID, const FString& Password){
	if (AuthService)
	{
		// 현재 인증 서버는 UserId와 Roles만 필요로 합니다.
		TArray<FString> Roles = { TEXT("player") };

		// 요청 성공/실패 시 호출될 함수들을 델리게이트에 바인딩합니다.
		FLoginSuccessDelegate SuccessDelegate;
		SuccessDelegate.BindUObject(this, &ALoginPlayerController::OnLoginSuccess_BP);

		FLoginFailureDelegate FailureDelegate;
		FailureDelegate.BindUObject(this, &ALoginPlayerController::OnLoginFailure_BP);

		UE_LOG(LogTemp, Log, TEXT("Requesting token for user: %s"), *UserID);
		AuthService->RequestToken(UserID, Roles, SuccessDelegate, FailureDelegate);
	}
	else
	{
		OnLoginFailure_BP(TEXT("AuthService is not available."));
	}
}


void ALoginPlayerController::ClientTravel_BP(const FString& HostURL, const FString& Token, ETravelType TravelType) {
	// 서버가 기대하는 형식으로 토큰 포맷 변경 ("Bearer " 접두사 추가)
	const FString FormattedToken = FString::Printf(TEXT("Bearer %s"), *Token);
	// URL에 토큰을 쿼리 파라미터로 추가
	const FString ConnectURL = FString::Printf(TEXT("%s?token=%s"), *HostURL, *FormattedToken);
	ClientTravel(ConnectURL, ETravelType::TRAVEL_Absolute);
}

//
// void ALoginPlayerController::OnLoginSuccess(const FString& Token){
// 	UE_LOG(LogTemp, Log, TEXT("Login successful. Received token. Traveling to server..."));
//
// 	OnLoginSuccess_BP(Token);
// 	// if (LoginWidgetInstance)
// 	// {
// 	// 	LoginWidgetInstance->RemoveFromParent();
// 	// }
// 	// bShowMouseCursor = false;
// 	// SetInputMode(FInputModeGameOnly());
// 	//
// 	// // 토큰을 포함하여 서버에 접속합니다.
// 	// const FString ConnectURL = FString::Printf(TEXT("127.0.0.1?token=%s"), *Token);
// 	// ClientTravel(ConnectURL, ETravelType::TRAVEL_Absolute);
// }
//
// void ALoginPlayerController::OnLoginFailure(const FString& ErrorMessage) {
// 	UE_LOG(LogTemp, Error, TEXT("Login failed: %s"), *ErrorMessage);
// 	// 이곳에서 로그인 실패 UI를 업데이트하는 로직을 추가할 수 있습니다.
// 	OnLoginFailure_BP(ErrorMessage);
// }
