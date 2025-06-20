// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class ClangDB : ModuleRules
{
	public ClangDB(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.NoPCHs;

		PrivateDependencyModuleNames.AddRange(new string[] {
            "MyGame", "ClientModule", "ServerModule",
            "Core", "CoreUObject", "Engine", "InputCore", "GameplayAbilities", "GameplayTasks", "GameplayTags", "EnhancedInput","AIModule","NavigationSystem", "HTTP"
		});
	}
}
