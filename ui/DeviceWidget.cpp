#include "DeviceWidget.h"
#include "ui_DeviceWidget.h"
#include <QInputDialog>

DeviceWidget::DeviceWidget(QWidget *parent, ControllerZone* controller_zone, bool in_group) :
    QWidget(parent),
    ui(new Ui::DeviceWidget),
    controller_zone(controller_zone),
    in_group(in_group)
{
    ui->setupUi(this);

    updateName();

    if(in_group)
    {
        ui->frame->setFrameShape(QFrame::NoFrame);
        ui->frame->layout()->setMargin(0);
    }
}

DeviceWidget::~DeviceWidget()
{
    delete ui;
}

void DeviceWidget::on_enable_toggled(bool state)
{
    ui->enable->setText(state?"-":"+");
    emit Enabled(state);
}

void DeviceWidget::on_name_clicked()
{
    ui->select->toggle();
}

void DeviceWidget::on_select_toggled(bool state)
{
    emit Selected(state);
}

void DeviceWidget::updateName()
{    
    ui->name->setText(
                in_group?"• " + QString::fromStdString(controller_zone->zone_display_name()) :
                QString::fromStdString(controller_zone->controller_display_name()));
}

ControllerZone* DeviceWidget::getControllerZone()
{
    return controller_zone;
}

void DeviceWidget::on_rename_clicked()
{
    QString new_name = QInputDialog::getText(
                nullptr, "Rename zone", "Set the new name",
                QLineEdit::Normal, ui->name->text()).trimmed();

    if(!new_name.isEmpty())
    {
        controller_zone->custom_zone_name = new_name.toStdString();
        updateName();
        emit Renamed(new_name);
    }
}

void DeviceWidget::setEnabled(bool state)
{    
    ui->enable->blockSignals(true);
    ui->enable->setChecked(state);
    ui->enable->setText(state?"-":"+");
    ui->enable->blockSignals(false);
}

bool DeviceWidget::isEnabled()
{
    return ui->enable->isChecked();
}

void DeviceWidget::setSelected(bool state)
{
    ui->select->blockSignals(true);
    ui->select->setChecked(state);
    ui->select->blockSignals(false);
}

bool DeviceWidget::isSelected()
{
    return ui->select->isChecked();
}
