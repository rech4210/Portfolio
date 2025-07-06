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

private:
	void HandleMapLoading();
};
