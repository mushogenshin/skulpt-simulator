#pragma once

#include "Modules/ModuleManager.h"

class FSistineSimulatorEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
