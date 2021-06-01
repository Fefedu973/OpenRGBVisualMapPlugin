#ifndef LEDITEM_H
#define LEDITEM_H

#include <QPainter>
#include <QPen>
#include <QGraphicsItem>
#include <QGraphicsTextItem>
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

    LedPosition* GetLedPosition();

    void Restrict();

signals:
      void Released();
      void RectSelectionRequest();

private:
    LedPosition*  led_position;
    GridSettings* settings;

    bool hover = false;
    bool pressed = false;

    const QBrush selected_brush  = QBrush(QColor("#c7956d"));
    const QBrush focus_brush       =   QBrush(QColor("#965d62"));
    const QBrush default_brush     =  QBrush(QColor("#f2d974"));
    const QBrush hover_brush   =    QBrush(QColor("#00ff00"));

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent*);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) ;

};
#endif // LEDITEM_H
