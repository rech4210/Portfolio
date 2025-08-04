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
#include "SkillModule/Public/Utill/LocalDataBaseLoader.h"
#include "Engine/Engine.h"
#include "Entities/SkillSlot.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UIManagerSubsystem.h"
#include "Utill/USkillHelper.h"
#include "Utils/ClientUIMapping.h"

UClientUIComponent::UClientUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	GGwaHUD = nullptr;
	OwnerController = nullptr;
}

void UClientUIComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerController = Cast<AGGwaPlayerController>(GetOwner());
	if (OwnerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::BeginPlay - Owner set to: %s"), 
			*OwnerController->GetClass()->GetName());
		UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::BeginPlay - IsLocalController: %s"), 
			OwnerController->IsLocalPlayerController() ? TEXT("YES") : TEXT("NO"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UI_INIT_DEBUG] ClientUIComponent::BeginPlay - Failed to cast Owner to AGGwaPlayerController"));
		UE_LOG(LogTemp, Error, TEXT("[UI_INIT_DEBUG] ClientUIComponent::BeginPlay - Owner class: %s"), 
			GetOwner() ? *GetOwner()->GetClass()->GetName() : TEXT("NULL"));
		return;
	}
	
	SetupClientInputMode();
}

// ============================================================================
// IClientUIInterface IMPLEMENTATION
// ============================================================================

void UClientUIComponent::InitializeUI()
{
	UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - Start"));
	
	if (!OwnerController || !OwnerController->IsLocalPlayerController())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - Not local controller, exiting"));
		OwnerController = Cast<AGGwaPlayerController>(GetOwner());
	}
	
	if (WidgetClass && BossStatusWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - Widget classes available"));
		
		UGGwaWidget* Widget = CreateWidget<UGGwaWidget>(OwnerController, WidgetClass);
		UBossStatusWidget* BossWidget = CreateWidget<UBossStatusWidget>(OwnerController, BossStatusWidgetClass);
		
		if (Widget && BossWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - Widgets created successfully"));
			
			Widget->AddToViewport();
			BossWidget->AddToViewport();
			BossWidget->SetVisibility(ESlateVisibility::Hidden);

			UClass* HUDClass = FClientUIMapping::LoadUIClass(EClientUIKey::HUD);
			OwnerController->ClientSetHUD(HUDClass);
			GGwaHUD = Cast<AGGwaHUD>(OwnerController->GetHUD());
			
			if (!GGwaHUD)
			{
				UE_LOG(LogTemp, Error, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - Failed to load HUD class"));
				return;
			}
			
			if (GGwaHUD)
			{
				GGwaHUD->SetBaseWidget(Widget);
				GGwaHUD->SetBossWidget(BossWidget);
				OwnerController->OnBossDataReceived.AddDynamic(GGwaHUD, &AGGwaHUD::HandleBossDataReceived);
				UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - HUD setup completed"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - Failed to create widgets"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - Widget classes not set"));
	}

	if (AGGwaPlayerState* PS = Cast<AGGwaPlayerState>(OwnerController->GetPlayerState<APlayerState>()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - Setting up ASC"));
				
		auto ASC = PS->GetAbilitySystemComponent();
		UGGwaAbilitySystemComponent* GGawASC = CastChecked<UGGwaAbilitySystemComponent>(ASC);
		const UGGwaAttributeSet* GGwaAttributeSet = Cast<UGGwaAttributeSet>(GGawASC->GetAttributeSet(UGGwaAttributeSet::StaticClass()));
				
		if (GGawASC && GGwaAttributeSet)
		{
			UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - ASC and AttributeSet found"));
			GGwaHUD->GetBaseWidget()->InitWidget(GGawASC, GGwaAttributeSet);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - ASC or AttributeSet is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - PlayerState is null"));
	}

	bUIReady = true;
	if (bHasBufferedData)
	{
		ReceiveSkillReplicationData(BufferedSlotReplicationData);
		bHasBufferedData = false;
		BufferedSlotReplicationData.Items.Reset();
	}
	
	BP_InitClientWidget();
	UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::InitializeUI - Complete"));
}

void UClientUIComponent::HandleMouseOverDetection()
{
	if (!OwnerController || !OwnerController->IsLocalPlayerController() || !GGwaHUD)
	{
		return;
	}

	FHitResult Hit;
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

				UEnemyAbilitySystemComponent* ASC = Cast<UEnemyAbilitySystemComponent>(Enemy->GetAbilitySystemComponent());
				const UEnemyAttributeSet* AttrSet = ASC ? ASC->GetSet<UEnemyAttributeSet>() : nullptr;
				if (!ASC || !AttrSet) return;

				FBossDataStruct BossData;
				BossData.Health = AttrSet->GetHealth();
				BossData.MaxHealth = AttrSet->GetMaxHealth();
				BossData.Damage = AttrSet->GetDamage();

				FEnemyWidgetData WidgetData = Enemy->GetWidgetData();
				GGwaHUD->GetBossWidget()->SetWidget(WidgetData, BossData);
			}
			return;
		}
	}

	if (LastHoveredEnemy.IsValid())
	{
		LastHoveredEnemy = nullptr;
		GGwaHUD->GetBossWidget()->SetVisibility(ESlateVisibility::Hidden);
	}
	
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
	
	BP_NotifyClientStateChanged();
}

void UClientUIComponent::ReceiveBossData(const FBossDataStruct& BossData)
{
	UE_LOG(LogTemp, Log, TEXT("ClientUIComponent: Received boss data from server"));
	
	BP_ReceiveBossDataFromServer(BossData);
}

void UClientUIComponent::ReceiveSkillReplicationData(const FSkillSlotReplicationArray& SkillSlotsReplication)
{
	if (!OwnerController || !OwnerController->IsLocalPlayerController() || !GGwaHUD)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientUIComponent::ReceiveSkillReplicationData - Invalid controller or HUD"));
		if (!bUIReady)
		{
			BufferedSlotReplicationData = SkillSlotsReplication;
			bHasBufferedData = true;
			return;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ClientUIComponent::ReceiveSkillReplicationData - Processing %d skill slots"), 
		SkillSlotsReplication.Items.Num());

	if (!ULocalDataBaseLoader::IsInitialized())
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientUIComponent::ReceiveSkillReplicationData - LocalDataBaseLoader not initialized, initializing now"));
		ULocalDataBaseLoader::Initialize();
	}

	TArray<USkillDataAsset*> ReconstructedSkillAssets;
	TMap<int32, FSkillSlotReplicationData> SlotIndexToReplicationData;
	
	for (const FSkillSlotReplicationItem& Item : SkillSlotsReplication.Items)
	{
		const FSkillSlotReplicationData& SlotData = Item.SlotData;
		SlotIndexToReplicationData.Add(SlotData.SlotIndex, SlotData);

		if (SlotData.SkillId > 0)
		{
			FPrimaryAssetId AssetId;
			if (ULocalDataBaseLoader::CheckPrimaryAssetId(SlotData.SkillId, AssetId))
			{
				USkillDataAsset* SkillAsset = ULocalDataBaseLoader::GetDataFromAssetId<USkillDataAsset>(AssetId, true);
				if (SkillAsset)
				{
					SkillAsset->SkillSlotIndex = SlotData.SlotIndex;
					SkillAsset->SkillSlotKey = SlotData.SlotKey;
					ReconstructedSkillAssets.Add(SkillAsset);
					UE_LOG(LogTemp, Log, TEXT("ClientUIComponent::ReceiveSkillReplicationData - Reconstructed SkillAsset: %s (ID: %d) for Slot[%d]"), 
						*SkillAsset->DisplayName.ToString(), SlotData.SkillId, SlotData.SlotIndex);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("ClientUIComponent::ReceiveSkillReplicationData - Failed to load SkillAsset for ID: %d"), SlotData.SkillId);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ClientUIComponent::ReceiveSkillReplicationData - No AssetId found for SkillID: %d"), SlotData.SkillId);
			}
		}
	}

	if (GGwaHUD->GetBaseWidget())
	{
		GGwaHUD->GetBaseWidget()->UpdateSkillWidgetFromServer(ReconstructedSkillAssets);
		UE_LOG(LogTemp, Log, TEXT("ClientUIComponent::ReceiveSkillReplicationData - Updated widget with reconstructed skill data"));
	}
}

AGGwaPlayerController* UClientUIComponent::GetGGwaPlayerController() const
{
	return OwnerController;
}

void UClientUIComponent::SetupClientInputMode()
{
	UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::SetupClientInputMode - Start"));
	
	if (!OwnerController || !OwnerController->IsLocalPlayerController())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::SetupClientInputMode - Not local controller, skipping"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::SetupClientInputMode - Setting up input mode"));
	
	OwnerController->bEnableMouseOverEvents = true;
	
	FInputModeGameAndUI inputMode;
	inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	inputMode.SetHideCursorDuringCapture(false);
	OwnerController->SetInputMode(inputMode);
	
	UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] ClientUIComponent::SetupClientInputMode - Input mode configured"));
}

