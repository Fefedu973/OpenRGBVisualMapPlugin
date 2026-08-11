/*---------------------------------------------------------*\
| DeviceList.h                                              |
|                                                           |
|   Device list for visual map plugin                       |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <QWidget>
#include "ControllerZone.h"
#include "DeviceWidget.h"

namespace Ui
{
    class DeviceList;
}

class DeviceList : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceList(QWidget *parent = nullptr);
    ~DeviceList();

    void Clear();
    void Init(std::vector<ControllerZone*>);
    void SetSelection(std::vector<ControllerZone*>);
    void UpdateControllerState(ControllerZone*);

signals:
    void DeviceAdded(ControllerZone*);
    void DeviceRemoved(ControllerZone*);
    void SelectionChanged(std::vector<ControllerZone*>);

private:
    Ui::DeviceList *ui;
    std::vector<DeviceWidget*> device_widgets;
};
