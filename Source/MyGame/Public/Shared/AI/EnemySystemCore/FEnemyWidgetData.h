// @Needmodifi
#pragma once
#include "CoreMinimal.h"
#include "FEnemyWidgetData.generated.h"
class UTexture2D;

USTRUCT(BlueprintType)
struct MYGAME_API FEnemyWidgetData {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Widget")
	TObjectPtr<UTexture2D> Symbol;

	UPROPERTY(EditAnywhere, Category = "Widget")
	FName Name;

	UPROPERTY(EditAnywhere, Category = "Widget")
	FName Type;
	
	UPROPERTY(EditAnywhere, Category = "Widget")
	float LimitTime;
};
