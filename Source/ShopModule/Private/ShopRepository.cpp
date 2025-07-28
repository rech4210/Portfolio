#include "ShopRepository.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Components/ShopComponent.h"
#include "ShopDomain.h"
#include "Engine/World.h"
#include "Tasks/Task.h"
#include "Async/Async.h"

void UShopRepository::Initialize()
{
	// Get DatabaseManager from GameInstance subsystem
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			DBManager = GameInstance->GetSubsystem<UDatabaseManager>();
			if (!DBManager)
			{
				UE_LOG(LogTemp, Error, TEXT("ShopRepository: Failed to get DatabaseManager subsystem"));
			}
		}
	}
}

// ============================================================================
// DDD-based methods (pure domain operations)
// ============================================================================

UE::Tasks::TTask<FShopRepositoryResult> UShopRepository::LoadShopByID(int32 ShopID)
{
	if (!DBManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository: DBManager is null"));
		return UE::Tasks::MakeCompletedTask<FShopRepositoryResult>(FShopRepositoryResult{false, TEXT("DBManager is null"), FShopDomain{}});
	}

	// Execute database operation using the same pattern as SkillDomainService
	auto LoadTask = DBManager->LoadShopByID(ShopID);
	
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
	if (!DBManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository: DBManager is null"));
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	auto SaveTask = DBManager->SaveShop(ShopData);
	
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
	if (!DBManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository: DBManager is null"));
		return UE::Tasks::MakeCompletedTask<TArray<FShopRepositoryResult>>(TArray<FShopRepositoryResult>());
	}

	auto LoadTask = DBManager->LoadShopsByIDs(ShopIDs);
	
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, ShopIDs, LoadTask]() mutable -> TArray<FShopRepositoryResult>
	{
		TArray<FShopRepositoryResult> Results = LoadTask.GetResult();
		UE_LOG(LogTemp, Log, TEXT("ShopRepository: Bulk data access completed for %d shops"), ShopIDs.Num());
		return Results;
	});
}

UE::Tasks::TTask<TArray<FShopRepositoryResult>> UShopRepository::LoadShopsForArea(int32 AreaID)
{
	if (!DBManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository: DBManager is null"));
		return UE::Tasks::MakeCompletedTask<TArray<FShopRepositoryResult>>(TArray<FShopRepositoryResult>());
	}

	auto LoadTask = DBManager->LoadShopsForArea(AreaID);
	
	return UE::Tasks::Launch(UE_SOURCE_LOCATION, [this, AreaID, LoadTask]() mutable -> TArray<FShopRepositoryResult>
	{
		TArray<FShopRepositoryResult> Results = LoadTask.GetResult();
		UE_LOG(LogTemp, Log, TEXT("ShopRepository: Area data access completed for %d"), AreaID);
		return Results;
	});
}

UE::Tasks::TTask<bool> UShopRepository::DeleteShop(int32 ShopID)
{
	if (!DBManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ShopRepository: DBManager is null"));
		return UE::Tasks::MakeCompletedTask<bool>(false);
	}

	auto DeleteTask = DBManager->DeleteShop(ShopID);
	
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
	// 비즈?�스 로직 ?�닌가? check?
	auto CheckTask = DBManager->CheckShopExists(ShopID);
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
