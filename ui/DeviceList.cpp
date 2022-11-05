#include "OpenRGBVisualMapPlugin.h"
#include "DeviceList.h"
#include "ui_DeviceList.h"
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>

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
    /*-------------------------------------------------*\
    | Group by RGBController                            |
    \*-------------------------------------------------*/
    std::map<RGBController*,std::vector<ControllerZone*>> groups;

    for(ControllerZone* controller_zone: controller_zones)
    {
        if (groups.find(controller_zone->controller) == groups.end())
        {
            std::vector<ControllerZone*> zones;
            groups[controller_zone->controller] = zones;
        }

        groups[controller_zone->controller].push_back(controller_zone);
    }

    /*-------------------------------------------------*\
    | Iterate groups, add widgets                       |
    \*-------------------------------------------------*/

    std::vector<RGBController*> controllers = OpenRGBVisualMapPlugin::RMPointer->GetRGBControllers();

    for (RGBController* controller: controllers)
    {
        if(controller->serial == VirtualController::VIRTUAL_CONTROLLER_SERIAL)
        {
            continue;
        }

        std::vector<ControllerZone*> zones = groups[controller];

        QLayout* _layout;
        bool in_group = zones.size() > 1;

        if(!in_group)
        {
            _layout = layout();
        }
        else
        {
            QFrame* group_frame = new QFrame(this);
            group_frame->setFrameShape(QFrame::StyledPanel);
            group_frame->setFrameShadow(QFrame::Sunken);
            group_frame->setLayout(new QVBoxLayout(group_frame));
            group_frame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

            QLabel* group_name_label = new QLabel(QString::fromStdString(controller->name));
            group_name_label->setWordWrap(true);
            group_name_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

            group_frame->layout()->addWidget(group_name_label);
            layout()->addWidget(group_frame);

            _layout = group_frame->layout();
        }

        for(ControllerZone* controller_zone: zones)
        {
            DeviceWidget* widget = new DeviceWidget(this, controller_zone, in_group);

            device_widgets.push_back(widget);

            _layout->addWidget(widget);

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

    ((QVBoxLayout*) layout())->addStretch();
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
