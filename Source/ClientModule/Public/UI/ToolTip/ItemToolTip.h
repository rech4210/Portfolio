
#pragma once

#include "CoreMinimal.h"
#include "BaseToolTip.h"
#include "ItemToolTip.generated.h"

/**
 * 
 */
UCLASS()
class CLIENTMODULE_API UItemToolTip : public UBaseToolTip {
	GENERATED_BODY()
public:
	virtual void SetToolTipData(UPrimaryDataAsset* Data) override;
};
