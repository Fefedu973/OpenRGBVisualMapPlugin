#include "ItemOptions.h"
#include "WidgetEditor.h"
#include "ui_ItemOptions.h"
#include "ZoneManager.h"

ItemOptions::ItemOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemOptions)
{
    ui->setupUi(this);
    ui->shape_comboBox->addItems(ZONE_SHAPES);
}

ItemOptions::~ItemOptions()
{
    delete ui;
}

void ItemOptions::SetControllerZone(int zone_idx)
{
    ctrl_zone = ZoneManager::Get()->GetZone(zone_idx);
    Update();
}

void ItemOptions::Update()
{
    if(ctrl_zone)
    {
        ui->x_spinBox->setValue(ctrl_zone->settings.x);
        ui->y_spinBox->setValue(ctrl_zone->settings.y);
        ui->led_spacing_spinBox->setValue(ctrl_zone->settings.led_spacing);
        ui->shape_comboBox->setCurrentIndex(ctrl_zone->settings.shape);
        ui->reverse_checkBox->setChecked(ctrl_zone->settings.reverse);
        ui->edit_shape_button->setVisible(ctrl_zone->isCustomShape());
    }
}

void ItemOptions::on_x_spinBox_valueChanged(int x)
{
    if(ctrl_zone)
    {
        ctrl_zone->settings.x = x;
        emit ItemOptionsChanged();
    }
}

void ItemOptions::on_y_spinBox_valueChanged(int y)
{
    if(ctrl_zone)
    {
        ctrl_zone->settings.y = y;
        emit ItemOptionsChanged();
    }
}

void ItemOptions::on_led_spacing_spinBox_valueChanged(int led_spacing)
{
    if(ctrl_zone)
    {
        ctrl_zone->settings.led_spacing = led_spacing;
        emit ItemOptionsChanged();
    }
}

void ItemOptions::on_shape_comboBox_currentIndexChanged(int i)
{
    if(ctrl_zone)
    {
        ctrl_zone->settings.shape = static_cast<ZoneShape>(i);
        ui->edit_shape_button->setVisible(ctrl_zone->isCustomShape());

        // needs custon shape init
        if(ctrl_zone->isCustomShape() && !ctrl_zone->settings.custom_shape)
        {
            ctrl_zone->settings.custom_shape = new CustomShape();
            ctrl_zone->settings.custom_shape->w = ctrl_zone->led_count();
            ctrl_zone->settings.custom_shape->h = ctrl_zone->led_count();

            // really needed ?
            ctrl_zone->settings.custom_shape->led_positions = std::vector<QPoint*>();
            // ---------------

            for(int i = 0; i < ctrl_zone->led_count(); i++)
            {
                ctrl_zone->settings.custom_shape->led_positions.push_back(new QPoint(i, 0));
            }
        }

        emit ItemOptionsChanged();
    }
}
void ItemOptions::on_reverse_checkBox_stateChanged(int state)
{
    if(ctrl_zone)
    {
        ctrl_zone->settings.reverse = state;
        emit ItemOptionsChanged();
    }
}

void ItemOptions::on_identifyButton_clicked()
{
    if(ctrl_zone)
    {
        ZoneManager::Get()->IdentifyZone(ctrl_zone);
    }
}


void ItemOptions::on_edit_shape_button_clicked()
{
    if(ctrl_zone)
    {
        int result = WidgetEditor::Show(ctrl_zone);
        printf("Result = %d\n", result);
    }
}

