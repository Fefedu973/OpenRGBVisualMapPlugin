#ifndef VISUALMAPSETTINGSMANAGER_H
#define VISUALMAPSETTINGSMANAGER_H

#include "RGBController.h"
#include "json.hpp"

using json = nlohmann::json;

class VisualMapSettingsManager
{
public:
    static void SaveSettings(std::string, json);
    static json LoadSettings(std::string);
    static std::vector<std::string> GetFileNames();
    static bool CreateSettingsDirectory();

private:
    static const std::string settings_folder;
    static const std::string saves_folder;
};

#endif // VISUALMAPSETTINGSMANAGER_H
