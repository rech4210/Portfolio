#include "ShopRepository.h"
#include "Engine/World.h"
#include "Tasks/Task.h"
#include "Async/Async.h"
#include "GameSharedModule/Public/Interface/IDBProviderInfra.h"
#include "GameSharedModule/Public/Interface/Provider/IShopDBProvider.h"

void UShopRepository::Initialize(IDBProviderInfra* Infra)
{
	if (!Infra)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository Initialize: Infra is null"));
		return;
	}
	ShopProvider = Infra->GetShopDbProvider();
	if (!ShopProvider.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository: ShopProvider is not available"));
	}
}

// ============================================================================
// DDD-based methods (pure domain operations)
// ============================================================================

UE::Tasks::TTask<FShopRepositoryResult> UShopRepository::LoadShopByID(int32 ShopID)
{
	if (!ShopProvider.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository: ShopProvider is null"));
		return UE::Tasks::MakeCompletedTask<FShopRepositoryResult>(FShopRepositoryResult{false, TEXT("DBManager is null"), FShopDomain{}});
	}
	auto LoadTask = ShopProvider->LoadShopByID(ShopID);
	
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, LoadTask]() mutable -> FShopRepositoryResult
	{
		FShopRepositoryResult Result = LoadTask.GetResult();
		
		if (Result.bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("ShopRepository: Data access completed for shop %d"), ShopID);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopRepository: Failed to load shop %d: %s"), ShopID, *Result.ErrorMessage);
		}
		return Result;
	});
}

UE::Tasks::TTask<bool> UShopRepository::SaveShop(const FShopDomain& ShopData)
{
	if (!ShopProvider.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository: ShopProvider is null"));
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	auto SaveTask = ShopProvider->SaveShop(ShopData);
	
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopData, SaveTask]() mutable -> bool
	{
		bool bResult = SaveTask.GetResult();
		
		if (bResult)
		{
			UE_LOG(LogTemp, Log, TEXT("ShopRepository: Shop %d saved successfully"), ShopData.ShopID);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopRepository: Failed to save shop %d"), ShopData.ShopID);
		}
		return bResult;
	});
}

UE::Tasks::TTask<TArray<FShopRepositoryResult>> UShopRepository::LoadShopsByIDs(const TArray<int32>& ShopIDs)
{
	if (!ShopProvider.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository: ShopProvider is null"));
		return UE::Tasks::MakeCompletedTask<TArray<FShopRepositoryResult>>(TArray<FShopRepositoryResult>());
	}

	auto LoadTask = ShopProvider->LoadShopsByIDs(ShopIDs);
	
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopIDs, LoadTask]() mutable -> TArray<FShopRepositoryResult>
	{
		TArray<FShopRepositoryResult> Results = LoadTask.GetResult();
		UE_LOG(LogTemp, Log, TEXT("ShopRepository: Bulk data access completed for %d shops"), ShopIDs.Num());
		return Results;
	});
}

UE::Tasks::TTask<TArray<FShopRepositoryResult>> UShopRepository::LoadShopsForArea(int32 AreaID)
{
	if (!ShopProvider.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository: ShopProvider is null"));
		return UE::Tasks::MakeCompletedTask<TArray<FShopRepositoryResult>>(TArray<FShopRepositoryResult>());
	}

	auto LoadTask = ShopProvider->LoadShopsForArea(AreaID);
	
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, AreaID, LoadTask]() mutable -> TArray<FShopRepositoryResult>
	{
		TArray<FShopRepositoryResult> Results = LoadTask.GetResult();
		UE_LOG(LogTemp, Log, TEXT("ShopRepository: Area data access completed for %d"), AreaID);
		return Results;
	});
}

UE::Tasks::TTask<bool> UShopRepository::DeleteShop(int32 ShopID)
{
	if (!ShopProvider.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository: ShopProvider is null"));
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	auto DeleteTask = ShopProvider->DeleteShop(ShopID);
	
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, DeleteTask]() mutable -> bool
	{
		bool bResult = DeleteTask.GetResult();
		
		if (bResult)
		{
			UE_LOG(LogTemp, Log, TEXT("ShopRepository: Shop %d deleted successfully"), ShopID);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopRepository: Failed to delete shop %d"), ShopID);
		}
		return bResult;
	});
}

UE::Tasks::TTask<bool> UShopRepository::ShopExists(int32 ShopID) {
	auto CheckTask = ShopProvider->CheckShopExists(ShopID);
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopID, CheckTask]() mutable -> bool
	{
		bool bExists = CheckTask.GetResult();
		
		if (bExists)
		{
			UE_LOG(LogTemp, Log, TEXT("ShopRepository: Shop %d exists"), ShopID);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ShopRepository: Shop %d does not exist"), ShopID);
		}
		return bExists;
	});
}
