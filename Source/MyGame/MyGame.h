#pragma once

#include "CoreMinimal.h"

class FMyGameModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

