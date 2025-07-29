#ifndef VIRTUALCONTROLLER_H
#define VIRTUALCONTROLLER_H

#define NA 0xFFFFFFFF

#include <functional>
#include <QImage>
#include "RGBControllerInterface.h"
#include "ControllerZone.h"

class VirtualController
{
public:
    static std::string VIRTUAL_CONTROLLER_SERIAL;

    VirtualController();
    ~VirtualController();

    // Virtual RGBController Functions
    void                            DeviceUpdateLEDs();

    // Internals
    void                            Add(ControllerZone*);
    void                            ApplyImage(const QImage&);
    void                            ApplyToDevice(const QImage&);
    void                            ApplyToZone(ControllerZone*, const QImage&);
    void                            Clear();
    std::string                     GetName();
    unsigned int                    GetTotalLeds();
    std::vector<ControllerZone*>    GetZones();
    bool                            HasZone(ControllerZone*);
    bool                            IsEmpty();
    void                            Register(bool, bool);
    void                            Remove(ControllerZone*);
    void                            SetName(std::string name);
    void                            SetPostUpdateCallBack(std::function<void(const QImage&)>);
    void                            UpdateSize(int,int);
    void                            UpdateVirtualZone();
    
private:
    RGBControllerInterface*         virtual_controller;
    unsigned int                    width;
    unsigned int                    height;
    bool                            registered = false;
    bool                            members_hidden = false;
    std::function<void(QImage)>     callback;
    std::vector<ControllerZone*>    added_zones;
    RGBController_Setup             setup;

    void                            ForceDirectMode();

    static void                     DeviceUpdateLEDs_func(void* object_ptr);
};

#endif // VIRTUALCONTROLLER_H
