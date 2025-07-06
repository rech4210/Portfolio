// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IShopRepositoryInterface.h"
#include "ShopSubsystem.generated.h"

class UShopRepository;
class APlayerState;
class UShopComponent;

/**
 * 
 */
UCLASS()
class SHOPMODULE_API UShopSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	TScriptInterface<IShopRepositoryInterface> GetShopRepository() const;

	/** Entry point for loading shop data. Can be called from PlayerState's BeginPlay. */
	void RequestLoadShopData(APlayerState* PlayerState);

	/** Called by the ShopComponent on clients when shop data is replicated. */
	void Client_OnShopStateUpdated(UShopComponent* ShopComponent);

private:
	UPROPERTY()
	UShopRepository* ShopRepository;
};
