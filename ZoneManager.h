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

    void IdentifyZone(ControllerZone*);
    void IdentifyLeds(ControllerZone*, std::vector<unsigned int>);

private:
    ZoneManager(){};
    static ZoneManager* instance;

    void SetControllerZoneColor(ControllerZone*, QColor);
    void InitMatrixCustomShape(ControllerZone*);
};

#endif // ZONEMANAGER_H
