#include "VirtualController.h"
#include "OpenRGBVisualMapPlugin.h"
#include "RGBController.h"
#include <set>

std::string VirtualController::VIRTUAL_CONTROLLER_SERIAL = "VISUAL_MAP_VISUAL_CONTROLLER_SERIAL";

VirtualController::VirtualController()
{
    width       = 1;
    height      = 1;

    name        = "VisualMap controller";
    vendor      = "VisualMap plugin";
    description = "Virtual controller provided by VisualMap plugin";
    version     = "1.0.0";
    serial      = VIRTUAL_CONTROLLER_SERIAL;
    location    = "Somewhere over the rainbow";
    active_mode = 0;
    type        = DEVICE_TYPE_VIRTUAL;

    zones.clear();
    modes.clear();

    zone new_zone;

    /*-------------------------------------------------*\
    | Setup zone                                        |
    \*-------------------------------------------------*/
    new_zone.name           = "Virtual zone";
    new_zone.start_idx      = 0;
    new_zone.type           = ZONE_TYPE_MATRIX;
    new_zone.matrix_map     = new matrix_map_type();

    zones.push_back(new_zone);

    /*-------------------------------------------------*\
    | Setup mode details                                |
    \*-------------------------------------------------*/

    mode new_mode;
    new_mode.name               = "Direct";
    new_mode.value              = 0;
    new_mode.flags              = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    new_mode.brightness         = 100;
    new_mode.brightness_max     = 100;
    new_mode.brightness_min     = 0;
    new_mode.color_mode         = MODE_COLORS_PER_LED;

    modes.push_back(new_mode);
}

VirtualController::~VirtualController()
{
    delete[] zones[0].matrix_map->map;

    Register(false, false);
}

void VirtualController::UpdateVirtualZone()
{
    unsigned int size       = width * height;
    unsigned int *map       = new unsigned int[size];

    std::vector<std::vector<std::string>> real_leds;
    real_leds.resize(size);

    /*-------------------------------------------------*\
    | Fill the map with NA                              |
    \*-------------------------------------------------*/
    memset(map, NA, size * sizeof(unsigned int));

    /*-------------------------------------------------*\
    | Iterate controllers, count and place leds         |
    | Count real leds in the same loop                  |
    \*-------------------------------------------------*/
    unsigned int map_leds_count = 0;

    for(ControllerZone* ctrl_zone: added_zones)
    {
        RGBController* controller = ctrl_zone->controller;
        const ControllerZoneSettings& settings = ctrl_zone->settings;
        unsigned int leds_count = controller->zones[ctrl_zone->zone_idx].leds_count;

        switch (ctrl_zone->settings.shape) {
        case HORIZONTAL_LINE:
            for(unsigned int i = 0; i < leds_count; i++)
            {
                unsigned int idx = settings.reverse ? leds_count - 1 - i : i;

                unsigned int x = idx * settings.led_spacing + settings.x;
                unsigned int y = settings.y;

                if(y < height && x < width)
                {
                    unsigned int xy = y * width + x;

                    if(real_leds[xy].empty())
                    {
                        map_leds_count++;
                    }

                    real_leds[xy].push_back(controller->leds[i].name);
                }
            }
            break;

        case VERTICAL_LINE:
            for(unsigned int i = 0; i < leds_count; i++)
            {
                unsigned int idx = settings.reverse ? leds_count - 1 - i : i;

                unsigned int x = settings.x;
                unsigned int y = idx * settings.led_spacing + settings.y;

                if(y < height && x < width)
                {
                    unsigned int xy = y * width + x;

                    if(real_leds[xy].empty())
                    {
                        map_leds_count++;
                    }

                    real_leds[xy].push_back(controller->leds[i].name);
                }
            }
            break;

        case CUSTOM:
            std::vector<LedPosition*> led_positions = ctrl_zone->settings.custom_shape->led_positions;

            for(unsigned int i = 0; i < led_positions.size(); i++)
            {
                unsigned int x = settings.x + led_positions[i]->x();
                unsigned int y = settings.y + led_positions[i]->y();

                if(y < height && x < width)
                {
                    unsigned int xy = y * width + x;

                    if(real_leds[xy].empty())
                    {
                        map_leds_count++;
                    }

                    real_leds[xy].push_back(controller->leds[led_positions[i]->led_num].name);
                }
            }

            break;
        }
    }

    /*-------------------------------------------------*\
    | Setup map, colors and leds                        |
    \*-------------------------------------------------*/
    colors.resize(map_leds_count);
    leds.resize(map_leds_count);

    int i = 0;

    for(unsigned int h = 0; h < height; h++)
    {
        for(unsigned int w = 0; w < width; w++)
        {
            unsigned int xy = (h*width) + w;

            if(!real_leds[xy].empty())
            {
                map[xy] = i;

                colors[i] = ToRGBColor(0,0,0);

                if(real_leds[xy].size() > 1)
                {
                    leds[i].name = "Multiple LEDs (" + std::to_string(real_leds[xy].size()) + ")";
                }
                else
                {
                    leds[i].name = real_leds[xy][0];
                }

                i++;
            }
        }
    }

    /*-------------------------------------------------*\
    | Update zone data                                  |
    \*-------------------------------------------------*/
    zones[0].leds_count             = map_leds_count;
    zones[0].leds_min               = map_leds_count;
    zones[0].leds_max               = map_leds_count;
    zones[0].matrix_map->width      = width;
    zones[0].matrix_map->height     = height;

    if(colors.size() > 0 && leds.size() > 0)
    {
        zones[0].colors                 = &colors[0];
        zones[0].leds                   = &leds[0];
    }

    /*-------------------------------------------------*\
    | Clean up old map and set the new one              |
    \*-------------------------------------------------*/
    delete[] zones[0].matrix_map->map;
    zones[0].matrix_map->map = map;
}

void VirtualController::DeviceUpdateLEDs() {
    QImage image(width, height, QImage::Format_ARGB32);

    float brightness = modes[0].brightness / 100.f;

    unsigned int color_index = 0;

    QColor transparent("#00000000");

    for(unsigned int h = 0; h < height; h++)
    {
        for(unsigned int w = 0; w < width; w++)
        {
            QColor color;

            if(zones[0].matrix_map->map[(h*width) + w] == NA)
            {
                color = transparent;
            }
            else
            {
                const RGBColor& rgb = colors[color_index++];
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

void VirtualController::SetPostUpdateCallBack(std::function<void(const QImage&)> callback)
{
    this->callback = callback;
}

void VirtualController::Register(bool state, bool unregister_members)
{
    if(state)
    {
        if(!registered)
        {
            ForceDirectMode();
            OpenRGBVisualMapPlugin::RMPointer->RegisterRGBController(this);

            printf("[OpenRGBVisualMapPlugin] Virtual map \"%s\" registered\n", name.c_str());

            if(unregister_members)
            {
                std::set<RGBController*> controllers;

                for(ControllerZone* ctrl_zone: added_zones)
                {
                    controllers.insert(ctrl_zone->controller);
                }

                for(RGBController* controller : controllers)
                {
                    OpenRGBVisualMapPlugin::RMPointer->UnregisterRGBController(controller);
                }

                members_unregistered = true;
            }

            registered = true;
        }
    }
    else
    {
        if(registered)
        {
            OpenRGBVisualMapPlugin::RMPointer->UnregisterRGBController(this);
            registered = false;

            if(members_unregistered)
            {
                std::set<RGBController*> controllers;

                for(ControllerZone* ctrl_zone: added_zones)
                {
                    controllers.insert(ctrl_zone->controller);
                }

                for(RGBController* controller : controllers)
                {
                    OpenRGBVisualMapPlugin::RMPointer->RegisterRGBController(controller);
                }

                members_unregistered = false;
            }
        }
    }
}

void VirtualController::ForceDirectMode()
{
    std::set<RGBController*> controllers;

    for(ControllerZone* ctrl_zone: added_zones)
    {
        controllers.insert(ctrl_zone->controller);
    }

    for(RGBController* controller : controllers)
    {
        for(unsigned int i = 0; i < controller->modes.size(); i++)
        {
            if(controller->modes[i].name == "Direct")
            {
                controller->SetMode(i);
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
    if(!HasZone(ctrl_zone))
    {
        added_zones.push_back(ctrl_zone);

        // make sure to have the correct led size
        if(ctrl_zone->isCustomShape() && ctrl_zone->led_count() !=  ctrl_zone->settings.custom_shape->led_positions.size())
        {
            ctrl_zone->settings.custom_shape->resizeCustomShape(ctrl_zone->led_count());
        }
    }
}

void VirtualController::Remove(ControllerZone* ctrl_zone)
{
    if(HasZone(ctrl_zone))
    {
        added_zones.erase(std::find(added_zones.begin(), added_zones.end(), ctrl_zone));
    }
}

void VirtualController::Clear()
{
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
    // Make sure the image only targets the existing LEDs
    QImage image(width, height, QImage::Format_ARGB32);

    float brightness = modes[0].brightness / 100.f;

    QColor transparent("#00000000");

    unsigned int color_idx = 0;

    for(unsigned int h = 0; h < height; h++)
    {
        for(unsigned int w = 0; w < width; w++)
        {
            QColor color;

            if(zones[0].matrix_map->map[(h*width) + w] == NA)
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

                if(color_idx < colors.size())
                {
                    colors[color_idx] = ToRGBColor(red,grn,blu);
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
    // make sure we update the controller only once by using a set
    std::set<RGBController*> controllers;

    for(ControllerZone* ctrl_zone: added_zones)
    {
        ApplyToZone(ctrl_zone, image);
        controllers.insert(ctrl_zone->controller);
    }

    for(RGBController* controller : controllers)
    {
        controller->UpdateLEDs();
    }

    callback(image);
}

void VirtualController::ApplyToZone(ControllerZone* ctrl_zone, const QImage& image)
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

            unsigned int x = idx * settings.led_spacing + settings.x;
            unsigned int y = settings.y;

            if(image.valid(x,y))
            {
                QColor color = image.pixelColor(x, y);
                controller->SetLED(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
            }

        }
        break;

    case VERTICAL_LINE:
        for(int i = 0; i < leds_count; i++)
        {
            int idx = settings.reverse ? leds_count - 1 - i : i;

            unsigned int x = settings.x;
            unsigned int y = idx * settings.led_spacing + settings.y;

            if(image.valid(x,y))
            {
                QColor color = image.pixelColor(x, y);
                controller->SetLED(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
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
                controller->SetLED(start_idx + led_positions[i]->led_num, ToRGBColor(color.red(), color.green(), color.blue()));
            }
        }

        break;
    }
}
