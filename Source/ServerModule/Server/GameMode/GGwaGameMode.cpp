#include "GGwaGameMode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Engine/Engine.h"
#include "HttpModule.h"
#include "GameMode/BattleFlowController.h"
#include "AuthVerificationService.h" 
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"
#include "InventorySubsystem.h"
#include "ShopSubsystem.h"
#include "SkillSubsystem.h"
#include "MyGame/Public/Shared/GameState/GGwaGameState.h"
#include "MyGame/Public/Shared/Cache/UIConfigCacheActor.h"
#include "Shared/Player/GGwaCharacter.h"
#include "Utill/LocalDataBaseLoader.h"

constexpr static float GAME_MODE_FREQUENCY = 3.0f;
AGGwaGameMode::AGGwaGameMode()
{
	PlayerControllerClass = AGGwaPlayerController::StaticClass();
	PlayerStateClass = AGGwaPlayerState::StaticClass();
	DefaultPawnClass = AGGwaCharacter::StaticClass();
	
	AuthVerificationService = NewObject<UAuthVerificationService>(this, TEXT("AuthVerificationService"));
}

void AGGwaGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	UE_LOG(LogTemp, Warning, TEXT("=== PRELOGIN DEBUG ==="));
	UE_LOG(LogTemp, Warning, TEXT("NetMode: %d | HasAuthority: %s"), GetWorld()->GetNetMode(), HasAuthority() ? TEXT("YES") : TEXT("NO"));
	UE_LOG(LogTemp, Warning, TEXT("[Game Server] Connection from address: %s"), *Address);
	UE_LOG(LogTemp, Warning, TEXT("[Game Server] Options: %s"), *Options);

	// ============================================================================
	// SAFE PARAMETER PARSING WITH VALIDATION
	// ============================================================================
	
	FString Token, ProvidedUserId, PIEFlag;
	
	if (!FParse::Value(*Options, TEXT("token="), Token))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Game Server] No token parameter found in Options"));
	}
	
	FString RawUserId;
	if (FParse::Value(*Options, TEXT("userid="), RawUserId))
	{
		FString LeftPart, RightPart;
		if (RawUserId.Split(TEXT("?"), &LeftPart, &RightPart))
		{
			ProvidedUserId = LeftPart;
		}
		else
		{
			ProvidedUserId = RawUserId;
		}
		UE_LOG(LogTemp, Log, TEXT("[Game Server] Extracted clean UserId: %s (from raw: %s)"), *ProvidedUserId, *RawUserId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Game Server] No userid parameter found in Options"));
	}
	
	if (!FParse::Value(*Options, TEXT("pie="), PIEFlag))
	{
		UE_LOG(LogTemp, Log, TEXT("[Game Server] No PIE flag found (normal for production)"));
	}
	
	bool bValidToken = !Token.IsEmpty() && Token.Len() >= 10 && Token.Len() <= 512;
	bool bValidUserId = !ProvidedUserId.IsEmpty() && ProvidedUserId.Len() >= 32 && ProvidedUserId.Len() <= 50;
	
	UE_LOG(LogTemp, Warning, TEXT("[Game Server] Token Valid: %s | UserId Valid: %s"), 
		bValidToken ? TEXT("YES") : TEXT("NO"), bValidUserId ? TEXT("YES") : TEXT("NO"));
	
	FPlayerIdentityFair PlayerIdentityFair;
	PlayerIdentityFair.UserId = ProvidedUserId;
	PlayerIdentityFair.Token = Token;
	PendingPlayers.Add(UniqueId, PlayerIdentityFair);
	if (!bValidToken || !bValidUserId) {
		PlayerIdentityFair.bIsValid = false;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[Game Server] Provided Token: %s..."), *PlayerIdentityFair.Token.Left(20));
	UE_LOG(LogTemp, Warning, TEXT("[Game Server] Provided UserId: %s"), *PlayerIdentityFair.UserId);
	UE_LOG(LogTemp, Warning, TEXT("[Game Server] PIE Environment: %s"), *PIEFlag);
	UE_LOG(LogTemp, Warning, TEXT("=== END PRE LOGIN ==="));
}

void AGGwaGameMode::BeginPlay()
{
	Super::BeginPlay();
	ULocalDataBaseLoader::Initialize();

	UE_LOG(LogTemp, Warning, TEXT("=== AGGwaGameMode::BeginPlay ==="));

	UWorld* World = GetWorld();
	bool bIsPIEEnvironment = World && World->WorldType == EWorldType::PIE;
	bool bIsDedicatedServer = HasAuthority() && World->GetNetMode() == NM_DedicatedServer;
	FString CurrentMapName = World->GetMapName();
	
	UE_LOG(LogTemp, Warning, TEXT("PIE: %s | DedicatedServer: %s | CurrentMap: %s"), 
		bIsPIEEnvironment ? TEXT("Yes") : TEXT("No"),
		bIsDedicatedServer ? TEXT("Yes") : TEXT("No"),
		*CurrentMapName);

	bool bDevelopmentMode = UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG;
	bool bIsLoginLevel = CurrentMapName.Contains(TEXT("LoginLevel"));
	
	UE_LOG(LogTemp, Warning, TEXT("[DEV MODE] bDevelopmentMode: %s | bIsDedicatedServer: %s | bIsLoginLevel: %s"), 
		bDevelopmentMode ? TEXT("Yes") : TEXT("No"),
		bIsDedicatedServer ? TEXT("Yes") : TEXT("No"),
		bIsLoginLevel ? TEXT("Yes") : TEXT("No"));
	

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("GGwaGameMode::BeginPlay - Creating UI Cache Actor on server"));

		// Spawn the UI configuration cache actor
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = nullptr;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AUIConfigCacheActor* CacheActor = GetWorld()->SpawnActor<AUIConfigCacheActor>(
			AUIConfigCacheActor::StaticClass(), 
			FVector::ZeroVector, 
			FRotator::ZeroRotator, 
			SpawnParams);

		if (CacheActor)
		{
			UE_LOG(LogTemp, Log, TEXT("GGwaGameMode::BeginPlay - UI Cache Actor spawned successfully"));

			CacheActor->InitializeDefaultMappings();

			if (AGGwaGameState* GGwaGameState = GetGameState<AGGwaGameState>())
			{
				GGwaGameState->SetCacheActor(CacheActor);
				UE_LOG(LogTemp, Log, TEXT("GGwaGameMode::BeginPlay - Cache Actor assigned to GameState"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GGwaGameMode::BeginPlay - Failed to get GGwaGameState"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GGwaGameMode::BeginPlay - Failed to spawn UI Cache Actor"));
		}
	}
	
	// ============================================================================
	// BIND SKILL SUBSYSTEM DELEGATE (ONE-TIME BINDING)
	// ============================================================================
	if (auto SkillSubsystem = GetGameInstance()->GetSubsystem<USkillSubsystem>())
	{
		SkillSubsystem->OnSkillDataLoadCompleted.AddDynamic(this, &AGGwaGameMode::OnSkillDataLoadCompleted);
		UE_LOG(LogTemp, Log, TEXT("[SKILL] Skill completion delegate bound in BeginPlay"));
	}
}

void AGGwaGameMode::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	
	static float LogTimer = 0.0f;
	LogTimer += DeltaSeconds;
	if (LogTimer >= GAME_MODE_FREQUENCY)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server is ALIVE. Current GameMode is [ %s ] running in map [ %s ]"), *GetName(), *GetWorld()->GetName());
		LogTimer = 0.0f;
	}
}

void AGGwaGameMode::InitializeServerManagers()
{
	Super::InitializeServerManagers();
	if (HasAuthority())
	{
		BattleFlowController = NewObject<UBattleFlowController>(this, TEXT("BattleFlowController"));
		if (TScriptInterface<IShopRepositoryInterface> Repo = GetGameInstance()->GetSubsystem<UShopSubsystem>()->GetShopRepository())
		{
			Repo->LoadShopByID(0);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGGwaGameMode: ShopRepository is not initialized!"));
		}
	}
}

void AGGwaGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!NewPlayer || !NewPlayer->PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("[FAIL] NewPlayer or PlayerState is not available on PostLogin!"));
		return;
	}

	// ============================================================================
	// AUTHENTICATION & USER ID RESOLUTION
	// ============================================================================
	
	AGGwaPlayerController* GGwaPC = Cast<AGGwaPlayerController>(NewPlayer);
	AGGwaPlayerState* GGwaPlayerState = NewPlayer->GetPlayerState<AGGwaPlayerState>();

	if (!GGwaPC)
	{
		UE_LOG(LogTemp, Error, TEXT("[FAIL] Could not cast to AGGwaPlayerController for player %s"), 
			*GGwaPlayerState->GetPlayerName());
		return;
	}
	FPlayerIdentityFair* PlayerIdentityFair = PendingPlayers.Find(NewPlayer->PlayerState->GetUniqueId());
	GGwaPC->InitializeClientComponent();
	if (!PlayerIdentityFair) {
		return;
	}
	if (!PlayerIdentityFair->bIsValid) {
		return;
	}
	GGwaPC->SetCachedUserId(PlayerIdentityFair->UserId);
	GGwaPC->SetCachedAuthToken(PlayerIdentityFair->Token);
	FString FinalUserId = *PlayerIdentityFair->UserId;
	
	UE_LOG(LogTemp, Warning, TEXT("[AUTH] Using UserID as string: '%s'"), *FinalUserId);

	if (GGwaPlayerState)
	{
		GGwaPlayerState->SetPlayerGuid(FinalUserId);
	}
	UE_LOG(LogTemp, Log, TEXT("[SUCCESS] Player %s authenticated with UserId: %s"), 
		*GGwaPlayerState->GetPlayerName(), *FinalUserId);

	PendingPlayers.Remove(GGwaPlayerState->GetUniqueId());

	// ============================================================================
	// MAP TRANSITION LOGIC - Check if we should move to MainMap
	// ============================================================================
	
	FString CurrentMapName = GetWorld()->GetMapName();
	UE_LOG(LogTemp, Warning, TEXT("[SERVER] Current Map: %s)"), *CurrentMapName);
	
	InitializePlayerDDDSystems(NewPlayer, GGwaPlayerState, FinalUserId);
	UE_LOG(LogTemp, Warning, TEXT("=== END POST LOGIN ==="));
}

void AGGwaGameMode::InitializePlayerDDDSystems(APlayerController* NewPlayer, AGGwaPlayerState* PlayerState, const FString& UserId)
{
	
	UE_LOG(LogTemp, Log, TEXT("=== Initializing DDD Systems for Player %s (UserId: %s) ==="), 
		*PlayerState->GetPlayerName(), *UserId);

	// ============================================================================
	// 1. INVENTORY SYSTEM INITIALIZATION
	// ============================================================================
	
	UE_LOG(LogTemp, Log, TEXT("[INVENTORY] Initializing inventory system..."));
	if (auto InventorySubsystem = GetGameInstance()->GetSubsystem<UInventorySubsystem>())
	{
		InventorySubsystem->RequestLoadPlayerInventory(PlayerState);
		UE_LOG(LogTemp, Log, TEXT("[INVENTORY] ??Inventory loading initiated"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY] ??InventorySubsystem not available"));
	}

	// ============================================================================
	// 2. SKILL SYSTEM INITIALIZATION  
	// ============================================================================
	
	UE_LOG(LogTemp, Log, TEXT("[SKILL] Initializing skill system..."));
	if (auto SkillSubsystem = GetGameInstance()->GetSubsystem<USkillSubsystem>())
	{
		// Note: Delegate binding is already done in BeginPlay() to prevent duplicate bindings
		SkillSubsystem->RequestLoadPlayerSkills(PlayerState, UserId);
		UE_LOG(LogTemp, Log, TEXT("[SKILL] ??Skill loading initiated for UserId: %s"), *UserId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL] ??SkillSubsystem not available"));
	}

	// ============================================================================
	// 3. SHOP SYSTEM INITIALIZATION
	// ============================================================================
	
	UE_LOG(LogTemp, Log, TEXT("[SHOP] Initializing shop system..."));
	if (auto ShopSubsystem = GetGameInstance()->GetSubsystem<UShopSubsystem>())
	{
		// Load shop data for player (if needed)
		// ShopSubsystem->RequestLoadPlayerShopData(PlayerState, UserId);
		UE_LOG(LogTemp, Log, TEXT("[SHOP] ??Shop system ready"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[SHOP] ??ShopSubsystem not available"));
	}

	// ============================================================================
	// 4. EQUIPMENT SYSTEM INITIALIZATION (Future Implementation)
	// ============================================================================
	
	UE_LOG(LogTemp, Log, TEXT("[EQUIPMENT] Equipment system initialization deferred (not implemented)"));
	// TODO: Implement equipment system initialization when EquipmentSubsystem is available
	
	// ============================================================================
	// 5. CHARACTER DATA SYSTEM INITIALIZATION (Future Implementation)
	// ============================================================================
	
	UE_LOG(LogTemp, Log, TEXT("[CHARACTER] Character system initialization deferred (not implemented)"));
	// TODO: Implement character data system initialization when CharacterSubsystem is available

	UE_LOG(LogTemp, Log, TEXT("=== DDD Systems Initialization Completed ==="));
}

void AGGwaGameMode::OnSkillDataLoadCompleted(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, USkillComponent* SkillComponent)
{
	UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] OnSkillDataLoadCompleted called"));
	
	if (!PlayerIdentity)
	{
		UE_LOG(LogTemp, Error, TEXT("[UI] PlayerIdentity is null"));
		return;
	}
	
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[UI] SkillComponent is null"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[UI] Valid parameters - PlayerIdentity: %p, SkillComponent: %p"), 
		PlayerIdentity.GetObject(), SkillComponent);
	
	if (AGGwaPlayerState* GGwaState = Cast<AGGwaPlayerState>(PlayerIdentity.GetObject())) 
	{
		GGwaState->SetSkillComponent(SkillComponent);
		UE_LOG(LogTemp, Warning, TEXT("[UI_INIT_DEBUG] Found controller for player %s"), *GGwaState->GetPlayerName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UI] Failed to cast PlayerIdentity to GGwaPlayerState"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[UI] PlayerController not found for UI initialization"));
}

void AGGwaGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void AGGwaGameMode::Server_SkillLog(FString Name, const FString& SkillName, FVector SkillLocation) {
	FHttpModule& Http = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http.CreateRequest();

	Request->SetURL(TEXT("http://localhost:8000/api/skill-log"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	FString Payload = FString::Printf(TEXT(
		"{\"player_id\":\"%s\", \"skill\":\"%s\", \"location\":{\"x\":%.2f, \"y\":%.2f, \"z\":%.2f}}"),
		*Name,
		*SkillName,
		SkillLocation.X, SkillLocation.Y, SkillLocation.Z);

	Request->SetContentAsString(Payload);
	Request->ProcessRequest();
}

void AGGwaGameMode::RequestFlowControllerInit(EModeType ModeType)
{
	if (HasAuthority() && BattleFlowController)
	{
		BattleFlowController->InitBattleMode(ModeType);
	}
}

// ============================================================================
// ASYNC TOKEN VERIFICATION
// ============================================================================

void AGGwaGameMode::ProcessPendingTokenVerifications()
{
	// Process completed token verifications
	TArray<FUniqueNetIdRepl> CompletedVerifications;
	
	for (auto& Pair : PendingTokenVerifications)
	{
		const FUniqueNetIdRepl& UniqueId = Pair.Key;
		FPendingTokenVerification& Verification = Pair.Value;
		
		if (Verification.bCompleted)
		{
			CompletedVerifications.Add(UniqueId);
			
			if (!Verification.bSuccess)
			{
				UE_LOG(LogTemp, Warning, TEXT("[Game Server] Disconnecting player due to failed token verification: %s"), 
					*Verification.ErrorMessage);
				
				for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
				{
					APlayerController* PC = Iterator->Get();
					if (PC && PC->PlayerState->GetUniqueId() == UniqueId)
					{
						PC->ClientTravel(TEXT(""), ETravelType::TRAVEL_Absolute);
						break;
					}
				}
			}
		}
		else
		{
			double CurrentTime = FPlatformTime::Seconds();
			if (CurrentTime - Verification.StartTime > 10.0f)
			{
				CompletedVerifications.Add(UniqueId);
				
				UE_LOG(LogTemp, Warning, TEXT("[Game Server] Token verification timed out for UniqueId: %s"), 
					*UniqueId->ToString());
				
				for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
				{
					APlayerController* PC = Iterator->Get();
					if (PC && PC->PlayerState->GetUniqueId() == UniqueId)
					{
						PC->ClientTravel(TEXT(""), ETravelType::TRAVEL_Absolute);
						break;
					}
				}
			}
		}
	}
	
	for (const FUniqueNetIdRepl& UniqueId : CompletedVerifications)
	{
		PendingTokenVerifications.Remove(UniqueId);
	}
}
