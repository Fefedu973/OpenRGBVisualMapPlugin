#ifndef CONTROLLERZONEITEM_H
#define CONTROLLERZONEITEM_H

#include <QPainter>
#include <QPen>
#include <QGraphicsItem>

#include "ZoneManager.h"

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

signals:
    void Selected();
    void Moved();

private:
    ControllerZone* ctrl_zone;
    bool selected = false;
    bool pressed = false;
    bool moving = false;

    inline static const QBrush selected_brush = QBrush(QColor("#c7956d"));
    inline static const QBrush moving_brush =   QBrush(QColor("#965d62"));
    inline static const QBrush default_brush =  QBrush(QColor("#f2d974"));

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

};

#endif // CONTROLLERZONEITEM_H
