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
    std::vector<ControllerZone*> GetAddedZones();
    ControllerZone* GetZone(int);

    bool HasZone(int);
    void AddZone(int);
    void RemoveZone(int);
    void ClearZones();

    void IdentifyZone(ControllerZone*);
    void IdentifyLed(ControllerZone*, int);

    void ApplyImage(QImage*);

private:
    ZoneManager();
    static ZoneManager* instance;

    void SetControllerZoneColor(ControllerZone*, QColor);
    void ApplyImage(ControllerZone*, QImage*);

    std::vector<ControllerZone*> available_zones;
    std::vector<ControllerZone*> added_zones;
};

#endif // ZONEMANAGER_H
