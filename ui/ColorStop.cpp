#include "ColorStop.h"
#include "ColorPicker.h"

#include "ui_ColorStop.h"

ColorStop::ColorStop(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorStop)
{
    ui->setupUi(this);

    //default values
    stop.first = 0;
    stop.second = QColor("#000000");

    // remove button style
    ui->remove_button->setStyleSheet("image:url(:images/close.png);");

    connect(ui->color_picker, &ColorPicker::ColorSelected, [=](QColor color){
        stop.second = color;
        emit GradientStopChanged();
    });
}

void ColorStop::SetGradientStop(const QGradientStop& gradient_stop)
{
    stop.first = gradient_stop.first;
    stop.second = gradient_stop.second;
    ui->stop->setValue(stop.first * 100);
    ui->color_picker->SetColor(stop.second);
}

QGradientStop ColorStop::GetGradientStop()
{    
    return stop;
}

void ColorStop::on_stop_valueChanged(int)
{
    stop.first = ui->stop->value() / 100.f;
    emit GradientStopChanged();
}

void ColorStop::on_remove_button_clicked()
{
    emit RemoveRequest();
}

ColorStop::~ColorStop()
{
    delete ui;
}
