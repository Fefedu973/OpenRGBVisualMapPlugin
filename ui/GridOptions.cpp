#include "GridOptions.h"
#include "ui_GridOptions.h"

GridOptions::GridOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GridOptions)
{
    ui->setupUi(this);
}

GridOptions::~GridOptions()
{
    delete ui;
}

void GridOptions::Init(GridSettings* settings)
{
    SetSettings(settings);
}

void GridOptions::on_w_spinBox_valueChanged(int value)
{
    settings->w = value;
    emit SettingsChanged();
}

void GridOptions::on_h_spinBox_valueChanged(int value)
{
    settings->h = value;
    emit SettingsChanged();
}

void GridOptions::on_grid_checkBox_stateChanged(int value)
{
    settings->show_grid = value;
    emit SettingsChanged();
}

void GridOptions::on_bounds_checkBox_stateChanged(int value)
{
    settings->show_bounds = value;
    emit SettingsChanged();
}

void GridOptions::on_auto_load_checkBox_stateChanged(int value)
{
    settings->auto_load = value;
    emit SettingsChanged();
}

void GridOptions::on_auto_register_checkBox_stateChanged(int value)
{
    settings->auto_register = value;
    emit SettingsChanged();
}

void GridOptions::on_unregister_members_checkBox_stateChanged(int value)
{
    settings->unregister_members = value;
    emit SettingsChanged();
}

void GridOptions::on_auto_resize_clicked()
{
    emit AutoResizeRequest();
}

void GridOptions::Update()
{
    ui->bounds_checkBox->setChecked(settings->show_bounds);
    ui->grid_checkBox->setChecked(settings->show_grid);
    ui->auto_load_checkBox->setChecked(settings->auto_load);
    ui->auto_register_checkBox->setChecked(settings->auto_register);
    ui->unregister_members_checkBox->setChecked(settings->unregister_members);
    ui->w_spinBox->setValue(settings->w);
    ui->h_spinBox->setValue(settings->h);
}

void GridOptions::SetSettings(GridSettings* settings)
{
    this->settings = settings;
    Update();
}
