#include "Enums/EClientUIKey.h"

FString FClientUIKeyUtils::ToString(EClientUIKey Key)
{
	switch (Key)
	{
	case EClientUIKey::None: return TEXT("None");
	case EClientUIKey::AuthComponent: return TEXT("AuthComponent");
	case EClientUIKey::LoginUI: return TEXT("LoginUI");
	default: return TEXT("Unknown");
	}
}

EClientUIKey FClientUIKeyUtils::FromString(const FString& KeyString)
{
	if (KeyString == TEXT("AuthComponent")) return EClientUIKey::AuthComponent;
	if (KeyString == TEXT("LoginUI")) return EClientUIKey::LoginUI;
	
	return EClientUIKey::None;
}

bool FClientUIKeyUtils::IsValidKey(EClientUIKey Key)
{
	return Key != EClientUIKey::None && Key < EClientUIKey::MAX;
}
