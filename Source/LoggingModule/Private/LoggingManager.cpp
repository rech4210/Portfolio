#include "LoggingManager.h"

void ULoggingManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("ULoggingManager Initialized."));
}

void ULoggingManager::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("ULoggingManager Deinitialized."));
}

void ULoggingManager::LogItemPurchase(const FString& PlayerID, const FString& ItemID, int32 Quantity, int32 Cost)
{
	FString LogMessage = FString::Printf(TEXT("Item Purchase: PlayerID=%s, ItemID=%s, Quantity=%d, Cost=%d"), *PlayerID, *ItemID, Quantity, Cost);
	UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);

	// Example of asynchronous logging:
	// TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	// HttpRequest->SetURL("https://my-logging-service.com/log");
	// HttpRequest->SetVerb("POST");
	// HttpRequest->SetHeader("Content-Type", "application/json");
	// HttpRequest->SetContentAsString(LogMessage);
	// HttpRequest->ProcessRequest();
} 