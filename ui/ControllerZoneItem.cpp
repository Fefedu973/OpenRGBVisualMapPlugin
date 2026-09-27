#include "ControllerZoneItem.h"

#include <algorithm>
#include <cmath>
#include <QApplication>
#include <QCursor>
#include <QPalette>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QString>
#include <utility>

ControllerZoneItem::ControllerZoneItem(ControllerZone* ctrl_zone, GridSettings* settings) :
    ctrl_zone(ctrl_zone),
    settings(settings)
{
    RefreshGeometry();
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges | ItemAcceptsInputMethod);
    setAcceptHoverEvents(true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    setPos(ctrl_zone->settings.x, ctrl_zone->settings.y);
    UpdateZValue(false);

    std::string tooltip =
            "<div style=\"display:inline-block; padding:10px; font-weight:bold; background-color:#ffffff; color: #000000\">"
            + ctrl_zone->full_display_name()
            + "</div>";

    setToolTip(QString::fromUtf8(tooltip.c_str()));
    setCursor(Qt::OpenHandCursor);
}

const QRectF& ControllerZoneItem::DeviceRect() const
{
    return device_rect;
}

qreal ControllerZoneItem::ResizeHandleSize(qreal pixels, qreal maximum) const
{
    qreal view_scale = 1.0;

    if(scene() && !scene()->views().isEmpty())
    {
        view_scale = std::abs(scene()->views().front()->transform().m11());
    }

    return std::min(maximum, pixels / std::max<qreal>(view_scale, 0.01));
}

QRectF ControllerZoneItem::ResizeHandleRect(ResizeCorner corner, qreal size) const
{
    const QRectF rect = DeviceRect();
    QPointF center;

    switch(corner)
    {
    case ResizeCorner::TopLeft:     center = rect.topLeft(); break;
    case ResizeCorner::TopRight:    center = rect.topRight(); break;
    case ResizeCorner::BottomLeft:  center = rect.bottomLeft(); break;
    case ResizeCorner::BottomRight: center = rect.bottomRight(); break;
    case ResizeCorner::None:        return QRectF();
    }

    return QRectF(center.x() - size / 2.0,
                  center.y() - size / 2.0,
                  size,
                  size);
}

QRectF ControllerZoneItem::boundingRect() const
{
    const qreal margin = RESIZE_HANDLE_MAX_HIT_SIZE / 2.0 + ITEM_BORDER_WIDTH;
    auto bounds=DeviceRect();
    // Centre-origin cells may extend half a unit beyond the declared surface.
    for(const auto& cell:led_cells)bounds=bounds.united(cell.local_rect);
    return bounds.adjusted(-margin, -margin, margin, margin);
}

QPainterPath ControllerZoneItem::shape() const
{
    QPainterPath path;
    path.addRect(DeviceRect());
    for(const auto& cell:led_cells)path.addPolygon(cell.local_polygon);

    if(isSelected())
    {
        const qreal hit_size = ResizeHandleSize(RESIZE_HANDLE_HIT_PX, RESIZE_HANDLE_MAX_HIT_SIZE);
        path.addRect(ResizeHandleRect(ResizeCorner::TopLeft, hit_size));
        path.addRect(ResizeHandleRect(ResizeCorner::TopRight, hit_size));
        path.addRect(ResizeHandleRect(ResizeCorner::BottomLeft, hit_size));
        path.addRect(ResizeHandleRect(ResizeCorner::BottomRight, hit_size));
    }

    return path;
}

void ControllerZoneItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    const QRectF device_rect = DeviceRect();

    QPalette pal = QApplication::palette();
    QColor highlight = pal.color(QPalette::Highlight);
    QColor text = pal.color(QPalette::Text);

    painter->setPen(QPen(text, ITEM_BORDER_WIDTH));
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    for(const LedRouting::LedCell& cell : led_cells)
    {
        QBrush brush = default_brush;

        if(cell.led_index < preview_colors.size() && preview_colors[cell.led_index].alpha() > 0)
        {
            brush = QBrush(preview_colors[cell.led_index], Qt::SolidPattern);
        }

        painter->setBrush(brush);
        painter->drawPolygon(cell.local_polygon);
    }

    if(isSelected())
    {
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

        QPen selection_pen(highlight);
        selection_pen.setCosmetic(true);
        selection_pen.setWidth(2);
        painter->setPen(selection_pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(device_rect);

        QPen handle_pen(pal.color(QPalette::Base));
        handle_pen.setCosmetic(true);
        painter->setPen(handle_pen);
        painter->setBrush(highlight);
        const qreal handle_size = ResizeHandleSize(RESIZE_HANDLE_PX, RESIZE_HANDLE_MAX_SIZE);
        painter->drawRect(ResizeHandleRect(ResizeCorner::TopLeft, handle_size));
        painter->drawRect(ResizeHandleRect(ResizeCorner::TopRight, handle_size));
        painter->drawRect(ResizeHandleRect(ResizeCorner::BottomLeft, handle_size));
        painter->drawRect(ResizeHandleRect(ResizeCorner::BottomRight, handle_size));
    }
}

ControllerZoneItem::ResizeCorner ControllerZoneItem::ResizeCornerAt(const QPointF& position) const
{
    const qreal hit_size = ResizeHandleSize(RESIZE_HANDLE_HIT_PX, RESIZE_HANDLE_MAX_HIT_SIZE);
    const ResizeCorner corners[] = {
        ResizeCorner::TopLeft,
        ResizeCorner::TopRight,
        ResizeCorner::BottomLeft,
        ResizeCorner::BottomRight
    };

    for(ResizeCorner corner : corners)
    {
        if(ResizeHandleRect(corner, hit_size).contains(position))
        {
            return corner;
        }
    }

    return ResizeCorner::None;
}

qreal ControllerZoneItem::SnapValue(qreal value) const
{
    const qreal step = std::max(1, settings->grid_size);
    return std::round(value / step) * step;
}

void ControllerZoneItem::RefreshGeometry()
{
    device_rect = LedRouting::LocalBounds(ctrl_zone);
    unscaled_size = device_rect.size()/ctrl_zone->settings.scale;
    led_cells = LedRouting::BuildCells(ctrl_zone);
}

void ControllerZoneItem::ResizeTo(const QPointF& scene_position)
{
    const qreal horizontal_scale = std::abs(scene_position.x() - resize_anchor.x()) / unscaled_size.width();
    const qreal vertical_scale = std::abs(scene_position.y() - resize_anchor.y()) / unscaled_size.height();
    qreal new_scale = std::max(MIN_ITEM_SCALE, std::max(horizontal_scale, vertical_scale));

    if(settings->snap_to_grid)
    {
        const qreal step = std::max(1, settings->grid_size);
        const bool width_is_dominant = horizontal_scale >= vertical_scale;
        const qreal base_length = width_is_dominant ? unscaled_size.width() : unscaled_size.height();
        const qreal snapped_length = std::max(step, std::round(base_length * new_scale / step) * step);
        new_scale = std::max<qreal>(MIN_ITEM_SCALE, snapped_length / base_length);
    }

    const qreal ratio=new_scale/ctrl_zone->settings.scale;
    const QRectF new_rect(device_rect.topLeft()*ratio,device_rect.size()*ratio);
    QPointF new_position;

    switch(resize_corner)
    {
    case ResizeCorner::TopLeft:
        new_position = resize_anchor - new_rect.bottomRight();
        break;
    case ResizeCorner::TopRight:
        new_position = resize_anchor - new_rect.bottomLeft();
        break;
    case ResizeCorner::BottomLeft:
        new_position = resize_anchor - new_rect.topRight();
        break;
    case ResizeCorner::BottomRight:
        new_position = resize_anchor - new_rect.topLeft();
        break;
    case ResizeCorner::None:
        return;
    }

    prepareGeometryChange();
    preview_routes_dirty = true;
    ctrl_zone->settings.scale = new_scale;
    RefreshGeometry();
    UpdateZValue(isSelected());
    ctrl_zone->settings.x = new_position.x();
    ctrl_zone->settings.y = new_position.y();
    setPos(new_position);
    update();
}

void ControllerZoneItem::UpdateCursor(const QPointF& position)
{
    if(!isSelected())
    {
        setCursor(Qt::OpenHandCursor);
        return;
    }

    switch(ResizeCornerAt(position))
    {
    case ResizeCorner::TopLeft:
    case ResizeCorner::BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case ResizeCorner::TopRight:
    case ResizeCorner::BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case ResizeCorner::None:
        setCursor(Qt::OpenHandCursor);
        break;
    }
}

void ControllerZoneItem::UpdateZValue(bool selected)
{
    setZValue(selected ? 1.0 : -device_rect.width() * device_rect.height());
}

void ControllerZoneItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = true;

    if(event->modifiers() == Qt::ShiftModifier)
    {
        event->accept();
        return;
    }

    if(event->button() == Qt::LeftButton && isSelected())
    {
        resize_corner = ResizeCornerAt(event->pos());

        if(resize_corner != ResizeCorner::None)
        {
            const QRectF rect = DeviceRect();
            QPointF anchor;

            switch(resize_corner)
            {
            case ResizeCorner::TopLeft:     anchor = rect.bottomRight(); break;
            case ResizeCorner::TopRight:    anchor = rect.bottomLeft(); break;
            case ResizeCorner::BottomLeft:  anchor = rect.topRight(); break;
            case ResizeCorner::BottomRight: anchor = rect.topLeft(); break;
            case ResizeCorner::None: break;
            }

            resizing = true;
            resize_anchor = mapToScene(anchor);
            event->accept();
            return;
        }
    }

    setCursor(Qt::ClosedHandCursor);
    QGraphicsItem::mousePressEvent(event);
}

void ControllerZoneItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(resizing)
    {
        ResizeTo(event->scenePos());
        event->accept();
        return;
    }

    QGraphicsItem::mouseMoveEvent(event);
}

void ControllerZoneItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = false;

    if(resizing)
    {
        resizing = false;
        resize_corner = ResizeCorner::None;
        UpdateCursor(event->pos());
        event->accept();
        emit Released();
        return;
    }

    setCursor(Qt::OpenHandCursor);

    if(event->modifiers() == Qt::ShiftModifier)
    {
        emit RectSelectionRequest();
        event->accept();
    }
    else
    {
        QGraphicsItem::mouseReleaseEvent(event);
    }

    emit Released();
}

void ControllerZoneItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    UpdateCursor(event->pos());
    QGraphicsItem::hoverEnterEvent(event);
}

void ControllerZoneItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!pressed)
    {
        UpdateCursor(event->pos());
    }

    QGraphicsItem::hoverMoveEvent(event);
}

void ControllerZoneItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!pressed)
    {
        setCursor(Qt::OpenHandCursor);
    }

    QGraphicsItem::hoverLeaveEvent(event);
}

void ControllerZoneItem::CommitPosition()
{
    QPointF new_position = pos();
    const QPointF saved_position(ctrl_zone->settings.x, ctrl_zone->settings.y);

    if(settings->snap_to_grid && new_position != saved_position)
    {
        new_position.setX(SnapValue(new_position.x()));
        new_position.setY(SnapValue(new_position.y()));
        setPos(new_position);
    }

    ctrl_zone->settings.x = new_position.x();
    ctrl_zone->settings.y = new_position.y();
}

void ControllerZoneItem::SyncFromSettings()
{
    prepareGeometryChange();
    preview_routes_dirty = true;
    RefreshGeometry();
    UpdateZValue(isSelected());
    setPos(ctrl_zone->settings.x, ctrl_zone->settings.y);
    update();
}

void ControllerZoneItem::UpdatePreview(const QImage& image)
{
    if(pressed)
    {
        return;
    }

    const QSize scene_size(settings->w, settings->h);
    if(preview_routes_dirty || preview_canvas_size != image.size() || preview_scene_size != scene_size)
    {
        preview_routes = LedRouting::BuildRoutes(ctrl_zone, pos(), image.size(), QSizeF(settings->w, settings->h));
        preview_canvas_size = image.size();
        preview_scene_size = scene_size;
        preview_routes_dirty = false;
    }

    std::vector<QColor> next_colors(ctrl_zone->led_count(), QColor(0, 0, 0, 0));

    for(const LedRouting::LedRoute& route : preview_routes)
    {
        if(route.led_index < next_colors.size())
        {
            next_colors[route.led_index] = LedRouting::MixColor(image, route);
        }
    }

    if(next_colors != preview_colors)
    {
        preview_colors = std::move(next_colors);
        update();
    }
}

QVariant ControllerZoneItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if(change == ItemPositionHasChanged)
    {
        preview_routes_dirty = true;
    }
    else if(change == ItemSelectedHasChanged)
    {
        UpdateZValue(value.toBool());
    }

    return QGraphicsItem::itemChange(change, value);
}

ControllerZone* ControllerZoneItem::GetControllerZone()
{
    return ctrl_zone;
}

QPointF ControllerZoneItem::point() const
{
    return pos();
}
