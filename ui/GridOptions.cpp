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

void GridOptions::on_live_preview_checkBox_stateChanged(int value)
{
    settings->live_preview = value;
    emit SettingsChanged();
}

void GridOptions::Update()
{
    ui->bounds_checkBox->setChecked(settings->show_bounds);
    ui->grid_checkBox->setChecked(settings->show_grid);
    ui->live_preview_checkBox->setChecked(settings->live_preview);
    ui->w_spinBox->setValue(settings->w);
    ui->h_spinBox->setValue(settings->h);
}

void GridOptions::SetSettings(GridSettings* settings)
{
    this->settings = settings;
    Update();
}
