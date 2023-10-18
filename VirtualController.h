#ifndef VIRTUALCONTROLLER_H
#define VIRTUALCONTROLLER_H

#define NA 0xFFFFFFFF

#include "RGBController.h"
#include "ControllerZone.h"
#include <QImage>
#include <functional>

class VirtualController: public RGBController
{

public:
    static std::string VIRTUAL_CONTROLLER_SERIAL;

    VirtualController();
    ~VirtualController();

    // RGBController overrides
    void                            DeviceUpdateLEDs()    override;
    void                            SetupZones()          override {};
    void                            SetupColors()         override {};
    void                            ResizeZone(int, int)  override {};
    void                            SetCustomMode()       override {};
    void                            DeviceUpdateMode()    override {};
    void                            UpdateZoneLEDs(int)   override {};
    void                            UpdateSingleLED(int)  override {};

    // Internals
    void                            UpdateSize(int,int);
    void                            SetPostUpdateCallBack(std::function<void(const QImage&)>);
    void                            Register(bool, bool);
    bool                            HasZone(ControllerZone*);
    void                            Add(ControllerZone*);
    void                            Remove(ControllerZone*);
    void                            Clear();
    std::vector<ControllerZone*>    GetZones();
    bool                            IsEmpty();
    void                            ApplyToDevice(const QImage&);
    void                            ApplyImage(const QImage&);
    void                            ApplyToZone(ControllerZone*, const QImage&);
    unsigned int                    GetTotalLeds();
    void                            UpdateVirtualZone();

private:
    unsigned int                    width;
    unsigned int                    height;
    bool                            registered = false;
    bool                            members_unregistered = false;
    std::function<void(QImage)>     callback;
    std::vector<ControllerZone*>    added_zones;

    void                            ForceDirectMode();
};

#endif // VIRTUALCONTROLLER_H
