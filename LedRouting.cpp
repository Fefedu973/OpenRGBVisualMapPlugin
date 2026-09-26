#include "LedRouting.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr qreal OVERLAP_EPSILON = 1e-9;

int ToColorChannel(qreal value)
{
    return std::clamp(static_cast<int>(std::lround(value)), 0, 255);
}

std::vector<LedRouting::PixelWeight> BuildPixelWeights(const QRectF& canvas_rect,
                                                       const QSize& canvas_size);
}

QSizeF LedRouting::UnscaledSize(const ControllerZone* ctrl_zone)
{
    const qreal led_count = ctrl_zone->led_count();

    switch(ctrl_zone->settings.shape)
    {
    case HORIZONTAL_LINE:
        return QSizeF(std::max<qreal>(1.0,
                                     (led_count - 1) * ctrl_zone->settings.led_spacing + 1),
                      1.0);
    case VERTICAL_LINE:
        return QSizeF(1.0,
                      std::max<qreal>(1.0,
                                     (led_count - 1) * ctrl_zone->settings.led_spacing + 1));
    case CUSTOM:
        if(!ctrl_zone->settings.custom_shape) return QSizeF(1.0,1.0);
        return QSizeF(ctrl_zone->settings.custom_shape->w,
                      ctrl_zone->settings.custom_shape->h);
    }

    return QSizeF(1.0, 1.0);
}

std::vector<LedRouting::LedCell> LedRouting::BuildCells(const ControllerZone* ctrl_zone)
{
    const ControllerZoneSettings& settings = ctrl_zone->settings;
    const unsigned int led_count = ctrl_zone->led_count();
    const qreal scale = settings.scale;
    std::vector<LedCell> cells;
    if(!std::isfinite(scale) || scale <= 0 || (settings.shape == CUSTOM && !settings.custom_shape)) return cells;
    cells.reserve(led_count);

    switch(settings.shape)
    {
    case HORIZONTAL_LINE:
        for(unsigned int i = 0; i < led_count; i++)
        {
            const unsigned int position = settings.reverse ? led_count - 1 - i : i;
            cells.push_back({
                i,
                QRectF(position * settings.led_spacing * scale, 0.0, scale, scale)
            });
        }
        break;

    case VERTICAL_LINE:
        for(unsigned int i = 0; i < led_count; i++)
        {
            const unsigned int position = settings.reverse ? led_count - 1 - i : i;
            cells.push_back({
                i,
                QRectF(0.0, position * settings.led_spacing * scale, scale, scale)
            });
        }
        break;

    case CUSTOM:
        for(LedPosition* position : settings.custom_shape->led_positions)
        {
            cells.push_back({
                position->led_num,
                QRectF(position->x() * scale, position->y() * scale, scale, scale)
            });
        }
        break;
    }

    return cells;
}

std::vector<LedRouting::LedRoute> LedRouting::BuildRoutes(const ControllerZone* ctrl_zone,
                                                          const QPointF& origin,
                                                          const QSize& canvas_size,
                                                          const QSizeF& scene_size)
{
    std::vector<LedRoute> routes;

    if(canvas_size.width() <= 0 || canvas_size.height() <= 0)
    {
        return routes;
    }

    const std::vector<LedCell> cells = BuildCells(ctrl_zone);
    routes.reserve(cells.size());

    for(const LedCell& cell : cells)
    {
        QRectF canvas_rect = cell.local_rect.translated(origin);
        if(scene_size.width() > 0 && scene_size.height() > 0)
        {
            const qreal sx = canvas_size.width()/scene_size.width();
            const qreal sy = canvas_size.height()/scene_size.height();
            canvas_rect = QRectF(canvas_rect.x()*sx, canvas_rect.y()*sy,
                                 canvas_rect.width()*sx, canvas_rect.height()*sy);
        }
        std::vector<PixelWeight> overlaps = BuildPixelWeights(canvas_rect, canvas_size);

        if(overlaps.empty())
        {
            continue;
        }

        routes.push_back({cell.led_index, cell.local_rect, std::move(overlaps)});
    }

    return routes;
}

namespace
{
std::vector<LedRouting::PixelWeight> BuildPixelWeights(const QRectF& canvas_rect,
                                                       const QSize& canvas_size)
{
    std::vector<LedRouting::PixelWeight> overlaps;

    if(canvas_size.width() <= 0 || canvas_size.height() <= 0)
    {
        return overlaps;
    }

    if(!std::isfinite(canvas_rect.x()) || !std::isfinite(canvas_rect.y())
       || !std::isfinite(canvas_rect.width()) || !std::isfinite(canvas_rect.height())
       || canvas_rect.width() <= 0 || canvas_rect.height() <= 0) return overlaps;
    const QRectF clipped = canvas_rect.intersected(QRectF(QPointF(0,0), canvas_size));
    if(clipped.isEmpty()) return overlaps;

    const int first_x = std::max(0, static_cast<int>(std::floor(clipped.left())));
    const int first_y = std::max(0, static_cast<int>(std::floor(clipped.top())));
    const int last_x = std::min(canvas_size.width() - 1,
                                static_cast<int>(std::ceil(clipped.right())) - 1);
    const int last_y = std::min(canvas_size.height() - 1,
                                static_cast<int>(std::ceil(clipped.bottom())) - 1);
    qreal total_weight = 0.0;

    for(int y = first_y; y <= last_y; y++)
    {
        const qreal overlap_height = std::max<qreal>(
            0.0,
            std::min(canvas_rect.bottom(), y + 1.0) - std::max(canvas_rect.top(), qreal(y)));

        for(int x = first_x; x <= last_x; x++)
        {
            const qreal overlap_width = std::max<qreal>(
                0.0,
                std::min(canvas_rect.right(), x + 1.0) - std::max(canvas_rect.left(), qreal(x)));
            const qreal area = overlap_width * overlap_height;

            if(area > OVERLAP_EPSILON)
            {
                overlaps.push_back({QPoint(x, y), area});
                total_weight += area;
            }
        }
    }

    if(total_weight <= OVERLAP_EPSILON)
    {
        return {};
    }

    for(LedRouting::PixelWeight& overlap : overlaps)
    {
        overlap.weight /= total_weight;
    }

    return overlaps;
}
}

QColor LedRouting::MixColor(const QImage& image, const LedRoute& route)
{
    qreal red = 0.0;
    qreal green = 0.0;
    qreal blue = 0.0;
    qreal alpha = 0.0;
    qreal total_weight = 0.0;

    for(const PixelWeight& overlap : route.overlaps)
    {
        if(!image.valid(overlap.pixel))
        {
            continue;
        }

        const QRgb color = image.pixel(overlap.pixel);
        red += qRed(color) * overlap.weight;
        green += qGreen(color) * overlap.weight;
        blue += qBlue(color) * overlap.weight;
        alpha += qAlpha(color) * overlap.weight;
        total_weight += overlap.weight;
    }

    if(total_weight <= OVERLAP_EPSILON)
    {
        return QColor(0, 0, 0, 0);
    }

    return QColor(ToColorChannel(red / total_weight),
                  ToColorChannel(green / total_weight),
                  ToColorChannel(blue / total_weight),
                  ToColorChannel(alpha / total_weight));
}
