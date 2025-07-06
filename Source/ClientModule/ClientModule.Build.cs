using UnrealBuildTool;

public class ClientModule : ModuleRules
{
    public ClientModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "MyGame",
                "GameSharedModule",
                "SkillModule",
                "EquipmentModule"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UMG",
                "RenderCore",
                "RHI",
                "GameplayAbilities",
                "GameplayTags",
                "AuthClientModule"
            }
        );
    }
}