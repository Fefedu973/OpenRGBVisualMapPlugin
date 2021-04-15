#ifndef ZONEMANAGER_H
#define ZONEMANAGER_H

#include <QStringList>
#include <QColor>
#include "RGBController.h"

struct ControllerZoneSettings
{
    inline static const QStringList ZONE_SHAPES = {
        "Horizontal line",
        "Vertical line",
        "Circle"
    };

    enum ZoneShape {
        HORIZONTAL_LINE = 0,
        VERTICAL_LINE = 1,
        CIRCLE = 2
    };

    ZoneShape shape;
    unsigned int x;
    unsigned int y;
    unsigned int led_spacing;
    bool reverse;

    static ControllerZoneSettings defaults() {
        return {
            ControllerZoneSettings::HORIZONTAL_LINE, 0, 0, 1, false
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

    int led_count() const {
        return controller->zones[zone_idx].leds_count;
    }

    std::string display_name()
    {
        return this->controller->name + " " + this->controller->zones[this->zone_idx].name;
    }
};

class ZoneManager
{
public:
    static ZoneManager* Get();

    std::vector<ControllerZone*> GetAvailableZones();
    std::vector<ControllerZone*> GetAddedZones();
    ControllerZone* GetZone(int);

    bool HasZone(int);
    void AddZone(int);
    void RemoveZone(int);
    void ClearZones();

    void IdentifyZone(ControllerZone*);
    void SetControllerZoneColor(ControllerZone*, QColor);

private:
    ZoneManager();
    static ZoneManager* instance;

    std::vector<ControllerZone*> available_zones;
    std::vector<ControllerZone*> added_zones;
};

#endif // ZONEMANAGER_H
