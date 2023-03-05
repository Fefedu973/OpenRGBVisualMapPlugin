#include "ControllerZoneItem.h"
#include "math.h"
#include <QString>
#include <QCursor>
#include <QPalette>
#include <QApplication>

ControllerZoneItem::ControllerZoneItem(ControllerZone* ctrl_zone, GridSettings* settings) :
    ctrl_zone(ctrl_zone),
    settings(settings)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges | ItemAcceptsInputMethod);
    setAcceptHoverEvents(true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    setPos(ctrl_zone->settings.x, ctrl_zone->settings.y);

    std::string tooltip =
            "<div style=\"display:inline-block; padding:10px; font-weight:bold; background-color:#ffffff; color: #000000\">"
            + ctrl_zone->full_display_name()
            + "</div>";

    setToolTip(QString::fromUtf8(tooltip.c_str()));

    setCursor(Qt::OpenHandCursor);
}

QRectF ControllerZoneItem::boundingRect() const
{
    switch(ctrl_zone->settings.shape)
    {
    case HORIZONTAL_LINE :
        return QRectF(-0.1, -0.1, 0.1 + ctrl_zone->led_count() * ctrl_zone->settings.led_spacing, 1.1);
    case VERTICAL_LINE :
        return QRectF(-0.1, -0.1, 1.1, 0.1 + ctrl_zone->led_count() * ctrl_zone->settings.led_spacing);
    case CUSTOM:
        return QRectF(-0.1, -0.1, 0.1 + ctrl_zone->settings.custom_shape->w, 0.1 + ctrl_zone->settings.custom_shape->h);
    }

    return QRectF(-0.1, -0.1, 1.1, 1.1);
}

void ControllerZoneItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    setZValue(ctrl_zone->isCustomShape() ? -ctrl_zone->settings.custom_shape->h * ctrl_zone->settings.custom_shape->w :
                                           -ctrl_zone->led_count() * ctrl_zone->settings.led_spacing);


    QPalette pal = QApplication::palette();
    QColor col = pal.color(QPalette::Highlight);
    QColor txt_col = pal.color(QPalette::Text);
    QColor inv_col = QColor(0xff - col.red(),0xff - col.green(),0xff - col.blue());

    QBrush brush =  isSelected() ?
                QBrush(inv_col, Qt::BrushStyle::SolidPattern) : hover ?
                    QBrush(col, Qt::BrushStyle::SolidPattern) : default_brush;

    painter->setBrush(brush);

    QPen pen(txt_col, ITEM_BORDER_WIDTH);

    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    if(ctrl_zone->isCustomShape())
    {
        for(LedPosition* point : ctrl_zone->settings.custom_shape->led_positions)
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
        printf("[OpenRGBVisualMapPlugin] Unsupported shape\n");
    }
}

void ControllerZoneItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = true;
    setCursor(Qt::ClosedHandCursor);

    if(event->modifiers() == Qt::ShiftModifier)
    {
        event->accept();
    }
    else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}

void ControllerZoneItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = false;
    setCursor(Qt::OpenHandCursor);

    emit Released();

    if(event->modifiers() == Qt::ShiftModifier)
    {
        emit RectSelectionRequest();
        event->accept();
    }
    else
    {
        QGraphicsItem::mouseReleaseEvent(event);
    }
}

void ControllerZoneItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    hover = true;
    QGraphicsItem::hoverEnterEvent( event );
}

void ControllerZoneItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    hover = false;
    QGraphicsItem::hoverLeaveEvent( event );
}

void ControllerZoneItem::Snap()
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

    // Snap to grid
    new_x /= 10;
    new_y /= 10;

    setX(new_x);
    setY(new_y);

    ctrl_zone->settings.x = new_x;
    ctrl_zone->settings.y = new_y;
}

ControllerZone* ControllerZoneItem::GetControllerZone()
{
    return ctrl_zone;
}

QPoint ControllerZoneItem::point()
{
    //return QPoint(ctrl_zone->settings.x, ctrl_zone->settings.y);
    return QPoint(x(),y());
}
