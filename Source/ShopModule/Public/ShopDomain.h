
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ShopDomain.generated.h"

class UShopItemData;

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
