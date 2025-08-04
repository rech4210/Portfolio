#pragma once

#include "CoreMinimal.h"
#include "Enums/EClientUIKey.h"

class GAMESHAREDMODULE_API FClientUIMapping
{
public:
	static FString GetClassPath(EClientUIKey UIKey);
	
	static UClass* LoadUIClass(EClientUIKey UIKey);
	
	static bool IsValidClassPath(const FString& ClassPath);
	
	static TMap<EClientUIKey, FString> GetAllMappings();

private:
	static void InitializeDefaultMappings();
	
	static TMap<EClientUIKey, FString> UIClassMappings;
	
	static bool bIsInitialized;
};
