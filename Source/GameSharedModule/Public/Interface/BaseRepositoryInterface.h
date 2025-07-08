// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "BaseRepositoryInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class GAMESHAREDMODULE_API UBaseRepositoryInterface : public UInterface {
	GENERATED_BODY()
};

/**
 * 
 */
class GAMESHAREDMODULE_API IBaseRepositoryInterface {
	GENERATED_BODY()

public:
	virtual void Initialize() = 0;
};
