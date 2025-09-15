
#include "Provider/DBProviderInfra.h"
#include "DatabaseManager.h"
#include "Provider/EquipDBProvider.h"
#include "Provider/InventoryDBProvider.h"
#include "Provider/ShopDBProvider.h"
#include "Provider/SkillDBProvider.h"
#include "Provider/AuthDBProvider.h"

void UDBProviderInfra::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UDatabaseManager* DBmanager = GetGameInstance()->GetSubsystem<UDatabaseManager>();
	if(DBmanager)
	{
		EquipProvider = MakeShared<EquipDBProvider>(DBmanager);
		ShopProvider = MakeShared<ShopDBProvider>(DBmanager);
		SkillProvider = MakeShared<SkillDBProvider>(DBmanager);
		InventoryProvider = MakeShared<InventoryDBProvider>(DBmanager);
		AuthProvider = MakeShared<AuthDBProvider>(DBmanager);
	}
}

void UDBProviderInfra::Deinitialize()
{
	EquipProvider.Reset();
	ShopProvider.Reset();
	SkillProvider.Reset();
	InventoryProvider.Reset();
	AuthProvider.Reset();
	Super::Deinitialize();
}

