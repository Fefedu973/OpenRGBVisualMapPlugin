#ifndef ZONEMANAGER_H
#define ZONEMANAGER_H

#include <QImage>
#include <vector>
#include "ControllerZone.h"

class ZoneManager
{
public:
    static ZoneManager* Get();

    void IdentifyZone(ControllerZone*);
    void IdentifyLeds(ControllerZone*, std::vector<unsigned int>);

    void UpdateControllerZones();

private:
    ZoneManager(){};
    static ZoneManager* instance;

    void SetControllerZoneColor(ControllerZone*, QColor);
    void InitMatrixCustomShape(ControllerZone*);
};

#endif // ZONEMANAGER_H
