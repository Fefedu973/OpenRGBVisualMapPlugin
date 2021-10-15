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
    QGradientStops stops;
    unsigned int angle;
    unsigned int x_offset;
    unsigned int y_offset;
};

const std::vector<GradientPreset> GRADIENT_PRESETS = {
    {
        "Dark red",
        QGradient::LinearGradient,
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
        QGradientStops({
            QGradientStop(0,   "#00DBDE"),
            QGradientStop(1,   "#FC00FF"),
        }),
        90,
        100,
        100
    },
    // linear-gradient(90deg, #00DBDE 0%, #FC00FF 100%);

};


#endif // GRADIENTPRESETS_H
