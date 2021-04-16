#ifndef ZONEMANAGER_H
#define ZONEMANAGER_H

#include <QStringList>
#include <QColor>
#include <QPoint>
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
    void SetControllerZoneColor(ControllerZone*, QColor);

private:
    ZoneManager();
    static ZoneManager* instance;

    std::vector<ControllerZone*> available_zones;
    std::vector<ControllerZone*> added_zones;
};

#endif // ZONEMANAGER_H
