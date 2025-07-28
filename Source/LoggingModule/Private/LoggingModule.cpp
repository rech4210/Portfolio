// @Needmodifi
#include "LoggingModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FLoggingModule"

void FLoggingModule::StartupModule()
{
}

void FLoggingModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FLoggingModule, LoggingModule) 