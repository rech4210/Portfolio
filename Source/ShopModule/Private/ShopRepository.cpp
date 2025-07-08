// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopRepository.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Components/ShopComponent.h"


void UShopRepository::Initialize() {
	DBManager = GetWorld()->GetGameInstance()->GetSubsystem<UDatabaseManager>();
	if (!DBManager) {
		UE_LOG(LogTemp, Error, TEXT("SkillStateRepository: DatabaseManager is not available!"));
	}
}


TFuture<void> UShopRepository::LoadAllShops(const UObject* WorldContextObject)
{
	// 기존 구현 유지 (전체 상점 로딩)
	TPromise<void> Promise;
	TFuture<void> Future = Promise.GetFuture();
	// Mock implementation - 실제로는 비동기 DB 호출
	Promise.SetValue();

	return Future;
}

bool UShopRepository::LoadShopData(int32 PlayerInformation, UShopComponent& ShopComponentToPopulate)
{
	// 서버 권한이 있을 때만 복제된 프로퍼티를 수정
	if (!ShopComponentToPopulate.GetOwner() || !ShopComponentToPopulate.GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopRepository: LoadShopData can only be called on server authority"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("ShopRepository: Loading shop data for Player %d"), PlayerInformation);

	// Mock 상점 데이터 생성 (실제로는 DB에서 로드)
	// TArray<FShopItemState> MockShopItems;
	//
	// FShopItemState Item1;
	// Item1.ItemID = 2001;
	// Item1.Stock = 100;
	// Item1.Price = 150.0f;
	// Item1.bIsAvailable = true;
	// MockShopItems.Add(Item1);
	//
	// FShopItemState Item2;
	// Item2.ItemID = 2002;
	// Item2.Stock = 75;
	// Item2.Price = 300.0f;
	// Item2.bIsAvailable = true;
	// MockShopItems.Add(Item2);
	//
	// FShopItemState Item3;
	// Item3.ItemID = 2003;
	// Item3.Stock = 0;
	// Item3.Price = 500.0f;
	// Item3.bIsAvailable = false;
	// MockShopItems.Add(Item3);

	// 상점 아이템들을 컴포넌트에 추가 (복제 트리거)
	// for (const auto& Item : MockShopItems)
	// {
	// 	if (!ShopComponentToPopulate.AddShopItem(&Item))
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("ShopRepository: Failed to add shop item %d"), Item.ItemID);
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Log, TEXT("ShopRepository: Successfully added shop item %d"), Item.ItemID);
	// 	}
	// }
	//
	UE_LOG(LogTemp, Log, TEXT("ShopRepository: Completed loading shop data for Player %d"), PlayerInformation);
	return true;
}

bool UShopRepository::SaveShopData(int32 PlayerInformation, const UShopComponent* ShopComponentToSave)
{
	if (!ShopComponentToSave)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopRepository: SaveShopData called with null ShopComponent"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("ShopRepository: Saving shop data for Player %d with %d items"), 
		PlayerInformation, ShopComponentToSave->GetAllShopItems().Num());

	// 실제 구현에서는 여기서 데이터베이스에 저장
	// 현재는 Mock 구현
	
	return true;
}
