#ifndef CONTROLLERZONEITEM_H
#define CONTROLLERZONEITEM_H

#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QSize>
#include <vector>
#include "GridSettings.h"
#include "ControllerZone.h"
#include "LedRouting.h"

#define ITEM_BORDER_WIDTH 0.2
#define RESIZE_HANDLE_PX 7.0
#define RESIZE_HANDLE_HIT_PX 12.0
#define RESIZE_HANDLE_MAX_SIZE 2.0
#define RESIZE_HANDLE_MAX_HIT_SIZE 3.0
#define MIN_ITEM_SCALE 0.1

class ControllerZoneItem : public QObject, public QGraphicsItem
{
    Q_OBJECT;
    Q_INTERFACES(QGraphicsItem);

public:
    ControllerZoneItem(ControllerZone*, GridSettings*);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
    void CommitPosition();
    void SyncFromSettings();
    void UpdatePreview(const QImage&);
    ControllerZone* GetControllerZone();
    QPointF point() const;

signals:
    void Released();
    void RectSelectionRequest();

private:
    enum class ResizeCorner
    {
        None,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    ControllerZone* ctrl_zone;
    GridSettings* settings;

    bool pressed = false;
    bool resizing = false;
    ResizeCorner resize_corner = ResizeCorner::None;
    QPointF resize_anchor;
    QSizeF unscaled_size;
    QRectF device_rect;
    std::vector<LedRouting::LedCell> led_cells;
    std::vector<QColor> preview_colors;
    std::vector<LedRouting::LedRoute> preview_routes;
    QSize preview_canvas_size;
    bool preview_routes_dirty = true;

    const QBrush default_brush = QBrush(QColor("#f2d974"), Qt::BrushStyle::NoBrush);

    const QRectF& DeviceRect() const;
    qreal ResizeHandleSize(qreal, qreal) const;
    QRectF ResizeHandleRect(ResizeCorner, qreal) const;
    ResizeCorner ResizeCornerAt(const QPointF&) const;
    qreal SnapValue(qreal) const;
    void RefreshGeometry();
    void ResizeTo(const QPointF&);
    void UpdateCursor(const QPointF&);
    void UpdateZValue(bool);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent*) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent*) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) override;
    QVariant itemChange(GraphicsItemChange, const QVariant&) override;
};

#endif // CONTROLLERZONEITEM_H
