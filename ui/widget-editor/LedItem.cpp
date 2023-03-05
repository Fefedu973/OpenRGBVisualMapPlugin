#include "LedItem.h"

#include "math.h"
#include <QGraphicsSceneMouseEvent>
#include <QString>
#include <QCursor>
#include <QPalette>
#include <QApplication>

LedItem::LedItem(LedPosition* led_position, GridSettings* settings) :
    led_position(led_position),
    settings(settings)
{
    std::string tooltip =
            "<div style=\"display:inline-block; padding:10px; font-weight:bold; background-color:#ffffff; color: #000000\">"
            + std::to_string(led_position->led_num)
            + "</div>";

    setToolTip(QString::fromUtf8(tooltip.c_str()));
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges | ItemAcceptsInputMethod);
    setAcceptHoverEvents(true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
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

    QPalette pal = QApplication::palette();
    QColor col = pal.color(QPalette::Highlight);
    QColor txt_col = pal.color(QPalette::Text);
    QColor inv_col = QColor(0xff - col.red(),0xff - col.green(),0xff - col.blue());

    QBrush brush =  isSelected() ?
                QBrush(inv_col, Qt::BrushStyle::SolidPattern) : hover ?
                    QBrush(col, Qt::BrushStyle::SolidPattern) : default_brush;

    QPen pen(txt_col);
    QPen text_pen(txt_col);
    QFont font;
    font.setPixelSize(4);

    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(brush);
    painter->drawRect(rect);
    painter->setFont(font);
    painter->setPen(text_pen);
    painter->drawText(rect, Qt::AlignCenter, QString::number(led_position->led_num));
}

void LedItem::Snap()
{
    // update led position
    led_position->setX(round(x()));
    led_position->setY(round(y()));

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

    if(event->modifiers() == Qt::ShiftModifier)
    {
        event->accept();
    }
    else
    {
        QGraphicsItem::mousePressEvent(event);
    }
}

void LedItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    pressed = false;
    setZValue(1);
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

