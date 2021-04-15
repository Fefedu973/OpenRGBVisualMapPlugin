#include "VisualMapSettingsManager.h"
#include "OpenRGBVisualMapPlugin.h"

void VisualMapSettingsManager::SaveSettings(json settings)
{
    if(!CreateSettingsDirectory())
    {
        printf("Cannot create settings directory.\n");
        return;
    }

    std::ofstream SFile((OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + SettingsFolder + SettingsFileName), std::ios::out | std::ios::binary);

    if(SFile)
    {
        try{
            SFile << settings.dump(4);
        }
        catch(const std::exception&)
        {
            printf("Cannot write settings.\n");
        }
        SFile.close();
    }
}

json VisualMapSettingsManager::LoadSettings()
{
    json Settings;

    std::ifstream SFile(OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + SettingsFolder + SettingsFileName, std::ios::in | std::ios::binary);

    if(SFile)
    {
        try
        {
            SFile >> Settings;
            SFile.close();
        }
        catch(const std::exception&)
        {
             printf("Cannot read settings.\n");
        }
    }

    return Settings;
}

bool VisualMapSettingsManager::CreateSettingsDirectory()
{
    std::string directory = OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + SettingsFolder;

    if(std::filesystem::exists(directory))
    {
            return true;
    }

    return std::filesystem::create_directory(directory);
}
