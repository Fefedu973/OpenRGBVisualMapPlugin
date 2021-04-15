#include "ControllerZoneItem.h"
#include "math.h"
#include <QString>
#include <QCursor>

ControllerZoneItem::ControllerZoneItem(ControllerZone* ctrl_zone) :
   ctrl_zone(ctrl_zone)
{
    setFlag(ItemIsMovable);
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
    case ControllerZoneSettings::HORIZONTAL_LINE :
        return QRectF(0, 0, ctrl_zone->led_count() * ctrl_zone->settings.led_spacing, 2);
    case ControllerZoneSettings::VERTICAL_LINE :
        return QRectF(0, 0, 2, ctrl_zone->led_count() * ctrl_zone->settings.led_spacing);
    }

    return QRectF(0, 0, 1, 1);
}

void ControllerZoneItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

    if(!moving)
    {
        setPos(ctrl_zone->settings.x, ctrl_zone->settings.y);
    }

    QRectF rect = boundingRect();
    QPen pen(QColor(0, 0, 0, 0x00));

    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);


    QBrush brush = pressed ? moving_brush: selected ? selected_brush : default_brush;
    painter->setBrush(brush);

    switch(ctrl_zone->settings.shape)
    {
    case ControllerZoneSettings::HORIZONTAL_LINE :
        painter->fillRect(rect,brush);
    case ControllerZoneSettings::VERTICAL_LINE :
        painter->fillRect(rect, brush);
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

void ControllerZoneItem::ControllerZoneItem::SetSelected(bool value)
{
    selected = value;
}


void ControllerZoneItem::Restrict(int w, int h)
{
    // restrict to 0 - 0
    int new_x = std::min<int>(std::max<int>(0,x()), w);
    int new_y = std::min<int>(std::max<int>(0,y()), h);

    setX(new_x);
    setY(new_y);

    ctrl_zone->settings.x = new_x;
    ctrl_zone->settings.y = new_y;

    // todo : check if the shape is inside the bounds
}

