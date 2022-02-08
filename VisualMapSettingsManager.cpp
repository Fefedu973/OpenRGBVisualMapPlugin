#include "VisualMapSettingsManager.h"
#include "OpenRGBVisualMapPlugin.h"

#include <fstream>
#include "filesystem.h"

bool VisualMapSettingsManager::SaveMap(std::string filename, json j)
{
    if(!CreateSettingsDirectory())
    {
        return false;
    }

    if(!CreateMapsDirectory())
    {
        return false;
    }

    return write_file(MapsFolder() + folder_separator() + filename, j);
}

json VisualMapSettingsManager::LoadMap(std::string filename)
{
    json j;

    if(!CreateSettingsDirectory())
    {
        return j;
    }

    if(!CreateMapsDirectory())
    {
        return j;
    }

    return load_json_file(MapsFolder() + folder_separator() + filename);
}

std::vector<std::string> VisualMapSettingsManager::GetMapNames()
{
    return list_files(MapsFolder());
}

bool VisualMapSettingsManager::SaveGradient(std::string filename, json j)
{
    if(!CreateSettingsDirectory())
    {
        return false;
    }

    if(!CreateGradientsDirectory())
    {
        return false;
    }

    return write_file(GradientsFolder() + folder_separator() + filename, j);
}

json VisualMapSettingsManager::LoadGradient(std::string filename)
{
    json j;

    if(!CreateSettingsDirectory())
    {
        return j;
    }

    if(!CreateMapsDirectory())
    {
        return j;
    }

    return load_json_file(GradientsFolder() + folder_separator() + filename);
}

std::vector<std::string> VisualMapSettingsManager::GetGradientsNames()
{
    return list_files(GradientsFolder());
}

bool VisualMapSettingsManager::CreateSettingsDirectory()
{
    return create_dir(SettingsFolder());
}

bool VisualMapSettingsManager::CreateMapsDirectory()
{
    return create_dir(MapsFolder());
}

bool VisualMapSettingsManager::CreateGradientsDirectory()
{
    return create_dir(GradientsFolder());
}

std::string VisualMapSettingsManager::SettingsFolder()
{
    return OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() + "plugins" + folder_separator() + "settings";
}

std::string VisualMapSettingsManager::MapsFolder()
{
    return SettingsFolder() + folder_separator() + "virtual-controllers";
}

std::string VisualMapSettingsManager::GradientsFolder()
{
    return SettingsFolder() + folder_separator() + "gradients";
}

std::string VisualMapSettingsManager::folder_separator()
{
#if defined(WIN32) || defined(_WIN32)
    return "\\";
#else
    return "/";
#endif
}

bool VisualMapSettingsManager::write_file(std::string file_name, json j)
{
    std::ofstream file(file_name, std::ios::out | std::ios::binary);

    if(file)
    {
        try
        {
            file << j.dump(4);
            file.close();
        }
        catch(const std::exception& e)
        {
            printf("[OpenRGBVisualMapPlugin] Cannot write file: %s\n", e.what());
            return false;
        }
    }

    return true;
}

json VisualMapSettingsManager::load_json_file(std::string file_name)
{
    json j;

    std::ifstream file(file_name);

    if(file)
    {
        try
        {
            file >> j;
            file.close();
        }
        catch(const std::exception& e)
        {
             printf("[OpenRGBVisualMapPlugin] Cannot read file: %s\n", e.what());
        }
    }

    return j;
}

std::vector<std::string> VisualMapSettingsManager::list_files(std::string path)
{
    std::vector<std::string> filenames;

    if(filesystem::exists(path))
    {
        for (const auto & entry : filesystem::directory_iterator(path))
        {
            filenames.push_back(entry.path().filename().u8string());
        }
    }

    // alphabetical sort
    std::sort(filenames.begin(), filenames.end());

    return filenames;
}

bool VisualMapSettingsManager::create_dir(std::string directory)
{
    if(filesystem::exists(directory))
    {
        return true;
    }

    return filesystem::create_directories(directory);
}
