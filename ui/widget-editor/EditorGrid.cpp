#include "EditorGrid.h"
#include "ControllerZone.h"
#include "LedItem.h"

EditorGrid::EditorGrid(QWidget *parent) : QGraphicsView(parent){
    setStyleSheet("background-color: #534e52;");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(true);
    setFrameShadow(QFrame::Raised);
    setFrameStyle(QFrame::NoFrame);
    // setDragMode(QGraphicsView::ScrollHandDrag);
}

void EditorGrid::ApplySettings(GridSettings* s)
{
    settings = s;

    if(!scene)
    {
        scene = new Scene(settings);
        setScene(scene);
        resize(settings->w * settings->grid_scale_factor, settings->h * settings->grid_scale_factor);
    }

    setSceneRect(- (settings->w * settings->grid_scale_factor) / 2,
                 - (settings->h * settings->grid_scale_factor) / 2,
                 settings->w * settings->grid_scale_factor * 2,
                 settings->h * settings->grid_scale_factor * 2);

    scene->ApplySettings(settings);
}

void EditorGrid::UpdateItems()
{
    for(LedItem* led_item: led_items)
    {
        led_item->update();
    }
    scene->update();
}

void EditorGrid::wheelEvent(QWheelEvent *event)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    qreal factor;

    int angle = event->angleDelta().y();

    if (angle > 0) {
        factor = event->modifiers() == Qt::ControlModifier ? 1.3 : 1.05;
    } else {
        factor = event->modifiers() == Qt::ControlModifier ? 0.7 : 0.95;
    }

    scale(factor, factor);

    event->accept();
}

void EditorGrid::mousePressEvent(QMouseEvent *event)
{
    if(pressed)
    {
        return;
    }

    pressed = true;

    if(event->button() == Qt::RightButton)
    {
        setDragMode(QGraphicsView::DragMode::RubberBandDrag);

        mousePressEvent(new QMouseEvent(QEvent::GraphicsSceneMousePress,
                                        event->pos(), Qt::MouseButton::LeftButton,
                                        Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier));
    }
    else if(event->button() == Qt::LeftButton)
    {
        setDragMode(QGraphicsView::DragMode::ScrollHandDrag);

        mousePressEvent(new QMouseEvent(QEvent::GraphicsSceneMousePress,
                                        event->pos(), Qt::MouseButton::LeftButton,
                                        Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier));
    }

    QGraphicsView::mousePressEvent(event);
}

void EditorGrid::mouseReleaseEvent(QMouseEvent *event)
{
    pressed = false;

    setDragMode(QGraphicsView::DragMode::NoDrag);

    QGraphicsView::mouseReleaseEvent(event);
}

void EditorGrid::CreateLEDItems(CustomShape* shape)
{
    Clear();

    for(LedPosition* led_position: shape->led_positions)
    {
        LedItem* led_item = new LedItem(led_position, settings);

        led_items.push_back(led_item);

        led_item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

        scene->addItem(led_item);

        connect(led_item, &LedItem::Selected, [=](){
            SetSelected(led_position);
            emit ItemSelected(led_position);
        });

        connect(led_item, &LedItem::Moving, [=](){
            emit ItemMoved(led_position);
        });

        connect(led_item, &LedItem::Released, [=](){
            led_item->Restrict(settings->w * settings->grid_scale_factor, settings->h * settings->grid_scale_factor);
            emit ItemMoved(led_position);
        });

        connect(led_item, &LedItem::Restricted, [=](int delta_x, int delta_y){
            for(LedItem* item : led_items)
            {
                if(item->isSelected() && item != led_item)
                {
                    item->MoveBy(delta_x, delta_y);
                }
            }
        });
    }
}

void EditorGrid::Clear()
{
    scene->clear();
    led_items.clear();
}

void EditorGrid::SetSelected(LedPosition* led_position)
{
    selected = led_position;

    for(LedItem* led_item: led_items)
    {
        led_item->SetSelected(selected == led_item->GetLedPosition());
        led_item->update();
    }
}

LedPosition* EditorGrid::GetSelected()
{
    return selected;
}
