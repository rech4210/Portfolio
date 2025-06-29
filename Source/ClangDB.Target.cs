using UnrealBuildTool;
using System.Collections.Generic;

public class ClangDBTarget : TargetRules
{
	public ClangDBTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Program;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		LaunchModuleName = "ClangDB";
		bPrintToolChainTimingInfo = true;

		ExtraModuleNames.AddRange(new string[] { "MyGame", "ClientModule", "ServerModule", "DatabaseModule", "AuthClientModule" });
	}
}