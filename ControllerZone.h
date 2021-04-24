#ifndef CONTROLLERZONE_H
#define CONTROLLERZONE_H

#include "RGBController.h"
#include <vector>
#include <QPoint>
#include <QStringList>

struct LedPosition
{
    unsigned int led_num;

    QPoint point;

    unsigned int x()
    {
        return point.x();
    }

    unsigned int y()
    {
        return point.y();
    }

    void setX(int x)
    {
        point.setX(x);
    }

    void setY(int y)
    {
        point.setY(y);
    }

    LedPosition* clone()
    {
        LedPosition* clone = new LedPosition();
        clone->led_num = led_num;
        clone->point = QPoint(x(), y());
        return clone;
    }
};

struct CustomShape
{
    unsigned int w;
    unsigned int h;
    std::vector<LedPosition*> led_positions;

    CustomShape* clone()
    {
        CustomShape* clone = new CustomShape();
        clone->w = w;
        clone->h = h;

        for(LedPosition* led_position: led_positions)
        {
            clone->led_positions.push_back(led_position->clone());
        }

        return clone;
    }

    static CustomShape* HorizontalLine(unsigned int led_count)
    {
        CustomShape* shape = new CustomShape();
        shape->w = led_count;
        shape->h = 1;
        shape->led_positions.resize(led_count);

        for(unsigned int i = 0; i < led_count; i++)
        {
            LedPosition* led_position = new LedPosition();
            led_position->led_num = i;
            led_position->setX(i);
            led_position->setY(0);
            shape->led_positions[i] = led_position;
        }

        return shape;
    }

    bool differs(CustomShape* other)
    {

        if(w != other->w)
        {
            printf("w changed \n");
            return true;
        }

        if(h != other->h)
        {
            printf("h changed \n");
            return true;
        }

        if(led_positions.size() != other->led_positions.size())
        {
            printf("size changed\n");
            return true;
        }

        for(unsigned int i = 0; i < led_positions.size(); i++)
        {
            if(led_positions[i]->x() != other->led_positions[i]->x())
            {
                printf("led %d x changed \n", i);
                return true;
            }

            if(led_positions[i]->y() != other->led_positions[i]->y())
            {
                printf("led %d y changed \n", i);
                return true;
            }
        }

        return false;
    }
};

enum ZoneShape {
    HORIZONTAL_LINE = 0,
    VERTICAL_LINE = 1,
    CUSTOM = 2
};

inline static const QStringList ZONE_SHAPES = {
    "Horizontal line",
    "Vertical line",
    "Custom"
};

struct ControllerZoneSettings
{
    ZoneShape shape;
    CustomShape* custom_shape;

    unsigned int x;
    unsigned int y;
    unsigned int led_spacing;

    bool reverse;

    static ControllerZoneSettings defaults() {
        return {
            HORIZONTAL_LINE,  nullptr, 0, 0, 1, false
        };
    }
};

struct ControllerZone
{
    RGBController* controller;
    unsigned int zone_idx;

    std::string custom_zone_name;

    ControllerZoneSettings settings;

    bool operator==(ControllerZone const & rhs) const {
        return this->controller == rhs.controller && this->zone_idx == rhs.zone_idx;
    }

    unsigned int led_count() const {
        return controller->zones[zone_idx].leds_count;
    }

    std::string display_name()
    {
        return this->custom_zone_name.empty() ?
                    this->controller->name + " - " + this->controller->zones[this->zone_idx].name :
                    this->custom_zone_name;
    }

    bool isCustomShape()  {
        return this->settings.shape == CUSTOM;
    }
};

#endif // CONTROLLERZONE_H
