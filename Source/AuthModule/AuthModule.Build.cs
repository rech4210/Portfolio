// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AuthModule : ModuleRules
{
	public AuthModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Exception handling 활성화
		bEnableExceptions = true;
		
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] 
		{ 
			"DatabaseModule",
			"HTTP",
			"Json",
			"JsonUtilities",
			"GameSharedModule"
		});
	}
}
