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
    void IdentifyLeds(ControllerZone*, std::vector<unsigned int>);
    void ApplyImage(std::vector<ControllerZone*>, QImage);
    void ResetControllerZones();
    void Clear();

private:
    ZoneManager();    
    static ZoneManager* instance;

    std::vector<ControllerZone*> available_zones;

    void SetControllerZoneColor(ControllerZone*, QColor);
    void ApplyImage(ControllerZone*, QImage);
    void InitMatrixCustomShape(ControllerZone*);
};

#endif // ZONEMANAGER_H
