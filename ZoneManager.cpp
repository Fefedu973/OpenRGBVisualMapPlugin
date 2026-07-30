/*---------------------------------------------------------*\
| ZoneManager.cpp                                           |
|                                                           |
|   Zone management for Visual Map Plugin                   |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <set>
#include "OpenRGBVisualMapPlugin.h"
#include "VirtualController.h"
#include "ZoneManager.h"

ZoneManager* ZoneManager::instance;

ZoneManager* ZoneManager::Get()
{
    if(!instance)
    {
        instance = new ZoneManager();
    }

    return instance;
}

void ZoneManager::UpdateControllerZones()
{
    OpenRGBVisualMapPlugin::controller_zones.clear();
    
    /*-----------------------------------------------------*\
    | Create ControllerZones for new controllers            |
    \*-----------------------------------------------------*/
    for(RGBControllerInterface* controller : OpenRGBVisualMapPlugin::api->GetRGBControllers())
    {
        /*-------------------------------------------------*\
        | Create a ControllerZone for each zone and each    |
        | segment in the controller                         |
        \*-------------------------------------------------*/
        for(std::size_t zone_idx = 0; zone_idx < controller->GetZoneCount(); zone_idx++)
        {
            if((controller->GetZoneSegmentCount(zone_idx) != 0) && (controller->GetZoneType(zone_idx) == ZONE_TYPE_SEGMENTED))
            {
                for(std::size_t segment_idx = 0; segment_idx < controller->GetZoneSegmentCount(zone_idx); segment_idx++)
                {
                    ControllerZone* controller_zone     = new ControllerZone();

                    controller_zone->set_controller(controller);
                    controller_zone->zone_idx           = zone_idx;
                    controller_zone->segment_idx        = segment_idx;
                    controller_zone->is_segment         = true;
                    controller_zone->settings           = ControllerZoneSettings::defaults();
                    controller_zone->custom_zone_name   = "";

                    if(controller_zone->controller->GetZoneSegmentType(controller_zone->zone_idx, controller_zone->segment_idx) == ZONE_TYPE_MATRIX)
                    {
                        InitMatrixCustomShape(controller_zone);
                    }

                    OpenRGBVisualMapPlugin::controller_zones.push_back(controller_zone);
                }
            }
            else
            {
                ControllerZone* controller_zone     = new ControllerZone();

                controller_zone->set_controller(controller);
                controller_zone->zone_idx           = zone_idx;
                controller_zone->segment_idx        = 0;
                controller_zone->is_segment         = false;
                controller_zone->settings           = ControllerZoneSettings::defaults();
                controller_zone->custom_zone_name   = "";

                if(controller_zone->controller->GetZoneType(controller_zone->zone_idx) == ZONE_TYPE_MATRIX)
                {
                    InitMatrixCustomShape(controller_zone);
                }

                OpenRGBVisualMapPlugin::controller_zones.push_back(controller_zone);
            }
        }
    }
}


void ZoneManager::IdentifyZone(ControllerZone* ctrl_zone_to_identify)
{
    /*-----------------------------------------------------*\
    | Make sure we update the controller only once by using |
    | a set                                                 |
    \*-----------------------------------------------------*/
    std::set<RGBControllerInterface*>   controllers;

    for(ControllerZone* ctrl_zone: OpenRGBVisualMapPlugin::controller_zones)
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
    RGBControllerInterface* controller  = ctrl_zone->controller;
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

    for(unsigned int i = 0; i < leds_count; i++)
    {
        controller->SetColor(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
    }
}

void ZoneManager::IdentifyLeds(ControllerZone* ctrl_zone, std::vector<unsigned int> led_nums)
{
    RGBControllerInterface* controller  = ctrl_zone->controller;
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

    for(unsigned int i = 0; i < leds_count; i++)
    {
        QColor color                    = std::find(led_nums.begin(), led_nums.end(), i) != led_nums.end() ? Qt::green : Qt::black;

        controller->SetColor(start_idx + i, ToRGBColor(color.red(), color.green(), color.blue()));
    }

    controller->UpdateLEDs();
}

void ZoneManager::InitMatrixCustomShape(ControllerZone* ctrl_zone)
{
    RGBControllerInterface*  controller         = ctrl_zone->controller;

    ctrl_zone->settings.shape                   = CUSTOM;
    ctrl_zone->settings.custom_shape            = new CustomShape();
    
    if(ctrl_zone->is_segment)
    {
        ctrl_zone->settings.custom_shape->w     = controller->GetZoneSegmentMatrixMapWidth(ctrl_zone->zone_idx, ctrl_zone->segment_idx);
        ctrl_zone->settings.custom_shape->h     = controller->GetZoneSegmentMatrixMapWidth(ctrl_zone->zone_idx, ctrl_zone->segment_idx);
    }
    else
    {
        ctrl_zone->settings.custom_shape->w     = controller->GetZoneMatrixMapWidth(ctrl_zone->zone_idx);
        ctrl_zone->settings.custom_shape->h     = controller->GetZoneMatrixMapWidth(ctrl_zone->zone_idx);
    }

    for(unsigned int h = 0; h < ctrl_zone->settings.custom_shape->h; h++)
    {
        for(unsigned int w = 0; w < ctrl_zone->settings.custom_shape->w; w++)
        {
            unsigned int led_num;
            
            if(ctrl_zone->is_segment)
            {
                led_num                         = controller->GetZoneSegmentMatrixMapData(ctrl_zone->zone_idx, ctrl_zone->segment_idx)[h * ctrl_zone->settings.custom_shape->w + w];
            }
            else
            {
                led_num                         = controller->GetZoneMatrixMapData(ctrl_zone->zone_idx)[h * ctrl_zone->settings.custom_shape->w + w];
            }

            if(led_num != NA)
            {
                LedPosition* led_position       = new LedPosition();

                led_position->led_num           = led_num;
                led_position->setX(w);
                led_position->setY(h);

                ctrl_zone->settings.custom_shape->led_positions.push_back(led_position);
            }
        }
    }
}
