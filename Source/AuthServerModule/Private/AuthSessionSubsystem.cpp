#include "AuthSessionSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AuthExternalService.h"
#include "AuthJWTUtils.h"
#include "GameFramework/PlayerController.h"
#include "AuthTokenTypes.h"
#include "GameFramework/GameModeBase.h"
#include "GameSharedModule/Public/Interface/AuthRPCInterface.h" // for IAuthRPCInterface

// Console variables (runtime override)
static TAutoConsoleVariable<FString> CVarAuthServerUrl(TEXT("auth.Url"), TEXT("") , TEXT("Override Auth Server Base URL"));
static TAutoConsoleVariable<float> CVarAuthTimeout(TEXT("auth.Timeout"), -1.f , TEXT("Override Auth Request Timeout Seconds (-1 = use config)"));
static TAutoConsoleVariable<float> CVarAuthCacheSweep(TEXT("auth.CacheSweep"), -1.f , TEXT("Override Auth Cache Sweep Interval Seconds (-1 = use config)"));

DEFINE_LOG_CATEGORY(LogServerAuth);

void UAuthSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ExternalService = NewObject<UAuthExternalService>(this);

	FString Url = TEXT("http://127.0.0.1:3000");
	float Timeout = 10.f;
	float SweepInterval = CacheSweepIntervalSeconds;
	if (GConfig)
	{
		GConfig->GetString(TEXT("AuthServer"), TEXT("AuthServerUrl"), Url, GGameIni);
		GConfig->GetFloat(TEXT("AuthServer"), TEXT("AuthRequestTimeoutSeconds"), Timeout, GGameIni);
		GConfig->GetFloat(TEXT("AuthServer"), TEXT("AuthCacheSweepIntervalSeconds"), SweepInterval, GGameIni);
	}
	// CVar precedence
	{
		const FString CVarUrl = CVarAuthServerUrl.GetValueOnAnyThread();
		if (!CVarUrl.IsEmpty()) { Url = CVarUrl; }
		const float CVarTimeoutVal = CVarAuthTimeout.GetValueOnAnyThread();
		if (CVarTimeoutVal >= 0.f) { Timeout = CVarTimeoutVal; }
		const float CVarSweepVal = CVarAuthCacheSweep.GetValueOnAnyThread();
		if (CVarSweepVal >= 0.f) { SweepInterval = CVarSweepVal; }
	}
	CacheSweepIntervalSeconds = SweepInterval; // apply if changed
	ExternalService->Initialize(Url, Timeout);
	ScheduleCacheSweep();
	FAuthRouterRegistry::Register(GetGameInstance(), this);
	UE_LOG(LogServerAuth, Log, TEXT("AuthSessionSubsystem initialized and router registered"));
}

void UAuthSessionSubsystem::Deinitialize()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ExpiryTimerHandle);
	}
	TokenCache.Empty();
	ExternalService = nullptr;
	FAuthRouterRegistry::Unregister(GetGameInstance(), this);
	Super::Deinitialize();
	UE_LOG(LogServerAuth, Log, TEXT("AuthSessionSubsystem deinitialized and router unregistered"));
}

void UAuthSessionSubsystem::LoginUser(const FString& Username, const FString& Password, const FString& ClientIP, UObject* CallerContext)
{
	if (!ExternalService)
	{
		UE_LOG(LogServerAuth, Error, TEXT("LoginUser: ExternalService null"));
		return;
	}
	TWeakObjectPtr<UObject> WeakCaller = CallerContext;
	ExternalService->LoginAsync(Username, Password, ClientIP, FOnAuthLoginResult::CreateWeakLambda(this, [this, WeakCaller](bool bSuccess, const FString& UserId, const FString& Token)
	{
		UObject* Caller = WeakCaller.Get();
		if (!IsValid(Caller)) return;
		IAuthRPCInterface* Bridge = Cast<IAuthRPCInterface>(Caller);
		if (!Bridge) return;
		if (!bSuccess)
		{
			UE_LOG(LogServerAuth, Warning, TEXT("Login failed (user=%s)"), *UserId);
			Bridge->NotifyAuthLoginResult(false, UserId, FString(), TEXT("LoginFailed"));
			return;
		}
		StoreToken(UserId, Token);
		UE_LOG(LogServerAuth, Log, TEXT("Login success user=%s tokenPreview=%s"), *UserId, *Token.Left(12));
		Bridge->NotifyAuthLoginResult(true, UserId, Token, TEXT(""));
	}));
}

void UAuthSessionSubsystem::RegisterUser(const FString& Username, const FString& Password, const FString& ClientIP, UObject* CallerContext)
{
	if (!ExternalService)
	{
		UE_LOG(LogServerAuth, Error, TEXT("RegisterUser: ExternalService null"));
		return;
	}
	TWeakObjectPtr<UObject> WeakCaller = CallerContext;
	ExternalService->RegisterAsync(Username, Password, ClientIP, FOnAuthRegisterResult::CreateWeakLambda(this, [this, WeakCaller](bool bSuccess, const FString& UserId)
	{
		UObject* Caller = WeakCaller.Get();
		if (!IsValid(Caller)) return;
		IAuthRPCInterface* Bridge = Cast<IAuthRPCInterface>(Caller);
		if (!Bridge) return;
		if (!bSuccess)
		{
			UE_LOG(LogServerAuth, Warning, TEXT("Registration failed"));
			Bridge->NotifyAuthRegisterResult(false, UserId, FString(), TEXT("RegistrationFailed"));
			return;
		}
		UE_LOG(LogServerAuth, Log, TEXT("Registration success user=%s"), *UserId);
		Bridge->NotifyAuthRegisterResult(true, UserId, FString(), TEXT(""));
	}));
}

void UAuthSessionSubsystem::VerifyToken(const FString& Token, TFunction<void(bool bValid, const FString& UserId)> Callback)
{
	if (!ExternalService)
	{
		Callback(false, FString());
		return;
	}
	ExternalService->VerifyTokenAsync(Token, FOnAuthVerifyResult::CreateLambda([Callback](bool bValid, const FString& UserId){ Callback(bValid, UserId); }));
}

bool UAuthSessionSubsystem::TryGetTokenByUser(const FString& UserId, FString& OutToken) const
{
	if (const FAuthTokenRecord* Rec = TokenCache.Find(UserId))
	{
		if (!Rec->IsExpired())
		{
			OutToken = Rec->Token;
			return true;
		}
	}
	return false;
}

void UAuthSessionSubsystem::StoreToken(const FString& UserId, const FString& Token)
{
	FAuthTokenRecord Rec;
	Rec.UserId = UserId;
	Rec.Token = Token;
	Rec.IssuedAtUtc = FDateTime::UtcNow();
	int64 ExpEpoch = AuthJWTUtils::ExtractExp(Token);
	if (ExpEpoch > 0)
	{
		// JWT exp is seconds since epoch (UTC)
		FDateTime Epoch(1970,1,1);
		Rec.ExpiresAtUtc = Epoch + FTimespan::FromSeconds(ExpEpoch);
	}
	else
	{
		Rec.ExpiresAtUtc = Rec.IssuedAtUtc + FTimespan::FromHours(1);
	}
	TokenCache.Add(UserId, Rec);
}

void UAuthSessionSubsystem::ScheduleCacheSweep()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(ExpiryTimerHandle, this, &UAuthSessionSubsystem::CacheSweep, CacheSweepIntervalSeconds, true);
	}
}

void UAuthSessionSubsystem::CacheSweep()
{
	const FDateTime Now = FDateTime::UtcNow();
	int32 Removed=0;
	for (auto It = TokenCache.CreateIterator(); It; ++It)
	{
		if (It->Value.IsExpired(Now))
		{
			It.RemoveCurrent();
			++Removed;
		}
	}
	if (Removed>0){ UE_LOG(LogServerAuth, Log, TEXT("CacheSweep removed %d expired tokens"), Removed); }
}

void UAuthSessionSubsystem::PurgeExpired()
{
	CacheSweep();
}
