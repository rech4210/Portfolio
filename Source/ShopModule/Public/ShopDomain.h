// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "ShopDomain.generated.h"

class UShopItemData;

/**
 * DTO for shop item data transfer
 */
USTRUCT(BlueprintType)
struct SHOPMODULE_API FShopItemDTO
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopItem")
	FGuid ItemGuid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopItem")
	int32 ItemID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopItem")
	int32 Stock = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopItem")
	float Price = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopItem")
	bool bIsAvailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopItem")
	int32 ShopID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopItem")
	FDateTime LastRestockTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopItem")
	int32 MaxStock = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopItem")
	float RestockIntervalHours = 24.0f;

	FShopItemDTO()
	{
		ItemGuid = FGuid::NewGuid();
		LastRestockTime = FDateTime::Now();
	}

	FShopItemDTO(int32 InItemID, int32 InStock, float InPrice, int32 InShopID)
		: ItemID(InItemID), Stock(InStock), Price(InPrice), ShopID(InShopID)
	{
		ItemGuid = FGuid::NewGuid();
		LastRestockTime = FDateTime::Now();
		MaxStock = InStock;
		bIsAvailable = InStock > 0;
	}

	bool IsValid() const
	{
		return ItemID > 0 && Price >= 0.0f && ShopID > 0;
	}

	bool IsInStock() const
	{
		return bIsAvailable && Stock > 0;
	}

	bool CanRestock() const
	{
		if (MaxStock <= 0) return false;
		
		const FDateTime Now = FDateTime::Now();
		const FTimespan TimeSinceRestock = Now - LastRestockTime;
		return TimeSinceRestock.GetTotalHours() >= RestockIntervalHours;
	}

	void ConsumeStock(int32 Amount = 1)
	{
		Stock = FMath::Max(0, Stock - Amount);
		bIsAvailable = Stock > 0;
	}

	void Restock()
	{
		if (CanRestock())
		{
			Stock = MaxStock;
			bIsAvailable = Stock > 0;
			LastRestockTime = FDateTime::Now();
		}
	}
};

/**
 * Pure domain object for shop configuration (aggregate root)
 */
USTRUCT(BlueprintType)
struct SHOPMODULE_API FShopDomain
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 ShopID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FString ShopName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TArray<FShopItemDTO> ShopItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 MaxItems = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	bool bIsOpen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	float GlobalPriceModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FDateTime LastUpdateTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FString ShopOwnerName;

	FShopDomain()
	{
		LastUpdateTime = FDateTime::Now();
	}

	FShopDomain(int32 InShopID, const FString& InShopName, const TArray<FShopItemDTO>& InShopItems)
		: ShopID(InShopID), ShopName(InShopName), ShopItems(InShopItems)
	{
		LastUpdateTime = FDateTime::Now();
	}

	bool IsValid() const
	{
		return ShopID > 0 && !ShopName.IsEmpty() && ShopItems.Num() <= MaxItems;
	}

	bool IsOpen() const
	{
		return bIsOpen;
	}

	bool HasItem(int32 ItemID) const
	{
		return ShopItems.ContainsByPredicate([ItemID](const FShopItemDTO& Item)
		{
			return Item.ItemID == ItemID && Item.bIsAvailable;
		});
	}

	bool HasItemInStock(int32 ItemID, int32 RequiredQuantity = 1) const
	{
		const FShopItemDTO* Item = GetItem(ItemID);
		return Item && Item->IsInStock() && Item->Stock >= RequiredQuantity;
	}

	const FShopItemDTO* GetItem(int32 ItemID) const
	{
		return ShopItems.FindByPredicate([ItemID](const FShopItemDTO& Item)
		{
			return Item.ItemID == ItemID;
		});
	}

	FShopItemDTO* GetItem(int32 ItemID)
	{
		return ShopItems.FindByPredicate([ItemID](const FShopItemDTO& Item)
		{
			return Item.ItemID == ItemID;
		});
	}

	const FShopItemDTO* GetItemByGuid(const FGuid& ItemId) const
	{
		return ShopItems.FindByPredicate([ItemId](const FShopItemDTO& Item)
		{
			return Item.ItemGuid == ItemId;
		});
	}

	TArray<FShopItemDTO> GetAvailableItems() const
	{
		return ShopItems.FilterByPredicate([](const FShopItemDTO& Item)
		{
			return Item.bIsAvailable && Item.IsInStock();
		});
	}

	float GetItemPrice(int32 ItemID) const
	{
		if (const FShopItemDTO* Item = GetItem(ItemID))
		{
			return Item->Price * GlobalPriceModifier;
		}
		return 0.0f;
	}

	bool CanPurchaseItem(int32 ItemID, int32 Quantity, float PlayerCurrency) const
	{
		if (!IsOpen() || !HasItemInStock(ItemID, Quantity))
		{
			return false;
		}

		const float TotalCost = GetItemPrice(ItemID) * Quantity;
		return PlayerCurrency >= TotalCost;
	}

	bool PurchaseItem(int32 ItemID, int32 Quantity = 1)
	{
		if (!IsOpen()) return false;

		FShopItemDTO* Item = GetItem(ItemID);
		if (!Item || !Item->IsInStock() || Item->Stock < Quantity)
		{
			return false;
		}

		Item->ConsumeStock(Quantity);
		LastUpdateTime = FDateTime::Now();
		return true;
	}

	void RestockAllItems()
	{
		for (FShopItemDTO& Item : ShopItems)
		{
			Item.Restock();
		}
		LastUpdateTime = FDateTime::Now();
	}

	void SetGlobalPriceModifier(float NewModifier)
	{
		GlobalPriceModifier = FMath::Max(0.1f, NewModifier); // Minimum 10% of original price
		LastUpdateTime = FDateTime::Now();
	}

	void SetShopStatus(bool bNewIsOpen)
	{
		bIsOpen = bNewIsOpen;
		LastUpdateTime = FDateTime::Now();
	}

	int32 GetTotalItemCount() const
	{
		return ShopItems.Num();
	}

	int32 GetAvailableItemCount() const
	{
		return ShopItems.FilterByPredicate([](const FShopItemDTO& Item)
		{
			return Item.bIsAvailable && Item.IsInStock();
		}).Num();
	}
};

/**
 * Repository result wrapper for async operations
 */
USTRUCT(BlueprintType)
struct SHOPMODULE_API FShopRepositoryResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopRepositoryResult")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopRepositoryResult")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopRepositoryResult")
	FShopDomain ShopData;

	static FShopRepositoryResult Success(const FShopDomain& InShopData)
	{
		FShopRepositoryResult Result;
		Result.bSuccess = true;
		Result.ShopData = InShopData;
		return Result;
	}

	static FShopRepositoryResult Failure(const FString& InErrorMessage)
	{
		FShopRepositoryResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = InErrorMessage;
		return Result;
	}
};

/**
 * Purchase transaction result
 */
USTRUCT(BlueprintType)
struct SHOPMODULE_API FShopPurchaseResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopPurchaseResult")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopPurchaseResult")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopPurchaseResult")
	int32 ItemID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopPurchaseResult")
	int32 QuantityPurchased = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopPurchaseResult")
	float TotalCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShopPurchaseResult")
	float RemainingCurrency = 0.0f;

	static FShopPurchaseResult Success(int32 InItemID, int32 InQuantity, float InTotalCost, float InRemainingCurrency)
	{
		FShopPurchaseResult Result;
		Result.bSuccess = true;
		Result.ItemID = InItemID;
		Result.QuantityPurchased = InQuantity;
		Result.TotalCost = InTotalCost;
		Result.RemainingCurrency = InRemainingCurrency;
		return Result;
	}

	static FShopPurchaseResult Failure(const FString& InErrorMessage)
	{
		FShopPurchaseResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = InErrorMessage;
		return Result;
	}
};
