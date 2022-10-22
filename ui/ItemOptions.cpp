#include "ItemOptions.h"
#include "ui_ItemOptions.h"
#include "ZoneManager.h"

ItemOptions::ItemOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemOptions)
{
    ui->setupUi(this);

    QStringList ZONE_SHAPES = {
        "Horizontal",
        "Vertical",
        "Custom"
    };

    ui->shape_comboBox->addItems(ZONE_SHAPES);
}

ItemOptions::~ItemOptions()
{
    delete ui;
}

void ItemOptions::SetControllerZone(ControllerZone* ctrl_zone)
{
    this->ctrl_zone = ctrl_zone;
    Update();
}

void ItemOptions::Update()
{
    if(ctrl_zone)
    {        
        ui->x_spinBox->blockSignals(true);
        ui->y_spinBox->blockSignals(true);
        ui->led_spacing_spinBox->blockSignals(true);
        ui->shape_comboBox->blockSignals(true);
        ui->reverse_checkBox->blockSignals(true);

        ui->x_spinBox->setValue(ctrl_zone->settings.x);
        ui->y_spinBox->setValue(ctrl_zone->settings.y);
        ui->led_spacing_spinBox->setValue(ctrl_zone->settings.led_spacing);
        ui->shape_comboBox->setCurrentIndex(ctrl_zone->settings.shape);
        ui->reverse_checkBox->setChecked(ctrl_zone->settings.reverse);

        ui->x_spinBox->blockSignals(false);
        ui->y_spinBox->blockSignals(false);
        ui->led_spacing_spinBox->blockSignals(false);
        ui->shape_comboBox->blockSignals(false);
        ui->reverse_checkBox->blockSignals(false);

        UpdateWidgetsVisibility();
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

        UpdateWidgetsVisibility();

        // needs custon shape init
        if(ctrl_zone->isCustomShape() && !ctrl_zone->settings.custom_shape)
        {
            unsigned int leds_count = ctrl_zone->led_count();

            ctrl_zone->settings.custom_shape = new CustomShape();
            ctrl_zone->settings.custom_shape->w = leds_count;
            ctrl_zone->settings.custom_shape->h = 1;            

            for(unsigned int i = 0; i < leds_count; i++)
            {
                LedPosition* led_position = new LedPosition();                
                led_position->led_num = i;
                led_position->setX(i);
                led_position->setY(0);

                ctrl_zone->settings.custom_shape->led_positions.push_back(led_position);
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
    emit ShapeEditRequest(ctrl_zone);
}

void ItemOptions::UpdateWidgetsVisibility()
{
    ui->edit_shape_button->setVisible(ctrl_zone->isCustomShape());
    ui->reverse_checkBox->setVisible(!ctrl_zone->isCustomShape());
    ui->led_spacing_spinBox->setVisible(!ctrl_zone->isCustomShape());
    ui->led_spacing_label->setVisible(!ctrl_zone->isCustomShape());
}
