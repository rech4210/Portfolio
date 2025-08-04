#include "Utils/ClientUIMapping.h"
#include "Engine/Engine.h"

TMap<EClientUIKey, FString> FClientUIMapping::UIClassMappings;
bool FClientUIMapping::bIsInitialized = false;

FString FClientUIMapping::GetClassPath(EClientUIKey UIKey)
{
	if (!bIsInitialized)
	{
		InitializeDefaultMappings();
	}
	
	if (const FString* ClassPath = UIClassMappings.Find(UIKey))
	{
		return *ClassPath;
	}
	
	return FString();
}

UClass* FClientUIMapping::LoadUIClass(EClientUIKey UIKey)
{
#if UE_SERVER
	return nullptr;
#else
	FString ClassPath = GetClassPath(UIKey);
	if (ClassPath.IsEmpty())
	{
		return nullptr;
	}
	
	UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath);
	if (!LoadedClass)
	{
		return nullptr;
	}
	
	return LoadedClass;
#endif
}

bool FClientUIMapping::IsValidClassPath(const FString& ClassPath)
{
	return !ClassPath.IsEmpty() && ClassPath.Contains(TEXT("/Script/"));
}

TMap<EClientUIKey, FString> FClientUIMapping::GetAllMappings()
{
	if (!bIsInitialized)
	{
		InitializeDefaultMappings();
	}
	
	return UIClassMappings;
}

void FClientUIMapping::InitializeDefaultMappings()
{
	if (bIsInitialized)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[CLIENT_UI] Initializing default UI class mappings"));
	
	UIClassMappings.Add(EClientUIKey::AuthComponent, TEXT("/ClientPlugin/BP_ClientAuthComponent.BP_ClientAuthComponent_C"));
	UIClassMappings.Add(EClientUIKey::LoginUI, TEXT("/ClientPlugin/BP_ClientUIComponent.BP_ClientUIComponent_C"));
	UIClassMappings.Add(EClientUIKey::HUD, TEXT("/ClientPlugin/UI/HUD/BP_GGwaHUD.BP_GGwaHUD_C"));

	
	bIsInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("[CLIENT_UI] Initialized %d UI class mappings"), UIClassMappings.Num());
}
