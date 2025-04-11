// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MyGame : ModuleRules
{
	public MyGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "GameplayAbilities", "GameplayTasks", "GameplayTags", "EnhancedInput","AIModule","NavigationSystem"});

		
		//Circular dependency
		// if (Target.Type == TargetType.Client || Target.Type == TargetType.Editor)
		// {
		// 	PrivateDependencyModuleNames.Add("ClientModule");
		// }
		//
		// if (Target.Type == TargetType.Server)
		// {
		// 	PrivateDependencyModuleNames.Add("ServerModule");
		// }

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
