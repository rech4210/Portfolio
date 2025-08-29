#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkillComponent.h"
#include "Engine/Engine.h"
#include "MyGame/Public/Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "GameSharedModule/Public/Interface/IClientComponentProvider.h"

class AGGwaHUD;
class UGGwaWidget;
class UBossStatusWidget;
class ABossCharacter;
class USkillComponent;
class UEnemyAbilitySystemComponent;
class UEnemyAttributeSet;

#include "ClientUIComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class CLIENTMODULE_API UClientUIComponent : public UActorComponent, public IClientUIInterface
{
	GENERATED_BODY()

public:
	UClientUIComponent();

protected:
	virtual void BeginPlay() override;

public:
	
	virtual void InitializeUI() override;

	virtual void HandleMouseOverDetection() override;

	virtual void NotifyClientEvent(FGameplayTag Tag) override;

	virtual void ReceiveBossData(const FBossDataStruct& BossData) override;

	virtual void ReceiveSkillReplicationData(const struct FSkillSlotReplicationArray& SkillSlotsReplication) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Client UI")
	void BP_InitClientWidget();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Client UI")
	void BP_HandleMouseOverDetection();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Client UI")
	void BP_NotifyClientStateChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Client UI")
	void BP_ReceiveBossDataFromServer(const FBossDataStruct& BossData);

	UFUNCTION(BlueprintCallable, Category = "Client UI")
	class AGGwaHUD* GetGGwaHUD() const { return GGwaHUD; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Client UI")
	TSubclassOf<class UGGwaWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Client UI")
	TSubclassOf<class UBossStatusWidget> BossStatusWidgetClass;

private:

	UPROPERTY()
	TObjectPtr<AGGwaHUD> GGwaHUD;

	TWeakObjectPtr<ABossCharacter> LastHoveredEnemy;

	UPROPERTY()
	TObjectPtr<class AGGwaPlayerController> OwnerController;

	class AGGwaPlayerController* GetGGwaPlayerController() const;

	void SetupClientInputMode();

	bool bUIReady = false;
	bool bHasBufferedData = false;
	UPROPERTY()
	FSkillSlotReplicationArray BufferedSlotReplicationData;
};
