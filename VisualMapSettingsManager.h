#ifndef VISUALMAPSETTINGSMANAGER_H
#define VISUALMAPSETTINGSMANAGER_H

#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>

#include "RGBController.h"
#include "json.hpp"

using json = nlohmann::json;

class VisualMapSettingsManager
{
public:
    static void SaveSettings(json);
    static json LoadSettings();
private:
    static bool CreateSettingsDirectory();

    static inline const std::string SettingsFolder     = "/plugins/settings/";
    static inline const std::string SettingsFileName   = "VisualMapSettings.json";
};

#endif // VISUALMAPSETTINGSMANAGER_H
