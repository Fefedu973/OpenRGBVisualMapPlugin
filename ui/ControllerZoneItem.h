#ifndef CONTROLLERZONEITEM_H
#define CONTROLLERZONEITEM_H

#include <QPainter>
#include <QPen>
#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

#include "ControllerZone.h"

class ControllerZoneItem : public QObject, public QGraphicsItem
{
    Q_OBJECT;
    Q_INTERFACES(QGraphicsItem);

public:

    ControllerZoneItem(ControllerZone*);

    QRectF boundingRect() const;

    void paint(QPainter * painter,
               const QStyleOptionGraphicsItem * option,
               QWidget * widget);

    void SetSelected(bool);

    void Restrict(int,int);

signals:
    void Selected();
    void Moved();

private:
    ControllerZone* ctrl_zone;
    bool selected = false;
    bool pressed = false;
    bool moving = false;
    bool hover = false;

    inline static const QBrush selected_brush = QBrush(QColor("#c7956d"));
    inline static const QBrush moving_brush =   QBrush(QColor("#965d62"));
    inline static const QBrush default_brush =  QBrush(QColor("#f2d974"));
    inline static const QBrush hover_brush =    QBrush(QColor("#00ff00"));

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);    
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
};

#endif // CONTROLLERZONEITEM_H
