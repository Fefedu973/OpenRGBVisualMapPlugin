#ifndef ZONEMANAGER_H
#define ZONEMANAGER_H

#include <QImage>
#include <vector>
#include "ControllerZone.h"

class ZoneManager
{
public:
    static ZoneManager* Get();

    std::vector<ControllerZone*> GetAvailableZones();
    ControllerZone* GetZone(int);

    void IdentifyZone(ControllerZone*);
    void IdentifyLed(ControllerZone*, int);
    void ApplyImage(std::vector<ControllerZone*>, QImage);

private:
    ZoneManager();
    static ZoneManager* instance;

    void SetControllerZoneColor(ControllerZone*, QColor);

    std::vector<ControllerZone*> available_zones;

    void ApplyImage(ControllerZone*, QImage);

    void InitMatrixCustomShape(ControllerZone*);
};

#endif // ZONEMANAGER_H
