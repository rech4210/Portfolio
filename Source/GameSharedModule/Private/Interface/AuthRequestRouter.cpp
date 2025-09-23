#include "Interface/AuthRequestRouter.h"
#include "Engine/GameInstance.h"

TMap<TWeakObjectPtr<UGameInstance>, IAuthRequestRouter*> FAuthRouterRegistry::Routers;

void FAuthRouterRegistry::Register(UGameInstance* GI, IAuthRequestRouter* Router)
{
	if (!GI || !Router) return;
	IAuthRequestRouter** Existing = Routers.Find(GI);
	if (Existing && *Existing && *Existing != Router)
	{
		UE_LOG(LogTemp, Warning, TEXT("FAuthRouterRegistry::Register overriding existing router for GI=%s"), *GetNameSafe(GI));
	}
	Routers.Add(GI, Router);
}

void FAuthRouterRegistry::Unregister(UGameInstance* GI, IAuthRequestRouter* Router)
{
	if (!GI) return;
	IAuthRequestRouter** Existing = Routers.Find(GI);
	if (Existing && *Existing == Router)
	{
		Routers.Remove(GI);
	}
}

IAuthRequestRouter* FAuthRouterRegistry::Get(UGameInstance* GI)
{
	if (!GI) return nullptr;
	if (IAuthRequestRouter** Found = Routers.Find(GI))
	{
		return *Found;
	}
	return nullptr;
}
