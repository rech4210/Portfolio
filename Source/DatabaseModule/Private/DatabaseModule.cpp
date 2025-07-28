// @Needmodifi
#include "DatabaseModule.h"
#include "Data/DatabaseSettings.h"
#include "ISettingsModule.h"
#include "UObject/CoreNet.h"

#define LOCTEXT_NAMESPACE "FDatabaseModuleModule"

void FDatabaseModuleModule::StartupModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Game", "DatabaseSettings",
			LOCTEXT("DatabaseSettingsName", "Database Settings"),
			LOCTEXT("DatabaseSettingsDescription", "Configure settings for the game database connection."),
			GetMutableDefault<UDatabaseSettings>()
		);
	}
}

void FDatabaseModuleModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Game", "DatabaseSettings");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDatabaseModuleModule, DatabaseModule) 