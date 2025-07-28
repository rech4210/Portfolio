// @Needmodifi`r`n// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DatabaseManager.h"
#include "InventoryItemData.h"
#include "InventoryDomain.generated.h"

/**
 * Pure domain object for inventory data - no engine dependencies
 * Used for Repository layer to maintain separation of concerns
 */
USTRUCT(BlueprintType)
struct INVENTORYMODULE_API FInventoryDomain
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FGuid PlayerId;

	UPROPERTY(BlueprintReadOnly)
	TArray<FInventoryItemDTO> Items;

	FInventoryDomain() = default;
	
	FInventoryDomain(const FGuid& InPlayerId, const TArray<FInventoryItemDTO>& InItems)
		: PlayerId(InPlayerId), Items(InItems)
	{
	}

	// Domain validation methods
	bool IsValid() const 
	{ 
		return PlayerId.IsValid(); 
	}

	bool HasItem(const FName& ItemID) const
	{
		return Items.ContainsByPredicate([&ItemID](const FInventoryItemDTO& Item)
		{
			return Item.ItemID == ItemID;
		});
	}

	int32 GetItemQuantity(const FName& ItemID) const
	{
		const FInventoryItemDTO* Found = Items.FindByPredicate([&ItemID](const FInventoryItemDTO& Item)
		{
			return Item.ItemID == ItemID;
		});
		return Found ? Found->Quantity : 0;
	}
};

/**
 * Repository result wrapper for async operations
 */
USTRUCT(BlueprintType)
struct INVENTORYMODULE_API FInventoryRepositoryResult
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly)
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly)
	FInventoryDomain InventoryData;

	static FInventoryRepositoryResult Success(const FInventoryDomain& Data)
	{
		FInventoryRepositoryResult Result;
		Result.bSuccess = true;
		Result.InventoryData = Data;
		return Result;
	}

	static FInventoryRepositoryResult Failure(const FString& Error)
	{
		FInventoryRepositoryResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = Error;
		return Result;
	}
};
