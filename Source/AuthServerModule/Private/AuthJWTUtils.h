#pragma once
#include "CoreMinimal.h"

namespace AuthJWTUtils
{
	// Very lightweight base64url decode (no padding). Returns empty string on failure.
	static bool Base64UrlDecode(const FString& Input, FString& OutJson)
	{
		FString Working = Input;
		Working.ReplaceInline(TEXT("-"), TEXT("+"));
		Working.ReplaceInline(TEXT("_"), TEXT("/"));
		while ((Working.Len() % 4) != 0) { Working.AppendChar('='); }
		TArray<uint8> Bytes;
		if (!FBase64::Decode(Working, Bytes)) return false;
		OutJson = FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(Bytes.GetData())));
		return true;
	}

	// Extract exp claim (seconds since epoch) from JWT; returns 0 on failure.
	static int64 ExtractExp(const FString& Jwt)
	{
		TArray<FString> Parts; Jwt.ParseIntoArray(Parts, TEXT("."));
		if (Parts.Num() != 3) return 0;
		FString PayloadJson;
		if (!Base64UrlDecode(Parts[1], PayloadJson)) return 0;
		TSharedPtr<FJsonObject> Obj; TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(PayloadJson);
		if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid()) return 0;
		int64 Exp=0; if (Obj->TryGetNumberField(TEXT("exp"), Exp)) return Exp; return 0;
	}
}
