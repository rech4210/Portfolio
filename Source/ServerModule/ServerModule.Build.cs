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
                "MyGame",
                "AIModule",
                "NavigationSystem",
                "GameplayAbilities",
                "AuthServerModule"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "HTTP",
                "GameplayTags",
                "DatabaseModule"
            }
        );
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("GameplayTagsEditor");
        }
    }
}