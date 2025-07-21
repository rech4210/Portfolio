using UnrealBuildTool;

public class AuthServerModule : ModuleRules
{
	public AuthServerModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Json", "JsonUtilities", "HTTP" ,"NetCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Add jwt-cpp library dependency here later
	}
} 