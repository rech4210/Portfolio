#include "Player/ClientUIComponent.h"

#include "Data/SkillDataAsset.h"
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
#include "Entities/SkillSlot.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UIManagerSubsystem.h"

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
	check(OwnerController)
	
	SetupClientInputMode();
}

void UClientUIComponent::SetOwnerController(AGGwaPlayerController* Controller) {
	OwnerController = Controller;
}

// ============================================================================
// IClientUIInterface IMPLEMENTATION
// ============================================================================

// SkillComponent를 제공받고 있으나, 복제가 제대로 이루어지지 않는다고 판단됨.
// 아예 nullptr이 뜰때고 있고, 어쩔때는 skillcomponent는 메모리를 참조하나, 내부 skillslot이 비어있음.
// 디버깅을 해보니, Client RPC로 SkillComponent를 전송해주는 시점에서 복제가 제대로 수행되지 않는다고 판단.
// GGwaGameMOde에서 인자로 넘겨주는 시점에는 정상적으로 메모리를 점유중. GameMode -> RPC -> (복제 이슈) Controller -> UI ! 실패 흐름

void UClientUIComponent::InitializeUI(const USkillComponent* SkillComponent)
{
	if (!OwnerController || !OwnerController->IsLocalPlayerController())
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientUIComponent: is not Local controller"));
		return;
	}

	//GetPlayerState -> Nullptr
	
	if (auto State = OwnerController->GetPlayerState<AGGwaPlayerState>()) {
		if (auto Component = State->GetSkillComponent()) {
			for (auto Element : Component->GetAllSkillSlots()) {
				UE_LOG(LogTemp, Warning, TEXT("===ClientUIComponent: Skill %s, Id %d ==="), *Element->SkillData->DisplayName.ToString(), Element->SkillId);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ClientUIComponent: Client widget initialization"));
	
	if (WidgetClass && BossStatusWidgetClass)
	{
		UGGwaWidget* Widget = CreateWidget<UGGwaWidget>(OwnerController, WidgetClass);
		UBossStatusWidget* BossWidget = CreateWidget<UBossStatusWidget>(OwnerController, BossStatusWidgetClass);
		
		if (Widget && BossWidget)
		{
			//Widget은 정상 출력.
			Widget->AddToViewport();
			BossWidget->AddToViewport();
			BossWidget->SetVisibility(ESlateVisibility::Hidden);
			
			// Setup HUD references
			// GetHud -> nullptr
			// Note : ServerGameMode Doesn't have HUD, so we need to check if OwnerController is valid
			// 해당 HUD도 enum key mapping을 통해서 tsoftptr path string 데이터로 가져와야하나?
			GGwaHUD = Cast<AGGwaHUD>(OwnerController->GetHUD());
			if (GGwaHUD)
			{
				GGwaHUD->SetBaseWidget(Widget);
				GGwaHUD->SetBossWidget(BossWidget);

				OwnerController->OnBossDataReceived.AddDynamic(GGwaHUD, &AGGwaHUD::HandleBossDataReceived);
			}
			
			//GetPlayerState -> Nullptr
			if (AGGwaPlayerState* PS = OwnerController->GetPlayerState<AGGwaPlayerState>())
			{
				auto ASC = PS->GetAbilitySystemComponent();
				UGGwaAbilitySystemComponent* GGawASC = CastChecked<UGGwaAbilitySystemComponent>(ASC);
				const UGGwaAttributeSet* GGwaAttributeSet = Cast<UGGwaAttributeSet>(GGawASC->GetAttributeSet(UGGwaAttributeSet::StaticClass()));
				
				Widget->InitWidget(GGawASC, GGwaAttributeSet);
				Widget->UpdateSkillWidgetFromServer(SkillComponent);
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
	if (!OwnerController || !OwnerController->IsLocalPlayerController() || !GGwaHUD)
	{
		return;
	}

	FHitResult Hit;
	// Visibility 채널�?마우??�?Actor ?�별
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

				// ASC?�?AttributeSet 가?�오�?
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

	// 커서가 ???�외 ?�역???�을 ?? ?�리??
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
	if (!OwnerController || !OwnerController->IsLocalPlayerController() || !GGwaHUD)
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
	if (!OwnerController || !OwnerController->IsLocalPlayerController() || !GGwaHUD)
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
	if (!OwnerController || !OwnerController->IsLocalPlayerController())
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
