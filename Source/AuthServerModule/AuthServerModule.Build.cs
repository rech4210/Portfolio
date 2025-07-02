using UnrealBuildTool;

public class AuthServerModule : ModuleRules
{
	public AuthServerModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Add jwt-cpp library dependency here later
	}
} 