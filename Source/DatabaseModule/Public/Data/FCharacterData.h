#pragma once

#include "CoreMinimal.h"
#include "InventoryModule/Public/InventoryComponent.h"
#include "FCharacterData.generated.h"

USTRUCT(BlueprintType)
struct DATABASEMODULE_API FCharacterData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
    int32 UserId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
    int32 CharacterId;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
    int32 Level;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
    int32 Exp;

    // JSONB data from the database, can be parsed into another USTRUCT if needed
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
    FString JsonData; 

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Data")
    TArray<UFInventoryItem*> Inventory;

    FCharacterData()
        : UserId(0), CharacterId(0), Level(1), Exp(0)
    {}
}; 