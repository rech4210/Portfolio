#pragma once

#include "CoreMinimal.h"
#include "ShopDTOs.generated.h"

USTRUCT(BlueprintType)
struct GAMESHAREDMODULE_API FShopItemDTO {
	GENERATED_BODY()
	UPROPERTY() int32 ItemID;
	UPROPERTY() FString ItemName;
	UPROPERTY() FString ItemDescription;
	UPROPERTY() float Price;
	UPROPERTY() int32 Stock;
	UPROPERTY() bool bIsAvailable;
	UPROPERTY() FString Category;
	UPROPERTY() int32 MaxStock;
	UPROPERTY() float RestockIntervalHours;
	FShopItemDTO() : ItemID(0), Price(0.f), Stock(0), bIsAvailable(false), MaxStock(0), RestockIntervalHours(24.f) {}
	FShopItemDTO(int32 InItemID, const FString& InItemName, const FString& InItemDescription,
		float InPrice, int32 InStock, bool InIsAvailable, const FString& InCategory, int32 InMaxStock)
		: ItemID(InItemID), ItemName(InItemName), ItemDescription(InItemDescription), Price(InPrice), Stock(InStock), bIsAvailable(InIsAvailable), Category(InCategory), MaxStock(InMaxStock), RestockIntervalHours(24.f) {}
	UE_DEPRECATED(5.0, "Use full parameter constructor")
	FShopItemDTO(int32 InItemID, int32 InStock, float InPrice, int32 /*InShopID*/)
		: ItemID(InItemID), ItemName(FString::Printf(TEXT("Item %d"), InItemID)), ItemDescription(TEXT("Legacy item")), Price(InPrice), Stock(InStock), bIsAvailable(InStock > 0), Category(TEXT("General")), MaxStock(InStock), RestockIntervalHours(24.f) {}
};

USTRUCT(BlueprintType)
struct GAMESHAREDMODULE_API FShopDomain {
	GENERATED_BODY()
	UPROPERTY() int32 ShopID;
	UPROPERTY() FString ShopName;
	UPROPERTY() FString ShopDescription;
	UPROPERTY() bool bIsOpen;
	UPROPERTY() int32 AreaID;
	UPROPERTY() FVector ShopLocation;
	UPROPERTY() FDateTime LastRestockTime;
	UPROPERTY() float GlobalPriceModifier;
	UPROPERTY() FString ShopOwnerName;
	UPROPERTY() TArray<FShopItemDTO> ShopItems;
	FShopDomain() : ShopID(0), bIsOpen(false), AreaID(0), ShopLocation(FVector::ZeroVector), GlobalPriceModifier(1.f) {}
};

USTRUCT(BlueprintType)
struct GAMESHAREDMODULE_API FShopRepositoryResult {
	GENERATED_BODY()
	UPROPERTY() bool bSuccess;
	UPROPERTY() FString ErrorMessage;
	UPROPERTY() FShopDomain ShopData;
	FShopRepositoryResult() : bSuccess(false) {}
	FShopRepositoryResult(bool InSuccess, const FString& InErrorMessage, const FShopDomain& InShopData)
		: bSuccess(InSuccess), ErrorMessage(InErrorMessage), ShopData(InShopData) {}
	static FShopRepositoryResult Success(const FShopDomain& InShopData) {
		FShopRepositoryResult R; R.bSuccess = true; R.ShopData = InShopData; return R; }
	static FShopRepositoryResult Failure(const FString& InErrorMessage) {
		FShopRepositoryResult R; R.bSuccess = false; R.ErrorMessage = InErrorMessage; return R; }
};
