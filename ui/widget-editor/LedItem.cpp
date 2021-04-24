#include "LedItem.h"

#include "math.h"
#include <QGraphicsSceneMouseEvent>
#include <QString>
#include <QCursor>

LedItem::LedItem(LedPosition* led_position, GridSettings* settings) :
    led_position(led_position),
    settings(settings)
{
    std::string tooltip =
            "<div style=\"display:inline-block; padding:10px; font-weight:bold; background-color:#ffffff; color: #000000\">"
            + std::to_string(led_position->led_num)
            + "</div>";

    setToolTip(QString::fromUtf8(tooltip.c_str()));
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIsFocusable | ItemSendsScenePositionChanges | ItemAcceptsInputMethod);
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);
    setScale(0.1);
    setX(led_position->x());
    setY(led_position->y());
    setZValue(1);
}

QRectF LedItem::boundingRect() const
{
    return QRectF(0, 0, 10, 10);
}

void LedItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    QRectF rect = boundingRect();

    QPen pen(QColor(0, 0, 0, 0x80), 0.05);

    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);  

    QBrush brush =  isSelected() ? selected_brush : hasFocus() ? focus_brush:  hover ? hover_brush :  default_brush;
    painter->setBrush(brush);

    painter->drawRect(rect);
    painter->setBrush(QColor("#534e52"));

    QPen text_pen(QColor("#534e52"));

    QFont font;
    font.setPixelSize(4);
    painter->setFont(font);

    painter->setPen(text_pen);
    painter->drawText(rect, Qt::AlignCenter, QString::number(led_position->led_num));
}

void LedItem::Restrict()
{
    //printf("LedItem::Restrict %d \n", led_position->led_num);
    int round_x = round(x());
    int round_y = round(y());

    // restrict to bounds
    int new_x = std::min<int>(std::max<int>(0,round_x), settings->w - 1);
    int new_y = std::min<int>(std::max<int>(0,round_y), settings->h - 1);

    // update led position
    led_position->setX(new_x);
    led_position->setY(new_y);

    setX(led_position->x());
    setY(led_position->y());

    update();
}

LedPosition* LedItem::GetLedPosition()
{
    return led_position;
}

void LedItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    hover = true;
    QGraphicsItem::hoverEnterEvent( event );
}

void LedItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    hover = false;
    QGraphicsItem::hoverLeaveEvent( event );
}

void LedItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = true;
    setZValue(10);
    setCursor(Qt::ClosedHandCursor);
    QGraphicsItem::mousePressEvent(event);
}

void LedItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = false;
    setZValue(1);
    setCursor(Qt::OpenHandCursor);
    emit Released();
    QGraphicsItem::mouseReleaseEvent(event);
}

