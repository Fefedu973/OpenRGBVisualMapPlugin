#include "GridOptions.h"
#include "ui_GridOptions.h"

GridOptions::GridOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GridOptions)
{
    ui->setupUi(this);

    Update();
}

GridOptions::~GridOptions()
{
    delete ui;
}

void GridOptions::on_w_spinBox_valueChanged(int value)
{
    settings.w = value;
    emit OptionsChanged(settings);
}

void GridOptions::on_h_spinBox_valueChanged(int value)
{
    settings.h = value;
    emit OptionsChanged(settings);
}

void GridOptions::on_grid_checkBox_stateChanged(int value)
{
    settings.show_grid = value;
    emit OptionsChanged(settings);
}

void GridOptions::on_bounds_checkBox_stateChanged(int value)
{
    settings.show_bounds = value;
    emit OptionsChanged(settings);
}

void GridOptions::Update()
{
    ui->bounds_checkBox->setChecked(settings.show_bounds);
    ui->grid_checkBox->setChecked(settings.show_grid);
    ui->w_spinBox->setValue(settings.w);
    ui->h_spinBox->setValue(settings.h);
}

GridSettings GridOptions::GetSettings()
{
    return settings;
}

void GridOptions::SetSettings(GridSettings value)
{
    settings = value;
    Update();
    emit OptionsChanged(settings);
}
