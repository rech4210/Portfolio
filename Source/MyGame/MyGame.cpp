// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "Modules/ModuleManager.h"
#include "Shared/Utill/UEnumTagMatchHelper.h"

void FMyGameModule::StartupModule()
{
	UEnumTagMatchHelper::InitializeHelper();
}

void FMyGameModule::ShutdownModule() {
	UEnumTagMatchHelper::Shutdown();
}

IMPLEMENT_PRIMARY_GAME_MODULE(FMyGameModule, MyGame, "MyGame");
