#include "AuthSubsystem.h"
#include "Data/AuthDTO.h"
#include "Repository/AuthRepository.h"
#include "Domain/AuthDomainService.h"
#include "DatabaseModule/Public/DatabaseManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Json.h"
#include "JsonUtilities.h"

void UAuthSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Ensure DatabaseManager is initialized first
	Collection.InitializeDependency(UDatabaseManager::StaticClass());
	Super::Initialize(Collection);

	// Create default repository implementation
	DefaultAuthRepository = NewObject<UAuthRepository>(this, TEXT("DefaultAuthRepository"));
	DefaultAuthRepository->Initialize();

	// Set as default if no other repository is injected
	if (!AuthRepositoryInterface.GetInterface())
	{
		AuthRepositoryInterface = DefaultAuthRepository;
	}

	// Create domain service
	DomainService = NewObject<UAuthDomainService>(this);
	DomainService->Initialize(AuthRepositoryInterface);

	UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Initialized - Repository and Domain Service ready"));
	UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Auth Server URL: %s"), *AuthServerUrl);
}

void UAuthSubsystem::Deinitialize()
{
	DefaultAuthRepository = nullptr;
	AuthRepositoryInterface = nullptr;
	DomainService = nullptr;
	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Deinitialized"));
}

TScriptInterface<IAuthRepositoryInterface> UAuthSubsystem::GetAuthRepository() const
{
	return AuthRepositoryInterface;
}

// ============================================================================
// Use Case Orchestration - App Layer Responsibilities Only
// ============================================================================

void UAuthSubsystem::RequestServerRegistration(const FString& Username, const FString& Password, const FString& ClientIP)
{
	// 1. Authority Validation (App Layer responsibility)
	if (!ValidateServerAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Registration requests should only be made from server"));
		BroadcastRegistrationResult(false, TEXT("Unauthorized request"));
		return;
	}

	if (Username.IsEmpty() || Password.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Invalid registration parameters"));
		BroadcastRegistrationResult(false, TEXT("Username and password are required"));
		return;
	}

	// 2. Transaction Boundary & Logging (App Layer responsibility)
	LogSecurityEvent(TEXT("registration_attempt"), FString::Printf(TEXT("Username: %s, IP: %s"), *Username, *ClientIP));

	// 3. Route to appropriate service based on configuration
	if (bUseExternalAuthServer)
	{
		// Use external Node.js auth server
		FAuthRequestDTO Request;
		Request.Username = Username;
		Request.Password = Password;
		Request.ClientIP = ClientIP;
		Request.UserAgent = TEXT("UE5-Server");

		SendRegistrationToAuthServer(Request);
	}
	else
	{
		// Use internal domain service
		if (!DomainService)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthSubsystem: DomainService not initialized"));
			BroadcastRegistrationResult(false, TEXT("Authentication service unavailable"));
			return;
		}

		FAuthRequestDTO Request;
		Request.Username = Username;
		Request.Password = Password;
		Request.ClientIP = ClientIP;

		// Call domain service asynchronously
		auto RegistrationTask = DomainService->RegisterUser(Request);
		// Note: In a real implementation, you'd need to handle the async result properly
		UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Registration request sent to domain service"));
	}
}

void UAuthSubsystem::RequestServerAuthentication(const FString& Username, const FString& Password, const FString& ClientIP, APlayerController* RequestingController)
{
	// 1. Authority Validation
	if (!ValidateServerAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Authentication requests should only be made from server"));
		BroadcastAuthenticationResult(false, TEXT(""), TEXT(""), RequestingController);
		return;
	}

	if (!RequestingController)
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Invalid PlayerController for authentication"));
		BroadcastAuthenticationResult(false, TEXT(""), TEXT(""));
		return;
	}

	if (Username.IsEmpty() || Password.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Invalid authentication parameters"));
		BroadcastAuthenticationResult(false, TEXT(""), TEXT(""), RequestingController);
		return;
	}

	// 2. Transaction Boundary & Logging
	LogSecurityEvent(TEXT("authentication_attempt"), 
		FString::Printf(TEXT("Username: %s, IP: %s, Controller: %s"), 
			*Username, *ClientIP, *RequestingController->GetName()));

	// 3. Route to appropriate service
	if (bUseExternalAuthServer)
	{
		// Use external Node.js auth server
		FAuthRequestDTO Request;
		Request.Username = Username;
		Request.Password = Password;
		Request.ClientIP = ClientIP;
		Request.UserAgent = TEXT("UE5-Server");

		SendAuthenticationToAuthServer(Request, RequestingController);
	}
	else
	{
		// Use internal domain service
		if (!DomainService)
		{
			UE_LOG(LogTemp, Error, TEXT("AuthSubsystem: DomainService not initialized"));
			BroadcastAuthenticationResult(false, TEXT(""), TEXT(""), RequestingController);
			return;
		}

		FAuthRequestDTO Request;
		Request.Username = Username;
		Request.Password = Password;
		Request.ClientIP = ClientIP;

		// Call domain service asynchronously
		auto AuthTask = DomainService->AuthenticateUser(Request);
		UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Authentication request sent to domain service"));
	}
}

void UAuthSubsystem::VerifyTokenWithAuthServer(const FString& Token, const FString& UserId)
{
	if (!ValidateServerAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Token verification should only be called from server"));
		return;
	}

	if (bUseExternalAuthServer)
	{
		SendTokenVerificationToAuthServer(Token, UserId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Token verification requires external auth server"));
	}
}

void UAuthSubsystem::AdminLockUserAccount(const FString& UserId, const FString& Reason)
{
	if (!ValidateServerAuthority() || !DomainService)
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Admin functions require server authority and domain service"));
		return;
	}

	LogSecurityEvent(TEXT("admin_lock_account"), FString::Printf(TEXT("UserId: %s, Reason: %s"), *UserId, *Reason));

	auto LockTask = DomainService->LockUserAccount(UserId, Reason);
	UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Admin lock request sent for user %s"), *UserId);
}

void UAuthSubsystem::AdminUnlockUserAccount(const FString& UserId)
{
	if (!ValidateServerAuthority() || !DomainService)
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Admin functions require server authority and domain service"));
		return;
	}

	LogSecurityEvent(TEXT("admin_unlock_account"), FString::Printf(TEXT("UserId: %s"), *UserId));

	auto UnlockTask = DomainService->UnlockUserAccount(UserId);
	UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Admin unlock request sent for user %s"), *UserId);
}

void UAuthSubsystem::AdminDeactivateUserAccount(const FString& UserId, const FString& Reason)
{
	if (!ValidateServerAuthority() || !DomainService)
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Admin functions require server authority and domain service"));
		return;
	}

	LogSecurityEvent(TEXT("admin_deactivate_account"), FString::Printf(TEXT("UserId: %s, Reason: %s"), *UserId, *Reason));

	auto DeactivateTask = DomainService->DeactivateUserAccount(UserId, Reason);
	UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Admin deactivate request sent for user %s"), *UserId);
}

// ============================================================================
// External Auth Server Communication (Node.js)
// ============================================================================

void UAuthSubsystem::SendRegistrationToAuthServer(const FAuthRequestDTO& Request)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule.CreateRequest();

	HttpRequest->SetURL(AuthServerUrl + TEXT("/register"));
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetTimeout(RequestTimeoutSeconds);
	HttpRequest->SetContentAsString(CreateAuthRequestJson(Request));

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UAuthSubsystem::OnRegistrationResponse);

	if (HttpRequest->ProcessRequest())
	{
		UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Registration request sent to auth server"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AuthSubsystem: Failed to send registration request"));
		BroadcastRegistrationResult(false, TEXT("Failed to connect to authentication server"));
	}
}

void UAuthSubsystem::SendAuthenticationToAuthServer(const FAuthRequestDTO& Request, APlayerController* RequestingController)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule.CreateRequest();

	HttpRequest->SetURL(AuthServerUrl + TEXT("/login"));
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetTimeout(RequestTimeoutSeconds);
	HttpRequest->SetContentAsString(CreateAuthRequestJson(Request));

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UAuthSubsystem::OnAuthenticationResponse, RequestingController);

	if (HttpRequest->ProcessRequest())
	{
		UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Authentication request sent to auth server"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AuthSubsystem: Failed to send authentication request"));
		BroadcastAuthenticationResult(false, TEXT(""), TEXT(""), RequestingController);
	}
}

void UAuthSubsystem::SendTokenVerificationToAuthServer(const FString& Token, const FString& UserId)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule.CreateRequest();

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("token"), Token);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	HttpRequest->SetURL(AuthServerUrl + TEXT("/verify"));
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetTimeout(RequestTimeoutSeconds);
	HttpRequest->SetContentAsString(RequestBody);

	HttpRequest->OnProcessRequestComplete().BindLambda([this, UserId](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		OnTokenVerificationResponse(Request, Response, bWasSuccessful, UserId);
	});

	if (HttpRequest->ProcessRequest())
	{
		UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Token verification request sent to auth server"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AuthSubsystem: Failed to send token verification request"));
	}
}

// ============================================================================
// HTTP Response Handlers
// ============================================================================

void UAuthSubsystem::OnRegistrationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AuthSubsystem: Registration request failed"));
		BroadcastRegistrationResult(false, TEXT("Failed to connect to authentication server"));
		return;
	}

	const int32 ResponseCode = Response->GetResponseCode();
	const FString ResponseBody = Response->GetContentAsString();

	UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Registration response received - Code: %d"), ResponseCode);

	if (ResponseCode == 201) // Created
	{
		// Parse success response
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FString Message;
			JsonObject->TryGetStringField(TEXT("message"), Message);
			
			LogSecurityEvent(TEXT("registration_success"), Message);
			BroadcastRegistrationResult(true, Message);
		}
		else
		{
			BroadcastRegistrationResult(true, TEXT("User registered successfully"));
		}
	}
	else
	{
		// Parse error response
		FString ErrorMessage = TEXT("Registration failed");
		
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			JsonObject->TryGetStringField(TEXT("message"), ErrorMessage);
		}

		LogSecurityEvent(TEXT("registration_failed"), FString::Printf(TEXT("Code: %d, Message: %s"), ResponseCode, *ErrorMessage));
		BroadcastRegistrationResult(false, ErrorMessage);
	}
}

void UAuthSubsystem::OnAuthenticationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, APlayerController* RequestingController)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AuthSubsystem: Authentication request failed"));
		BroadcastAuthenticationResult(false, TEXT(""), TEXT(""), RequestingController);
		return;
	}

	const int32 ResponseCode = Response->GetResponseCode();
	const FString ResponseBody = Response->GetContentAsString();

	UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Authentication response received - Code: %d"), ResponseCode);

	if (ResponseCode == 200) // OK
	{
		// Parse success response
		FAuthResponseDTO AuthResponse;
		if (ParseAuthResponseJson(ResponseBody, AuthResponse) && AuthResponse.bIsSuccess)
		{
			LogSecurityEvent(TEXT("authentication_success"), 
				FString::Printf(TEXT("UserId: %s"), *AuthResponse.UserId));

			// Load game data for authenticated user
			LoadGameDataForUser(AuthResponse.UserId, RequestingController);
			
			BroadcastAuthenticationResult(true, AuthResponse.Token, AuthResponse.UserId, RequestingController);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AuthSubsystem: Failed to parse authentication response"));
			BroadcastAuthenticationResult(false, TEXT(""), TEXT(""), RequestingController);
		}
	}
	else
	{
		// Parse error response
		FString ErrorMessage = TEXT("Authentication failed");
		
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			JsonObject->TryGetStringField(TEXT("message"), ErrorMessage);
		}

		LogSecurityEvent(TEXT("authentication_failed"), FString::Printf(TEXT("Code: %d, Message: %s"), ResponseCode, *ErrorMessage));
		BroadcastAuthenticationResult(false, TEXT(""), TEXT(""), RequestingController);
	}
}

void UAuthSubsystem::OnTokenVerificationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, const FString& UserId)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AuthSubsystem: Token verification request failed for user %s"), *UserId);
		return;
	}

	const int32 ResponseCode = Response->GetResponseCode();
	const FString ResponseBody = Response->GetContentAsString();

	if (ResponseCode == 200)
	{
		UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Token verification successful for user %s"), *UserId);
		LogSecurityEvent(TEXT("token_verification_success"), FString::Printf(TEXT("UserId: %s"), *UserId));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Token verification failed for user %s"), *UserId);
		LogSecurityEvent(TEXT("token_verification_failed"), FString::Printf(TEXT("UserId: %s, Code: %d"), *UserId, ResponseCode));
	}
}

// ============================================================================
// Helper Methods
// ============================================================================

bool UAuthSubsystem::ValidateServerAuthority() const
{
	if (GetWorld())
	{
		return GetWorld()->GetNetMode() != NM_Client;
	}
	return false;
}

void UAuthSubsystem::BroadcastAuthenticationResult(bool bSuccess, const FString& Token, const FString& UserId, APlayerController* TargetController)
{
	OnServerAuthenticationComplete.Broadcast(bSuccess, Token, UserId);
	
	// Additional logging
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Broadcasting successful authentication for user %s"), *UserId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Broadcasting failed authentication"));
	}
}

void UAuthSubsystem::BroadcastRegistrationResult(bool bSuccess, const FString& Message)
{
	OnServerRegistrationComplete.Broadcast(bSuccess, Message);
	
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Broadcasting successful registration"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Broadcasting failed registration - %s"), *Message);
	}
}

void UAuthSubsystem::LogSecurityEvent(const FString& Event, const FString& Details) const
{
	UE_LOG(LogTemp, Warning, TEXT("SECURITY EVENT - %s: %s"), *Event, *Details);
	
	// TODO: In production, send to security monitoring system
	// This could integrate with your existing DatabaseManager for security audit logs
}

FString UAuthSubsystem::CreateAuthRequestJson(const FAuthRequestDTO& Request) const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("username"), Request.Username);
	JsonObject->SetStringField(TEXT("password"), Request.Password);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	return RequestBody;
}

bool UAuthSubsystem::ParseAuthResponseJson(const FString& ResponseBody, FAuthResponseDTO& OutResponse) const
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}

	FString Token;
	if (JsonObject->TryGetStringField(TEXT("token"), Token))
	{
		OutResponse.bIsSuccess = true;
		OutResponse.Token = Token;
		
		// Extract userId from JWT payload (simplified for this example)
		// In production, you'd properly decode the JWT
		OutResponse.UserId = TEXT("extracted_from_jwt");
		return true;
	}

	return false;
}

void UAuthSubsystem::LoadGameDataForUser(const FString& UserId, APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthSubsystem: Cannot load game data - invalid PlayerController"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Loading game data for user %s"), *UserId);

	// TODO: Load character data, inventory, skills etc. from DatabaseManager
	// This should integrate with your existing game data loading systems
	
	// For now, simulate successful data loading
	OnGameDataLoaded(true, UserId, PlayerController);
}

void UAuthSubsystem::OnGameDataLoaded(bool bSuccess, const FString& UserId, APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("AuthSubsystem: Game data loaded successfully for user %s"), *UserId);
		
		// TODO: Trigger ClientTravel to move player to game world
		// PlayerController->ClientTravel(GameWorldURL, TRAVEL_Relative);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AuthSubsystem: Failed to load game data for user %s"), *UserId);
		
		// TODO: Handle data loading failure
		// Maybe disconnect the player or retry
	}
}
