using UnrealBuildTool;

public class AuthClientModule : ModuleRules
{
	public AuthClientModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore"});

		PrivateDependencyModuleNames.AddRange(new string[] { "HTTP", "Json", "JsonUtilities" });
	}
} 