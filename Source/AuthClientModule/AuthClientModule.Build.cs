using UnrealBuildTool;

public class AuthClientModule : ModuleRules
{
	public AuthClientModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"GameSharedModule" // Add dependency to GameSharedModule for AuthRPC interface
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "HTTP", "Json", "JsonUtilities" });
	}
} 