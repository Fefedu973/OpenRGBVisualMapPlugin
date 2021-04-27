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
    static inline const std::string settings_folder = "plugins/settings/";
    static inline const std::string saves_folder = "plugins/settings/virtual-controllers/";
};

#endif // VISUALMAPSETTINGSMANAGER_H
