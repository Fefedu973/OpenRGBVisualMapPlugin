#include "ZoneManager.h"
#include "OpenRGBVisualMapPlugin.h"

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
    available_zones.clear();

    std::vector<RGBController*> controllers = OpenRGBVisualMapPlugin::RMPointer->GetRGBControllers();

    for (unsigned int i = 0; i < controllers.size(); i++)
    {
        for (unsigned int mode_idx = 0; mode_idx < controllers[i]->modes.size(); mode_idx++)
        {
            if (controllers[i]->modes[mode_idx].name == "Direct")
            {
                for(unsigned int zone_idx = 0; zone_idx < controllers[i]->zones.size(); zone_idx++)
                {
                    ControllerZone* ctrl_zone = (struct ControllerZone*) malloc( sizeof(struct ControllerZone));

                    ctrl_zone->controller = controllers[i];
                    ctrl_zone->zone_idx = zone_idx;
                    ctrl_zone->settings = ControllerZoneSettings::defaults();

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

std::vector<ControllerZone*> ZoneManager::GetAddedZones()
{
    return added_zones;
}

void ZoneManager::AddZone(int idx) {
    if(!HasZone(idx)) {
        added_zones.push_back(available_zones[idx]);
    }
}

void ZoneManager::RemoveZone(int idx) {
    std::vector<ControllerZone*>::iterator position = std::find(added_zones.begin(), added_zones.end(), available_zones[idx]);

    if (position != added_zones.end())
    {
        added_zones.erase(position);
    }
}

bool ZoneManager::HasZone(int idx) {
    return std::find(added_zones.begin(), added_zones.end(), available_zones[idx]) != added_zones.end();
}

ControllerZone* ZoneManager::GetZone(int idx)
{
    return available_zones[idx];
}

void ZoneManager::ClearZones()
{
    added_zones.clear();
}

void ZoneManager::IdentifyZone(ControllerZone* ctrl_zone_to_identify)
{

    for(ControllerZone* ctrl_zone: available_zones)
    {
        SetControllerZoneColor(ctrl_zone, ctrl_zone == ctrl_zone_to_identify ? Qt::green : Qt::black);
    }

}

void ZoneManager::SetControllerZoneColor(ControllerZone* ctrl_zone, QColor color)
{
    RGBController* controller = ctrl_zone->controller;
    zone z = controller->zones[ctrl_zone->zone_idx];
    ControllerZoneSettings settings = ctrl_zone->settings;
    int leds_count = z.leds_count;
    int start_idx = z.start_idx;

    for(int i = 0; i < leds_count; i++)
    {
        controller->SetLED(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
    }

    controller->UpdateLEDs();
}
















