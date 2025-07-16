#pragma once

#include "CoreMinimal.h"

class FAuthModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};