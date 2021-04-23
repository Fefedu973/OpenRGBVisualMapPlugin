#ifndef VIRTUALCONTROLLERTAB_H
#define VIRTUALCONTROLLERTAB_H

#include <QWidget>
#include <QTabBar>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSignalMapper>

#include "ui_VirtualControllerTab.h"
#include "VirtualController.h"
#include "RGBController.h"
#include "Grid.h"
#include "GridOptions.h"
#include "ItemOptions.h"
#include "BackgroundApplier.h"


namespace Ui {
class VirtualControllerTab;
}

class VirtualControllerTab : public QTabBar
{
    Q_OBJECT

public:
    explicit VirtualControllerTab(QWidget *parent = nullptr);
    ~VirtualControllerTab();

    void RenameController(std::string);
    std::string GetControllerName();

private slots:
    void OnZoneSelectionChanged();
    void OnItemOptionsChanged();
    void OnBackgroundApplied(QImage);

    void on_saveButton_clicked();
    void on_loadButton_clicked();
    void on_resetButton_clicked();
    void on_register_controller_stateChanged(int);

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

    void DecorateButton(QPushButton*, QIcon);
    void UpdateZoneButtons();
    void InitZoneList();
    void resizeEvent(QResizeEvent*);

    std::vector<ControllerZone*> added_zones;
};

#endif // VIRTUALCONTROLLERTAB_H
