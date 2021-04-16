#include "LedItem.h"

#include "MathUtils.h"
#include "math.h"
#include <QGraphicsSceneMouseEvent>
#include <QString>
#include <QCursor>

LedItem::LedItem(int led_num, QPoint* led_position) :
   led_num(led_num),
   led_position(led_position)
{
    setFlag(ItemIsMovable);

    std::string tooltip =
        "<div style=\"display:inline-block; padding:10px; font-weight:bold; background-color:#ffffff; color: #000000\">"
            + std::to_string(led_num + 1)
        + "</div>";

    setToolTip(QString::fromUtf8(tooltip.c_str()));

    setCursor(Qt::OpenHandCursor);
}

QRectF LedItem::boundingRect() const
{
    return QRectF(0, 0, 1, 1);
}

void LedItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    // scale to display
    setX(led_position->x());
    setY(led_position->y());

    QRectF rect = boundingRect();
    QPen pen(QColor(0, 0, 0, 0x00));

    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);

    QBrush brush = pressed ? moving_brush: selected ? selected_brush : default_brush;
    painter->setBrush(brush);

    painter->fillRect(rect, brush);

    //QPen text_pen(QColor("#534e52"));
    //painter->setPen(text_pen);
    //painter->setBrush(QColor("#534e52"));
    //painter->drawText(rect, Qt::AlignCenter, QString("%1").arg(led_num+1));

}

void LedItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = true;
    setCursor(Qt::ClosedHandCursor);

    emit Selected();

    update();

    QGraphicsItem::mousePressEvent(event);
}

void LedItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = false;
    moving = false;

    setCursor(Qt::OpenHandCursor);

    emit Released();

    QGraphicsItem::mouseReleaseEvent(event);
}

void LedItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    moving = true;

    led_position->setX(x());
    led_position->setY(y());

    emit Moving();

    QGraphicsItem::mouseMoveEvent(event);
}

void LedItem::SetSelected(bool value)
{
    selected = value;
}

void LedItem::Restrict(int w, int h)
{
    int new_x = std::min<int>(std::max<int>(0,x()), w-1);
    int new_y = std::min<int>(std::max<int>(0,y()), h-1);

    //new_x = MathUtils::FloorToNearestTen(new_x);
    //new_y = MathUtils::FloorToNearestTen(new_y);

    led_position->setX(new_x);
    led_position->setY(new_y);

    update();
}

