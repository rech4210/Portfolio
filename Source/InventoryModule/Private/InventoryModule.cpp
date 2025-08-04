#include "InventoryModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FInventoryModule"

void FInventoryModule::StartupModule()
{
}

void FInventoryModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FInventoryModule, InventoryModule) 