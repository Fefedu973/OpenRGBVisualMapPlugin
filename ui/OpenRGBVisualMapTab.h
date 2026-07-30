/*---------------------------------------------------------*\
| OpenRGBVisualMapTab.h                                     |
|                                                           |
|   OpenRGB Visual Map tab                                  |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <QWidget>
#include "ui_OpenRGBVisualMapTab.h"
#include "VirtualControllerTab.h"

namespace Ui
{
    class OpenRGBVisualMapTab;
}

class OpenRGBVisualMapTab : public QWidget
{
    Q_OBJECT

public:
    explicit OpenRGBVisualMapTab(QWidget *parent = nullptr);
    ~OpenRGBVisualMapTab();
    void HideAll();

public slots:
    void Clear();
    void Recreate();
    void PauseForDetection();

private slots:
    void AddTabSlot();
    void AboutSlot();
    void OpenVmapsFolder();

private:
    Ui::OpenRGBVisualMapTab*            ui;
    std::vector<VirtualControllerTab*>  controller_tabs;

    bool                    SearchAndAutoLoad();
    VirtualControllerTab*   AddTab();

};
