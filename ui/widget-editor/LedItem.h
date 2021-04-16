#ifndef LEDITEM_H
#define LEDITEM_H

#include <QPainter>
#include <QPen>
#include <QGraphicsItem>

class LedItem: public QObject, public QGraphicsItem
{
    Q_OBJECT;
    Q_INTERFACES(QGraphicsItem);

public:

    LedItem(int, QPoint*);

    QRectF boundingRect() const;

    void paint(QPainter * painter,
               const QStyleOptionGraphicsItem * option,
               QWidget * widget);

    void SetSelected(bool);

    void Restrict(int,int);

signals:
    void Selected();
    void Moving();
    void Released();

private:
    int  led_num;
    QPoint*  led_position;

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
#endif // LEDITEM_H
