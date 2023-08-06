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

    std::vector<RGBController*> controllers = OpenRGBVisualMapPlugin::RMPointer->GetRGBControllers();

    for (unsigned int i = 0; i < controllers.size(); i++)
    {
        if(controllers[i]->serial == VirtualController::VIRTUAL_CONTROLLER_SERIAL)
        {
            continue;
        }

        for(unsigned int zone_idx = 0; zone_idx < controllers[i]->zones.size(); zone_idx++)
        {
            ControllerZone* ctrl_zone = new ControllerZone();

            ctrl_zone->controller = controllers[i];
            ctrl_zone->zone_idx = zone_idx;
            ctrl_zone->settings = ControllerZoneSettings::defaults();
            ctrl_zone->custom_zone_name = "";

            if(ctrl_zone->controller->zones[ctrl_zone->zone_idx].type == ZONE_TYPE_MATRIX)
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
    std::set<RGBController*> controllers;

    std::vector<ControllerZone*> available_zones = GetAvailableZones();

    for(ControllerZone* ctrl_zone: available_zones)
    {
        SetControllerZoneColor(ctrl_zone, ctrl_zone->compare(ctrl_zone_to_identify) ? Qt::green : Qt::black);
        controllers.insert(ctrl_zone->controller);
    }

    for(RGBController* controller : controllers)
    {
        controller->UpdateLEDs();
    }
}

void ZoneManager::SetControllerZoneColor(ControllerZone* ctrl_zone, QColor color)
{
    RGBController* controller = ctrl_zone->controller;
    zone z = controller->zones[ctrl_zone->zone_idx];
    int leds_count = z.leds_count;
    int start_idx = z.start_idx;

    for(int i = 0; i < leds_count; i++)
    {
        controller->SetLED(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
    }
}

void ZoneManager::IdentifyLeds(ControllerZone* ctrl_zone, std::vector<unsigned int> led_nums)
{
    RGBController* controller = ctrl_zone->controller;
    zone z = controller->zones[ctrl_zone->zone_idx];

    unsigned int leds_count = z.leds_count;
    unsigned int start_idx = z.start_idx;

    for(unsigned int i = 0; i < leds_count; i++)
    {
        QColor color = std::find(led_nums.begin(), led_nums.end(), i) != led_nums.end() ? Qt::green : Qt::black;
        controller->SetLED(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
    }

    controller->UpdateLEDs();
}

void ZoneManager::InitMatrixCustomShape(ControllerZone* ctrl_zone)
{
    matrix_map_type* matrix_map = ctrl_zone->controller->zones[ctrl_zone->zone_idx].matrix_map;

    ctrl_zone->settings.shape = CUSTOM;
    ctrl_zone->settings.custom_shape = new CustomShape();
    ctrl_zone->settings.custom_shape->w = matrix_map->width;
    ctrl_zone->settings.custom_shape->h = matrix_map->height;

    for(unsigned int h = 0; h < matrix_map->height; h++)
    {
        for(unsigned int w = 0; w < matrix_map->width; w++)
        {
            unsigned int led_num = matrix_map->map[h * matrix_map->width + w];

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
