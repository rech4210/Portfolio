#pragma once

#include "CoreMinimal.h"

class IAuthRequestRouter
{
public:
	virtual ~IAuthRequestRouter() = default;
	virtual void RequestLogin(const FString& Username, const FString& Password, const FString& IP, UObject* CallerContext) = 0;
	virtual void RequestRegistration(const FString& Username, const FString& Password, const FString& IP, UObject* CallerContext) = 0;
};

class GAMESHAREDMODULE_API FAuthRouterRegistry
{
public:
	static void Register(UGameInstance* GI, IAuthRequestRouter* Router);
	static void Unregister(UGameInstance* GI, IAuthRequestRouter* Router);
	static IAuthRequestRouter* Get(UGameInstance* GI);
private:
	static TMap<TWeakObjectPtr<UGameInstance>, IAuthRequestRouter*> Routers;
};
