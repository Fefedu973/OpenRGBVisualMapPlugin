#ifndef BACKGROUNDAPPLIER_H
#define BACKGROUNDAPPLIER_H

#include "ColorStop.h"
#include <QWidget>


namespace Ui {
class BackgroundApplier;
}


class BackgroundApplier : public QWidget
{
    Q_OBJECT

public:
    explicit BackgroundApplier(QWidget *parent = nullptr);
    ~BackgroundApplier();

    void SetSize(int,int);

    QImage GetImage();

signals:
    void BackgroundUpdated(QImage) const;

private slots:
    void on_gradient_type_currentIndexChanged(int);
    void on_spread_comboBox_currentIndexChanged(int);
    void on_rotate_valueChanged(int);
    void on_x_offset_valueChanged(int);
    void on_y_offset_valueChanged(int);
    void on_add_color_stop_button_clicked();
    void on_choose_image_button_clicked();
    void on_presets_comboBox_currentIndexChanged(int);

    void on_brightness_valueChanged(int);
    void on_save_gradient_clicked();
    void on_load_gradient_clicked();

private:
    Ui::BackgroundApplier *ui;
    int w;
    int h;

    QImage image;

    std::vector<ColorStop*> color_stops;

    void ApplyCustom();
    void OpenFileDialog();
    void AddColorStop(ColorStop*);

    QBrush ApplyLinearGradient(QGradientStops, QGradient::Spread, int, int, int);
    QBrush ApplyRadialGradient(QGradientStops, QGradient::Spread, int, int);
    QBrush ApplyConicalGradient(QGradientStops, QGradient::Spread, int, int, int);

    QPointF EdgeOfView(int, int, int);

    const QStringList custom_names = {
        "Linear", "Radial", "Conical"
    };

    const std::vector<QGradient::Type> types = {
        QGradient::LinearGradient,
        QGradient::RadialGradient,
        QGradient::ConicalGradient
    };

    const QStringList spread_names = {
        "Pad", "Repeat", "Reflect"
    };

    const std::vector<QGradient::Spread> spreads = {
        QGradient::PadSpread,
        QGradient::RepeatSpread,
        QGradient::ReflectSpread
    };


};

#endif // BACKGROUNDAPPLIER_H
