#include "VisualMapSettingsManager.h"
#include "OpenRGBVisualMapPlugin.h"

#include <fstream>
#include "filesystem.h"

const std::string VisualMapSettingsManager::settings_folder = "plugins/settings/";
const std::string VisualMapSettingsManager::saves_folder = "plugins/settings/virtual-controllers/";

std::vector<std::string> VisualMapSettingsManager::GetFileNames()
{
    std::string path = OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + saves_folder;
    std::vector<std::string> filenames;

    for (filesystem::directory_entry entry : filesystem::directory_iterator(path))
    {
        filenames.push_back(entry.path().filename().u8string());
    }

    return filenames;
}

void VisualMapSettingsManager::SaveSettings(std::string filename, json settings)
{
    if(!CreateSettingsDirectory())
    {
        printf("[OpenRGBVisualMapPlugin] Cannot create settings directory.\n");
        return;
    }

    std::ofstream SFile((OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + saves_folder + filename), std::ios::out | std::ios::binary);

    if(SFile)
    {
        try{
            SFile << settings.dump(4);
            SFile.close();
            printf("[OpenRGBVisualMapPlugin] Virtual controller file successfully written.\n");
        }
        catch(const std::exception& e)
        {
            printf("[OpenRGBVisualMapPlugin] Cannot write virtual controller file.\n %s\n", e.what());
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
        catch(const std::exception& e)
        {
             printf("[OpenRGBVisualMapPlugin] Cannot read virtual controller file.\n %s\n", e.what());
        }
    }

    return Settings;
}

bool VisualMapSettingsManager::CreateSettingsDirectory()
{
    std::string settings_directory = OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + settings_folder;

    if(!filesystem::exists(settings_directory))
    {
        if(!filesystem::create_directory(settings_directory))
        {
            return false;
        }
    }    

    std::string saves_directory = OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + saves_folder;

    if(filesystem::exists(saves_directory))
    {
        return true;
    }

    return filesystem::create_directory(saves_directory);
}
