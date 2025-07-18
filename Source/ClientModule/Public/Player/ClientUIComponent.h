#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "MyGame/Public/Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "MyGame/Public/Shared/AI/EnemySystemCore/FEnemyWidgetData.h"
#include "MyGame/Public/Shared/Interface/IClientComponentProvider.h"

// Forward declarations for client UI classes
class AGGwaHUD;
class UGGwaWidget;
class UBossStatusWidget;
class ABossCharacter;
class USkillComponent;
class UEnemyAbilitySystemComponent;
class UEnemyAttributeSet;

#include "ClientUIComponent.generated.h"

/**
 * UClientUIComponent
 * 
 * Handles all client-specific UI management functionality.
 * Separated from ClientAuthComponent to follow single responsibility principle.
 * This component manages widget creation, mouse over detection, and UI state updates.
 * Implements IClientUIInterface for IoC pattern integration.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class CLIENTMODULE_API UClientUIComponent : public UActorComponent, public IClientUIInterface
{
	GENERATED_BODY()

public:
	UClientUIComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ============================================================================
	// IClientUIInterface IMPLEMENTATION
	// ============================================================================

	virtual void InitializeUI(const USkillComponent* SkillComponent) override;

	virtual void HandleMouseOverDetection() override;

	virtual void NotifyStateChanged() override;

	virtual void ReceiveBossData(const FBossDataStruct& BossData) override;

	virtual void ReceiveSkillData(const USkillComponent* SkillComponent) override;

	// ============================================================================
	// LEGACY BLUEPRINT INTERFACE (for backward compatibility)
	// ============================================================================

	UFUNCTION(BlueprintImplementableEvent, Category = "Client UI")
	void BP_InitClientWidget(const USkillComponent* SkillComponent);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Client UI")
	void BP_HandleMouseOverDetection();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Client UI")
	void BP_NotifyClientStateChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Client UI")
	void BP_ReceiveSkillDataFromServer(const USkillComponent* SkillComponent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Client UI")
	void BP_ReceiveBossDataFromServer(const FBossDataStruct& BossData);

	
	UFUNCTION(BlueprintCallable, Category = "Client UI")
	class AGGwaHUD* GetGGwaHUD() const { return GGwaHUD; }

	// ============================================================================
	// WIDGET CLASS REFERENCES
	// ============================================================================

	/**
	 * Main game widget class reference
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Client UI")
	TSubclassOf<class UGGwaWidget> WidgetClass;

	/**
	 * Boss status widget class reference
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Client UI")
	TSubclassOf<class UBossStatusWidget> BossStatusWidgetClass;

private:
	// ============================================================================
	// PRIVATE UI MEMBERS
	// ============================================================================

	/**
	 * HUD reference for UI management
	 */
	UPROPERTY()
	TObjectPtr<AGGwaHUD> GGwaHUD;

	/**
	 * Currently hovered enemy for mouse over detection
	 */
	TWeakObjectPtr<ABossCharacter> LastHoveredEnemy;

	/**
	 * Owner PlayerController reference
	 */
	UPROPERTY()
	TObjectPtr<class AGGwaPlayerController> OwnerController;

	// ============================================================================
	// PRIVATE HELPER METHODS
	// ============================================================================

	/**
	 * Get the owner PlayerController with proper casting
	 */
	class AGGwaPlayerController* GetGGwaPlayerController() const;

	/**
	 * Setup input mode for client UI interaction
	 */
	void SetupClientInputMode();
};
