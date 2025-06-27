  #include "AuthServerModule.h"

  #define LOCTEXT_NAMESPACE "FAuthServerModule"

  void FAuthServerModule::StartupModule()
  {
      UE_LOG(LogTemp, Log, TEXT("AuthServerModule has started."));
  }

  void FAuthServerModule::ShutdownModule()
  {
      UE_LOG(LogTemp, Log, TEXT("AuthServerModule has shut down."));
  }

  #undef LOCTEXT_NAMESPACE

  IMPLEMENT_MODULE(FAuthServerModule, AuthServerModule)