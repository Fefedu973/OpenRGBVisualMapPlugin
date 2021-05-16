#ifndef OPENRGBVISUALMAPTAB_H
#define OPENRGBVISUALMAPTAB_H

#include <QWidget>
#include "ui_OpenRGBVisualMapTab.h"
#include "VirtualControllerTab.h"

namespace Ui {
class OpenRGBVisualMapTab;
}

class OpenRGBVisualMapTab : public QWidget
{
    Q_OBJECT

public:
    explicit OpenRGBVisualMapTab(QWidget *parent = nullptr);
    ~OpenRGBVisualMapTab();

public slots:
    void Clear();
    void DeviceListChanged();

private slots:
    void AddTabSlot();

private:
    Ui::OpenRGBVisualMapTab*   ui;

    std::vector<VirtualControllerTab*> controller_tabs;

    bool SearchAndAutoLoad();
    VirtualControllerTab* AddTab();
};

#endif // OPENRGBVISUALMAPTAB_H
