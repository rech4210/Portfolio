
#include "InventoryRepository.h"
#include "InventoryDomain.h"
#include "Tasks/Task.h"
#include "Async/Async.h"
#include "GameSharedModule/Public/Interface/IDBProviderInfra.h"
#include "GameSharedModule/Public/Interface/Provider/IInventoryDBProvider.h"

using namespace UE::Tasks;

void UInventoryRepository::Initialize(IDBProviderInfra* Infra) 
{
	if (!Infra)
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryRepository Initialize: Infra is null"));
		return;
	}
	InventoryProvider = Infra->GetInventoryDbProvider();
	if (!InventoryProvider.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryRepository: InventoryProvider is not available!"));
	}
}

// ============================================================================
// PURE REPOSITORY METHODS - NO ENGINE DEPENDENCIES
// ============================================================================

UE::Tasks::TTask<FInventoryRepositoryResult> UInventoryRepository::LoadInventoryByPlayerId(const FGuid& PlayerId)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId]() -> FInventoryRepositoryResult
	{
		if (!InventoryProvider.IsValid())
		{
			return FInventoryRepositoryResult::Failure(TEXT("InventoryProvider not available"));
		}

		FString UserId = PlayerId.ToString();

		auto LoadTask = InventoryProvider->LoadInventoryForPlayer(UserId);
		TArray<FInventoryItemDTO> LoadedItems = LoadTask.GetResult();
		FInventoryDomain InventoryData(PlayerId, LoadedItems);
		return FInventoryRepositoryResult::Success(InventoryData);
	});
}

UE::Tasks::TTask<FInventoryRepositoryResult> UInventoryRepository::SaveInventoryData(const FInventoryDomain& InventoryData)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, InventoryData]() -> FInventoryRepositoryResult
	{
		if (!InventoryProvider.IsValid())
		{
			return FInventoryRepositoryResult::Failure(TEXT("InventoryProvider not available"));
		}

		if (!InventoryData.IsValid())
		{
			return FInventoryRepositoryResult::Failure(TEXT("Invalid inventory data"));
		}

		// Convert FGuid PlayerId to FString UserId using helper
		FString UserId = InventoryData.PlayerId.ToString();

		// Execute database operation on worker thread
		auto SaveTask = InventoryProvider->SaveInventoryForPlayer(UserId, InventoryData.Items);
		bool bSuccess = SaveTask.GetResult();

		if (bSuccess)
		{
			return FInventoryRepositoryResult::Success(InventoryData);
		}
		else
		{
			return FInventoryRepositoryResult::Failure(TEXT("Failed to save inventory to database"));
		}
	});
}

UE::Tasks::TTask<FInventoryRepositoryResult> UInventoryRepository::AddItemByPlayerId(
	const FGuid& PlayerId, const FInventoryItemDTO& Item)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, Item]() -> FInventoryRepositoryResult {
		if (!InventoryProvider.IsValid())
		{
			return FInventoryRepositoryResult::Failure(TEXT("InventoryProvider not available"));
		}
		
		FString UserId = PlayerId.ToString();
		
		auto bSuccess = InventoryProvider->AddInventoryItem(UserId, Item).GetResult();
		if (bSuccess) {
			FInventoryRepositoryResult& Result = LoadInventoryByPlayerId(PlayerId).GetResult();
			return FInventoryRepositoryResult::Success(Result.InventoryData);
		}
		
		auto PrerequisitesTask = InventoryProvider->AddInventoryItem(UserId, Item);
		Launch(UE_SOURCE_LOCATION, [this, PlayerId, PrerequisitesTask]() mutable ->FInventoryRepositoryResult {
			auto Ok = PrerequisitesTask.GetResult();
			if (Ok) {
				TTask<FInventoryRepositoryResult> Result = LoadInventoryByPlayerId(PlayerId);
				return FInventoryRepositoryResult::Success(Result.GetResult().InventoryData);
			}
			else {
				return FInventoryRepositoryResult::Failure(TEXT("Failed to load inventory"));
			}
		},PrerequisitesTask);
		
		return FInventoryRepositoryResult::Failure(TEXT("Failed to add inventory item"));
	});
}

UE::Tasks::TTask<FInventoryRepositoryResult> UInventoryRepository::RemoveItemByPlayerId(
	const FGuid& PlayerId, const FName& ItemID, int32 Quantity)
{
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, PlayerId, ItemID, Quantity]() -> FInventoryRepositoryResult
	{
		if (!InventoryProvider.IsValid())
		{
			return FInventoryRepositoryResult::Failure(TEXT("InventoryProvider not available"));
		}

		FString UserId = PlayerId.ToString();

		auto RemoveTask = InventoryProvider->RemoveInventoryItem(UserId, ItemID, Quantity);
		bool bSuccess = RemoveTask.GetResult();

		if (bSuccess)
		{
			auto ReloadTask = LoadInventoryByPlayerId(PlayerId);
			return ReloadTask.GetResult();
		}
		return FInventoryRepositoryResult::Failure(TEXT("Failed to remove item from database"));
	});
}
