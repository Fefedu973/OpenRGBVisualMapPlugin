/*---------------------------------------------------------*\
| VisualMapJsonDefinitions.cpp                              |
|                                                           |
|   Functions for converting various VisualMap objects to   |
|   and from JSON                                           |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <nlohmann/json.hpp>
#include "ControllerZone.h"
#include "GridSettings.h"
#include "RGBControllerInterface.h"
#include <cmath>
#include <memory>
#include <stdexcept>

using json = nlohmann::json;

void to_json(json& j, LedPosition* led_position)
{
    j = json{
    {"led_num",led_position->led_num},
    {"x", led_position->x()},
    {"y", led_position->y()}};
}

void from_json(const json& j, std::vector<LedPosition*>& led_positions)
{
    for(json::const_iterator it = j.begin(); it != j.end(); ++it)
    {
        auto owner = std::make_unique<LedPosition>();
        LedPosition* led_position = owner.get();
        led_position->led_num = it.value().at("led_num");
        led_position->setX(it.value().at("x"));
        led_position->setY(it.value().at("y"));
        if(!std::isfinite(led_position->x()) || !std::isfinite(led_position->y()))
            throw std::invalid_argument("LED coordinates must be finite");
        led_positions.push_back(owner.release());
    }
}

void to_json(json& j, const std::vector<LedPosition*>& led_positions)
{
    for(unsigned int i = 0; i < led_positions.size(); i++)
    {
        j[i]=led_positions[i];
    }
}

void from_json(const json& j, CustomShape* s)
{
    if(!j.is_null())
    {
        j.at("w").get_to(s->w);
        j.at("h").get_to(s->h);
        if(!std::isfinite(s->w) || !std::isfinite(s->h) || s->w<=0 || s->h<=0)
            throw std::invalid_argument("Custom shape dimensions must be positive and finite");
        j.at("led_positions").get_to(s->led_positions);
    }
}

void to_json(json& j, const CustomShape* custom_shape)
{
    if(custom_shape)
    {
        j = json{
        {"w", custom_shape->w},
        {"h", custom_shape->h},
        {"led_positions", custom_shape->led_positions}};
    }
}

void to_json(json& j, const ControllerZoneSettings settings)
{
    j = json{
    {"shape", settings.shape},
    {"x", settings.x},
    {"y", settings.y},
    {"scale", settings.scale},
    {"led_spacing", settings.led_spacing},
    {"reverse", settings.reverse}};
    j["affine"]={{"scale_x",settings.scale_x},{"scale_y",settings.scale_y},{"rotation",settings.rotation},
                 {"flip_x",settings.flip_x},{"flip_y",settings.flip_y}};
    j["point_origin"]=settings.point_is_center?"center":"cell";
    j["brightness"]=settings.brightness;

    if(settings.shape == CUSTOM)
    {
        j["custom_shape"] = settings.custom_shape;
    }
    else
    {
        j["custom_shape"] = nullptr;
    }
}

void from_json(const json& j, ControllerZoneSettings& s)
{
    j.at("x").get_to(s.x);
    j.at("y").get_to(s.y);
    s.scale = j.value("scale", 1.0);
    const auto affine=j.value("affine",json::object());
    if(!affine.is_object())throw std::invalid_argument("affine must be an object");
    s.scale_x=affine.value("scale_x",1.0);s.scale_y=affine.value("scale_y",1.0);
    s.rotation=affine.value("rotation",0.0);s.flip_x=affine.value("flip_x",false);s.flip_y=affine.value("flip_y",false);
    const auto origin=j.value("point_origin",std::string("cell"));
    if(origin!="cell" && origin!="center")throw std::invalid_argument("point_origin must be cell or center");
    s.point_is_center=origin=="center";s.brightness=j.value("brightness",1.0);
    if(!std::isfinite(s.x) || !std::isfinite(s.y) || !std::isfinite(s.scale) || s.scale<=0
       || !std::isfinite(s.scale_x) || !std::isfinite(s.scale_y) || s.scale_x<=0 || s.scale_y<=0
       || !std::isfinite(s.rotation) || !std::isfinite(s.brightness) || s.brightness<0 || s.brightness>1)
        throw std::invalid_argument("Invalid affine placement or brightness");
    j.at("led_spacing").get_to(s.led_spacing);
    s.shape = static_cast<ZoneShape>(j.at("shape"));
    j.at("reverse").get_to(s.reverse);

    json custom_shape = j.at("custom_shape");

    if(!custom_shape.is_null() && s.shape == CUSTOM)
    {
        auto shape=std::make_unique<CustomShape>();
        auto* pointer=shape.get();j.at("custom_shape").get_to(pointer);
        s.custom_shape = shape.release();
    }
    else
    {
        s.custom_shape = nullptr;
    }
}

void to_json(json& j, const ControllerInfo info)
{
    j = json{
    {"name", info.name},
    {"vendor", info.vendor},
    {"description", info.description},
    {"version", info.version},
    {"serial", info.serial},
    {"location", info.location},};
};

void to_json(json& j, const ControllerZone* ctrl_zone)
{
    j = json{
    {"controller", ctrl_zone->controller_info},
    {"zone_idx", ctrl_zone->zone_idx},
    {"is_segment", ctrl_zone->is_segment},
    {"segment_idx", ctrl_zone->is_segment ? ctrl_zone->segment_idx : 0},
    {"custom_zone_name", ctrl_zone->custom_zone_name},
    {"settings", ctrl_zone->settings}};
}

void to_json(json& j, const GridSettings* settings)
{
    j = json{
    {"h", settings->h},
    {"w", settings->w},
    {"show_grid", settings->show_grid},
    {"show_bounds", settings->show_bounds},
    {"grid_size", settings->grid_size},
    {"snap_to_grid", settings->snap_to_grid},
    {"auto_load", settings->auto_load},
    {"auto_register", settings->auto_register},
    {"hide_members", settings->hide_members},
};
}

void from_json(const json& j, GridSettings* s)
{
    j.at("h").get_to(s->h);
    j.at("w").get_to(s->w);
    j.at("show_grid").get_to(s->show_grid);
    j.at("show_bounds").get_to(s->show_bounds);
    j.at("grid_size").get_to(s->grid_size);

    s->snap_to_grid = j.value("snap_to_grid", false);

    if(j.contains("auto_load"))
    {
        j.at("auto_load").get_to(s->auto_load);
    }

    if(j.contains("auto_register"))
    {
        j.at("auto_register").get_to(s->auto_register);
    }

    if(j.contains("hide_members"))
    {
        j.at("hide_members").get_to(s->hide_members);
    }
}
