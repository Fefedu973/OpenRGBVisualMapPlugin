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

    void Snap();

signals:
      void Released();
      void RectSelectionRequest();

private:
    LedPosition*  led_position;
    GridSettings* settings;

    bool hover = false;
    bool pressed = false;

    const QBrush default_brush     =  QBrush(QColor("#f2d974"), Qt::BrushStyle::NoBrush);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent*);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) ;

};
#endif // LEDITEM_H
