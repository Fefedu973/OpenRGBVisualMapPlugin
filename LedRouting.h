#pragma once

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QPointF>
#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <vector>
#include "ControllerZone.h"

namespace LedRouting
{
struct LedCell
{
    unsigned int led_index;
    QRectF local_rect;
};

struct PixelWeight
{
    QPoint pixel;
    qreal weight;
};

struct LedRoute
{
    unsigned int led_index;
    QRectF local_rect;
    std::vector<PixelWeight> overlaps;
};

QSizeF UnscaledSize(const ControllerZone* ctrl_zone);
std::vector<LedCell> BuildCells(const ControllerZone* ctrl_zone);
std::vector<LedRoute> BuildRoutes(const ControllerZone* ctrl_zone,
                                  const QPointF& origin,
                                  const QSize& canvas_size,
                                  const QSizeF& scene_size = QSizeF());
QColor MixColor(const QImage& image, const LedRoute& route);
}
