#include "ZoneManager.h"
#include "OpenRGBVisualMapPlugin.h"
#include "VirtualController.h"

#include <set>

ZoneManager* ZoneManager::instance;

ZoneManager* ZoneManager::Get()
{
    if(!instance)
    {
        instance = new ZoneManager();
    }

    return instance;
}

std::vector<ControllerZone*> ZoneManager::GetAvailableZones()
{
    std::vector<ControllerZone*> available_zones;

    std::vector<RGBControllerInterface*> controllers = OpenRGBVisualMapPlugin::api->GetRGBControllers();

    for (unsigned int i = 0; i < controllers.size(); i++)
    {
        if(controllers[i]->GetSerial() == VirtualController::VIRTUAL_CONTROLLER_SERIAL)
        {
            continue;
        }

        for(unsigned int zone_idx = 0; zone_idx < controllers[i]->GetZoneCount(); zone_idx++)
        {
            ControllerZone* ctrl_zone = new ControllerZone();

            ctrl_zone->controller = controllers[i];
            ctrl_zone->zone_idx = zone_idx;
            ctrl_zone->settings = ControllerZoneSettings::defaults();
            ctrl_zone->custom_zone_name = "";

            if(ctrl_zone->controller->GetZoneType(ctrl_zone->zone_idx) == ZONE_TYPE_MATRIX)
            {
                InitMatrixCustomShape(ctrl_zone);
            }

            available_zones.push_back(ctrl_zone);
        }

    }

    return available_zones;
}


void ZoneManager::IdentifyZone(ControllerZone* ctrl_zone_to_identify)
{
    // make sure we update the controller only once by using a set
    std::set<RGBControllerInterface*> controllers;

    std::vector<ControllerZone*> available_zones = GetAvailableZones();

    for(ControllerZone* ctrl_zone: available_zones)
    {
        SetControllerZoneColor(ctrl_zone, ctrl_zone->compare(ctrl_zone_to_identify) ? Qt::green : Qt::black);
        controllers.insert(ctrl_zone->controller);
    }

    for(RGBControllerInterface* controller : controllers)
    {
        controller->UpdateLEDs();
    }
}

void ZoneManager::SetControllerZoneColor(ControllerZone* ctrl_zone, QColor color)
{
    RGBControllerInterface*  controller  = ctrl_zone->controller;
    unsigned int    leds_count  = controller->GetZoneLEDsCount(ctrl_zone->zone_idx);
    unsigned int    start_idx   = controller->GetZoneStartIndex(ctrl_zone->zone_idx);

    for(unsigned int i = 0; i < leds_count; i++)
    {
        controller->SetColor(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
    }
}

void ZoneManager::IdentifyLeds(ControllerZone* ctrl_zone, std::vector<unsigned int> led_nums)
{
    RGBControllerInterface*  controller  = ctrl_zone->controller;
    unsigned int    leds_count  = controller->GetZoneLEDsCount(ctrl_zone->zone_idx);
    unsigned int    start_idx   = controller->GetZoneStartIndex(ctrl_zone->zone_idx);

    for(unsigned int i = 0; i < leds_count; i++)
    {
        QColor color = std::find(led_nums.begin(), led_nums.end(), i) != led_nums.end() ? Qt::green : Qt::black;
        controller->SetColor(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
    }

    controller->UpdateLEDs();
}

void ZoneManager::InitMatrixCustomShape(ControllerZone* ctrl_zone)
{
    RGBControllerInterface*  controller          = ctrl_zone->controller;

    ctrl_zone->settings.shape           = CUSTOM;
    ctrl_zone->settings.custom_shape    = new CustomShape();
    ctrl_zone->settings.custom_shape->w = controller->GetZoneMatrixMapWidth(ctrl_zone->zone_idx);
    ctrl_zone->settings.custom_shape->h = controller->GetZoneMatrixMapWidth(ctrl_zone->zone_idx);

    for(unsigned int h = 0; h < ctrl_zone->settings.custom_shape->h; h++)
    {
        for(unsigned int w = 0; w < ctrl_zone->settings.custom_shape->w; w++)
        {
            unsigned int led_num = controller->GetZoneMatrixMapData(ctrl_zone->zone_idx)[h * ctrl_zone->settings.custom_shape->w + w];

            if(led_num != NA)
            {
                LedPosition* led_position = new LedPosition();
                led_position->led_num = led_num;
                led_position->setX(w);
                led_position->setY(h);

                ctrl_zone->settings.custom_shape->led_positions.push_back(led_position);
            }
        }
    }
}
