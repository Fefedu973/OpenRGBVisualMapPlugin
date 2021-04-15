#ifndef OPENRGBVISUALMAPTAB_H
#define OPENRGBVISUALMAPTAB_H

#include <QWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSignalMapper>

#include "ui_OpenRGBVisualMapTab.h"
#include "RGBController.h"
#include "Grid.h"
#include "GridOptions.h"
#include "ItemOptions.h"
#include "Gradient.h"

namespace Ui {
class OpenRGBVisualMapTab;
}

class OpenRGBVisualMapTab : public QWidget
{
    Q_OBJECT

public:
    explicit OpenRGBVisualMapTab(QWidget *parent = nullptr);
    ~OpenRGBVisualMapTab();

private slots:
    void OnZoneSelectionChanged();
    void OnItemOptionsChanged();
    void OnGradientApplied(QImage*);

    void on_saveButton_clicked();
    void on_loadButton_clicked();

private:
    Ui::OpenRGBVisualMapTab*   ui;

    QIcon add_icon = QIcon(":/add.png");
    QIcon remove_icon = QIcon(":/remove.png");

    void DecorateButton(QPushButton*, QIcon);
    void InitZoneList();
    void resizeEvent(QResizeEvent*);
    void UpdateControllerZone(ControllerZone*,QImage*);
};

#endif // OPENRGBVISUALMAPTAB_H
