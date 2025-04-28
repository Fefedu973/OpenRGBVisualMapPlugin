#ifndef GRADIENTPRESETS_H
#define GRADIENTPRESETS_H

#include <vector>
#include <string>
#include <QGradient>
#include <QGradientStop>
#include <QGradientStops>

struct GradientPreset
{
    std::string name;
    QGradient::Type type;
    QGradient::Spread spread;
    QGradientStops stops;
    unsigned int angle;
    unsigned int x_offset;
    unsigned int y_offset;
};

const std::vector<GradientPreset> GRADIENT_PRESETS = {
    {
        "Empty preset",
        QGradient::LinearGradient,
        QGradient::PadSpread,
        QGradientStops({
            QGradientStop(0,Qt::black),
            QGradientStop(1,Qt::black)
        }),
        0,
        100,
        100
    },
    {
        "Dark red",
        QGradient::LinearGradient,
        QGradient::PadSpread,
        QGradientStops({
            QGradientStop(0,Qt::black),
            QGradientStop(1,Qt::red),
        }),
        0,
        100,
        100
    },
    {
        "Dark blue",
        QGradient::LinearGradient,
        QGradient::PadSpread,
        QGradientStops({
            QGradientStop(0,Qt::black),
            QGradientStop(1,Qt::blue),
        }),
        0,
        100,
        100
    },
    {
        "Dark green",
        QGradient::LinearGradient,
        QGradient::PadSpread,
        QGradientStops({
            QGradientStop(0,Qt::black),
            QGradientStop(1,Qt::green),
        }),
        0,
        100,
        100
    },
    {
        "Dark yellow",
        QGradient::LinearGradient,
        QGradient::PadSpread,
        QGradientStops({
            QGradientStop(0,Qt::black),
            QGradientStop(1,Qt::yellow),
        }),
        0,
        100,
        100
    },
    {
        "Halloween pumpkin",
        QGradient::LinearGradient,
        QGradient::PadSpread,
        QGradientStops({
            QGradientStop(0,   "#090B06"),
            QGradientStop(0.2, "#1B3711"),
            QGradientStop(0.4, "#2A5420"),
            QGradientStop(0.6, "#F5D913"),
            QGradientStop(0.8, "#EB912D"),
            QGradientStop(1,   "#F46D0E"),
        }),
        0,
        100,
        100
    },
    {
        "Fabled sunset",
        QGradient::LinearGradient,
        QGradient::PadSpread,
        QGradientStops({
            QGradientStop(0,   "#231557"),
            QGradientStop(0.29, "#44107A"),
            QGradientStop(0.67, "#FF1361"),
            QGradientStop(1,   "#FFF800"),
        }),
        225,
        100,
        100
    },
    {
        "Pink blue",
        QGradient::LinearGradient,
        QGradient::PadSpread,
        QGradientStops({
            QGradientStop(0,   "#00DBDE"),
            QGradientStop(1,   "#FC00FF"),
        }),
        90,
        100,
        100
    },
    {
        "Green to pink",
        QGradient::LinearGradient,
        QGradient::PadSpread,
        QGradientStops({
            QGradientStop(0,   "#FA8BFF"),
            QGradientStop(0.52,   "#2BD2FF"),
            QGradientStop(1,   "#2BFF88"),
        }),
        315,
        100,
        100
    },
    {
        "Burning Spring",
        QGradient::LinearGradient,
        QGradient::PadSpread,
        QGradientStops({
            QGradientStop(0,   "#4fb576"),
            QGradientStop(0.3,   "#44c489"),
            QGradientStop(0.46,   "#28a9ae"),
            QGradientStop(0.59,   "#28a2b7"),
            QGradientStop(0.71,   "#4c7788"),
            QGradientStop(0.86,   "#6c4f63"),
            QGradientStop(1,   "#432c39"),
        }),
        90,
        100,
        100
    },
    {
        "HexaFox",
        QGradient::LinearGradient,
        QGradient::PadSpread,
        QGradientStops({
            QGradientStop(0,   "#e66000"),
            QGradientStop(0.3,   "#ff9500"),
            QGradientStop(0.46,   "#ffcb00"),
            QGradientStop(0.59,   "#00539f"),
            QGradientStop(0.71,   "#0095dd"),
            QGradientStop(0.86,   "#331e54"),
            QGradientStop(1,   "#002147"),
        }),
        0,
        100,
        100
    },

};


#endif // GRADIENTPRESETS_H
