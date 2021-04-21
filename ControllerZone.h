#ifndef CONTROLLERZONE_H
#define CONTROLLERZONE_H

#include "RGBController.h"
#include <vector>
#include <QPoint>
#include <QStringList>

struct CustomShape
{
    unsigned int w;
    unsigned int h;
    std::vector<QPoint*> led_positions;
};

enum ZoneShape {
    HORIZONTAL_LINE = 0,
    VERTICAL_LINE = 1,
    CUSTOM = 2
};

inline static const QStringList ZONE_SHAPES = {
    "Horizontal line",
    "Vertical line",
    "Custom"
};

struct ControllerZoneSettings
{
    ZoneShape shape;
    CustomShape* custom_shape;

    unsigned int x;
    unsigned int y;
    unsigned int led_spacing;

    bool reverse;

    static ControllerZoneSettings defaults() {
        return {
            HORIZONTAL_LINE,  nullptr, 0, 0, 1, false
        };
    }
};

struct ControllerZone
{
    RGBController* controller;
    unsigned int zone_idx;

    ControllerZoneSettings settings;

    bool operator==(ControllerZone const & rhs) const {
        return this->controller == rhs.controller && this->zone_idx == rhs.zone_idx;
    }

    unsigned int led_count() const {
        return controller->zones[zone_idx].leds_count;
    }

    std::string display_name()
    {
        return this->controller->name + " " + this->controller->zones[this->zone_idx].name;
    }

    bool isCustomShape()  {
        return this->settings.shape == CUSTOM;
    }
};

#endif // CONTROLLERZONE_H
