/*---------------------------------------------------------*\
| VirtualControllerTab.h                                    |
|                                                           |
|   Virtual controller tab for visual map plugin            |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <QDesktopServices>
#include <QSignalMapper>
#include <QTabBar>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>
#include <nlohmann/json.hpp>
#include "ui_VirtualControllerTab.h"
#include "VirtualController.h"

using json = nlohmann::json;

namespace Ui
{
    class VirtualControllerTab;
}

class VirtualControllerTab : public QWidget
{
    Q_OBJECT

public:
    explicit VirtualControllerTab(QWidget *parent = nullptr);
    ~VirtualControllerTab();

    void        RenameController(std::string);
    std::string GetControllerName();

    void        LoadFile(std::string);
    void        LoadJson(json);
    void        Clear();
    void        Hide();
    void        Recreate();

private slots:
    /*-----------------------------------------------------*\
    | UI element signals                                    |
    \*-----------------------------------------------------*/
    void on_backgroundApplier_BackgroundUpdated(const QImage&);
    void on_gridOptions_SettingsChanged();
    void on_gridOptions_AutoResizeRequest();

    void on_itemOptions_ShapeEditRequest(ControllerZone*);
    void on_itemOptions_ItemOptionsChanged();

    void on_grid_Changed();
    void on_grid_SelectionChanged(std::vector<ControllerZone*>);

    void on_device_list_DeviceAdded(ControllerZone*);
    void on_device_list_DeviceRemoved(ControllerZone*);
    void on_device_list_SelectionChanged(std::vector<ControllerZone*>);

    void VirtualControllerPostUpdateSlot(const QImage&);

    /*-----------------------------------------------------*\
    | Main menu actions                                     |
    \*-----------------------------------------------------*/
    void SaveVmapAction();
    void LoadVmapAction();
    void ClearVmapAction();
    void RegisterAction();
    void AddBackgroundAction();

signals:
    void ControllerRenamed(std::string);
    void VirtualControllerPostUpdateSignal(const QImage&);

private:
    void AddActiveZone(ControllerZone* added_zone);
    void RemoveActiveZone(ControllerZone* removed_zone);
    void CreateMainMenu();
    void InitZoneList();
    void UpdateVirtualControllerDetails();
    void ReassignZones();
    void UpdateItemOptions(std::vector<ControllerZone*>);

    Ui::VirtualControllerTab*   ui;
    VirtualController*          virtual_controller;
    GridSettings*               settings;
    ControllerZone*             selected_ctrl_zone = nullptr;
    QAction*                    register_controller;
    QAction*                    add_background;
    json                        active_state;

protected:
    void resizeEvent(QResizeEvent*) override;

};