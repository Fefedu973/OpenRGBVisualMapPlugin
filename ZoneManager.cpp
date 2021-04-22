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

ZoneManager::ZoneManager()
{
    std::vector<RGBController*> controllers = OpenRGBVisualMapPlugin::RMPointer->GetRGBControllers();

    for (unsigned int i = 0; i < controllers.size(); i++)
    {
        for (unsigned int mode_idx = 0; mode_idx < controllers[i]->modes.size(); mode_idx++)
        {
            if(controllers[i]->serial == VirtualController::VIRTUAL_CONTROLLER_SERIAL)
            {
                continue;
            }

            if (controllers[i]->modes[mode_idx].name == "Direct")
            {
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

                break;
            }
        }
    }
}

std::vector<ControllerZone*> ZoneManager::GetAvailableZones()
{
    return available_zones;
}


ControllerZone* ZoneManager::GetZone(int idx)
{
    return available_zones[idx];
}

void ZoneManager::IdentifyZone(ControllerZone* ctrl_zone_to_identify)
{
    // make sure we update the controller only once by using a set
    std::set<RGBController*> controllers;

    for(ControllerZone* ctrl_zone: available_zones)
    {
        SetControllerZoneColor(ctrl_zone, ctrl_zone == ctrl_zone_to_identify ? Qt::green : Qt::black);
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

void ZoneManager::IdentifyLed(ControllerZone* ctrl_zone,  int led_num)
{
    RGBController* controller = ctrl_zone->controller;
    zone z = controller->zones[ctrl_zone->zone_idx];
    int leds_count = z.leds_count;
    int start_idx = z.start_idx;

    for(int i = 0; i < leds_count; i++)
    {
        QColor color = i == led_num ? Qt::green : Qt::black;
        controller->SetLED(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
    }

    controller->UpdateLEDs();

}

void ZoneManager::ApplyImage(std::vector<ControllerZone*> ctrl_zones, QImage* image)
{
    // make sure we update the controller only once by using a set
    std::set<RGBController*> controllers;

    for(ControllerZone* ctrl_zone: ctrl_zones)
    {
        ApplyImage(ctrl_zone, image);
        controllers.insert(ctrl_zone->controller);
    }

    for(RGBController* controller : controllers)
    {
        controller->UpdateLEDs();
    }
}

void ZoneManager::ApplyImage(ControllerZone* ctrl_zone, QImage* image)
{
    RGBController* controller = ctrl_zone->controller;
    zone z = controller->zones[ctrl_zone->zone_idx];
    ControllerZoneSettings settings = ctrl_zone->settings;
    int leds_count = z.leds_count;
    int start_idx = z.start_idx;

    switch (ctrl_zone->settings.shape) {
        case HORIZONTAL_LINE:
            for(int i = 0; i < leds_count; i++)
            {
                int idx = settings.reverse ? leds_count - 1 - i : i;
                QColor color = image->pixelColor(idx * settings.led_spacing + settings.x, settings.y);
                controller->SetLED(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
            }
            break;

        case VERTICAL_LINE:
            for(int i = 0; i < leds_count; i++)
            {
                int idx = settings.reverse ? leds_count - 1 - i : i;

                QColor color = image->pixelColor(settings.x, idx * settings.led_spacing + settings.y);
                controller->SetLED(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
            }
            break;

        case CUSTOM:
            std::vector<LedPosition*> led_positions = ctrl_zone->settings.custom_shape->led_positions;

            for(unsigned int i = 0; i < led_positions.size(); i++)
            {                
                QColor color = image->pixelColor(settings.x + led_positions[i]->x(), settings.y + led_positions[i]->y());
                controller->SetLED(start_idx + led_positions[i]->led_num, ToRGBColor(color.red(), color.green(), color.blue()));
            }

            break;
    }
}

void ZoneManager::InitMatrixCustomShape(ControllerZone* ctrl_zone)
{
    unsigned int NA = 0xFFFFFFFF;

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
