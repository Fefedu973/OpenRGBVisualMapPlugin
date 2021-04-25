#include "VisualMapSettingsManager.h"
#include "OpenRGBVisualMapPlugin.h"

#include <fstream>
#include <filesystem>

std::vector<std::string> VisualMapSettingsManager::GetFileNames()
{
    std::string path = OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + saves_folder;
    std::vector<std::string> filenames;

    for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(path))
    {
        filenames.push_back(entry.path().filename().u8string());
    }

    return filenames;
}

void VisualMapSettingsManager::SaveSettings(std::string filename, json settings)
{
    if(!CreateSettingsDirectory())
    {
        printf("Cannot create settings directory.\n");
        return;
    }

    std::ofstream SFile((OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + saves_folder + filename), std::ios::out | std::ios::binary);

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

json VisualMapSettingsManager::LoadSettings(std::string filename)
{
    json Settings;

    std::ifstream SFile(OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + saves_folder + filename, std::ios::in | std::ios::binary);

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
    std::string settings_directory = OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + settings_folder;

    if(std::filesystem::exists(settings_directory))
    {
            return true;
    }

    if(!std::filesystem::create_directory(settings_directory))
    {
        return false;
    }

    std::string saves_directory = OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + saves_folder;

    if(std::filesystem::exists(saves_directory))
    {
            return true;
    }

    return std::filesystem::create_directory(saves_directory);
}
