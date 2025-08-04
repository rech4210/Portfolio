#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameSharedModule/Public/Interface/IClientComponentProvider.h"

class UAuthService;

#include "ClientAuthComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class CLIENTMODULE_API UClientAuthComponent : public UActorComponent, public IClientAuthInterface
{
	GENERATED_BODY()

public:
	UClientAuthComponent();

protected:
	virtual void BeginPlay() override;

public:

	virtual void InitializeAuth() override;

	virtual void RequestRegistration(const FString& Username, const FString& Password) override;

	virtual void RequestLogin(const FString& Username, const FString& Password) override;

	UFUNCTION()
	virtual void OnServerRegistrationResult(bool bSuccess, const FString& Message) override;

	UFUNCTION()
	virtual void OnServerLoginResult(bool bSuccess, const FString& Token, const FString& UserId) override;

	UFUNCTION(BlueprintCallable, Category = "Client Auth")
	void InitializeAuthService() { InitializeAuth(); }

	UFUNCTION(BlueprintCallable, Category = "Client Auth")
	void BP_RequestRegistration(const FString& Username, const FString& Password) { RequestRegistration(Username, Password); }

	UFUNCTION(BlueprintCallable, Category = "Client Auth")
	void BP_RequestLogin(const FString& Username, const FString& Password) { RequestLogin(Username, Password); }

	UFUNCTION(BlueprintImplementableEvent, Category = "Client Auth")
	void BP_OnRegistrationResult(bool bSuccess, const FString& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "Client Auth")
	void BP_OnLoginResult(bool bSuccess, const FString& Token, const FString& UserId);
	virtual TScriptInterface<IClientManagerInterface> GetClientSubSystem() override;

private:

	UPROPERTY()
	TObjectPtr<UAuthService> AuthService;

	UPROPERTY()
	TObjectPtr<class AGGwaPlayerController> OwnerController;

};
