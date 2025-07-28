#include "GameSharedModule.h"
#include "Modules/ModuleManager.h"
#include "Utill/UEnumTagMatchHelper.h"

#define LOCTEXT_NAMESPACE "FGameSharedModule"

void FGameSharedModule::StartupModule()
{
	UEnumTagMatchHelper::InitializeHelper();
}

void FGameSharedModule::ShutdownModule()
{
	UEnumTagMatchHelper::Shutdown();
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGameSharedModule, GameSharedModule) 