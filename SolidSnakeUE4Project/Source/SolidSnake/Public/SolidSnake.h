// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"


class FSolidSnake : public IModuleInterface
{
public:
    // FSolidSnake();
    void test();
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    static inline FSolidSnake& Get()
    {
        return FModuleManager::LoadModuleChecked<FSolidSnake>("SolidSnake");
    }
    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("SolidSnake");

    }
};
