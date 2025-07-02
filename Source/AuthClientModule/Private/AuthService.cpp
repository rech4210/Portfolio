#include "AuthService.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"

UAuthService::UAuthService()
{
    AuthServerUrl = TEXT("http://127.0.0.1:3000");
}

void UAuthService::RequestToken(const FString& UserId, const TArray<FString>& Roles, FLoginSuccessDelegate OnSuccess, FLoginFailureDelegate OnFailure)
{
    UE_LOG(LogTemp, Warning, TEXT("[Auth Service] Requesting token for user: %s"), *UserId);
    UE_LOG(LogTemp, Warning, TEXT("[Auth Service] Server URL: %s"), *AuthServerUrl);

    TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
    RequestObj->SetStringField(TEXT("userId"), UserId);

    TArray<TSharedPtr<FJsonValue>> RolesArray;
    for (const FString& Role : Roles)
    {
        RolesArray.Add(MakeShareable(new FJsonValueString(Role)));
    }
    RequestObj->SetArrayField(TEXT("roles"), RolesArray);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer);

    FHttpModule& HttpModule = FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule.CreateRequest();

    HttpRequest->SetURL(AuthServerUrl / TEXT("login"));
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetContentAsString(RequestBody);
    
    // Binding the response delegate
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UAuthService::OnTokenRequestResponse, OnSuccess, OnFailure);

    HttpRequest->ProcessRequest();
}

void UAuthService::OnTokenRequestResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FLoginSuccessDelegate OnSuccess, FLoginFailureDelegate OnFailure)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        OnFailure.ExecuteIfBound(TEXT("Request failed or response is invalid."));
        return;
    }

    if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
        
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            FString Token;
            if (JsonObject->TryGetStringField(TEXT("token"), Token))
            {
                OnSuccess.ExecuteIfBound(Token);
                return;
            }
        }
        OnFailure.ExecuteIfBound(TEXT("Failed to parse token from response."));
    }
    else
    {
        const FString ErrorMessage = FString::Printf(TEXT("Request failed with code: %d. Response: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
        OnFailure.ExecuteIfBound(ErrorMessage);
    }
}