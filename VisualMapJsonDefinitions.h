#ifndef VISUALMAPJSONDEFINITIONS_H
#define VISUALMAPJSONDEFINITIONS_H

#include <nlohmann/json.hpp>
#include "ControllerZone.h"
#include "RGBController.h"
#include "GridSettings.h"

using json = nlohmann::json;

void to_json(json& j, LedPosition* led_position) {
        j = json{
        {"led_num",led_position->led_num},
        {"x", led_position->x()},
        {"y", led_position->y()}
    };
}

void from_json(const json& j, std::vector<LedPosition*>& led_positions) {
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        LedPosition* led_position = new LedPosition();
        led_position->led_num = it.value().at("led_num");
        led_position->setX(it.value().at("x"));
        led_position->setY(it.value().at("y"));
        led_positions.push_back(led_position);
    }
}

void to_json(json& j, const std::vector<LedPosition*>& led_positions) {
    for(unsigned int i = 0; i < led_positions.size(); i++)
    {
        j[i]=led_positions[i];
    }
}

void from_json(const json& j, CustomShape* s) {
    if(!j.is_null())
    {
        j.at("w").get_to(s->w);
        j.at("h").get_to(s->h);
        j.at("led_positions").get_to(s->led_positions);
    }
}

void to_json(json& j, const CustomShape* custom_shape) {
    if(custom_shape)
    {
        j = json{
        {"w", custom_shape->w},
        {"h", custom_shape->h},
        {"led_positions", custom_shape->led_positions}
    };
    }
}

void to_json(json& j, const RGBController* controller) {
    j = json{
    {"name", controller->name},
    {"location", controller->location},
    {"serial", controller->serial},
    {"vendor", controller->vendor}
};
}

void to_json(json& j, const ControllerZoneSettings settings) {
    j = json{
    {"shape", settings.shape},
    {"x", settings.x},
    {"y", settings.y},
    {"led_spacing", settings.led_spacing},
    {"reverse", settings.reverse}
};
    if(settings.shape == CUSTOM)
    {
        j["custom_shape"] = settings.custom_shape;
    }
    else
    {
        j["custom_shape"] = nullptr;
    }
}

void from_json(const json& j, ControllerZoneSettings& s) {
    j.at("x").get_to(s.x);
    j.at("y").get_to(s.y);
    j.at("led_spacing").get_to(s.led_spacing);
    s.shape = static_cast<ZoneShape>(j.at("shape"));
    j.at("reverse").get_to(s.reverse);

    auto custom_shape = j.at("custom_shape");

    if(!custom_shape.is_null() && s.shape == CUSTOM)
    {
        s.custom_shape = new CustomShape();
        j.at("custom_shape").get_to(s.custom_shape);
    }
    else
    {
        s.custom_shape = nullptr;
    }
}

void to_json(json& j, const ControllerZone* ctrl_zone) {
    j = json{
    {"controller", ctrl_zone->controller},
    {"zone_idx", ctrl_zone->zone_idx},
    {"custom_zone_name", ctrl_zone->custom_zone_name},
    {"settings", ctrl_zone->settings}
};
}

void to_json(json& j, const GridSettings* settings) {
    j = json{
    {"h", settings->h},
    {"w", settings->w},
    {"show_grid", settings->show_grid},
    {"show_bounds", settings->show_bounds},
    {"grid_size", settings->grid_size},
    {"auto_load", settings->auto_load},
    {"auto_register", settings->auto_register},
    {"unregister_members", settings->unregister_members},
};
}

void from_json(const json& j, GridSettings* s) {
    j.at("h").get_to(s->h);
    j.at("w").get_to(s->w);
    j.at("show_grid").get_to(s->show_grid);
    j.at("show_bounds").get_to(s->show_bounds);
    j.at("grid_size").get_to(s->grid_size);

    if(j.contains("auto_load"))
    {
        j.at("auto_load").get_to(s->auto_load);
    }

    if(j.contains("auto_register"))
    {
        j.at("auto_register").get_to(s->auto_register);
    }

    if(j.contains("unregister_members"))
    {
        j.at("unregister_members").get_to(s->unregister_members);
    }
}

class DummyController: public RGBController
{
public:
    DummyController(){};
    ~DummyController(){};
    void DeviceUpdateLEDs()    override {};
    void SetupZones()          override {};
    void SetupColors()         override {};
    void ResizeZone(int, int)  override {};
    void SetCustomMode()       override {};
    void DeviceUpdateMode()    override {};
    void UpdateZoneLEDs(int)   override {};
    void UpdateSingleLED(int)  override {};
};

void from_json(const json& j, ControllerZone* z) {

    z->controller = new DummyController();

    if(!j.is_null())
    {
        j["controller"].at("name").get_to(z->controller->name);
        j["controller"].at("vendor").get_to(z->controller->vendor);
        j["controller"].at("description").get_to(z->controller->description);
        j["controller"].at("version").get_to(z->controller->version);
        j["controller"].at("serial").get_to(z->controller->serial);
        j["controller"].at("location").get_to(z->controller->location);

        j.at("zone_idx").get_to(z->zone_idx);
        j.at("custom_zone_name").get_to(z->custom_zone_name);
        j.at("settings").get_to(z->settings);
    }
}


#endif // VISUALMAPJSONDEFINITIONS_H
