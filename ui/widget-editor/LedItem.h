#ifndef LEDITEM_H
#define LEDITEM_H

#include <QPainter>
#include <QPen>
#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

#include "GridSettings.h"
#include "ControllerZone.h"

class LedItem: public QObject, public QGraphicsItem
{
    Q_OBJECT;
    Q_INTERFACES(QGraphicsItem);

public:
    LedItem(LedPosition*, GridSettings*);
    QRectF boundingRect() const;
    void paint(QPainter*, const QStyleOptionGraphicsItem*,QWidget*);
    void SetSelected(bool);
    void Restrict(int,int);
    void MoveBy(int,int);
    LedPosition* GetLedPosition();

signals:
    void Selected();
    void Moving();
    void Released();
    void Restricted(int,int);

private:
    LedPosition*  led_position;
    GridSettings* settings;

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
#endif // LEDITEM_H
