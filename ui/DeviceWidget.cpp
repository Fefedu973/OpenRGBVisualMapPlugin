#include "DeviceWidget.h"
#include "ui_DeviceWidget.h"
#include <QInputDialog>
#include "OpenRGBPluginsFont.h"

DeviceWidget::DeviceWidget(QWidget *parent, ControllerZone* controller_zone, bool in_group) :
    QWidget(parent),
    ui(new Ui::DeviceWidget),
    controller_zone(controller_zone),
    in_group(in_group)
{
    ui->setupUi(this);

    ui->enable->setFont(OpenRGBPluginsFont::GetFont());
    ui->rename->setFont(OpenRGBPluginsFont::GetFont());
    ui->select->setFont(OpenRGBPluginsFont::GetFont());

    ui->rename->setText(OpenRGBPluginsFont::icon(OpenRGBPluginsFont::rename));

    ui->enable->setText(OpenRGBPluginsFont::icon(OpenRGBPluginsFont::math_plus));

    updateName();

    UpdateCheckState();

    if(in_group)
    {
        ui->frame->setFrameShape(QFrame::NoFrame);
        ui->frame->layout()->setContentsMargins(0, 0, 0, 0);
    }
}

DeviceWidget::~DeviceWidget()
{
    delete ui;
}

void DeviceWidget::on_enable_toggled(bool state)
{
    ui->enable->setText(
                state ?
                    OpenRGBPluginsFont::icon(OpenRGBPluginsFont::math_minus):
                    OpenRGBPluginsFont::icon(OpenRGBPluginsFont::math_plus)
                    );

    emit Enabled(state);
}

void DeviceWidget::on_name_clicked()
{
    ui->select->toggle();
}

void DeviceWidget::on_select_toggled(bool state)
{
    UpdateCheckState();
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
    ui->enable->setText(
                state ?
                    OpenRGBPluginsFont::icon(OpenRGBPluginsFont::math_minus):
                    OpenRGBPluginsFont::icon(OpenRGBPluginsFont::math_plus)
                    );

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

    UpdateCheckState();
}

bool DeviceWidget::isSelected()
{
    return ui->select->isChecked();
}

void DeviceWidget::UpdateCheckState()
{
    ui->select->setText(
                ui->select->isChecked()?
                    OpenRGBPluginsFont::icon(OpenRGBPluginsFont::check_o):
                    OpenRGBPluginsFont::icon(OpenRGBPluginsFont::check)
                    );
}
