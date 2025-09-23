using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class MyGameServerTarget : TargetRules
{
	public MyGameServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		// bPrintToolChainTimingInfo = true;

		ExtraModuleNames.AddRange( new string[] { "MyGame", "ServerModule", "AuthServerModule", "DatabaseModule"} );
	}
}
