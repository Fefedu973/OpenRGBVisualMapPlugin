#include "VisualMapSettingsManager.h"
#include "OpenRGBVisualMapPlugin.h"
#include <QFile>
#include <QString>
#include <QDir>
#include <fstream>

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

    return write_file(MapsFolder() / filename, j);
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

    return load_json_file(MapsFolder() / filename);
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

    return write_file(GradientsFolder() / filename, j);
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

    return load_json_file(GradientsFolder() / filename);
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

filesystem::path VisualMapSettingsManager::SettingsFolder()
{
    return OpenRGBVisualMapPlugin::RMPointer->GetConfigurationDirectory() / "plugins" / "settings";
}

filesystem::path VisualMapSettingsManager::MapsFolder()
{
    return SettingsFolder() / "virtual-controllers";
}

filesystem::path VisualMapSettingsManager::GradientsFolder()
{
    return SettingsFolder() / "gradients";
}

bool VisualMapSettingsManager::write_file(filesystem::path file_name, json j)
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

json VisualMapSettingsManager::load_json_file(filesystem::path file_name)
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

std::vector<std::string> VisualMapSettingsManager::list_files(filesystem::path path)
{
    std::vector<std::string> filenames;

    QDir dir(QString::fromStdString(path.string()));

    if(dir.exists())
    {
        for (const QString & entry : dir.entryList(QDir::Files))
        {
            filenames.push_back(entry.toStdString());
        }
    }

    // alphabetical sort
    std::sort(filenames.begin(), filenames.end());

    return filenames;
}

bool VisualMapSettingsManager::create_dir(filesystem::path directory)
{
    QDir dir(QString::fromStdString(directory.string()));

    if(dir.exists())
    {
        return true;
    }

    return QDir().mkpath(dir.path());
}
