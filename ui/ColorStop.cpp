#include "ColorStop.h"
#include "ColorPicker.h"

#include "ui_ColorStop.h"

ColorStop::ColorStop(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorStop)
{
    ui->setupUi(this);

    //default values
    stop.second = Qt::white;

    // remove button style
    ui->remove_button->setStyleSheet("image:url(:close.png);");

    connect(ui->color_picker, &ColorPicker::ColorSelected, [=](QColor color){
        stop.second = color;
        emit GradientStopChanged(GetGradientStop());
    });

}

QGradientStop ColorStop::GetGradientStop()
{
    stop.first = ui->stop->value() / 100.f;
    return stop;
}

void ColorStop::on_stop_valueChanged(int)
{
    emit GradientStopChanged(GetGradientStop());
}

void ColorStop::on_remove_button_clicked()
{
    emit RemoveRequest();
}

ColorStop::~ColorStop()
{
    delete ui;
}
