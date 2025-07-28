using UnrealBuildTool;

public class GameSharedModule : ModuleRules
{
    public GameSharedModule(ReadOnlyTargetRules Target) : base(Target)
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
                "UMG",
                "Json",
                "JsonUtilities"
            }
        );
    }
}