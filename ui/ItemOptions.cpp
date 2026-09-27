#include "ItemOptions.h"
#include "ui_ItemOptions.h"
#include "ZoneManager.h"
#include <QLabel>
#include <QSignalBlocker>

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
    const QString labels[]={tr("Scale X factor"),tr("Scale Y factor"),tr("Rotation (degrees)"),tr("Brightness (%)")};
    int row=ui->gridLayout->rowCount();
    for(unsigned i=0;i<4;++i) {
        auto* spin=affine_values[i]=new QDoubleSpinBox(this);
        spin->setObjectName(QString("affine_%1").arg(i));spin->setDecimals(6);
        spin->setRange(i==2?-360000.0:i==3?0.0:0.000001,i==3?100.0:360000.0);
        spin->setSingleStep(i<2?0.1:1.0);
        ui->gridLayout->addWidget(new QLabel(labels[i],this),row,0);ui->gridLayout->addWidget(spin,row++,1);
        connect(spin,qOverload<double>(&QDoubleSpinBox::valueChanged),this,[this,i](double value){
            if(!ctrl_zone)return;
            auto& s=ctrl_zone->settings;
            if(i==0)s.scale_x=value;else if(i==1)s.scale_y=value;else if(i==2)s.rotation=value;else s.brightness=value/100.0;
            emit ItemOptionsChanged();
        });
    }
    for(unsigned i=0;i<2;++i) {
        auto* check=affine_flips[i]=new QCheckBox(i?tr("Mirror vertically"):tr("Mirror horizontally"),this);
        ui->gridLayout->addWidget(check,row++,0,1,2);
        connect(check,&QCheckBox::toggled,this,[this,i](bool value){
            if(!ctrl_zone)return;
            if(i)ctrl_zone->settings.flip_y=value;else ctrl_zone->settings.flip_x=value;
            emit ItemOptionsChanged();
        });
    }
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
        const auto& s=ctrl_zone->settings;
        const double values[]={s.scale_x,s.scale_y,s.rotation,s.brightness*100.0};
        for(unsigned i=0;i<4;++i){QSignalBlocker block(affine_values[i]);affine_values[i]->setValue(values[i]);}
        for(unsigned i=0;i<2;++i){QSignalBlocker block(affine_flips[i]);affine_flips[i]->setChecked(i?s.flip_y:s.flip_x);}
        ui->x_spinBox->blockSignals(true);
        ui->y_spinBox->blockSignals(true);
        ui->scale_spinBox->blockSignals(true);
        ui->led_spacing_spinBox->blockSignals(true);
        ui->shape_comboBox->blockSignals(true);
        ui->reverse_checkBox->blockSignals(true);

        ui->x_spinBox->setValue(ctrl_zone->settings.x);
        ui->y_spinBox->setValue(ctrl_zone->settings.y);
        ui->scale_spinBox->setValue(ctrl_zone->settings.scale);
        ui->led_spacing_spinBox->setValue(ctrl_zone->settings.led_spacing);
        ui->shape_comboBox->setCurrentIndex(ctrl_zone->settings.shape);
        ui->reverse_checkBox->setChecked(ctrl_zone->settings.reverse);

        ui->x_spinBox->blockSignals(false);
        ui->y_spinBox->blockSignals(false);
        ui->scale_spinBox->blockSignals(false);
        ui->led_spacing_spinBox->blockSignals(false);
        ui->shape_comboBox->blockSignals(false);
        ui->reverse_checkBox->blockSignals(false);

        UpdateWidgetsVisibility();
    }
}

void ItemOptions::on_x_spinBox_valueChanged(double x)
{
    if(ctrl_zone)
    {
        ctrl_zone->settings.x = x;
        emit ItemOptionsChanged();
    }
}

void ItemOptions::on_y_spinBox_valueChanged(double y)
{
    if(ctrl_zone)
    {
        ctrl_zone->settings.y = y;
        emit ItemOptionsChanged();
    }
}

void ItemOptions::on_scale_spinBox_valueChanged(double scale)
{
    if(ctrl_zone)
    {
        ctrl_zone->settings.scale = scale;
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
