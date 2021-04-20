#ifndef VISUALMAPJSONDEFINITIONS_H
#define VISUALMAPJSONDEFINITIONS_H

#include "ControllerZone.h"
#include "RGBController.h"
#include "GridOptions.h"
#include "json.hpp"

using json = nlohmann::json;

void to_json(json& j, const QPoint* point) {
    j = json{
    {"x", point->x()},
    {"y", point->y()}
};
}

void from_json(const json& j, std::vector<QPoint*>& points) {
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        points.push_back(new QPoint(it.value().at("x"),  it.value().at("y")));
    }
}

void to_json(json& j, const std::vector<QPoint*>& points) {
    for(unsigned int i = 0; i < points.size(); i++)
    {
        j[i]=points[i];
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
    {"reverse", settings.reverse},
    {"custom_shape", settings.custom_shape}
};
}

void from_json(const json& j, ControllerZoneSettings& s) {
    j.at("x").get_to(s.x);
    j.at("y").get_to(s.y);
    j.at("led_spacing").get_to(s.led_spacing);
    s.shape = static_cast<ZoneShape>(j.at("shape"));
    j.at("reverse").get_to(s.reverse);
    s.custom_shape = new CustomShape();
    j.at("custom_shape").get_to(s.custom_shape);
}

void to_json(json& j, const ControllerZone* ctrl_zone) {
    j = json{
    {"controller", ctrl_zone->controller},
    {"zone_idx", ctrl_zone->zone_idx},
    {"settings", ctrl_zone->settings}
};
}

void to_json(json& j, const GridSettings* settings) {
    j = json{
    {"h", settings->h},
    {"w", settings->w},
    {"show_grid", settings->show_grid},
    {"show_bounds", settings->show_bounds},
    {"live_preview", settings->live_preview},
    {"grid_size", settings->grid_size}
};
}

void from_json(const json& j, GridSettings* s) {
    j.at("h").get_to(s->h);
    j.at("w").get_to(s->w);
    j.at("show_grid").get_to(s->show_grid);
    j.at("show_bounds").get_to(s->show_bounds);
    j.at("live_preview").get_to(s->live_preview);
    j.at("grid_size").get_to(s->grid_size);
}

#endif // VISUALMAPJSONDEFINITIONS_H
