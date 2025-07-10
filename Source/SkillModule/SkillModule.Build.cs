using UnrealBuildTool;

public class SkillModule : ModuleRules
{
    public SkillModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks",
                "GameSharedModule",
                "DatabaseModule"
            }
        );
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "EnhancedInput",
                "InputCore",
                "Json"
            }
        );
    }
} 