/*---------------------------------------------------------*\
| ControllerZone.h                                          |
|                                                           |
|   OpenRGB Visual Map Plugin Controller Zone               |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <QPoint>
#include <QPointF>
#include <QStringList>
#include <vector>
#include "RGBControllerInterface.h"

struct LedPosition
{
    unsigned int led_num;

    QPointF point;

    qreal x() const
    {
        return point.x();
    }

    qreal y() const
    {
        return point.y();
    }

    void setX(qreal x)
    {
        point.setX(x);
    }

    void setY(qreal y)
    {
        point.setY(y);
    }

    void shift(qreal shift_x, qreal shift_y)
    {
        setX(x() + shift_x);
        setY(y() + shift_y);
    }

    LedPosition* clone()
    {
        LedPosition* clone  = new LedPosition();
        
        clone->led_num      = led_num;
        clone->point        = point;

        return clone;
    }
};

struct CustomShape
{
    qreal                       w;
    qreal                       h;
    std::vector<LedPosition*>   led_positions;

    ~CustomShape()
    {
        for(LedPosition* led_position: led_positions)
        {
            delete led_position;
        }
    }

    CustomShape* clone()
    {
        CustomShape* clone  = new CustomShape();

        clone->w            = w;
        clone->h            = h;

        for(LedPosition* led_position: led_positions)
        {
            clone->led_positions.push_back(led_position->clone());
        }

        return clone;
    }

    static CustomShape* HorizontalLine(unsigned int led_count)
    {
        CustomShape* shape  = new CustomShape();

        shape->w            = led_count;
        shape->h            = 1;
        
        shape->led_positions.resize(led_count);

        for(unsigned int i = 0; i < led_count; i++)
        {
            LedPosition* led_position   = new LedPosition();

            led_position->led_num       = i;
            led_position->setX(i);
            led_position->setY(0);

            shape->led_positions[i]     = led_position;
        }

        return shape;
    }

    bool differs(CustomShape* other)
    {

        if(w != other->w)
        {
            return true;
        }

        if(h != other->h)
        {
            return true;
        }

        if(led_positions.size() != other->led_positions.size())
        {
            return true;
        }

        for(unsigned int i = 0; i < led_positions.size(); i++)
        {
            if(led_positions[i]->x() != other->led_positions[i]->x())
            {
                return true;
            }

            if(led_positions[i]->y() != other->led_positions[i]->y())
            {
                return true;
            }
        }

        return false;
    }

    void resizeCustomShape(unsigned int led_count)
    {
        led_positions.resize(led_count);

        for(unsigned int i = 0; i < led_count; i++)
        {
            if(led_positions[i] == nullptr)
            {
                LedPosition* led_position = new LedPosition();
                led_position->led_num = i;
                led_position->setX(i);
                led_position->setY(0);
                led_positions[i] = led_position;
            }
        }
    }
};

struct ControllerInfo
{
    std::string name;
    std::string vendor;
    std::string description;
    std::string version;
    std::string serial;
    std::string location;
};

enum ZoneShape
{
    HORIZONTAL_LINE = 0,
    VERTICAL_LINE = 1,
    CUSTOM = 2
};

struct ControllerZoneSettings
{
    ZoneShape       shape;
    CustomShape*    custom_shape;

    qreal           x;
    qreal           y;
    qreal           scale;
    unsigned int    led_spacing;

    bool            reverse;
    qreal           scale_x = 1.0;
    qreal           scale_y = 1.0;
    qreal           rotation = 0.0;
    bool            flip_x = false;
    bool            flip_y = false;
    bool            point_is_center = false; // Old JSON uses the top-left of a unit LED cell.
    qreal           brightness = 1.0;

    static ControllerZoneSettings defaults()
    {
        return {HORIZONTAL_LINE, nullptr, 0.0, 0.0, 1.0, 1, false};
    }
};

struct ControllerZone
{
    RGBControllerInterface* controller;
    unsigned int            zone_idx = 0;
    unsigned int            segment_idx = 0;
    bool                    is_segment = false;
    std::string             custom_zone_name;
    ControllerZoneSettings  settings;
    ControllerInfo          controller_info;

    void set_controller(RGBControllerInterface* new_controller)
    {
        this->controller                    = new_controller;
        this->controller_info.name          = this->controller->GetName();
        this->controller_info.vendor        = this->controller->GetVendor();
        this->controller_info.description   = this->controller->GetDescription();
        this->controller_info.version       = this->controller->GetVersion();
        this->controller_info.serial        = this->controller->GetSerial();
        this->controller_info.location      = this->controller->GetLocation();
    }

    ControllerZone* clone()
    {
        ControllerZone* clone           = new ControllerZone();

        clone->controller               = controller;
        clone->zone_idx                 = zone_idx;
        clone->segment_idx              = segment_idx;
        clone->is_segment               = is_segment;
        clone->custom_zone_name         = custom_zone_name;
        clone->settings                 = settings;
        clone->controller_info          = controller_info;

        if(settings.custom_shape)
        {
            clone->settings.custom_shape = settings.custom_shape->clone();
        }

        return clone;
    }

    bool compare_controller(RGBControllerInterface* other) const
    {
        return
                this->controller_info.name          == other->GetName() &&
                this->controller_info.vendor        == other->GetVendor() &&
                this->controller_info.description   == other->GetDescription() &&
                this->controller_info.version       == other->GetVersion() &&
                this->controller_info.serial        == other->GetSerial() &&
                this->controller_info.location      == other->GetLocation() ;
    }

    bool compare(ControllerZone* rhs) const
    {
        return this->compare_controller(rhs->controller) && this->zone_idx == rhs->zone_idx
            && this->is_segment == rhs->is_segment
            && (!this->is_segment || this->segment_idx == rhs->segment_idx);
    }

    bool operator==(ControllerZone* rhs) const
    {
        return this->compare(rhs);
    }

    unsigned int led_count() const
    {
        if(is_segment)
        {
            return controller->GetZoneSegmentLEDsCount(zone_idx, segment_idx);
        }
        else
        {
            return controller->GetZoneLEDsCount(zone_idx);
        }
    }

    unsigned int start_idx() const
    {
        return controller->GetZoneStartIndex(zone_idx)
            + (is_segment ? controller->GetZoneSegmentStartIndex(zone_idx,segment_idx) : 0);
    }

    std::string full_display_name()
    {
        if(is_segment)
        {
            return this->custom_zone_name.empty() ?
                        this->controller->GetName() + " " + this->controller->GetZoneName(this->zone_idx) + " " + this->controller->GetZoneSegmentName(this->zone_idx, this->segment_idx):
                        this->custom_zone_name;
        }
        else
        {
            return this->custom_zone_name.empty() ?
                        this->controller->GetName() + " " + this->controller->GetZoneName(this->zone_idx):
                        this->custom_zone_name;
        }
    }

    std::string zone_display_name()
    {
        if(is_segment)
        {
            return this->custom_zone_name.empty() ?
                        this->controller->GetZoneName(this->zone_idx) + " " + this->controller->GetZoneSegmentName(this->zone_idx, this->segment_idx):
                        this->custom_zone_name;
        }
        else
        {
            return this->custom_zone_name.empty() ?
                        this->controller->GetZoneName(this->zone_idx):
                        this->custom_zone_name;
        }
    }

    std::string controller_display_name()
    {
        return this->custom_zone_name.empty() ?
                    this->controller->GetName() :
                    this->custom_zone_name;
    }

    bool isCustomShape() const
    {
        return this->settings.shape == CUSTOM;
    }

    qreal width () const
    {
        switch(this->settings.shape)
        {
            case CUSTOM: return this->settings.custom_shape->w * this->settings.scale;
            case HORIZONTAL_LINE: return (this->led_count() > 0 ? (this->led_count() - 1) * this->settings.led_spacing + 1 : 1) * this->settings.scale;
            case VERTICAL_LINE: return this->settings.scale;
        }

        return 0;
    }

    qreal height () const
    {
        switch(this->settings.shape)
        {
            case CUSTOM: return this->settings.custom_shape->h * this->settings.scale;
            case HORIZONTAL_LINE: return this->settings.scale;
            case VERTICAL_LINE: return (this->led_count() > 0 ? (this->led_count() - 1) * this->settings.led_spacing + 1 : 1) * this->settings.scale;
        }

        return 0;
    }
};
