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
std::vector<LedRouting::PixelWeight> BuildPolygonWeights(const QPolygonF& polygon,const QSize& canvas_size);
}

bool LedRouting::ValidGeometry(const ControllerZone* zone)
{
    if(!zone || !zone->controller || (zone->settings.shape==CUSTOM && !zone->settings.custom_shape))return false;
    const auto& s=zone->settings;const auto size=UnscaledSize(zone);
    return std::isfinite(s.scale) && s.scale>0 && std::isfinite(s.scale_x) && s.scale_x>0
        && std::isfinite(s.scale_y) && s.scale_y>0 && std::isfinite(s.rotation)
        && std::isfinite(s.x) && std::isfinite(s.y) && std::isfinite(size.width()) && size.width()>0
        && std::isfinite(size.height()) && size.height()>0
        && std::isfinite(size.width()*s.scale*s.scale_x) && std::isfinite(size.height()*s.scale*s.scale_y)
        && std::isfinite(s.brightness) && s.brightness>=0 && s.brightness<=1;
}

QTransform LedRouting::LocalTransform(const ControllerZone* zone)
{
    const auto size=UnscaledSize(zone);const auto& s=zone->settings;
    const qreal sx=s.scale*s.scale_x,sy=s.scale*s.scale_y;
    QTransform result;
    result.translate(size.width()*sx/2,size.height()*sy/2);
    result.rotate(std::remainder(s.rotation,360.0));
    result.scale(s.flip_x?-sx:sx,s.flip_y?-sy:sy);
    result.translate(-size.width()/2,-size.height()/2);
    return result;
}

QRectF LedRouting::LocalBounds(const ControllerZone* zone)
{
    return ValidGeometry(zone)?LocalTransform(zone).mapRect(QRectF(QPointF(),UnscaledSize(zone))):QRectF();
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
    std::vector<LedCell> cells;
    if(!ValidGeometry(ctrl_zone)) return cells;
    cells.reserve(led_count);

    switch(settings.shape)
    {
    case HORIZONTAL_LINE:
        for(unsigned int i = 0; i < led_count; i++)
        {
            const unsigned int position = settings.reverse ? led_count - 1 - i : i;
            cells.push_back({
                i,
                QRectF(position * settings.led_spacing, 0.0, 1.0, 1.0), {}
            });
        }
        break;

    case VERTICAL_LINE:
        for(unsigned int i = 0; i < led_count; i++)
        {
            const unsigned int position = settings.reverse ? led_count - 1 - i : i;
            cells.push_back({
                i,
                QRectF(0.0, position * settings.led_spacing, 1.0, 1.0), {}
            });
        }
        break;

    case CUSTOM:
        for(LedPosition* position : settings.custom_shape->led_positions)
        {
            if(!position || !std::isfinite(position->x()) || !std::isfinite(position->y()))continue;
            const qreal offset=settings.point_is_center?-0.5:0.0;
            cells.push_back({
                position->led_num,
                QRectF(position->x()+offset, position->y()+offset, 1.0, 1.0), {}
            });
        }
        break;
    }

    const auto transform=LocalTransform(ctrl_zone);
    for(auto& cell:cells) {
        cell.local_polygon=transform.map(QPolygonF(cell.local_rect));
        cell.local_rect=cell.local_polygon.boundingRect();
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
        QPolygonF polygon=cell.local_polygon.translated(origin);
        if(scene_size.width() > 0 && scene_size.height() > 0)
        {
            const qreal sx = canvas_size.width()/scene_size.width();
            const qreal sy = canvas_size.height()/scene_size.height();
            polygon=QTransform::fromScale(sx,sy).map(polygon);
        }
        std::vector<PixelWeight> overlaps = BuildPolygonWeights(polygon, canvas_size);

        if(overlaps.empty())
        {
            continue;
        }

        routes.push_back({cell.led_index, cell.local_rect, std::move(overlaps),ctrl_zone->settings.brightness});
    }

    return routes;
}

namespace
{
QPolygonF Clip(const QPolygonF& input,unsigned axis,qreal edge,bool greater)
{
    QPolygonF result;if(input.isEmpty())return result;
    const auto coordinate=[&](const QPointF& p){return axis?p.y():p.x();};
    auto previous=input.last();bool previous_in=greater?coordinate(previous)>=edge:coordinate(previous)<=edge;
    for(const auto& point:input) {
        const bool inside=greater?coordinate(point)>=edge:coordinate(point)<=edge;
        if(inside!=previous_in) {
            const qreal d=coordinate(point)-coordinate(previous);
            if(std::abs(d)>1e-20)result<<previous+(point-previous)*((edge-coordinate(previous))/d);
        }
        if(inside)result<<point;
        previous=point;previous_in=inside;
    }return result;
}
qreal Area(const QPolygonF& polygon)
{
    qreal area=0;for(int i=0;i<polygon.size();++i) {
        const auto& a=polygon[i];const auto& b=polygon[(i+1)%polygon.size()];area+=a.x()*b.y()-b.x()*a.y();
    }return std::abs(area)/2;
}
std::vector<LedRouting::PixelWeight> BuildPolygonWeights(const QPolygonF& polygon,const QSize& canvas_size)
{
    if(polygon.isEmpty())return {};
    for(const auto& p:polygon)if(!std::isfinite(p.x()) || !std::isfinite(p.y()))return {};
    const auto bounds=polygon.boundingRect();
    // Preserve the exact, cheap legacy axis-aligned path, including 90 degree rotations.
    bool aligned=true;for(int i=1;i<polygon.size();++i) {
        const auto d=polygon[i]-polygon[i-1];if(std::abs(d.x())>1e-9 && std::abs(d.y())>1e-9){aligned=false;break;}
    }
    if(aligned)return BuildPixelWeights(bounds,canvas_size);
    const auto clipped=bounds.intersected(QRectF(QPointF(),canvas_size));if(clipped.isEmpty())return {};
    std::vector<LedRouting::PixelWeight> result;qreal total=0;
    for(int y=std::max(0,int(std::floor(clipped.top())));y<std::min(canvas_size.height(),int(std::ceil(clipped.bottom())));++y)
    for(int x=std::max(0,int(std::floor(clipped.left())));x<std::min(canvas_size.width(),int(std::ceil(clipped.right())));++x) {
        auto p=Clip(Clip(Clip(Clip(polygon,0,x,true),0,x+1,false),1,y,true),1,y+1,false);
        const auto area=Area(p);if(area>OVERLAP_EPSILON){result.push_back({QPoint(x,y),area});total+=area;}
    }
    if(total<=OVERLAP_EPSILON)return {};
    for(auto& pixel:result)pixel.weight/=total;return result;
}
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

    return QColor(ToColorChannel(red / total_weight * route.brightness),
                  ToColorChannel(green / total_weight * route.brightness),
                  ToColorChannel(blue / total_weight * route.brightness),
                  ToColorChannel(alpha / total_weight));
}
