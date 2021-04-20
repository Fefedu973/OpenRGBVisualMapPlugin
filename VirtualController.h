#ifndef VIRTUALCONTROLLER_H
#define VIRTUALCONTROLLER_H

#include "RGBController.h"
#include "ControllerZone.h"
#include <QImage>

class VirtualController: public RGBController
{

public:
    inline const static std::string VIRTUAL_CONTROLLER_SERIAL = "VISUAL_MAP_VISUAL_CONTROLLER_SERIAL";

    VirtualController();

    // RGBController overrides
    void DeviceUpdateLEDs()    override;

    void SetupZones()          override {};
    void SetupColors()         override {};
    void ResizeZone(int, int)  override {};
    void SetCustomMode()       override {};
    void DeviceUpdateMode()    override {};
    void UpdateZoneLEDs(int)   override {};
    void UpdateSingleLED(int)  override {};

    // Internals
    void UpdateSize(int,int);

    void SetCallBack(std::function<void(QImage*)>);

private:
    int width;
    int height;

    void SetupVirtualZone();    

    std::function<void(QImage*)> callback;
};

#endif // VIRTUALCONTROLLER_H
