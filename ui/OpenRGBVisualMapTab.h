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
#include "BackgroundApplier.h"

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
    void OnBackgroundApplied(QImage*);

    void on_saveButton_clicked();
    void on_loadButton_clicked();
    void on_resetButton_clicked();

private:
    Ui::OpenRGBVisualMapTab*   ui;
    GridSettings* settings;

    QIcon add_icon = QIcon(":/add.png");
    QIcon remove_icon = QIcon(":/remove.png");

    void DecorateButton(QPushButton*, QIcon);
    void UpdateZoneButtons();
    void InitZoneList();
    void resizeEvent(QResizeEvent*);
};

#endif // OPENRGBVISUALMAPTAB_H
