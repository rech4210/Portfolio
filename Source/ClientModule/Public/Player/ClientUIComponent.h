#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "MyGame/Public/Shared/AI/EnemySystemCore/FBossDataStruct.h"
#include "MyGame/Public/Shared/AI/EnemySystemCore/FEnemyWidgetData.h"

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
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class CLIENTMODULE_API UClientUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UClientUIComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ============================================================================
	// CLIENT UI MANAGEMENT
	// ============================================================================

	/**
	 * Initialize client-side UI widgets and HUD components
	 * @param SkillComponent - Skill data for UI initialization
	 */
	UFUNCTION(BlueprintCallable, Category = "Client UI")
	void InitClientWidget(const USkillComponent* SkillComponent);

	/**
	 * Handle mouse over detection for enemy widgets
	 * Called from PlayerController's PlayerTick
	 */
	UFUNCTION(BlueprintCallable, Category = "Client UI")
	void HandleMouseOverDetection();

	/**
	 * Notify UI about player state changes
	 */
	UFUNCTION(BlueprintCallable, Category = "Client UI")
	void NotifyClientStateChanged();

	/**
	 * Update skill widget from server data
	 * @param SkillComponent - Updated skill data from server
	 */
	UFUNCTION(BlueprintCallable, Category = "Client UI")
	void ReceiveSkillDataFromServer(const USkillComponent* SkillComponent);

	/**
	 * Handle boss data received from server
	 * @param BossData - Boss information for UI display
	 */
	UFUNCTION(BlueprintCallable, Category = "Client UI")
	void ReceiveBossDataFromServer(const FBossDataStruct& BossData);

	/**
	 * Get access to the HUD for external binding
	 * Used by PlayerController to bind delegates
	 */
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
