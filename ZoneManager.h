/*---------------------------------------------------------*\
| ZoneManager.h                                             |
|                                                           |
|   Zone management for Visual Map Plugin                   |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <vector>
#include <QImage>
#include "ControllerZone.h"

class ZoneManager
{
public:
    static ZoneManager* Get();

    void IdentifyZone(ControllerZone*);
    void IdentifyLeds(ControllerZone*, std::vector<unsigned int>);

    void UpdateControllerZones();

    std::vector<ControllerZone*> CopyControllerZones();
    void FreeControllerZones(std::vector<ControllerZone*>& zones);

private:
    ZoneManager(){};
    static ZoneManager* instance;

    void SetControllerZoneColor(ControllerZone*, QColor);
    void InitMatrixCustomShape(ControllerZone*);
};
