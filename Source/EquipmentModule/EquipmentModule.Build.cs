using UnrealBuildTool;

public class EquipmentModule : ModuleRules
{
    public EquipmentModule(ReadOnlyTargetRules Target) : base(Target)
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

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                // "GameSharedModule" if needed
            }
        );
    }
}
