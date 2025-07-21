  #include "AuthVerificationService.h"

#include "HttpManager.h"
  #include "JsonUtilities.h"

  bool UAuthVerificationService::VerifyToken(const FString& Token, FString& OutUserId)
  {
      // For backward compatibility, keep the old blocking approach
      // but recommend using VerifyTokenAsync for new implementations
      UE_LOG(LogTemp, Warning, TEXT("AuthVerificationService::VerifyToken: Using deprecated blocking verification. Consider using VerifyTokenAsync instead."));
      return VerifyTokenWithServer(Token, OutUserId);
  }

  void UAuthVerificationService::VerifyTokenAsync(const FString& Token, FOnTokenVerified OnComplete)
  {
      if (Token.IsEmpty())
      {
          UE_LOG(LogTemp, Warning, TEXT("AuthVerificationService: Empty token provided for async verification"));
          OnComplete.ExecuteIfBound(false, FString());
          return;
      }

      if (!OnComplete.IsBound())
      {
          UE_LOG(LogTemp, Warning, TEXT("AuthVerificationService: No callback provided for async verification"));
          return;
      }

      UE_LOG(LogTemp, Log, TEXT("AuthVerificationService: Starting async token verification"));
      UE_LOG(LogTemp, VeryVerbose, TEXT("AuthVerificationService: Token: %s"), *Token.Left(20));

      // Create HTTP request to JWT server
      FHttpModule& HttpModule = FHttpModule::Get();
      TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule.CreateRequest();

      // Prepare JSON payload
      TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
      JsonObject->SetStringField(TEXT("token"), Token);

      FString RequestBody;
      TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
      FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

      // Configure request
      HttpRequest->SetURL(AuthServerUrl + TEXT("/verify"));
      HttpRequest->SetVerb(TEXT("POST"));
      HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
      HttpRequest->SetTimeout(RequestTimeoutSeconds);
      HttpRequest->SetContentAsString(RequestBody);

      // Bind async response handler
      HttpRequest->OnProcessRequestComplete().BindUObject(
          this, 
          &UAuthVerificationService::OnTokenVerificationResponse, 
          OnComplete
      );

      // Process request asynchronously
      if (HttpRequest->ProcessRequest())
      {
          UE_LOG(LogTemp, Log, TEXT("AuthVerificationService: Async token verification request sent"));
      }
      else
      {
          UE_LOG(LogTemp, Error, TEXT("AuthVerificationService: Failed to send async token verification request"));
          OnComplete.ExecuteIfBound(false, FString());
      }
  }

  void UAuthVerificationService::OnTokenVerificationResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FOnTokenVerified OnComplete)
  {
      if (!bWasSuccessful || !Response.IsValid())
      {
          UE_LOG(LogTemp, Error, TEXT("AuthVerificationService: Async token verification request failed"));
          OnComplete.ExecuteIfBound(false, FString());
          return;
      }

      const int32 ResponseCode = Response->GetResponseCode();
      const FString ResponseBody = Response->GetContentAsString();

      UE_LOG(LogTemp, Log, TEXT("AuthVerificationService: Async token verification response - Code: %d"), ResponseCode);
      UE_LOG(LogTemp, VeryVerbose, TEXT("AuthVerificationService: Response body: %s"), *ResponseBody);

      // Parse response
      if (ResponseCode == 200)
      {
          // Parse JSON response
          TSharedPtr<FJsonObject> ResponseJson;
          TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
          
          if (FJsonSerializer::Deserialize(Reader, ResponseJson) && ResponseJson.IsValid())
          {
              bool bSuccess = false;
              if (ResponseJson->TryGetBoolField(TEXT("success"), bSuccess) && bSuccess)
              {
                  // Extract decoded token information
                  const TSharedPtr<FJsonObject>* DecodedObject;
                  if (ResponseJson->TryGetObjectField(TEXT("decoded"), DecodedObject) && DecodedObject->IsValid())
                  {
                      FString UserId;
                      if ((*DecodedObject)->TryGetStringField(TEXT("userId"), UserId))
                      {
                          UE_LOG(LogTemp, Log, TEXT("AuthVerificationService: Async token verification successful for user: %s"), *UserId);
                          OnComplete.ExecuteIfBound(true, UserId);
                          return;
                      }
                  }
              }
          }
          
          UE_LOG(LogTemp, Warning, TEXT("AuthVerificationService: Failed to parse successful async response"));
          OnComplete.ExecuteIfBound(false, FString());
      }
      else
      {
          // Token verification failed
          UE_LOG(LogTemp, Warning, TEXT("AuthVerificationService: Async token verification failed - Code: %d, Message: %s"), 
              ResponseCode, *ResponseBody);
          OnComplete.ExecuteIfBound(false, FString());
      }
  }

  bool UAuthVerificationService::VerifyTokenWithServer(const FString& Token, FString& OutUserId)
  {
      UE_LOG(LogTemp, Warning, TEXT("AuthVerificationService::VerifyTokenWithServer: Using DEPRECATED blocking HTTP request method"));
      UE_LOG(LogTemp, Warning, TEXT("AuthVerificationService: This blocking approach may cause issues in PreLogin. Consider using VerifyTokenAsync instead."));

      if (Token.IsEmpty())
      {
          UE_LOG(LogTemp, Warning, TEXT("AuthVerificationService: Empty token provided"));
          return false;
      }

      UE_LOG(LogTemp, Log, TEXT("AuthVerificationService: Verifying token with JWT server (BLOCKING)"));
      UE_LOG(LogTemp, VeryVerbose, TEXT("AuthVerificationService: Token: %s"), *Token.Left(20));

      // Create HTTP request to JWT server
      FHttpModule& HttpModule = FHttpModule::Get();
      TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule.CreateRequest();

      // Prepare JSON payload
      TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
      JsonObject->SetStringField(TEXT("token"), Token);

      FString RequestBody;
      TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
      FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

      // Configure request
      HttpRequest->SetURL(AuthServerUrl + TEXT("/verify"));
      HttpRequest->SetVerb(TEXT("POST"));
      HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
      HttpRequest->SetTimeout(RequestTimeoutSeconds);
      HttpRequest->SetContentAsString(RequestBody);

      // Process request synchronously (blocking) - PROBLEMATIC APPROACH
      HttpRequest->ProcessRequest();

      // Wait for response (blocking approach for PreLogin) - THIS CAN CAUSE DEADLOCKS
      double StartTime = FPlatformTime::Seconds();
      while ((FPlatformTime::Seconds() - StartTime) < RequestTimeoutSeconds)
      {
          // Allow HTTP module to tick and process responses
          FHttpModule::Get().GetHttpManager().Tick(0.01f);
          
          if (HttpRequest->GetStatus() == EHttpRequestStatus::Succeeded || 
              HttpRequest->GetStatus() == EHttpRequestStatus::Failed)
          {
              break;
          }
          
          FPlatformProcess::Sleep(0.01f); // 10ms sleep
      }

      // Check if request completed
      if (HttpRequest->GetStatus() != EHttpRequestStatus::Succeeded)
      {
          UE_LOG(LogTemp, Error, TEXT("AuthVerificationService: HTTP request failed or timed out - Status: %d"), 
              (int32)HttpRequest->GetStatus());
          return false;
      }

      FHttpResponsePtr Response = HttpRequest->GetResponse();
      if (!Response.IsValid())
      {
          UE_LOG(LogTemp, Error, TEXT("AuthVerificationService: Invalid HTTP response"));
          return false;
      }

      const int32 ResponseCode = Response->GetResponseCode();
      const FString ResponseBody = Response->GetContentAsString();

      UE_LOG(LogTemp, Log, TEXT("AuthVerificationService: Token verification response - Code: %d"), ResponseCode);
      UE_LOG(LogTemp, VeryVerbose, TEXT("AuthVerificationService: Response body: %s"), *ResponseBody);

      // Parse response
      if (ResponseCode == 200)
      {
          // Parse JSON response
          TSharedPtr<FJsonObject> ResponseJson;
          TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
          
          if (FJsonSerializer::Deserialize(Reader, ResponseJson) && ResponseJson.IsValid())
          {
              bool bSuccess = false;
              if (ResponseJson->TryGetBoolField(TEXT("success"), bSuccess) && bSuccess)
              {
                  // Extract decoded token information
                  const TSharedPtr<FJsonObject>* DecodedObject;
                  if (ResponseJson->TryGetObjectField(TEXT("decoded"), DecodedObject) && DecodedObject->IsValid())
                  {
                      FString UserId;
                      if ((*DecodedObject)->TryGetStringField(TEXT("userId"), UserId))
                      {
                          OutUserId = UserId;
                          UE_LOG(LogTemp, Log, TEXT("AuthVerificationService: Token verification successful for user: %s"), *UserId);
                          return true;
                      }
                  }
              }
          }
          
          UE_LOG(LogTemp, Warning, TEXT("AuthVerificationService: Failed to parse successful response"));
          return false;
      }
      else
      {
          // Token verification failed
          UE_LOG(LogTemp, Warning, TEXT("AuthVerificationService: Token verification failed - Code: %d, Message: %s"), 
              ResponseCode, *ResponseBody);
          return false;
      }
  }