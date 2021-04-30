#include "BackgroundApplier.h"
#include "ColorStop.h"
#include "ui_BackgroundApplier.h"

#include "math.h"
#include <QImage>
#include <QBrush>
#include <QPainter>
#include <QGradient>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QGradientStops>
#include <QFileDialog>

BackgroundApplier::BackgroundApplier(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BackgroundApplier)
{
    ui->setupUi(this);

    // presets
    ui->presets_comboBox->blockSignals(true);
    ui->presets_comboBox->addItems(presets_names);
    ui->presets_comboBox->blockSignals(false);

    // custom
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->color_stops->setLayout(new QHBoxLayout());
    ui->color_stops->layout()->setSizeConstraint(QLayout::SetFixedSize);
    ui->scrollArea->setWidgetResizable(true);

    ui->gradient_type->blockSignals(true);
    ui->gradient_type->addItems(custom_names);
    ui->gradient_type->blockSignals(false);

    ui->spread_comboBox->blockSignals(true);
    ui->spread_comboBox->addItems(spread_names);
    ui->spread_comboBox->blockSignals(false);

    // files
    ui->images_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->images->setLayout(new QHBoxLayout());
    ui->images->layout()->setSizeConstraint(QLayout::SetFixedSize);
    ui->images_scrollArea->setWidgetResizable(true);
}

BackgroundApplier::~BackgroundApplier()
{
    delete ui;
}

QImage BackgroundApplier::GetImage()
{
    return image;
}

void  BackgroundApplier::SetSize(int w_value ,int h_value)
{
    w = w_value;
    h = h_value;
}

void BackgroundApplier::on_presets_comboBox_currentIndexChanged(int idx)
{
    preset = presets[idx];
    ApplyPreset();
}

void BackgroundApplier::on_gradient_type_currentIndexChanged(int)
{
    ApplyCustom();
}

void BackgroundApplier::on_spread_comboBox_currentIndexChanged(int)
{
    ApplyCustom();
}

void BackgroundApplier::on_rotate_valueChanged(int)
{
    ApplyCustom();
}

void BackgroundApplier::on_choose_image_button_clicked()
{
    OpenFileDialog();
}

void BackgroundApplier::on_add_color_stop_button_clicked()
{
    AddColorStop(new ColorStop());
    ApplyCustom();
}

void BackgroundApplier::AddColorStop(ColorStop* color_stop)
{
    color_stops.push_back(color_stop);

    ui->color_stops->layout()->addWidget(color_stop);
    ui->color_stops->layout()->setAlignment(color_stop, Qt::AlignLeft | Qt::AlignTop);

    connect(color_stop, &ColorStop::GradientStopChanged, [=](){
        ApplyCustom();
    });

    connect(color_stop, &ColorStop::RemoveRequest, [=](){
        ui->color_stops->layout()->removeWidget(color_stop);
        color_stops.erase(std::find(color_stops.begin(), color_stops.end(), color_stop));
    });

    ApplyCustom();
}

void BackgroundApplier::ApplyPreset()
{
    image = QImage(w, h, QImage::Format_RGB32);
    preset = presets[ui->presets_comboBox->currentIndex()];

    QGradient grad(preset);

    QBrush brush(grad);
    QRectF rect(0, 0, w, h);

    QPainter painter(&image);
    painter.fillRect(rect, brush);

    emit BackgroundApplied(image);
}

void BackgroundApplier::ApplyCustom()
{
    image = QImage(w, h, QImage::Format_RGB32);

    QBrush brush;

    QGradientStops stops;

    for(ColorStop* color_stop: color_stops)
    {
        stops << color_stop->GetGradientStop();
    }

    QGradient::Spread spread = spreads[ui->spread_comboBox->currentIndex()];

    switch(ui->gradient_type->currentIndex())
    {
    case 0: brush = ApplyLinearGradient(stops, spread); break;
    case 1: brush = ApplyRadialGradient(stops, spread); break;
    case 2: brush = ApplyConicalGradient(stops, spread); break;
    default: return;
    }

    QRectF rect(0, 0, w, h);

    QPainter painter(&image);
    painter.fillRect(rect, brush);

    emit BackgroundApplied(image);
}

QBrush BackgroundApplier::ApplyLinearGradient(QGradientStops stops, QGradient::Spread spread)
{
    int angle = ui->rotate->value();

    QPointF end_point = EdgeOfView(angle);
    QPointF start_point = EdgeOfView((angle + 180)%360);

    QLinearGradient grad(start_point , end_point);

    grad.setSpread(spread);
    grad.setStops(stops);

    return QBrush(grad);
}

QBrush BackgroundApplier::ApplyRadialGradient(QGradientStops stops, QGradient::Spread spread)
{
    float radius = sqrt(h*h + w*w) / 2;

    QRadialGradient grad(w/2, h/2, radius);

    grad.setSpread(spread);
    grad.setStops(stops);

    return QBrush(grad);
}

QBrush BackgroundApplier::ApplyConicalGradient(QGradientStops stops, QGradient::Spread spread)
{
    int angle = ui->rotate->value();

    QConicalGradient grad(w/2,h/2, angle);

    grad.setSpread(spread);
    grad.setStops(stops);

    return QBrush(grad);
}

QPointF BackgroundApplier::EdgeOfView(int deg) {

    float PI = 3.14159265359;
    float twoPI = PI*2;
    float theta = deg * PI / 180;

    while (theta < -PI) {
        theta += twoPI;
    }

    while (theta > PI) {
        theta -= twoPI;
    }

    float rectAtan = atan2(h, w);
    float tanTheta = tan(theta);

    int region;

    if ((theta > -rectAtan) && (theta <= rectAtan)) {
        region = 1;
    } else if ((theta > rectAtan) && (theta <= (PI - rectAtan))) {
        region = 2;
    } else if ((theta > (PI - rectAtan)) || (theta <= -(PI - rectAtan))) {
        region = 3;
    } else {
        region = 4;
    }

    QPointF edgePoint(w/2, h/2);

    float xFactor = 1.01;
    float yFactor = 1.01;

    switch (region) {
    case 1: yFactor = -1.01; break;
    case 2: yFactor = -1.01; break;
    case 3: xFactor = -1.01; break;
    case 4: xFactor = -1.01; break;
    }

    if ((region == 1) || (region == 3)) {
        edgePoint.setX(edgePoint.x() + xFactor * (w / 2.0f));
        edgePoint.setY(edgePoint.y() + yFactor * (w / 2.0f) * tanTheta);
    } else {
        edgePoint.setX(edgePoint.x() + xFactor * (h / (2. * tanTheta)));
        edgePoint.setY(edgePoint.y() + yFactor * (h /  2.0f));
    }

    return edgePoint;
};


void BackgroundApplier::OpenFileDialog()
{
    // Load and trigger the preview
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));

    QImage user_image;
    user_image.load(fileName);
    emit BackgroundApplied(user_image.scaled(w, h, Qt::IgnoreAspectRatio));

    // Add this image to already applied list
    QPixmap pm(fileName);
    QPushButton* button = new QPushButton();
    QPixmap pixmap = pm.scaled(64,64,Qt::KeepAspectRatio);
    QIcon icon(pixmap);

    button->setFixedWidth(64);
    button->setFixedHeight(64);
    button->setIcon(icon);
    button->show();
    button->setCursor(Qt::PointingHandCursor);
    button->setFlat(true);
    button->setIconSize(pixmap.rect().size());

    ui->images->layout()->addWidget(button);
    ui->images->layout()->setAlignment(button, Qt::AlignLeft | Qt::AlignTop);

    connect(button, &QPushButton::clicked, [=](){
        emit BackgroundApplied(user_image.scaled(w, h, Qt::IgnoreAspectRatio));
    });
}












