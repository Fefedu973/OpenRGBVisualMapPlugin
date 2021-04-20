#include "VirtualController.h"

VirtualController::VirtualController()
{
    width = 1;
    height = 1;

    name = "VisualMap controller";
    vendor = "VisualMap plugin";
    description = "Virtual controller provided by VisualMap plugin";
    version = "1.0.0";
    serial = VIRTUAL_CONTROLLER_SERIAL;
    location = "Somewhere over the rainbow";
    active_mode = 0;

    zones.resize(1);
    modes.resize(1);

    zones[0] = zone();
    modes[0] = mode();

    zones[0].matrix_map = new matrix_map_type();

    SetupVirtualZone();
}

void VirtualController::SetupVirtualZone()
{
    int size = width * height;

    unsigned int *map = new unsigned int[height * width];

    colors.resize(size);
    leds.resize(size);

    for(int h = 0; h<height; h++)
    {
        for(int w = 0; w < width; w++)
        {
            int idx = (h*width) + w;
            colors[idx] = ToRGBColor(0,0,0);
            leds[idx].name = "LED " +  std::to_string(idx);
            map[(h*width) + w] = idx;
        }
    }

    zones[0].name = "Virtual zone";
    zones[0].leds_count = size;
    zones[0].leds_min = size;
    zones[0].leds_max = size;    
    zones[0].matrix_map->width = width;
    zones[0].matrix_map->height = height;
    zones[0].matrix_map->map = map;

    zones[0].start_idx = 0;
    zones[0].type = ZONE_TYPE_MATRIX;
    zones[0].colors = &colors[0];
    zones[0].leds = &leds[0];

    modes[0].name = "Direct";
    modes[0].colors.resize(size);
    modes[0].colors = colors;
    modes[0].value = 0;
    modes[0].flags = MODE_FLAG_HAS_PER_LED_COLOR;
    modes[0].color_mode = MODE_COLORS_PER_LED;
}

void VirtualController::DeviceUpdateLEDs() {
    QImage* image = new QImage(width, height, QImage::Format_ARGB32);

    for(int h = 0; h<height; h++)
    {
        for(int w = 0; w < width; w++)
        {
            int rgb = colors[(h*width) + w];
            QColor color = QColor(RGBGetRValue(rgb), RGBGetGValue(rgb), RGBGetBValue(rgb));
            image->setPixelColor(w, h, color);
        }
    }

    callback(image);
};

void VirtualController::UpdateSize(int w, int h)
{
    width = w;
    height = h;

    SetupVirtualZone();
}

void VirtualController::SetCallBack(std::function<void(QImage*)> callback)
{
    this->callback = callback;
}

