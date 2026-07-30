/*---------------------------------------------------------*\
| VirtualController.cpp                                     |
|                                                           |
|   Virtual controller for visual map plugin                |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <set>
#include "OpenRGBVisualMapPlugin.h"
#include "RGBControllerInterface.h"
#include "VirtualController.h"

std::string VirtualController::VIRTUAL_CONTROLLER_SERIAL = "VISUAL_MAP_VISUAL_CONTROLLER_SERIAL";

VirtualController::VirtualController()
{
    width                                               = 1;
    height                                              = 1;

    /*-----------------------------------------------------*\
    | Setup controller details                              |
    \*-----------------------------------------------------*/
    setup.object_ptr                                    = this;
    setup.name                                          = "Visual Map Controller";
    setup.vendor                                        = "OpenRGB Visual Map Plugin";
    setup.description                                   = "Virtual controller provided by the OpenRGB Visual Map Plugin";
    setup.version                                       = VERSION_STRING;
    setup.serial                                        = VIRTUAL_CONTROLLER_SERIAL;
    setup.location                                      = "Somewhere over the rainbow";
    setup.type                                          = DEVICE_TYPE_VIRTUAL;
    setup.active_mode                                   = 0;

    /*-----------------------------------------------------*\
    | Setup zone                                            |
    \*-----------------------------------------------------*/
    setup.zones.resize(1);

    setup.zones[0].name                                 = "Virtual Zone";
    setup.zones[0].type                                 = ZONE_TYPE_MATRIX;

    /*-----------------------------------------------------*\
    | Setup mode details                                    |
    \*-----------------------------------------------------*/
    setup.modes.resize(1);

    setup.modes[0].name                                 = "Direct";
    setup.modes[0].value                                = 0;
    setup.modes[0].flags                                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    setup.modes[0].brightness                           = 100;
    setup.modes[0].brightness_max                       = 100;
    setup.modes[0].brightness_min                       = 0;
    setup.modes[0].color_mode                           = MODE_COLORS_PER_LED;

    /*-----------------------------------------------------*\
    | Setup function pointers                               |
    \*-----------------------------------------------------*/
    setup.DeviceConfigureZone                           = nullptr;
    setup.DeviceUpdateLEDs                              = DeviceUpdateLEDs_func;
    setup.DeviceUpdateZoneLEDs                          = nullptr;
    setup.DeviceUpdateSingleLED                         = nullptr;
    setup.DeviceUpdateMode                              = nullptr;
    setup.DeviceSaveMode                                = nullptr;
    setup.DeviceUpdateZoneMode                          = nullptr;
    setup.DeviceUpdateDeviceSpecificConfiguration       = nullptr;
    setup.DeviceUpdateDeviceSpecificZoneConfiguration   = nullptr;

    virtual_controller = OpenRGBVisualMapPlugin::api->CreateVirtualRGBController(&setup);
}

VirtualController::~VirtualController()
{
    Register(false, false);
}

void VirtualController::UpdateVirtualZone()
{
    /*-----------------------------------------------------*\
    | Resize matrix map                                     |
    \*-----------------------------------------------------*/
    setup.zones[0].matrix_map.height                    = height;
    setup.zones[0].matrix_map.width                     = width;
    setup.zones[0].matrix_map.map.resize(height * width);

    std::vector<std::vector<std::string>> real_leds;
    real_leds.resize(setup.zones[0].matrix_map.map.size());

    /*-----------------------------------------------------*\
    | Fill the map with NA                                  |
    \*-----------------------------------------------------*/
    for(std::size_t led_idx = 0; led_idx < setup.zones[0].matrix_map.map.size(); led_idx++)
    {
        setup.zones[0].matrix_map.map[led_idx]          = NA;
    }

    /*-----------------------------------------------------*\
    | Iterate controllers, count and place LEDs             |
    | Count real LEDs in the same loop                      |
    \*-----------------------------------------------------*/
    unsigned int    map_leds_count                      = 0;

    for(ControllerZone* ctrl_zone: added_zones)
    {
        RGBControllerInterface*         controller      = ctrl_zone->controller;
        const ControllerZoneSettings&   settings        = ctrl_zone->settings;
        unsigned int                    leds_count      = controller->GetZoneLEDsCount(ctrl_zone->zone_idx);

        switch(ctrl_zone->settings.shape)
        {
            case HORIZONTAL_LINE:
                for(unsigned int i = 0; i < leds_count; i++)
                {
                    unsigned int        idx             = settings.reverse ? leds_count - 1 - i : i;
                    unsigned int        x               = idx * settings.led_spacing + settings.x;
                    unsigned int        y               = settings.y;

                    if(y < height && x < width)
                    {
                        unsigned int    xy              = y * width + x;

                        if(real_leds[xy].empty())
                        {
                            map_leds_count++;
                        }

                        real_leds[xy].push_back(controller->GetLEDName(i));
                    }
                }
                break;

            case VERTICAL_LINE:
                for(unsigned int i = 0; i < leds_count; i++)
                {
                    unsigned int        idx             = settings.reverse ? leds_count - 1 - i : i;
                    unsigned int        x               = settings.x;
                    unsigned int        y               = idx * settings.led_spacing + settings.y;

                    if(y < height && x < width)
                    {
                        unsigned int    xy              = y * width + x;

                        if(real_leds[xy].empty())
                        {
                            map_leds_count++;
                        }

                        real_leds[xy].push_back(controller->GetLEDName(i));
                    }
                }
                break;

            case CUSTOM:
                std::vector<LedPosition*> led_positions = ctrl_zone->settings.custom_shape->led_positions;

                for(unsigned int i = 0; i < led_positions.size(); i++)
                {
                    unsigned int        x               = settings.x + led_positions[i]->x();
                    unsigned int        y               = settings.y + led_positions[i]->y();

                    if(y < height && x < width)
                    {
                        unsigned int    xy              = y * width + x;

                        if(real_leds[xy].empty())
                        {
                            map_leds_count++;
                        }

                        real_leds[xy].push_back(controller->GetLEDName(led_positions[i]->led_num));
                    }
                }
                break;
        }
    }

    /*-----------------------------------------------------*\
    | Update zone data                                      |
    \*-----------------------------------------------------*/
    setup.zones[0].leds_count                       = map_leds_count;
    setup.zones[0].leds_min                         = map_leds_count;
    setup.zones[0].leds_max                         = map_leds_count;
    setup.leds.resize(map_leds_count);

    /*-----------------------------------------------------*\
    | Update LED names and positions in matrix map          |
    \*-----------------------------------------------------*/
    int i = 0;

    for(unsigned int h = 0; h < height; h++)
    {
        for(unsigned int w = 0; w < width; w++)
        {
            unsigned int xy = (h*width) + w;

            if(!real_leds[xy].empty())
            {
                setup.zones[0].matrix_map.map[xy]   = i;

                if(real_leds[xy].size() > 1)
                {
                    setup.leds[i].name = "Multiple LEDs (" + std::to_string(real_leds[xy].size()) + ")";
                }
                else
                {
                    setup.leds[i].name = real_leds[xy][0];
                }

                i++;
            }
        }
    }

    OpenRGBVisualMapPlugin::api->UpdateVirtualRGBController(virtual_controller, &setup);
}

void VirtualController::DeviceUpdateLEDs()
{
    float           brightness = virtual_controller->GetModeBrightness(0) / 100.f;
    unsigned int    color_index = 0;
    QImage          image(width, height, QImage::Format_ARGB32);
    QColor          transparent("#00000000");

    for(unsigned int h = 0; h < height; h++)
    {
        for(unsigned int w = 0; w < width; w++)
        {
            QColor color;

            if(setup.zones[0].matrix_map.map[(h*width) + w] == NA)
            {
                color = transparent;
            }
            else
            {
                const RGBColor rgb = virtual_controller->GetColor(color_index++);
                color = QColor(RGBGetRValue(rgb) * brightness, RGBGetGValue(rgb)* brightness, RGBGetBValue(rgb)* brightness);
            }

            image.setPixelColor(w, h, color);
        }
    }

    ApplyToDevice(image);
}

void VirtualController::UpdateSize(int w, int h)
{
    width   = w;
    height  = h;

    UpdateVirtualZone();
}

void VirtualController::SetName(std::string name)
{
    setup.name = name;
    OpenRGBVisualMapPlugin::api->UpdateVirtualRGBController(virtual_controller, &setup);   
}

void VirtualController::SetPostUpdateCallBack(std::function<void(const QImage&)> callback)
{
    this->callback = callback;
}

void VirtualController::Register(bool state, bool hide_members)
{
    if(state)
    {
        if(!registered)
        {
            ForceDirectMode();

            if(hide_members)
            {
                std::set<RGBControllerInterface*> controllers;

                for(ControllerZone* ctrl_zone: added_zones)
                {
                    controllers.insert(ctrl_zone->controller);
                }

                /*-----------------------------------------*\
                | Ensure controller is in the latest list   |
                | as this function can be called during     |
                | list updates                              |
                \*-----------------------------------------*/
                std::vector<RGBControllerInterface*> available_controllers = OpenRGBVisualMapPlugin::api->GetRGBControllers();

                for(RGBControllerInterface* controller : controllers)
                {
                    if(std::find(available_controllers.begin(), available_controllers.end(), controller) == available_controllers.end())
                    {
                        controllers.erase(controller);
                    }
                }

                for(RGBControllerInterface* controller : controllers)
                {
                    controller->SetHidden(true);
                }

                members_hidden = true;
            }

            OpenRGBVisualMapPlugin::api->RegisterVirtualRGBControllerInThread(virtual_controller);
            registered = true;
        }
    }
    else
    {
        if(registered)
        {
            OpenRGBVisualMapPlugin::api->UnregisterVirtualRGBControllerInThread(virtual_controller);
            registered = false;

            if(members_hidden)
            {
                std::set<RGBControllerInterface*> controllers;

                for(ControllerZone* ctrl_zone: added_zones)
                {
                    controllers.insert(ctrl_zone->controller);
                }

                /*-----------------------------------------*\
                | Ensure controller is in the latest list   |
                | as this function can be called during     |
                | list updates                              |
                \*-----------------------------------------*/
                std::vector<RGBControllerInterface*> available_controllers = OpenRGBVisualMapPlugin::api->GetRGBControllers();

                for(RGBControllerInterface* controller : controllers)
                {
                    if(std::find(available_controllers.begin(), available_controllers.end(), controller) == available_controllers.end())
                    {
                        controllers.erase(controller);
                    }
                }

                for(RGBControllerInterface* controller : controllers)
                {
                    controller->SetHidden(false);
                }

                members_hidden = false;
            }
        }
    }
}

void VirtualController::ForceDirectMode()
{
    std::set<RGBControllerInterface*> controllers;

    for(ControllerZone* ctrl_zone: added_zones)
    {
        controllers.insert(ctrl_zone->controller);
    }

    for(RGBControllerInterface* controller : controllers)
    {
        for(unsigned int i = 0; i < controller->GetModeCount(); i++)
        {
            if(controller->GetModeName(i) == "Direct")
            {
                controller->SetActiveMode(i);
            }
        }
    }
}

bool VirtualController::HasZone(ControllerZone* ctrl_zone)
{
    return std::find(added_zones.begin(), added_zones.end(),ctrl_zone) != added_zones.end();
}

void VirtualController::Add(ControllerZone* ctrl_zone)
{
    std::lock_guard<std::mutex> lock(added_zones_mutex);

    if(!HasZone(ctrl_zone))
    {
        added_zones.push_back(ctrl_zone);

        /*-------------------------------------------------*\
        | Make sure to have the correct LED size            |
        \*-------------------------------------------------*/
        if(ctrl_zone->isCustomShape() && ctrl_zone->led_count() !=  ctrl_zone->settings.custom_shape->led_positions.size())
        {
            ctrl_zone->settings.custom_shape->resizeCustomShape(ctrl_zone->led_count());
        }

        /*-------------------------------------------------*\
        | Hide controller if necessary                      |
        \*-------------------------------------------------*/
        if(registered && members_hidden && !ctrl_zone->controller->GetHidden())
        {
            ctrl_zone->controller->SetHidden(true);
        }
    }
}

void VirtualController::Remove(ControllerZone* ctrl_zone)
{
    std::lock_guard<std::mutex> lock(added_zones_mutex);

    if(HasZone(ctrl_zone))
    {
        added_zones.erase(std::find(added_zones.begin(), added_zones.end(), ctrl_zone));
    }
}

void VirtualController::Clear()
{
    /*-----------------------------------------------------*\
    | Locked so a call in flight on the device thread       |
    | drains before the zones go away. Clearing here on     |
    | DETECTION_STARTED stops the device thread touching    |
    | controllers that are about to be freed.               |
    \*-----------------------------------------------------*/
    std::lock_guard<std::mutex> lock(added_zones_mutex);

    added_zones.clear();
}

std::vector<ControllerZone*> VirtualController::GetZones()
{
    return added_zones;
}

bool VirtualController::IsEmpty()
{
    return added_zones.empty();
}

std::string VirtualController::GetName()
{
    return virtual_controller->GetName();
}

unsigned int VirtualController::GetTotalLeds()
{
    unsigned int result = 0;

    for(ControllerZone* ctrl_zone : added_zones)
    {
        result += ctrl_zone->led_count();
    }

    return result;
}

void VirtualController::ApplyImage(const QImage& original)
{
    /*-----------------------------------------------------*\
    | Make sure the image only targets the existing LEDs    |
    \*-----------------------------------------------------*/
    QImage image(width, height, QImage::Format_ARGB32);

    float brightness = virtual_controller->GetModeBrightness(0) / 100.f;

    QColor transparent("#00000000");

    unsigned int color_idx = 0;

    for(unsigned int h = 0; h < height; h++)
    {
        for(unsigned int w = 0; w < width; w++)
        {
            QColor color;

            if(setup.zones[0].matrix_map.map[(h*width) + w] == NA)
            {
                color = transparent;
            }
            else
            {
                QColor original_color = original.pixelColor(QPoint(w, h));
                int red = original_color.red()   * brightness;
                int grn = original_color.green() * brightness;
                int blu = original_color.blue()  * brightness;
                color = QColor(red, grn, blu);

                if(color_idx < virtual_controller->GetLEDCount())
                {
                    virtual_controller->SetColor(color_idx, ToRGBColor(red,grn,blu));
                }

                color_idx++;
            }

            image.setPixelColor(w, h, color);
        }
    }

    ApplyToDevice(image);
}

void VirtualController::ApplyToDevice(const QImage& image)
{
    /*-----------------------------------------------------*\
    | Make sure we update the controller only once by using |
    | a set                                                 |
    \*-----------------------------------------------------*/
    std::set<RGBControllerInterface*> controllers;

    /*-----------------------------------------------------*\
    | Held across the apply so the zones (and the           |
    | controllers they point at) cannot be cleared/freed    |
    | mid-update by a DETECTION_STARTED on another thread.  |
    \*-----------------------------------------------------*/
    {
        std::lock_guard<std::mutex> lock(added_zones_mutex);

        for(ControllerZone* ctrl_zone: added_zones)
        {
            ApplyToZone(ctrl_zone, image);
            controllers.insert(ctrl_zone->controller);
        }

        for(RGBControllerInterface* controller : controllers)
        {
            controller->UpdateLEDs();
        }
    }

    callback(image);
}

void VirtualController::ApplyToZone(ControllerZone* ctrl_zone, const QImage& image)
{
    RGBControllerInterface* controller  = ctrl_zone->controller;
    ControllerZoneSettings  settings    = ctrl_zone->settings;
    unsigned int            leds_count;
    unsigned int            start_idx;

    if(ctrl_zone->is_segment)
    {
        leds_count                      = controller->GetZoneSegmentLEDsCount(ctrl_zone->zone_idx, ctrl_zone->segment_idx);
        start_idx                       = controller->GetZoneSegmentStartIndex(ctrl_zone->zone_idx, ctrl_zone->segment_idx);
    }
    else
    {
        leds_count                      = controller->GetZoneLEDsCount(ctrl_zone->zone_idx);
        start_idx                       = controller->GetZoneStartIndex(ctrl_zone->zone_idx);
    }
    
    switch(ctrl_zone->settings.shape)
    {
    case HORIZONTAL_LINE:
        for(int i = 0; i < (int)leds_count; i++)
        {
            int idx = settings.reverse ? (int)leds_count - 1 - i : i;

            unsigned int x = idx * settings.led_spacing + settings.x;
            unsigned int y = settings.y;

            if(image.valid(x,y))
            {
                QColor color = image.pixelColor(x, y);
                controller->SetColor(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
            }

        }
        break;

    case VERTICAL_LINE:
        for(int i = 0; i < (int)leds_count; i++)
        {
            int idx = settings.reverse ? (int)leds_count - 1 - i : i;

            unsigned int x = settings.x;
            unsigned int y = idx * settings.led_spacing + settings.y;

            if(image.valid(x,y))
            {
                QColor color = image.pixelColor(x, y);
                controller->SetColor(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
            }
        }
        break;

    case CUSTOM:
        std::vector<LedPosition*> led_positions = ctrl_zone->settings.custom_shape->led_positions;

        for(unsigned int i = 0; i < led_positions.size(); i++)
        {
            unsigned int x = settings.x + led_positions[i]->x();
            unsigned int y = settings.y + led_positions[i]->y();

            if(image.valid(x,y))
            {
                QColor color = image.pixelColor(x, y);
                controller->SetColor(start_idx + led_positions[i]->led_num, ToRGBColor(color.red(), color.green(), color.blue()));
            }
        }

        break;
    }
}

void VirtualController::DeviceUpdateLEDs_func(void* object_ptr)
{
    ((VirtualController*)object_ptr)->DeviceUpdateLEDs();
}