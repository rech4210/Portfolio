#include "ClientModule.h"

#define LOCTEXT_NAMESPACE "FClientModuleModule"

void FClientModuleModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("ClientModule has started!"));
}

void FClientModuleModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FClientModuleModule, ClientModule)