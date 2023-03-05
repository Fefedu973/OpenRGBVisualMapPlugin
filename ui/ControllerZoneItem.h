#ifndef CONTROLLERZONEITEM_H
#define CONTROLLERZONEITEM_H

#include <QPainter>
#include <QPen>
#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include "GridSettings.h"
#include "ControllerZone.h"

#define ITEM_BORDER_WIDTH 0.2

class ControllerZoneItem : public QObject, public QGraphicsItem
{
    Q_OBJECT;
    Q_INTERFACES(QGraphicsItem);

public:
    ControllerZoneItem(ControllerZone*, GridSettings*);
    QRectF boundingRect() const;
    void paint(QPainter*, const QStyleOptionGraphicsItem*,QWidget*);
    void Snap();
    ControllerZone* GetControllerZone();
    QPoint point();

signals:
    void Released();
    void RectSelectionRequest();

private:
    ControllerZone* ctrl_zone;
    GridSettings* settings;

    bool pressed = false;
    bool hover = false;

    const QBrush default_brush  = QBrush(QColor("#f2d974"), Qt::BrushStyle::NoBrush);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);    
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
};

#endif // CONTROLLERZONEITEM_H
