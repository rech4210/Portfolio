#include "Player/ClientUIComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerController.h"
#include "MyGame/Public/Shared/Player/GGwaPlayerState.h"
#include "MyGame/Public/Shared/GAS/GGwaAbilitySystemComponent.h"
#include "MyGame/Public/Shared/GAS/GGwaAttributeSet.h"
#include "MyGame/Public/Shared/AI/BossCharacter.h"
#include "MyGame/Public/Shared/AI/EnemyAbilitySystemComponent.h"
#include "MyGame/Public/Shared/AI/EnemyAttributeSet.h"
#include "UI/GGwaHUD.h"
#include "UI/Widget/GGwaWidget.h"
#include "UI/Enemy/BossStatusWidget.h"
#include "SkillModule/Public/Components/SkillComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UClientUIComponent::UClientUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	GGwaHUD = nullptr;
	OwnerController = nullptr;
}

void UClientUIComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Cache owner controller reference
	OwnerController = Cast<AGGwaPlayerController>(GetOwner());
	if (!OwnerController)
	{
		UE_LOG(LogTemp, Error, TEXT("ClientUIComponent: Owner is not AGGwaPlayerController"));
		return;
	}

	// Only initialize on client and if locally controlled
	if (OwnerController->IsLocalController())
	{
		SetupClientInputMode();

		// Register with ClientServiceManager using TScriptInterface pattern
		TScriptInterface<IClientUIInterface> UIInterface;
		UIInterface.SetObject(this);
		UIInterface.SetInterface(static_cast<IClientUIInterface*>(this));

		// Register with the service manager
		OwnerController->RegisterClientUIService(UIInterface);
		
		UE_LOG(LogTemp, Log, TEXT("ClientUIComponent: Initialized for local controller"));
	}
}

// ============================================================================
// IClientUIInterface IMPLEMENTATION
// ============================================================================

void UClientUIComponent::InitializeUI(const USkillComponent* SkillComponent)
{
	if (!OwnerController || !OwnerController->IsLocalController())
	{
		return;
	}

	if (OwnerController->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientUIComponent: Server controller - UI initialization skipped"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ClientUIComponent: Client widget initialization"));
	
	if (WidgetClass && BossStatusWidgetClass)
	{
		UGGwaWidget* Widget = CreateWidget<UGGwaWidget>(OwnerController, WidgetClass);
		UBossStatusWidget* BossWidget = CreateWidget<UBossStatusWidget>(OwnerController, BossStatusWidgetClass);
		
		if (Widget && BossWidget)
		{
			Widget->AddToViewport();
			BossWidget->AddToViewport();
			BossWidget->SetVisibility(ESlateVisibility::Hidden);
			
			// Setup HUD references
			GGwaHUD = Cast<AGGwaHUD>(OwnerController->GetHUD());
			if (GGwaHUD)
			{
				GGwaHUD->SetBaseWidget(Widget);
				GGwaHUD->SetBossWidget(BossWidget);

				// Bind to the PlayerController's OnBossDataReceived delegate
				OwnerController->OnBossDataReceived.AddDynamic(GGwaHUD, &AGGwaHUD::HandleBossDataReceived);
			}
			
			// Initialize widget with player data
			if (AGGwaPlayerState* PS = OwnerController->GetPlayerState<AGGwaPlayerState>())
			{
				auto ASC = PS->GetAbilitySystemComponent();
				UGGwaAbilitySystemComponent* GGawASC = CastChecked<UGGwaAbilitySystemComponent>(ASC);
				const UGGwaAttributeSet* GGwaAttributeSet = Cast<UGGwaAttributeSet>(GGawASC->GetAttributeSet(UGGwaAttributeSet::StaticClass()));
				
				Widget->UpdateSkillWidgetFromServer(SkillComponent);
				Widget->InitWidget(GGawASC, GGwaAttributeSet);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ClientUIComponent: Widget classes not set"));
	}
	
	// Call Blueprint event for additional initialization
	BP_InitClientWidget(SkillComponent);
}

void UClientUIComponent::HandleMouseOverDetection()
{
	if (!OwnerController || !OwnerController->IsLocalController() || !GGwaHUD)
	{
		return;
	}

	FHitResult Hit;
	// Visibility 채널로 마우스 밑 Actor 판별
	if (OwnerController->GetHitResultUnderCursorByChannel(
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			true, Hit))
	{
		if (ABossCharacter* Enemy = Cast<ABossCharacter>(Hit.GetActor()))
		{
			GGwaHUD->GetBossWidget()->SetVisibility(ESlateVisibility::Visible);
			
			if (Enemy != LastHoveredEnemy.Get())
			{
				LastHoveredEnemy = Enemy;

				// ASC와 AttributeSet 가져오기
				UEnemyAbilitySystemComponent* ASC = Cast<UEnemyAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
				const UEnemyAttributeSet* AttrSet = ASC ? ASC->GetSet<UEnemyAttributeSet>() : nullptr;
				if (!ASC || !AttrSet) return;

				FBossDataStruct BossData;
				BossData.Health = AttrSet->GetHealth();
				BossData.MaxHealth = AttrSet->GetMaxHealth();
				BossData.Damage = AttrSet->GetDamage();

				// FEnemyWidgetData 구성
				FEnemyWidgetData WidgetData = Enemy->GetWidgetData();
				GGwaHUD->GetBossWidget()->SetWidget(WidgetData, BossData);
			}
			return;
		}
	}

	// 커서가 적 이외 영역에 있을 때: 클리어
	if (LastHoveredEnemy.IsValid())
	{
		LastHoveredEnemy = nullptr;
		GGwaHUD->GetBossWidget()->SetVisibility(ESlateVisibility::Hidden);
	}
	
	// Call Blueprint event for additional handling
	BP_HandleMouseOverDetection();
}

void UClientUIComponent::NotifyStateChanged()
{
	if (!OwnerController || !OwnerController->IsLocalController() || !GGwaHUD)
	{
		return;
	}

	if (GGwaHUD->GetBaseWidget())
	{
		GGwaHUD->GetBaseWidget()->OnPlayerStateChanged.Broadcast();
	}
	
	// Call Blueprint event for additional handling
	BP_NotifyClientStateChanged();
}

void UClientUIComponent::ReceiveBossData(const FBossDataStruct& BossData)
{
	// This can be used for direct boss data updates from server
	// Currently forwarded through PlayerController's OnBossDataReceived delegate
	UE_LOG(LogTemp, Log, TEXT("ClientUIComponent: Received boss data from server"));
	
	// Call Blueprint event for additional handling
	BP_ReceiveBossDataFromServer(BossData);
}

void UClientUIComponent::ReceiveSkillData(const USkillComponent* SkillComponent)
{
	if (!OwnerController || !OwnerController->IsLocalController() || !GGwaHUD)
	{
		return;
	}

	if (GGwaHUD->GetBaseWidget())
	{
		GGwaHUD->GetBaseWidget()->UpdateSkillWidgetFromServer(SkillComponent);
	}
	
	// Call Blueprint event for additional handling
	BP_ReceiveSkillDataFromServer(SkillComponent);
}

// ============================================================================
// PRIVATE HELPER METHODS
// ============================================================================

AGGwaPlayerController* UClientUIComponent::GetGGwaPlayerController() const
{
	return OwnerController;
}

void UClientUIComponent::SetupClientInputMode()
{
	if (!OwnerController || !OwnerController->IsLocalController())
	{
		return;
	}

	OwnerController->bShowMouseCursor = true;
	OwnerController->bEnableMouseOverEvents = true;
	
	FInputModeGameAndUI inputMode;
	inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	inputMode.SetHideCursorDuringCapture(false);
	OwnerController->SetInputMode(inputMode);
}
