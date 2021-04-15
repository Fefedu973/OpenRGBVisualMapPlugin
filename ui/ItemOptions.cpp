#include "ItemOptions.h"
#include "ZoneManager.h"
#include "ui_ItemOptions.h"

ItemOptions::ItemOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemOptions)
{
    ui->setupUi(this);

    ui->shape_comboBox->addItems(ControllerZoneSettings::ZONE_SHAPES);
}

ItemOptions::~ItemOptions()
{
    delete ui;
}

void ItemOptions::SetControllerZone(int zone_idx)
{
    zone = ZoneManager::Get()->GetZone(zone_idx);
    Update();
}

void ItemOptions::Update()
{
    if(zone)
    {
        ui->x_spinBox->setValue(zone->settings.x);
        ui->y_spinBox->setValue(zone->settings.y);
        ui->led_spacing_spinBox->setValue(zone->settings.led_spacing);
        ui->shape_comboBox->setCurrentIndex(zone->settings.shape);
        ui->reverse_checkBox->setChecked(zone->settings.reverse);
    }
}

void ItemOptions::on_x_spinBox_valueChanged(int x)
{
    if(zone)
    {
        zone->settings.x = x;
        emit ItemOptionsChanged();
    }
}

void ItemOptions::on_y_spinBox_valueChanged(int y)
{
    if(zone)
    {
        zone->settings.y = y;
        emit ItemOptionsChanged();
    }
}

void ItemOptions::on_led_spacing_spinBox_valueChanged(int led_spacing)
{
    if(zone)
    {
        zone->settings.led_spacing = led_spacing;
        emit ItemOptionsChanged();
    }
}

void ItemOptions::on_shape_comboBox_currentIndexChanged(int i)
{
    if(zone)
    {
        zone->settings.shape = static_cast<ControllerZoneSettings::ZoneShape>(i);
        emit ItemOptionsChanged();
    }
}
void ItemOptions::on_reverse_checkBox_stateChanged(int state)
{
    if(zone)
    {
        zone->settings.reverse = state;
        emit ItemOptionsChanged();
    }
}
