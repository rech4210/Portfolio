// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interface/IDBProviderInfra.h"
#include "Provider/EquipDBProvider.h"
#include "Provider/ShopDBProvider.h"
#include "Provider/SkillDBProvider.h"
#include "Provider/InventoryDBProvider.h"
#include "Provider/AuthDBProvider.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DBProviderInfra.generated.h"

/**
 * 
 */
UCLASS()
class DATABASEMODULE_API UDBProviderInfra : public UGameInstanceSubsystem , public IDBProviderInfra{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual TSharedPtr<IEquipDBProvider> GetEquipDbProvider()override {return EquipProvider;}
	virtual TSharedPtr<IShopDBProvider> GetShopDbProvider() override {return ShopProvider;}
	virtual TSharedPtr<ISkillDBProvider> GetSkillDbProvider() override {return SkillProvider;}
	virtual TSharedPtr<IInventoryDBProvider> GetInventoryDbProvider() override {return InventoryProvider;}
	virtual TSharedPtr<IAuthDBProvider> GetAuthDbProvider() override {return AuthProvider;}
private:
	TSharedPtr<IEquipDBProvider> EquipProvider;
	TSharedPtr<IShopDBProvider> ShopProvider;
	TSharedPtr<ISkillDBProvider> SkillProvider;
	TSharedPtr<IInventoryDBProvider> InventoryProvider;
	TSharedPtr<IAuthDBProvider> AuthProvider;
};