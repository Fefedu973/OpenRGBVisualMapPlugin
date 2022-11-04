#include "DeviceList.h"
#include "ui_DeviceList.h"
#include <QVBoxLayout>

DeviceList::DeviceList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceList)
{
    ui->setupUi(this);
    setLayout(new QVBoxLayout(this));
}


DeviceList::~DeviceList()
{
    delete ui;
}

void DeviceList::Clear()
{
    device_widgets.clear();

    QLayoutItem *child;

    while ((child = layout()->takeAt(0)) != 0)
    {
        delete child->widget();
    }
}

void DeviceList::Init(std::vector<ControllerZone*> controller_zones)
{
    for(ControllerZone* controller_zone: controller_zones)
    {
        DeviceWidget* widget = new DeviceWidget(this, controller_zone);

        device_widgets.push_back(widget);

        layout()->addWidget(widget);

        connect(widget, &DeviceWidget::Enabled, [=](bool state){
            if(state)
            {
                emit DeviceAdded(controller_zone);
            }
            else
            {
                emit DeviceRemoved(controller_zone);
            }
        });

        connect(widget, &DeviceWidget::Selected, [=](bool){
            std::vector<ControllerZone*> selection;

            for(DeviceWidget* widget: device_widgets)
            {
                if(widget->isSelected())
                {
                    selection.push_back(widget->getControllerZone());
                }
            }

            emit SelectionChanged(selection);
        });
    }
}

void DeviceList::SetSelection(std::vector<ControllerZone*> controller_zones)
{
    for(DeviceWidget* widget: device_widgets)
    {
        widget->setSelected(false);

        for(ControllerZone* controller_zone: controller_zones)
        {
            if(widget->getControllerZone() == controller_zone)
            {
                widget->setSelected(true);
                break;
            }
        }
    }
}

void DeviceList::UpdateControllerState(ControllerZone* controller_zone)
{
    for(DeviceWidget* widget: device_widgets)
    {
        if(widget->getControllerZone() == controller_zone)
        {
            widget->setEnabled(true);
            widget->updateName();
            break;
        }
    }
}
