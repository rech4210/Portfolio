using UnrealBuildTool;

public class InventoryModule : ModuleRules
{
    public InventoryModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "GameSharedModule"
            }
        );
        //     PrivateDependencyModuleNames.AddRange(
        //     new string[]
        //     {
        //         "GameSharedModule"
        //     }
        // );

    }
} 