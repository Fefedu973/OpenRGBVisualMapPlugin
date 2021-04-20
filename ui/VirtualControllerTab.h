#ifndef VIRTUALCONTROLLERTAB_H
#define VIRTUALCONTROLLERTAB_H

#include <QWidget>
#include <QTabBar>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSignalMapper>

#include "ui_VirtualControllerTab.h"
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

private slots:
    void OnZoneSelectionChanged();
    void OnItemOptionsChanged();
    void OnBackgroundApplied(QImage*);

    void on_saveButton_clicked();
    void on_loadButton_clicked();
    void on_resetButton_clicked();

protected:
    //bool eventFilter(QObject * o, QEvent * e);

private:
    Ui::VirtualControllerTab*   ui;
    GridSettings* settings;

    QIcon add_icon = QIcon(":/add.png");
    QIcon remove_icon = QIcon(":/remove.png");

    void DecorateButton(QPushButton*, QIcon);
    void UpdateZoneButtons();
    void InitZoneList();
    void resizeEvent(QResizeEvent*);
};

#endif // VIRTUALCONTROLLERTAB_H
