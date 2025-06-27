using UnrealBuildTool;

public class ShopModule : ModuleRules
{
    public ShopModule(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InventoryModule",
                "DatabaseModule"
            }
        );
               PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "GameSharedModule"
            }
        );
    }
} 