#ifndef VIRTUALCONTROLLER_H
#define VIRTUALCONTROLLER_H

#include <QObject>
#include "RGBController.h"
#include "ControllerZone.h"

enum VirtualControllerType
{
    LINEAR_LEFT_TO_RIGHT = 0,
    LINEAR_RIGHT_TO_LEFT = 1,
    LINEAR_TOP_TO_BOTTOM = 2,
    LINEAR_BOTTOM_TO_TOP = 3,
    MATRIX = 4
};

class VirtualController: public RGBController
{

public:

    inline const static std::string VIRTUAL_CONTROLLER_SERIAL = "VISUAL_MAP_VISUAL_CONTROLLER_SERIAL";

    VirtualController();

    // RGBController overrides
    void SetupZones()          override {};
    void SetupColors()         override {};
    void ResizeZone(int, int)  override {};
    void SetCustomMode()       override {};
    void DeviceUpdateMode()    override {};
    void DeviceUpdateLEDs()    override;

    void UpdateZoneLEDs(int)   override {
        printf("UpdateZoneLEDs\n");
    };

    void UpdateSingleLED(int)  override {
        printf("UpdateSingleLED\n");
    };

    // Internals
    void SetType(VirtualControllerType);
    void UpdateSize(int,int);
    void SetupVirtualZone();

private:
   // static void UpdateCallback(void *);

    VirtualControllerType type;
    int width;
    int height;
};

#endif // VIRTUALCONTROLLER_H
