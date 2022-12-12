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
    void UnregisterAll();

public slots:
    void Clear();
    void Backup();
    void Recreate();

private slots:
    void AddTabSlot();
    void AboutSlot();
    void OpenVmapsFolder();

private:
    Ui::OpenRGBVisualMapTab*   ui;

    std::vector<VirtualControllerTab*> controller_tabs;

    bool SearchAndAutoLoad();
    VirtualControllerTab* AddTab();

};

#endif // OPENRGBVISUALMAPTAB_H
