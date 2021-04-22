#include "LedItem.h"

#include "math.h"
#include <QGraphicsSceneMouseEvent>
#include <QString>
#include <QCursor>

LedItem::LedItem(LedPosition* led_position, GridSettings* settings) :
   led_position(led_position),
   settings(settings)
{
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setAcceptHoverEvents(true);

    std::string tooltip =
        "<div style=\"display:inline-block; padding:10px; font-weight:bold; background-color:#ffffff; color: #000000\">"
            + std::to_string(led_position->led_num)
        + "</div>";

    setToolTip(QString::fromUtf8(tooltip.c_str()));

    setCursor(Qt::OpenHandCursor);
}

QRectF LedItem::boundingRect() const
{
    return QRectF(0, 0, 1 * settings->grid_scale_factor , 1 * settings->grid_scale_factor);
}

void LedItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    // scale to display
    setX(led_position->x() * settings->grid_scale_factor);
    setY(led_position->y() * settings->grid_scale_factor);

    QRectF rect = boundingRect();
    QPen pen(QColor(0, 0, 0, 0x80), 0.05);

    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);

    QBrush brush = pressed ? moving_brush: hover ? hover_brush : selected || isSelected() ? selected_brush :  default_brush;
    painter->setBrush(brush);

    painter->drawRect(rect);

    QPen text_pen(QColor("#534e52"));

    QFont font;
    font.setPixelSize(4);
    painter->setFont(font);

    painter->setPen(text_pen);
    painter->setBrush(QColor("#534e52"));
    painter->drawText(rect, Qt::AlignCenter, QString("%1").arg(led_position->led_num));

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

    emit Moving();

    QGraphicsItem::mouseMoveEvent(event);
}

void LedItem::SetSelected(bool value)
{
    selected = value;
}

void LedItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hover = true;
    QGraphicsItem::hoverEnterEvent(event);
}

void LedItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    hover = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void LedItem::MoveBy(int x, int y)
{
    int new_x = led_position->x() + x;
    int new_y = led_position->y() + y;

    new_x = std::min<int>(std::max<int>(0,new_x), settings->w-1);
    new_y = std::min<int>(std::max<int>(0,new_y), settings->h-1);

    led_position->setX(new_x);
    led_position->setY(new_y);

    update();
}

void LedItem::Restrict(int w, int h)
{
    int original_x = led_position->x();
    int original_y = led_position->y();

    int new_x = x();    
    int new_y = y();

    // ease moves
    if(new_x % settings->grid_scale_factor >= settings->grid_scale_factor/2)
    {
        new_x += settings->grid_scale_factor/2;
    }

    if(new_y % settings->grid_scale_factor >= settings->grid_scale_factor/2)
    {
        new_y += settings->grid_scale_factor/2;
    }

    // restrict to bounds
    new_x = std::min<int>(std::max<int>(0,new_x), w-1);
    new_y = std::min<int>(std::max<int>(0,new_y), h-1);

    // normalize
    new_x = (new_x/settings->grid_scale_factor);
    new_y = (new_y/settings->grid_scale_factor);

    // update led real position
    led_position->setX(new_x);
    led_position->setY(new_y);

    update();

    int delta_x = new_x - original_x;
    int delta_y = new_y - original_y;

    emit Restricted(delta_x, delta_y);

}

LedPosition* LedItem::GetLedPosition()
{
    return led_position;
}
