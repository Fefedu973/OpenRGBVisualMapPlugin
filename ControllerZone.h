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

    int x()
    {
        return point.x();
    }

    int y()
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

    void shift(int shift_x, int shift_y)
    {
        setX(x() + shift_x);
        setY(y() + shift_y);
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

enum ZoneShape {
    HORIZONTAL_LINE = 0,
    VERTICAL_LINE = 1,
    CUSTOM = 2
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

    bool compare_controller(RGBController* other) const {
        return
                this->controller->name == other->name &&
                this->controller->vendor == other->vendor &&
                this->controller->description == other->description &&
                this->controller->version == other->version &&
                this->controller->serial == other->serial &&
                this->controller->location == other->location ;
    }

    bool compare(ControllerZone* rhs) const {
        return this->compare_controller(rhs->controller) && this->zone_idx == rhs->zone_idx;
    }

    bool operator==(ControllerZone* rhs) const {
        return this->compare(rhs);
    }

    unsigned int led_count() const {
        return controller->zones[zone_idx].leds_count;
    }

    std::string full_display_name()
    {
        return this->custom_zone_name.empty() ?
                    this->controller->name + " " + this->controller->zones[this->zone_idx].name:
                    this->custom_zone_name;
    }

    std::string zone_display_name()
    {
        return this->custom_zone_name.empty() ?
                    this->controller->zones[this->zone_idx].name :
                    this->custom_zone_name;
    }

    std::string controller_display_name()
    {
        return this->custom_zone_name.empty() ?
                    this->controller->name :
                    this->custom_zone_name;
    }

    bool isCustomShape()  {
        return this->settings.shape == CUSTOM;
    }

    int width () const
    {
        switch (this->settings.shape) {
            case CUSTOM: return this->settings.custom_shape->w;
            case HORIZONTAL_LINE: return this->led_count() * this->settings.led_spacing - (this->settings.led_spacing - 1);
            case VERTICAL_LINE: return 1;
        }

        return 0;
    }

    int height () const
    {
        switch (this->settings.shape) {
            case CUSTOM: return this->settings.custom_shape->h;
            case HORIZONTAL_LINE: return 1;
            case VERTICAL_LINE: return  this->led_count() * this->settings.led_spacing - (this->settings.led_spacing - 1);
        }

        return 0;
    }
};

#endif // CONTROLLERZONE_H
