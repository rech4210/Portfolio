  #pragma once

  #include "CoreMinimal.h"
  #include "UObject/NoExportTypes.h"
  #include "AuthVerificationService.generated.h"

  UCLASS()
  class AUTHSERVERMODULE_API UAuthVerificationService : public UObject
  {
      GENERATED_BODY()

  public:
      /**
       * Verifies a JWT and extracts the user ID.
       * @param Token The JWT string from the client.
       * @param OutUserId If verification is successful, this will contain the user ID.
       * @return True if the token is valid, false otherwise.
       * jwt-cpp 연동: 현재 AuthVerificationService는 임시 로직으로 되어 있습니다. 실제 운영을 위해서는 jwt-cpp 라이브러리를 AuthServerModule에 연동하여 실제 토큰 검증을 구현해야 합니다.
       */

      
      bool VerifyToken(const FString& Token, FString& OutUserId);
  };