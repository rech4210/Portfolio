#include "GGwaGameMode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Shared/Player/GGwaPlayerController.h"
#include "Shared/Player/GGwaPlayerState.h"
#include "Engine/Engine.h"	// Options 문자열에서 토큰 파싱
#include "HttpModule.h"
#include "GameMode/BattleFlowController.h"
#include "AuthVerificationService.h" // 서비스 헤더 Include
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"
// #include "DatabaseModule/Public/DatabaseManager.h"
// #include "DatabaseModule/Public/Data/FCharacterData.h"
// #include "AuthSubsystem.h"
// #include "InventoryDomainService.h"
#include "InventorySubsystem.h"
#include "ShopSubsystem.h"
#include "SkillSubsystem.h"
#include "MyGame/Public/Shared/GameState/GGwaGameState.h"
#include "MyGame/Public/Shared/Cache/UIConfigCacheActor.h"
#include "Utill/LocalDataBaseLoader.h"

// TODO: The concrete implementation class headers would be here.
// For example: #include "DatabaseModule/Public/MySQLInventoryRepository.h"

constexpr static float GAME_MODE_FREQUENCY = 3.0f;

AGGwaGameMode::AGGwaGameMode()
{
	// 서비스 인스턴스 생성
	AuthVerificationService = NewObject<UAuthVerificationService>(this, TEXT("AuthVerificationService"));

	// Set custom GameState class
	GameStateClass = AGGwaGameState::StaticClass();

	// TODO: EquipmentRepository의 구체적인 구현 클래스로 초기화해야 합니다.
	// EquipmentRepository = NewObject<UEquipmentRepositoryImpl>(this, TEXT("EquipmentRepository")); 
	
	// TODO: DatabaseManager는 별도의 초기화 방식(예: 싱글톤 또는 서비스 로케이터)을 사용할 수 있습니다.
	// DatabaseManager = ...;
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
	
	// Safe parsing with null checks
	if (!FParse::Value(*Options, TEXT("token="), Token))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Game Server] No token parameter found in Options"));
	}
	
	// Parse UserId carefully - extract only the UUID part before any additional parameters
	FString RawUserId;
	if (FParse::Value(*Options, TEXT("userid="), RawUserId))
	{
		// Split by '?' to get only the UUID part, ignoring Name and other parameters
		FString LeftPart, RightPart;
		if (RawUserId.Split(TEXT("?"), &LeftPart, &RightPart))
		{
			ProvidedUserId = LeftPart; // Use only the UUID part
		}
		else
		{
			ProvidedUserId = RawUserId; // No '?' found, use as is
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
	
	// Parameter validation - updated for UUID format
	bool bValidToken = !Token.IsEmpty() && Token.Len() >= 10 && Token.Len() <= 512;
	bool bValidUserId = !ProvidedUserId.IsEmpty() && ProvidedUserId.Len() >= 32 && ProvidedUserId.Len() <= 50; // UUID format
	
	UE_LOG(LogTemp, Warning, TEXT("[Game Server] Token Valid: %s | UserId Valid: %s"), 
		bValidToken ? TEXT("YES") : TEXT("NO"), bValidUserId ? TEXT("YES") : TEXT("NO"));
	
	// if (!bValidToken)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[Game Server] Invalid token format - Length: %d"), Token.Len());
	// }
	
	// if (!bValidUserId)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[Game Server] Invalid UserId format - Length: %d (Expected UUID format)"), ProvidedUserId.Len());
	// }
	//


	// ============================================================================
	// SERVER MODE: SIMPLIFIED AUTHENTICATION 
	// ============================================================================
	
	// Server always allows connections - no external auth service dependency
	// UE_LOG(LogTemp, Warning, TEXT("[SERVER MODE] Direct connection allowed - bypassing external auth"));
	
	// Use provided UserId or generate a fallback
	// FString FinalUserId;
	// if (!ProvidedUserId.IsEmpty())
	// {
	// 	FinalUserId = ProvidedUserId;
	// 	UE_LOG(LogTemp, Log, TEXT("[SERVER MODE] Using provided UserId: %s"), *FinalUserId);
	// }
	// else
	// {
	// 	// Generate fallback UserId from UniqueId
	// 	FinalUserId = FString::Printf(TEXT("server_user_%s"), *UniqueId->ToString().Right(8));
	// 	UE_LOG(LogTemp, Warning, TEXT("[SERVER MODE] Generated fallback UserId: %s"), *FinalUserId);
	// }
	
	// Add to pending players for PostLogin processing
	FPlayerIdentityFair PlayerIdentityFair;
	PlayerIdentityFair.UserId = ProvidedUserId;
	PlayerIdentityFair.Token = Token;
	PendingPlayers.Add(UniqueId, PlayerIdentityFair);
	
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

	// Environment detection
	UWorld* World = GetWorld();
	bool bIsPIEEnvironment = World && World->WorldType == EWorldType::PIE;
	bool bIsDedicatedServer = HasAuthority() && World->GetNetMode() == NM_DedicatedServer;
	FString CurrentMapName = World->GetMapName();
	
	UE_LOG(LogTemp, Warning, TEXT("PIE: %s | DedicatedServer: %s | CurrentMap: %s"), 
		bIsPIEEnvironment ? TEXT("Yes") : TEXT("No"),
		bIsDedicatedServer ? TEXT("Yes") : TEXT("No"),
		*CurrentMapName);

	// ============================================================================
	// INITIAL MAP TRANSITION LOGIC (DEVELOPMENT MODE ONLY)
	// ============================================================================
	
	// Development mode auto-transition for standalone testing
	bool bDevelopmentMode = UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG;
	bool bIsLoginLevel = CurrentMapName.Contains(TEXT("LoginLevel"));
	
	UE_LOG(LogTemp, Warning, TEXT("[DEV MODE] bDevelopmentMode: %s | bIsDedicatedServer: %s | bIsLoginLevel: %s"), 
		bDevelopmentMode ? TEXT("Yes") : TEXT("No"),
		bIsDedicatedServer ? TEXT("Yes") : TEXT("No"),
		bIsLoginLevel ? TEXT("Yes") : TEXT("No"));
	
	// // PIE 환경에서는 자동 전환하지 않음 (클라이언트 인증 테스트를 위해)
	// if (bIsPIEEnvironment)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("[PIE MODE] Auto-transition disabled for authentication testing"));
	// 	return;
	// }
	//
	// if (bDevelopmentMode && bIsDedicatedServer && bIsLoginLevel)
	// {
	// 	// Check if we should auto-transition for testing
	// 	bool bAutoTransition = FParse::Param(FCommandLine::Get(), TEXT("AutoStart")) ||
	// 						   FParse::Param(FCommandLine::Get(), TEXT("TestMode"));
	// 	
	// 	if (bAutoTransition)
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("[DEV MODE] Auto-transitioning to MainMap for testing"));
	// 		
	// 		FTimerHandle AutoTransitionTimer;
	// 		GetWorldTimerManager().SetTimer(
	// 			AutoTransitionTimer,
	// 			[this]()
	// 			{
	// 				UE_LOG(LogTemp, Warning, TEXT("[DEV MODE] Executing auto ServerTravel to MainMap"));
	// 				// GetWorld()->ServerTravel(TEXT("/Game/Maps/ThirdPersonMap"), true, false);
	// 			},
	// 			3.0f, // 3 second delay to allow for potential player connections
	// 			false
	// 		);
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("[DEV MODE] LoginLevel detected, waiting for player authentication"));
	// 	}
	// }

	// ============================================================================
	// SERVER INITIALIZATION
	// ============================================================================

	// Server only: Create and configure UI Cache Actor
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

			// Initialize default mappings
			CacheActor->InitializeDefaultMappings();

			// Set cache actor in GameState
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
}

void AGGwaGameMode::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	
	// Token verification removed - server uses direct authentication
	
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
		// BattleFlowController creation
		BattleFlowController = NewObject<UBattleFlowController>(this, TEXT("BattleFlowController"));

		// 레포지토리 초기화는 생성자에서 수행됩니다.
		// 이곳에서는 필요 시 추가적인 초기화 로직을 수행할 수 있습니다.
		
		// After initialization, load all shop data.
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

	UE_LOG(LogTemp, Warning, TEXT("=== Player Data Initialization Pipeline Start ==="));

	if (!NewPlayer || !NewPlayer->PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("[FAIL] NewPlayer or PlayerState is not available on PostLogin!"));
		return;
	}


	// ============================================================================
	// AUTHENTICATION & USER ID RESOLUTION
	// ============================================================================
	
	// if (!*PlayerIdentityFair->UserId)
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("[FAIL] Player %s is not authenticated. Authentication not found in PendingPlayers."), 
	// 		*NewPlayer->PlayerState->GetPlayerName());
	// 	UE_LOG(LogTemp, Warning, TEXT("[ACTION] Redirecting unauthorized player to login screen"));
	// 	
	// 	NewPlayer->ClientTravel(TEXT("/Game/Login/LoginLevel"), ETravelType::TRAVEL_Absolute);
	// 	return;
	// }

	// Get cached credentials from PlayerController
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

	// Remove from pending list
	PendingPlayers.Remove(GGwaPlayerState->GetUniqueId());

	//
	// // Validate UserID format (should be UUID string)
	// if (FinalUserId.IsEmpty() || FinalUserId.Len() < 32)
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("[FAIL] Invalid UserId format: '%s' for player %s"), 
	// 		*FinalUserId, *NewPlayer->PlayerState->GetPlayerName());
	// 	NewPlayer->ClientTravel(TEXT("/Game/Login/LoginLevel"), ETravelType::TRAVEL_Absolute);
	// 	return;
	// }

	// ============================================================================
	// MAP TRANSITION LOGIC - Check if we should move to MainMap
	// ============================================================================
	
	// Check current map and decide on transition
	FString CurrentMapName = GetWorld()->GetMapName();
	UE_LOG(LogTemp, Warning, TEXT("[SERVER] Current Map: %s)"), *CurrentMapName);
	
	// if (bIsLoginLevel)
	// {
	// 	// Check if we have enough players for transition (configurable)
	// 	int32 AuthenticatedPlayerCount = GetNumPlayers();
	// 	int32 RequiredPlayers = 1; // Minimum players to start game
	// 	
	// 	UE_LOG(LogTemp, Warning, TEXT("[SERVER] Authenticated Players: %d | Required: %d"), 
	// 		AuthenticatedPlayerCount, RequiredPlayers);
	// 	
	// 	// Only transition when we have enough players
	// 	if (AuthenticatedPlayerCount >= RequiredPlayers)
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Sufficient players authenticated, scheduling MainMap transition"));
	// 		
	// 		// Schedule the transition to avoid immediate travel during PostLogin
	// 		FTimerHandle DelayedTransitionTimer;
	// 		GetWorldTimerManager().SetTimer(
	// 			DelayedTransitionTimer,
	// 			[this]()
	// 			{
	// 				UE_LOG(LogTemp, Warning, TEXT("[SERVER] Executing delayed ServerTravel to MainMap"));
	// 				// GetWorld()->ServerTravel(TEXT("/Game/Maps/ThirdPersonMap"), true, false);
	// 			},
	// 			2.0f, // 2 second delay to ensure all players are fully connected
	// 			false
	// 		);
	// 		
	// 		// Don't initialize DDD systems yet - wait for main map
	// 		UE_LOG(LogTemp, Warning, TEXT("[SERVER] DDD system initialization deferred until MainMap"));
	// 		return;
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Waiting for more players before transitioning to MainMap"));
	// 	}

	// ============================================================================
	// STEP 1: Initialize DDD Systems First (Required for UI Data)
	// ============================================================================
	
	InitializePlayerDDDSystems(NewPlayer, GGwaPlayerState, FinalUserId);

	UE_LOG(LogTemp, Warning, TEXT("=== END POST LOGIN ==="));
}

void AGGwaGameMode::InitializePlayerDDDSystems(APlayerController* NewPlayer, AGGwaPlayerState* PlayerState, const FString& UserId)
{
	// if (!NewPlayer || !PlayerState || UserId.IsEmpty())
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("[FAIL] Invalid parameters for DDD system initialization - UserId: '%s'"), *UserId);
	// 	return;
	// }

	UE_LOG(LogTemp, Log, TEXT("=== Initializing DDD Systems for Player %s (UserId: %s) ==="), 
		*PlayerState->GetPlayerName(), *UserId);

	// ============================================================================
	// 1. INVENTORY SYSTEM INITIALIZATION
	// ============================================================================
	
	UE_LOG(LogTemp, Log, TEXT("[INVENTORY] Initializing inventory system..."));
	if (auto InventorySubsystem = GetGameInstance()->GetSubsystem<UInventorySubsystem>())
	{
		InventorySubsystem->RequestLoadPlayerInventory(PlayerState);
		UE_LOG(LogTemp, Log, TEXT("[INVENTORY] ✓ Inventory loading initiated"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[INVENTORY] ✗ InventorySubsystem not available"));
	}

	// ============================================================================
	// 2. SKILL SYSTEM INITIALIZATION  
	// ============================================================================
	
	UE_LOG(LogTemp, Log, TEXT("[SKILL] Initializing skill system..."));
	if (auto SkillSubsystem = GetGameInstance()->GetSubsystem<USkillSubsystem>())
	{
		// Bind to skill data loading completion for UI initialization
		SkillSubsystem->OnSkillDataLoadCompleted.AddDynamic(this, &AGGwaGameMode::OnSkillDataLoadCompleted);
		
		// Load all player skills using the optimized LoadPlayerSkills method with string UserId
		SkillSubsystem->RequestLoadPlayerSkills(PlayerState, UserId);
		UE_LOG(LogTemp, Log, TEXT("[SKILL] ✓ Skill loading initiated for UserId: %s"), *UserId);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[SKILL] ✗ SkillSubsystem not available"));
	}

	// ============================================================================
	// 3. SHOP SYSTEM INITIALIZATION
	// ============================================================================
	
	UE_LOG(LogTemp, Log, TEXT("[SHOP] Initializing shop system..."));
	if (auto ShopSubsystem = GetGameInstance()->GetSubsystem<UShopSubsystem>())
	{
		// Load shop data for player (if needed)
		// ShopSubsystem->RequestLoadPlayerShopData(PlayerState, UserId);
		UE_LOG(LogTemp, Log, TEXT("[SHOP] ✓ Shop system ready"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[SHOP] ✗ ShopSubsystem not available"));
	}

	// ============================================================================
	// 4. EQUIPMENT SYSTEM INITIALIZATION (Future Implementation)
	// ============================================================================
	
	UE_LOG(LogTemp, Log, TEXT("[EQUIPMENT] Equipment system initialization deferred (not implemented)"));
	// TODO: Implement equipment system initialization when EquipmentSubsystem is available
	// if (auto EquipmentSubsystem = GetGameInstance()->GetSubsystem<UEquipmentSubsystem>())
	// {
	//     EquipmentSubsystem->RequestLoadPlayerEquipment(PlayerState, UserId);
	//     UE_LOG(LogTemp, Log, TEXT("[EQUIPMENT] ✓ Equipment loading initiated"));
	// }

	// ============================================================================
	// 5. CHARACTER DATA SYSTEM INITIALIZATION (Future Implementation)
	// ============================================================================
	
	UE_LOG(LogTemp, Log, TEXT("[CHARACTER] Character system initialization deferred (not implemented)"));
	// TODO: Implement character data system initialization when CharacterSubsystem is available
	// if (auto CharacterSubsystem = GetGameInstance()->GetSubsystem<UCharacterSubsystem>())
	// {
	//     CharacterSubsystem->RequestLoadPlayerCharacterData(PlayerState, UserId);
	//     UE_LOG(LogTemp, Log, TEXT("[CHARACTER] ✓ Character data loading initiated"));
	// }

	UE_LOG(LogTemp, Log, TEXT("=== DDD Systems Initialization Completed ==="));
}

void AGGwaGameMode::OnSkillDataLoadCompleted(TScriptInterface<IPlayerIdentityInterface> PlayerIdentity, USkillComponent* SkillComponent)
{
	if (!PlayerIdentity || !SkillComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[UI] Invalid parameters for UI initialization"));
		return;
	}
	if (AGGwaPlayerState* GGwaState = Cast<AGGwaPlayerState>(PlayerIdentity.GetObject())) {
		if (AGGwaPlayerController * GGwaController = Cast<AGGwaPlayerController>(GGwaState->GetPlayerController())) {
			GGwaController->InitializeUI(SkillComponent);
		}
	}
	// //
	// // UE_LOG(LogTemp, Log, TEXT("[UI] Skill data loaded, initializing UI for player %s"), 
	// // 	*PlayerIdentity->GetPlayerGuid().ToString());
	//
	// // Find the PlayerController for this PlayerState
	// UWorld* World = GetWorld();
	// if (!World)
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("[UI] World not available for UI initialization"));
	// 	return;
	// }
	//
	// for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	// {
	// 	APlayerController* PC = Iterator->Get();
	// 	if (!PC || !PC->PlayerState)
	// 		continue;
	//
	// 	// Check if this PlayerController belongs to the PlayerState we're looking for
	// 	if (PC->PlayerState == PlayerIdentity.GetObject())
	// 	{
	// 		// Cast to GGwaPlayerController to access InitializeUI
	// 		if (auto GGwaPC = Cast<AGGwaPlayerController>(PC))
	// 		{
	// 			UE_LOG(LogTemp, Log, TEXT("[UI] Initializing UI system for player %s with loaded skill data"), 
	// 				*PC->PlayerState->GetPlayerName());
	// 			
	// 			// Initialize UI with the loaded SkillComponent
	// 			GGwaPC->InitializeUI(SkillComponent);
	// 			
	// 			UE_LOG(LogTemp, Log, TEXT("[UI] ✓ UI initialization completed for player %s"), 
	// 				*PC->PlayerState->GetPlayerName());
	// 			return;
	// 		}
	// 		else
	// 		{
	// 			UE_LOG(LogTemp, Error, TEXT("[UI] Could not cast PlayerController to AGGwaPlayerController"));
	// 		}
	// 		break;
	// 	}
	// }
	
	UE_LOG(LogTemp, Warning, TEXT("[UI] PlayerController not found for UI initialization"));
}

void AGGwaGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	/*	FObjectPtr(const FObjectPtr& InOther)
		: Handle(InOther.Handle)
	{
	null 뜬다. 아마 로그아웃이라?
		ConditionallyMarkAsReachable(*this);
	}*/
	// auto Inventoryrepo = GetGameInstance()->GetSubsystem<UInventorySubsystem>()->GetInventoryRepository();
	// if (Exiting && Inventoryrepo)
	// {
	// 	APlayerController* PC = Cast<APlayerController>(Exiting);
	//
	// 	
	// 	if(PC && PC->PlayerState)
	// 	{
	// 		// Use the repository to save the player's inventory.
	// 		UE_LOG(LogTemp, Log, TEXT("Player logging out. Saving inventory data for %s."), *PC->PlayerState->GetPlayerName());
	// 		if (auto InventorySubsystem = GetGameInstance()->GetSubsystem<UInventorySubsystem>())
	// 		{
	// 			UE_LOG(LogTemp, Warning, TEXT("[START] Save inventory for player %s"), *PC->PlayerState->GetPlayerName());
	// 			// UInventoryDomainService* DomainService = InventorySubsystem->CreateDomainService();
	// 			// PC->PlayerState->GetComponentByClass<UInventoryComponent>()->GetItems();
	// 			// DomainService->SaveInventory(PC->PlayerState, DomainService->LoadInventory(PC->PlayerState).InventoryData);
	// 			// TODO: 저장 로직이 의도된 것인지 확인이 필요합니다. (예: 신규 유저의 기본 상태 저장)
	// 			// SkillStateRepository->SaveSkillState(PC->PlayerState->GetPlayerId(), PlayerState->GetSkillComponent(), TODO);
	// 			
	// 			UE_LOG(LogTemp, Warning, TEXT("[SUCCESS] Inventory Saved"));
	// 		}
	// 		// Unload Repo, Clear Component.
	// 	}
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("AGGwaGameMode: InventoryRepository is not available on Logout!"));
	// }
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

// void AGGwaGameMode::OnTokenVerificationComplete(bool bSuccess, const FString& VerifiedUserId, const FUniqueNetIdRepl& UniqueId)
// {
// 	// This function is no longer used - server uses direct authentication
// 	UE_LOG(LogTemp, Warning, TEXT("[LEGACY] OnTokenVerificationComplete called but no longer used in server mode"));
// }

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
				// Disconnect player due to failed token verification
				UE_LOG(LogTemp, Warning, TEXT("[Game Server] Disconnecting player due to failed token verification: %s"), 
					*Verification.ErrorMessage);
				
				// Find the player controller and disconnect
				for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
				{
					APlayerController* PC = Iterator->Get();
					if (PC && PC->PlayerState->GetUniqueId() == UniqueId)
					{
						// Use ClientTravel to disconnect player instead of Close()
						PC->ClientTravel(TEXT(""), ETravelType::TRAVEL_Absolute);
						break;
					}
				}
			}
		}
		else
		{
			// Check for timeout
			double CurrentTime = FPlatformTime::Seconds();
			if (CurrentTime - Verification.StartTime > 10.0f) // 10 second timeout
			{
				CompletedVerifications.Add(UniqueId);
				
				UE_LOG(LogTemp, Warning, TEXT("[Game Server] Token verification timed out for UniqueId: %s"), 
					*UniqueId->ToString());
				
				// Disconnect player due to timeout
				for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
				{
					APlayerController* PC = Iterator->Get();
					if (PC && PC->PlayerState->GetUniqueId() == UniqueId)
					{
						// Use ClientTravel to disconnect player instead of Close()
						PC->ClientTravel(TEXT(""), ETravelType::TRAVEL_Absolute);
						break;
					}
				}
			}
		}
	}
	
	// Remove completed verifications
	for (const FUniqueNetIdRepl& UniqueId : CompletedVerifications)
	{
		PendingTokenVerifications.Remove(UniqueId);
	}
}
