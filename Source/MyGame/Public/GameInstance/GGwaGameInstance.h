// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GGwaGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API UGGwaGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UGGwaGameInstance();
	virtual void Init() override;

	FString& GetGameWorldURL() { return GameWorldURL; }
	FString& GetLoginWorldURL() { return LoginWorldURL; }

private:
	void HandleMapLoading();
	UPROPERTY()
	FString GameWorldURL = TEXT("/Game/Maps/ThirdPersonMap?listen");
	UPROPERTY()
	FString LoginWorldURL = TEXT("/Game/Maps/LoginMap");
};
