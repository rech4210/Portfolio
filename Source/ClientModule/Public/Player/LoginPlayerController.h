// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AuthClientModule/Public/AuthService.h"
#include "LoginPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CLIENTMODULE_API ALoginPlayerController : public APlayerController {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Login")
	void RequestLogin(const FString& UserID, const FString& Password);

	/** 토큰 요청 성공 시 호출될 콜백 함수 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Login")
	void OnLoginSuccess_BP(const FString& Token);
	
	/** 토큰 요청 실패 시 호출될 콜백 함수 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="Login")
	void OnLoginFailure_BP(const FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, Category="Login")
	void ClientTravel_BP(const FString& HostURL ,const FString& Token, ETravelType TravelType);
protected:
	virtual void BeginPlay() override;

	
private:
	// void OnLoginSuccess(const FString& Token);
	// void OnLoginFailure(const FString& ErrorMessage);

	/** 화면에 표시할 로그인 UI 위젯 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Login", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> LoginWidgetClass;

	// 생성된 로그인 위젯 인스턴스
	UPROPERTY()
	TObjectPtr<UUserWidget> LoginWidgetInstance;

	// 인증 서비스를 담을 변수
	UPROPERTY()
	TObjectPtr<UAuthService> AuthService;
};
