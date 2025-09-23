#include "AuthExternalService.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "AuthSessionSubsystem.h" // for LogServerAuth category

static const FString LogPrefix = TEXT("[AuthExternalService]");
static FString GLastAuthErrorMessage; // transient diagnostic (not thread-safe)

void UAuthExternalService::Initialize(const FString& InBaseUrl, float InTimeoutSeconds)
{
	Configure(InBaseUrl, InTimeoutSeconds);
}

void UAuthExternalService::Configure(const FString& InBaseUrl, float InTimeoutSeconds)
{
	BaseUrl = InBaseUrl;
	TimeoutSeconds = InTimeoutSeconds;
	UE_LOG(LogServerAuth, Log, TEXT("%s Configured BaseUrl=%s Timeout=%.2f"), *LogPrefix, *BaseUrl, TimeoutSeconds);
}

void UAuthExternalService::LoginAsync(const FString& Username, const FString& Password, const FString& ClientIP, FOnAuthLoginResult Callback)
{
	if (Username.IsEmpty() || Password.IsEmpty())
	{
		UE_LOG(LogServerAuth, Warning, TEXT("%s LoginAsync invalid parameters"), *LogPrefix);
		GLastAuthErrorMessage = TEXT("InvalidParameters");
		Callback.ExecuteIfBound(false, FString(), FString());
		return;
	}

	FHttpModule& Http = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = Http.CreateRequest();

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("username"), Username);
	Json->SetStringField(TEXT("password"), Password);
	if (!ClientIP.IsEmpty()) Json->SetStringField(TEXT("clientIp"), ClientIP);

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	Req->SetURL(BaseUrl + TEXT("/login"));
	Req->SetVerb(TEXT("POST"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetTimeout(TimeoutSeconds);
	Req->SetContentAsString(Body);

	Req->OnProcessRequestComplete().BindUObject(this, &UAuthExternalService::OnLoginResponse, Callback);

	if (!Req->ProcessRequest())
	{
		UE_LOG(LogServerAuth, Error, TEXT("%s Failed to dispatch login request"), *LogPrefix);
		GLastAuthErrorMessage = TEXT("DispatchFailed");
		Callback.ExecuteIfBound(false, FString(), FString());
	}
	else
	{
		UE_LOG(LogServerAuth, Log, TEXT("%s Login request sent (user=%s)"), *LogPrefix, *Username);
	}
}

void UAuthExternalService::RegisterAsync(const FString& Username, const FString& Password, const FString& ClientIP, FOnAuthRegisterResult Callback)
{
	if (Username.IsEmpty() || Password.IsEmpty())
	{
		UE_LOG(LogServerAuth, Warning, TEXT("%s RegisterAsync invalid parameters"), *LogPrefix);
		GLastAuthErrorMessage = TEXT("InvalidParameters");
		Callback.ExecuteIfBound(false, FString());
		return;
	}

	FHttpModule& Http = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = Http.CreateRequest();

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("username"), Username);
	Json->SetStringField(TEXT("password"), Password);
	if (!ClientIP.IsEmpty()) Json->SetStringField(TEXT("clientIp"), ClientIP);

	FString Body; TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	Req->SetURL(BaseUrl + TEXT("/register"));
	Req->SetVerb(TEXT("POST"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetTimeout(TimeoutSeconds);
	Req->SetContentAsString(Body);
	Req->OnProcessRequestComplete().BindUObject(this, &UAuthExternalService::OnRegisterResponse, Callback);

	if (!Req->ProcessRequest())
	{
		UE_LOG(LogServerAuth, Error, TEXT("%s Failed to dispatch register request"), *LogPrefix);
		GLastAuthErrorMessage = TEXT("DispatchFailed");
		Callback.ExecuteIfBound(false, FString());
	}
	else
	{
		UE_LOG(LogServerAuth, Log, TEXT("%s Register request sent (user=%s)"), *LogPrefix, *Username);
	}
}

void UAuthExternalService::VerifyTokenAsync(const FString& Token, FOnAuthVerifyResult Callback)
{
	if (Token.IsEmpty())
	{
		UE_LOG(LogServerAuth, Warning, TEXT("%s VerifyTokenAsync empty token"), *LogPrefix);
		GLastAuthErrorMessage = TEXT("EmptyToken");
		Callback.ExecuteIfBound(false, FString());
		return;
	}

	FHttpModule& Http = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = Http.CreateRequest();
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("token"), Token);

	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	Req->SetURL(BaseUrl + TEXT("/verify"));
	Req->SetVerb(TEXT("POST"));
	Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Req->SetTimeout(TimeoutSeconds);
	Req->SetContentAsString(Body);

	Req->OnProcessRequestComplete().BindUObject(this, &UAuthExternalService::OnVerifyResponse, Callback);

	if (!Req->ProcessRequest())
	{
		UE_LOG(LogServerAuth, Error, TEXT("%s Failed to dispatch verify request"), *LogPrefix);
		GLastAuthErrorMessage = TEXT("DispatchFailed");
		Callback.ExecuteIfBound(false, FString());
	}
	else
	{
		UE_LOG(LogServerAuth, Log, TEXT("%s Verify request sent (token preview=%s)"), *LogPrefix, *Token.Left(12));
	}
}

void UAuthExternalService::OnLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnAuthLoginResult Callback)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogServerAuth, Error, TEXT("%s Login network failure"), *LogPrefix);
		GLastAuthErrorMessage = TEXT("NetworkFailure");
		Callback.ExecuteIfBound(false, FString(), FString());
		return;
	}

	const int32 Code = Response->GetResponseCode();
	const FString Body = Response->GetContentAsString();
	UE_LOG(LogServerAuth, Log, TEXT("%s Login response code=%d"), *LogPrefix, Code);

	if (Code == 200)
	{
		TSharedPtr<FJsonObject> Root; TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
		if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
		{
			bool bSuccess=false; Root->TryGetBoolField(TEXT("success"), bSuccess);
			if (bSuccess)
			{
				FString Token; Root->TryGetStringField(TEXT("token"), Token);
				const TSharedPtr<FJsonObject>* UserObjPtr=nullptr;
				FString UserId;
				if (Root->TryGetObjectField(TEXT("user"), UserObjPtr) && UserObjPtr && (*UserObjPtr)->TryGetStringField(TEXT("userId"), UserId))
				{
					GLastAuthErrorMessage.Empty();
					Callback.ExecuteIfBound(true, UserId, Token);
					return;
				}
			}
			else
			{
				Root->TryGetStringField(TEXT("message"), GLastAuthErrorMessage);
			}
		}
	}

	UE_LOG(LogServerAuth, Warning, TEXT("%s Login failed code=%d body=%s"), *LogPrefix, Response->GetResponseCode(), *Body.Left(120));
	if (GLastAuthErrorMessage.IsEmpty())
	{
		TSharedPtr<FJsonObject> Root; TSharedRef<TJsonReader<>> R2 = TJsonReaderFactory<>::Create(Body);
		if (FJsonSerializer::Deserialize(R2, Root) && Root.IsValid())
		{
			Root->TryGetStringField(TEXT("message"), GLastAuthErrorMessage);
		}
	}
	Callback.ExecuteIfBound(false, FString(), FString());
}

void UAuthExternalService::OnRegisterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnAuthRegisterResult Callback)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogServerAuth, Error, TEXT("%s Register network failure"), *LogPrefix);
		GLastAuthErrorMessage = TEXT("NetworkFailure");
		Callback.ExecuteIfBound(false, FString());
		return;
	}
	const int32 Code = Response->GetResponseCode();
	const FString Body = Response->GetContentAsString();
	if (Code == 201 || Code == 200)
	{
		FString UserId;
		TSharedPtr<FJsonObject> Root; TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
		if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
		{
			const TSharedPtr<FJsonObject>* UserObj=nullptr;
			if (Root->TryGetObjectField(TEXT("user"), UserObj) && UserObj && (*UserObj)->TryGetStringField(TEXT("userId"), UserId))
			{
				GLastAuthErrorMessage.Empty();
				Callback.ExecuteIfBound(true, UserId);
				return;
			}
			Root->TryGetStringField(TEXT("message"), GLastAuthErrorMessage); // optional message
			Root->TryGetStringField(TEXT("userId"), UserId); // fallback
		}
		Callback.ExecuteIfBound(true, UserId);
		return;
	}
	UE_LOG(LogServerAuth, Warning, TEXT("%s Register failed code=%d body=%s"), *LogPrefix, Code, *Body.Left(120));
	GLastAuthErrorMessage.Empty();
	TSharedPtr<FJsonObject> Root; TSharedRef<TJsonReader<>> R3 = TJsonReaderFactory<>::Create(Body);
	if (FJsonSerializer::Deserialize(R3, Root) && Root.IsValid())
	{
		Root->TryGetStringField(TEXT("message"), GLastAuthErrorMessage);
	}
	Callback.ExecuteIfBound(false, FString());
}

void UAuthExternalService::OnVerifyResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnAuthVerifyResult Callback)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogServerAuth, Error, TEXT("%s Verify network failure"), *LogPrefix);
		GLastAuthErrorMessage = TEXT("NetworkFailure");
		Callback.ExecuteIfBound(false, FString());
		return;
	}

	const int32 Code = Response->GetResponseCode();
	const FString Body = Response->GetContentAsString();
	if (Code == 200)
	{
		TSharedPtr<FJsonObject> Root; TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
		if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
		{
			bool bSuccess=false; Root->TryGetBoolField(TEXT("success"), bSuccess);
			if (bSuccess){
				const TSharedPtr<FJsonObject>* Decoded=nullptr; if (Root->TryGetObjectField(TEXT("decoded"), Decoded) && Decoded){
					FString UserId; if ((*Decoded)->TryGetStringField(TEXT("userId"), UserId)) { GLastAuthErrorMessage.Empty(); Callback.ExecuteIfBound(true, UserId); return; }
				}
			}
			else { Root->TryGetStringField(TEXT("message"), GLastAuthErrorMessage); }
		}
	}
	UE_LOG(LogServerAuth, Warning, TEXT("%s Verify failed code=%d body=%s"), *LogPrefix, Response->GetResponseCode(), *Body.Left(120));
	if (GLastAuthErrorMessage.IsEmpty())
	{
		TSharedPtr<FJsonObject> Root; TSharedRef<TJsonReader<>> R4 = TJsonReaderFactory<>::Create(Body);
		if (FJsonSerializer::Deserialize(R4, Root) && Root.IsValid())
		{
			Root->TryGetStringField(TEXT("message"), GLastAuthErrorMessage);
		}
	}
	Callback.ExecuteIfBound(false, FString());
}
