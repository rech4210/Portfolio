using UnrealBuildTool;

public class ServerModule : ModuleRules
{
    public ServerModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "MyGame"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "AIModule",
                "NavigationSystem"
            }
        );
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("GameplayTagsEditor");
        }
    }
}