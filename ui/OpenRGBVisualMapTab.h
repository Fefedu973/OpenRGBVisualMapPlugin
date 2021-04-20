#ifndef OPENRGBVISUALMAPTAB_H
#define OPENRGBVISUALMAPTAB_H

#include <QWidget>
#include "ui_OpenRGBVisualMapTab.h"

namespace Ui {
class OpenRGBVisualMapTab;
}

class OpenRGBVisualMapTab : public QWidget
{
    Q_OBJECT

public:
    explicit OpenRGBVisualMapTab(QWidget *parent = nullptr);
    ~OpenRGBVisualMapTab();

private:
    Ui::OpenRGBVisualMapTab*   ui;   
};

#endif // OPENRGBVISUALMAPTAB_H
