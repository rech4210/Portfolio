using UnrealBuildTool;
using System.IO;

public class DatabaseModule : ModuleRules
{
	public DatabaseModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnableExceptions = true;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameSharedModule",
				"Settings",
				"Json",
				"JsonUtilities"
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
			}
		);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicDefinitions.Add("STATIC_CONCPP");
			PublicDefinitions.Add("CONCPP_BUILD_SHARED=0");
			string MySQLConnectorPath = Path.Combine(ModuleDirectory, "ThirdParty", "MySQLConnector");

			PrivateIncludePaths.Add(Path.Combine(MySQLConnectorPath, "include"));

			string LibPath = Path.Combine(MySQLConnectorPath, "lib64", "vs14");
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "mysqlcppconn-static.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libcrypto.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "libssl.lib"));
			
			string cryptoDll = Path.Combine(MySQLConnectorPath, "lib64", "libcrypto-3-x64.dll");
			string sslDll = Path.Combine(MySQLConnectorPath, "lib64", "libssl-3-x64.dll");
			
			if (File.Exists(cryptoDll)) RuntimeDependencies.Add(cryptoDll);
			if (File.Exists(sslDll)) RuntimeDependencies.Add(sslDll);
		}
	}
}
