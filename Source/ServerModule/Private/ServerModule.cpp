#include "ServerModule.h"

#define LOCTEXT_NAMESPACE "FServerModuleModule"

void FServerModuleModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("ServerModule has started!"));
}

void FServerModuleModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FServerModuleModule, ServerModule)