#include <QImage>
#include <QPainter>
#include <QGradient>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QGradientStops>
#include <QFileDialog>
#include <QInputDialog>

#include <nlohmann/json.hpp>

#include "BackgroundApplier.h"
#include "ColorStop.h"
#include "ui_BackgroundApplier.h"
#include "math.h"
#include "VisualMapSettingsManager.h"
#include "GradientPresets.h"

using json = nlohmann::json;

BackgroundApplier::BackgroundApplier(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BackgroundApplier)
{
    ui->setupUi(this);

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

    // presets
    ui->presets_comboBox->blockSignals(true);

    for(GradientPreset presest:GRADIENT_PRESETS)
    {
        ui->presets_comboBox->addItem(QString::fromStdString(presest.name));
    }

    ui->presets_comboBox->blockSignals(false);
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

void BackgroundApplier::on_gradient_type_currentIndexChanged(int)
{
    ApplyCustom();
}

void BackgroundApplier::on_spread_comboBox_currentIndexChanged(int)
{
    ApplyCustom();
}


void BackgroundApplier::on_x_offset_valueChanged(int)
{
    ApplyCustom();
}

void BackgroundApplier::on_y_offset_valueChanged(int)
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
    ColorStop* color_stop = new ColorStop(this);

    QGradientStop stop;

    stop.first = 1.f;
    stop.second = QColor::fromHsv(rand()%360 ,255,255);

    color_stop->SetGradientStop(stop);

    AddColorStop(color_stop);
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
        ApplyCustom();
    });
}

void BackgroundApplier::ApplyCustom()
{
    image = QImage(w, h, QImage::Format_RGB32);

    if(color_stops.empty())
    {
        image.fill(Qt::black);
        emit BackgroundUpdated(image);
        return;
    }

    QBrush brush;

    QGradientStops stops;

    for(ColorStop* color_stop: color_stops)
    {
        QGradientStop stop = color_stop->GetGradientStop();
        float brightness = ui->brightness->value() / 100.f;
        stop.second.setRgb(stop.second.red() * brightness, stop.second.green() * brightness, stop.second.blue() * brightness);

        stops << stop;
    }

    QGradient::Spread spread = spreads[ui->spread_comboBox->currentIndex()];

    int angle = ui->rotate->value();
    int x_offset = ui->x_offset->value();
    int y_offset = ui->y_offset->value();

    switch(ui->gradient_type->currentIndex())
    {
    case 0: brush = ApplyLinearGradient(stops, spread, angle, x_offset, y_offset); break;
    case 1: brush = ApplyRadialGradient(stops, spread, x_offset, y_offset); break;
    case 2: brush = ApplyConicalGradient(stops, spread, angle, x_offset, y_offset); break;
    default: return;
    }

    QRectF rect(0, 0, w, h);

    QPainter painter(&image);

    painter.fillRect(rect, brush);

    emit BackgroundUpdated(image);
}

QBrush BackgroundApplier::ApplyLinearGradient(QGradientStops stops, QGradient::Spread spread, int angle, int x_offset, int y_offset)
{
    QPointF end_point = EdgeOfView(angle, x_offset, y_offset);
    QPointF start_point = EdgeOfView((angle + 180)%360, x_offset, y_offset);

    QLinearGradient grad(start_point , end_point);

    grad.setSpread(spread);
    grad.setStops(stops);

    return QBrush(grad);
}

QBrush BackgroundApplier::ApplyRadialGradient(QGradientStops stops, QGradient::Spread spread, int x_offset, int y_offset)
{
    float radius = sqrt(h*h + w*w) / 2;

    QRadialGradient grad(w * x_offset / 100.f ,h * y_offset / 100.f, radius);

    grad.setSpread(spread);
    grad.setStops(stops);

    return QBrush(grad);
}

QBrush BackgroundApplier::ApplyConicalGradient(QGradientStops stops, QGradient::Spread spread, int angle, int x_offset, int y_offset)
{
    QConicalGradient grad(w * x_offset / 100.f ,h * y_offset / 100.f, angle);

    grad.setSpread(spread);
    grad.setStops(stops);

    return QBrush(grad);
}

QPointF BackgroundApplier::EdgeOfView(int deg, int x_offset, int y_offset) {

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

    edgePoint.setX(edgePoint.x() * x_offset / 100.f);
    edgePoint.setY(edgePoint.y( )* y_offset / 100.f);

    return edgePoint;
};


void BackgroundApplier::OpenFileDialog()
{
    // Load and trigger the preview
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));

    QImage user_image;
    user_image.load(fileName);
    emit BackgroundUpdated(user_image.scaled(w, h, Qt::IgnoreAspectRatio));

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
        emit BackgroundUpdated(user_image.scaled(w, h, Qt::IgnoreAspectRatio));
    });
}

void BackgroundApplier::on_presets_comboBox_currentIndexChanged(int index)
{
    GradientPreset preset = GRADIENT_PRESETS[index];

    ui->rotate->blockSignals(true);
    ui->x_offset->blockSignals(true);
    ui->y_offset->blockSignals(true);
    ui->spread_comboBox->blockSignals(true);
    ui->gradient_type->blockSignals(true);

    ui->rotate->setValue(preset.angle);
    ui->x_offset->setValue(preset.x_offset);
    ui->y_offset->setValue(preset.y_offset);

    auto it_spread = std::find(spreads.begin(), spreads.end(), preset.spread);

    if (it_spread != spreads.end())
    {
        auto idx = std::distance(spreads.begin(), it_spread);
        ui->spread_comboBox->setCurrentIndex(idx);
    }

    auto it_type = std::find(types.begin(), types.end(), preset.type);

    if (it_type != types.end())
    {
        auto idx = std::distance(types.begin(), it_type);
        ui->gradient_type->setCurrentIndex(idx);
    }

    QLayoutItem *child;

    while ((child = ui->color_stops->layout()->takeAt(0)) != 0) {
        delete child->widget();
    }

    color_stops.clear();

    for(QGradientStop stop: preset.stops)
    {
        ColorStop* color_stop = new ColorStop(this);
        color_stop->SetGradientStop(stop);
        AddColorStop(color_stop);
    }

    ui->rotate->blockSignals(false);
    ui->x_offset->blockSignals(false);
    ui->y_offset->blockSignals(false);
    ui->spread_comboBox->blockSignals(false);
    ui->gradient_type->blockSignals(false);

    ApplyCustom();
}

void BackgroundApplier::on_brightness_valueChanged(int)
{
    ApplyCustom();
}

void BackgroundApplier::on_save_gradient_clicked()
{
    QString filename = QInputDialog::getText(
                nullptr, "Save gradient to file...", "Choose a filename",
                QLineEdit::Normal, QString("my-gradient")).trimmed();

    json j;

    j["type"] = ui->gradient_type->currentText().toStdString();
    j["spread"] = ui->spread_comboBox->currentText().toStdString();
    j["x_offset"] = ui->x_offset->value();
    j["y_offset"] = ui->y_offset->value();
    j["rotate"] = ui->rotate->value();

    std::vector<json> colors_array;

    for(ColorStop* color_stop: color_stops)
    {
        json c;
        QGradientStop stop = color_stop->GetGradientStop();
        c["position"] = stop.first;
        c["color"] = stop.second.name().toStdString();
        colors_array.push_back(c);
    }

    j["colors"] = colors_array;

    VisualMapSettingsManager::SaveGradient(filename.toStdString(), j);
}

void BackgroundApplier::on_load_gradient_clicked()
{
    std::vector<std::string> filenames = VisualMapSettingsManager::GetGradientsNames();

    QStringList file_list;

    for(std::string filename : filenames)
    {
        file_list << QString::fromUtf8(filename.c_str());
    }

    if(file_list.empty())
    {
        return;
    }

    QInputDialog *inp = new QInputDialog(this);

    inp->setOptions(QInputDialog::UseListViewForComboBoxItems);
    inp->setComboBoxItems(file_list);
    inp->setWindowTitle("Load gradient from file...");
    inp->setLabelText("Choose a gradient file from this list:");

    if(!inp->exec()){
        return;
    }

    QString filename = inp->textValue();

    json j = VisualMapSettingsManager::LoadGradient(filename.toStdString());


    ui->rotate->blockSignals(true);
    ui->x_offset->blockSignals(true);
    ui->y_offset->blockSignals(true);
    ui->spread_comboBox->blockSignals(true);
    ui->gradient_type->blockSignals(true);

    ui->gradient_type->setCurrentText(QString::fromStdString(j["type"]));
    ui->spread_comboBox->setCurrentText(QString::fromStdString(j["spread"]));
    ui->x_offset->setValue(j["x_offset"]);
    ui->y_offset->setValue(j["y_offset"]);
    ui->rotate->setValue(j["rotate"]);

    QLayoutItem *child;

    while ((child = ui->color_stops->layout()->takeAt(0)) != 0) {
        delete child->widget();
    }

    color_stops.clear();

    for(json c: j["colors"])
    {
        ColorStop* color_stop = new ColorStop(this);
        color_stop->SetGradientStop(QGradientStop(c["position"], QColor(QString::fromStdString(c["color"]))));
        AddColorStop(color_stop);
    }

    ui->rotate->blockSignals(false);
    ui->x_offset->blockSignals(false);
    ui->y_offset->blockSignals(false);
    ui->spread_comboBox->blockSignals(false);
    ui->gradient_type->blockSignals(false);

    ApplyCustom();
}
