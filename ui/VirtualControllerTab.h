#ifndef VIRTUALCONTROLLERTAB_H
#define VIRTUALCONTROLLERTAB_H

#include <QWidget>
#include <QTabBar>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSignalMapper>
#include <QDesktopServices>

#include "ui_VirtualControllerTab.h"
#include "VirtualController.h"
#include "RGBController.h"
#include "Grid.h"
#include "GridOptions.h"
#include "ItemOptions.h"
#include "BackgroundApplier.h"
#include "json.hpp"

using json = nlohmann::json;

namespace Ui {
class VirtualControllerTab;
}

class VirtualControllerTab : public QWidget
{
    Q_OBJECT

public:
    explicit VirtualControllerTab(QWidget *parent = nullptr);
    ~VirtualControllerTab();

    void RenameController(std::string);
    std::string GetControllerName();

    void LoadFile(std::string);
    void LoadJson(json);
    void Clear();
    void Unregister();
    void Recreate();
    void BackupZones();

private slots:
    void OnZoneSelectionChanged();
    void OnGridSelectionChanged();
    void OnZoneDoubleClick(int, int);
    void OnItemOptionsChanged();
    void OnAutoResizeRequest();
    void OnBackgroundApplied(QImage);
    void OnSettingsChanged();

    void SaveVmapAction();
    void LoadVmapAction();
    void ClearVmapAction();
    void OpenVmapsFolder();
    void on_register_controller_stateChanged(int);
    void RegisterAction();

signals:
    void ApplyBackground(QImage);
    void ControllerRenamed(std::string);

private:
    Ui::VirtualControllerTab*   ui;
    VirtualController* virtual_controller;
    GridSettings* settings;

    ControllerZone* selected_ctrl_zone = nullptr;

    QIcon add_icon = QIcon(":/add.png");
    QIcon remove_icon = QIcon(":/remove.png");

    QAction* register_controller;

    void DecorateButton(QPushButton*, QIcon);
    void UpdateZoneButtons();
    void InitZoneList();
    void UpdateVirtualControllerDetails();
    std::vector<ControllerZone*> retained_zones;

    json saved_zones;

    void ReassignZones();

protected:
    void resizeEvent(QResizeEvent*) override;

};

#endif // VIRTUALCONTROLLERTAB_H
