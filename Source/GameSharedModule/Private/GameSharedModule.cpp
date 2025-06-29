#include "GameSharedModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FGameSharedModule"

void FGameSharedModule::StartupModule()
{
}

void FGameSharedModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FGameSharedModule, GameSharedModule) 