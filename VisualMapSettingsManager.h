/*---------------------------------------------------------*\
| VisualMapSettingsManager.h                                |
|                                                           |
|   Settings management for Visual Map Plugin               |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <nlohmann/json.hpp>
#include "filesystem.h"

using json = nlohmann::json;

class VisualMapSettingsManager
{
public:
    static bool                     SaveMap(std::string, json);
    static json                     LoadMap(std::string);
    static std::vector<std::string> GetMapNames();
    static bool                     SaveWorkspace(json);
    static json                     LoadWorkspace();

    static bool                     SaveGradient(std::string, json);
    static json                     LoadGradient(std::string);
    static std::vector<std::string> GetGradientsNames();

    static bool                     CreateSettingsDirectory();
    static filesystem::path         MapsFolder();

private:
    static bool                     CreateMapsDirectory();
    static bool                     CreateGradientsDirectory();

    static filesystem::path         SettingsFolder();
    static filesystem::path         GradientsFolder();

    static bool                     create_dir(filesystem::path);
    static std::vector<std::string> list_files(filesystem::path);
    static json                     load_json_file(filesystem::path);
    static bool                     write_file(filesystem::path, json);
};
