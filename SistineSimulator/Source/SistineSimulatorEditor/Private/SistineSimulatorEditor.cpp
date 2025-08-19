#include "SistineSimulatorEditor.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ArtGraph/ArtGraph.h" 
#include "GraphElementDetailsCustomization.h" 


IMPLEMENT_MODULE(FSistineSimulatorEditorModule, SistineSimulatorEditor);

void FSistineSimulatorEditorModule::StartupModule()
{
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(
        UGraphElement::StaticClass()->GetFName(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FGraphElementDetailsCustomization::MakeInstance)
    );
}

void FSistineSimulatorEditorModule::ShutdownModule()
{
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomClassLayout(UGraphElement::StaticClass()->GetFName());
    }
}
