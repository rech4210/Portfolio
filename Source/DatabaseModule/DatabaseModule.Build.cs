using UnrealBuildTool;
using System.IO;

public class DatabaseModule : ModuleRules
{
	public DatabaseModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
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
				"Json",
				"JsonUtilities",
			}
		);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add a definition to indicate we're using the shared library (DLL)
			PublicDefinitions.Add("CONCPP_BUILD_SHARED");

			// Base path to the MySQL Connector/C++ library
			string MySQLConnectorPath = Path.Combine(ModuleDirectory, "ThirdParty", "MySQLConnector");

			// Add the include path
			PrivateIncludePaths.Add(Path.Combine(MySQLConnectorPath, "include"));

			// Add the library path
			PublicAdditionalLibraries.Add(Path.Combine(MySQLConnectorPath, "lib64", "vs14", "mysqlcppconn.lib"));

			// Add the runtime dependency (DLL)
			string DllPath = Path.Combine(MySQLConnectorPath, "lib64", "mysqlcppconn.dll");
			RuntimeDependencies.Add(DllPath);

			// Add the DLL to the build output
			PublicDelayLoadDLLs.Add("mysqlcppconn.dll");
		}
	}
} 