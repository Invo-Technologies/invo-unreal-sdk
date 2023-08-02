// Copyright Epic Games, Inc. All Rights Reserved.

#include "FunPlugin.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "FunFunctions.h"



#define LOCTEXT_NAMESPACE "FFunPluginModule"

void FFunPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
    if (SettingsModule != nullptr)
    {
        ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "MySection",
            LOCTEXT("RuntimeSettingsName", "Invo Plugin"),
            LOCTEXT("RuntimeSettingsDescription", "Invo plugin setup."),
            GetMutableDefault<UFunFunctions>()
        );

        if (SettingsSection.IsValid())
        {
            SettingsSection->OnModified().BindRaw(this, &FFunPluginModule::HandleSettingsSaved);
        }
    }
}

void FFunPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
    ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
    if (SettingsModule != nullptr)
    {
        SettingsModule->UnregisterSettings("Project", "Plugins", "MySection");
    }
}

bool FFunPluginModule::HandleSettingsSaved()
{
    UFunFunctions* Settings = GetMutableDefault<UFunFunctions>();
    bool ResaveSettings = false;
    // Handle settings changes

    // Don't forget to add code here to handle changes to the settings!
    //Settings->SaveConfig();

    if (ResaveSettings)
    {
        Settings->SaveConfig();
    }

    return true;
}
#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFunPluginModule, FunPlugin)