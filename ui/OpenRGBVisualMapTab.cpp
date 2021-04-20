#include "OpenRGBVisualMapTab.h"
#include "VirtualControllerTab.h"


OpenRGBVisualMapTab::OpenRGBVisualMapTab(QWidget *parent):
    QWidget(parent),
    ui(new Ui::OpenRGBVisualMapTab)
{
    ui->setupUi(this);

    ui->virtual_controller_tabs->clear();
    ui->virtual_controller_tabs->insertTab(0, new VirtualControllerTab(), "Virtual controller #1");


}

OpenRGBVisualMapTab::~OpenRGBVisualMapTab()
{
    delete ui;
}
