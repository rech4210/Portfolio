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
                "GameSharedModule",
                "SkillModule",
                "InventoryModule",
                "AIModule",
                "NavigationSystem",
                "GameplayAbilities",
                "AuthServerModule",
                "DatabaseModule",
                "EquipmentModule"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "MyGame",
                "Engine",
                "Slate",
                "SlateCore",
                "HTTP",
                "GameplayTags",
                "ShopModule",
                "OnlineSubsystem",
                "AuthModule"
            }
        );
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("GameplayTagsEditor");
        }
    }
}