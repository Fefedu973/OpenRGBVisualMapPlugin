#ifndef CONTROLLERZONEITEM_H
#define CONTROLLERZONEITEM_H

#include <QPainter>
#include <QPen>
#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include "GridSettings.h"

#include "ControllerZone.h"

class ControllerZoneItem : public QObject, public QGraphicsItem
{
    Q_OBJECT;
    Q_INTERFACES(QGraphicsItem);

public:
    ControllerZoneItem(ControllerZone*, GridSettings*);
    QRectF boundingRect() const;
    void paint(QPainter*, const QStyleOptionGraphicsItem*,QWidget*);
    void Restrict();
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

    const QBrush selected_brush = QBrush(QColor("#c7956d"), Qt::BrushStyle::SolidPattern);
    const QBrush focus_brush    = QBrush(QColor("#965d62"), Qt::BrushStyle::SolidPattern);
    const QBrush default_brush  = QBrush(QColor("#f2d974"), Qt::BrushStyle::NoBrush);
    const QBrush hover_brush    = QBrush(QColor("#00ff00"), Qt::BrushStyle::SolidPattern);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);    
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
};

#endif // CONTROLLERZONEITEM_H
