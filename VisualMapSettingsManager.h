#ifndef VISUALMAPSETTINGSMANAGER_H
#define VISUALMAPSETTINGSMANAGER_H

#include "RGBController.h"
#include "json.hpp"

using json = nlohmann::json;

class VisualMapSettingsManager
{
public:
    static bool SaveMap(std::string, json);
    static json LoadMap(std::string);
    static std::vector<std::string> GetMapNames();

    static bool SaveGradient(std::string, json);
    static json LoadGradient(std::string);
    static std::vector<std::string> GetGradientsNames();

    static bool CreateSettingsDirectory();

private:
    static bool CreateMapsDirectory();
    static bool CreateGradientsDirectory();

    static std::string SettingsFolder();
    static std::string MapsFolder();
    static std::string GradientsFolder();
    static std::string folder_separator();
    static bool create_dir(std::string);
    static std::vector<std::string> list_files(std::string);
    static json load_json_file(std::string);
    static bool write_file(std::string, json);
};

#endif // VISUALMAPSETTINGSMANAGER_H
