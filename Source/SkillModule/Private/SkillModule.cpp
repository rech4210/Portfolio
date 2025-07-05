#include "SkillModule.h"
#include "Modules/ModuleManager.h"
#include "Utill/LocalDataBaseLoader.h"

#define LOCTEXT_NAMESPACE "FSkillModule"

void FSkillModule::StartupModule()
{
	// ULocalDataBaseLoader::Initialize();
}

void FSkillModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSkillModule, SkillModule) 