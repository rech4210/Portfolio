#include "Utils/ClientUIMapping.h"
#include "Engine/Engine.h"

// Static 멤버 초기화
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
	// 서버에서는 클라이언트 UI 클래스 로딩 금지
	return nullptr;
#else
	FString ClassPath = GetClassPath(UIKey);
	if (ClassPath.IsEmpty())
	{
		return nullptr;
	}
	
	// 직접 클래스 로딩
	UClass* LoadedClass = LoadClass<UActorComponent>(nullptr, *ClassPath);
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
	
	// 기본 클라이언트 UI 컴포넌트 매핑 - Blueprint 경로 사용
	UIClassMappings.Add(EClientUIKey::AuthComponent, TEXT("/ClientPlugin/BP_ClientAuthComponent.BP_ClientAuthComponent_C"));
	UIClassMappings.Add(EClientUIKey::LoginUI, TEXT("/ClientPlugin/BP_ClientUIComponent.BP_ClientUIComponent_C"));
	
	bIsInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("[CLIENT_UI] Initialized %d UI class mappings"), UIClassMappings.Num());
}
