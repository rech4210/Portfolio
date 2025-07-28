#include "AuthModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FAuthModule"

void FAuthModule::StartupModule()
{
}
void FAuthModule::ShutdownModule()
{
}
#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FAuthModule, AuthModule);

