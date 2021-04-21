#include "ControllerZoneItem.h"
#include "math.h"
#include <QString>
#include <QCursor>

ControllerZoneItem::ControllerZoneItem(ControllerZone* ctrl_zone) :
   ctrl_zone(ctrl_zone)
{
    setFlag(ItemIsMovable);
    setAcceptHoverEvents(true);

    setPos(ctrl_zone->settings.x, ctrl_zone->settings.y);

    std::string tooltip =
        "<div style=\"display:inline-block; padding:10px; font-weight:bold; background-color:#ffffff; color: #000000\">"
            + ctrl_zone->display_name()
        + "</div>";

    setToolTip(QString::fromUtf8(tooltip.c_str()));

    setCursor(Qt::OpenHandCursor);
}

QRectF ControllerZoneItem::boundingRect() const
{
    switch(ctrl_zone->settings.shape)
    {
    case HORIZONTAL_LINE :
        return QRectF(0, 0, ctrl_zone->led_count() * ctrl_zone->settings.led_spacing, 1);
    case VERTICAL_LINE :
        return QRectF(0, 0, 1, ctrl_zone->led_count() * ctrl_zone->settings.led_spacing);
    case CUSTOM:
        return QRectF(0, 0, ctrl_zone->settings.custom_shape->w, ctrl_zone->settings.custom_shape->h);
    }

    return QRectF(0, 0, 1, 1);
}

void ControllerZoneItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    setZValue(ctrl_zone->isCustomShape() ? -ctrl_zone->settings.custom_shape->h * ctrl_zone->settings.custom_shape->w :
                                           -ctrl_zone->led_count() * ctrl_zone->settings.led_spacing);

    if(!moving)
    {
        setPos(ctrl_zone->settings.x, ctrl_zone->settings.y);
    }

    QBrush brush = pressed ? moving_brush: hover ? hover_brush : selected ? selected_brush :  default_brush;

    painter->setBrush(brush);
    QPen pen(QColor(0, 0, 0, 0x80), 0.05);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    if(ctrl_zone->isCustomShape())
    {
        std::vector<QPoint*> led_positions = ctrl_zone->settings.custom_shape->led_positions;

        for(QPoint* point : led_positions)
        {
            QRectF rect = QRectF(point->x(), point->y(), 1, 1);
            painter->drawRect(rect);
        }
    }
    else if(ctrl_zone->settings.shape == HORIZONTAL_LINE)
    {
        int led_count = ctrl_zone->led_count();
        int interval = ctrl_zone->settings.led_spacing;

        for (int i = 0; i < led_count; i++)
        {
            QRectF rect = QRectF(i * interval, 0, 1, 1);
            painter->drawRect(rect);
        }
    }
    else if(ctrl_zone->settings.shape == VERTICAL_LINE)
    {
        int led_count = ctrl_zone->led_count();
        int interval = ctrl_zone->settings.led_spacing;

        for (int i = 0; i < led_count; i++)
        {
            QRectF rect = QRectF(0, i * interval, 1, 1);
            painter->drawRect(rect);
        }
    }
    else
    {
        printf("Unsupported shape\n");
    }
}

void ControllerZoneItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = true;
    setCursor(Qt::ClosedHandCursor);


    emit Selected();

    update();
    QGraphicsItem::mousePressEvent(event);
}

void ControllerZoneItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = false;
    moving = false;
    setCursor(Qt::OpenHandCursor);

    emit Moved();

    update();

    QGraphicsItem::mouseReleaseEvent(event);    
}

void ControllerZoneItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    moving = true;

    ctrl_zone->settings.x = x();
    ctrl_zone->settings.y = y();

    emit Moved();

    QGraphicsItem::mouseMoveEvent(event);
}

void ControllerZoneItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    hover = true;
    QGraphicsItem::hoverEnterEvent(event);
}

void ControllerZoneItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    hover = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void ControllerZoneItem::ControllerZoneItem::SetSelected(bool value)
{
    selected = value;
}


void ControllerZoneItem::Restrict(int w, int h)
{
    int new_x = 10 * x();
    int new_y = 10 * y();

    // ease moves
    if(new_x % 10 >= 5)
    {
        new_x += 5;
    }

    if(new_y % 10 >= 5)
    {
        new_y += 5;
    }

    // restrict to bounds
    new_x = std::min<int>(std::max<int>(0,new_x/10), w-1);
    new_y = std::min<int>(std::max<int>(0,new_y/10), h-1);

    setX(new_x);
    setY(new_y);

    ctrl_zone->settings.x = new_x;
    ctrl_zone->settings.y = new_y;

    // todo : check if the shape is inside the bounds
}

