#include "EditorGrid.h"
#include "ControllerZone.h"
#include "LedItem.h"
#include "stdlib.h"

EditorGrid::EditorGrid(QWidget *parent) : QGraphicsView(parent){
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(true);
}

void EditorGrid::ApplySettings(GridSettings* s)
{
    settings = s;

    if(!scene)
    {
        scene = new Scene(settings);
        setScene(scene);
        resize(settings->w, settings->h);

        // auto scale on first run
        if(settings->w > 0 && settings->h > 0)
        {
            qreal w_factor = width() / settings->w;
            qreal h_factor = height() / settings->h;

            qreal factor = std::min<qreal>(w_factor, h_factor);

            scale(factor, factor);
        }

    }

    setSceneRect(- (settings->w) / 2,
                 - (settings->h) / 2,
                 settings->w * 2,
                 settings->h * 2);

    scene->ApplySettings(settings);   
}

void EditorGrid::UpdateItems()
{
    for(LedItem* led_item: led_items)
    {
        led_item->setX(led_item->GetLedPosition()->x());
        led_item->setY(led_item->GetLedPosition()->y());
        led_item->update();
    }

    scene->update();

    update();
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
    if(left_button_pressed)
    {    
        return;
    }

    if(right_button_pressed)
    {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    if(event->button() == Qt::LeftButton)
    {
        left_button_pressed = true;

        setDragMode(QGraphicsView::DragMode::RubberBandDrag);

        mousePressEvent(new QMouseEvent(QEvent::GraphicsSceneMousePress,
                                        event->pos(), QCursor::pos(), Qt::MouseButton::LeftButton,
                                        Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier));

        QGraphicsView::mousePressEvent(event);
    }
    else  if(event->button() == Qt::RightButton)
    {
        right_button_pressed = true;

        setDragMode(QGraphicsView::DragMode::ScrollHandDrag);

        mousePressEvent(new QMouseEvent(QEvent::GraphicsSceneMousePress,
                                        event->pos(), QCursor::pos(), Qt::MouseButton::LeftButton,
                                        Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier));

        QGraphicsView::mousePressEvent(event);
    }
}

void EditorGrid::mouseReleaseEvent(QMouseEvent *event)
{
    if(left_button_pressed)
    {
        emit SelectionChanged();
    }

    left_button_pressed = false;
    right_button_pressed = false;

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

        scene->addItem(led_item);

        connect(led_item, &LedItem::Released, [=](){
            for(LedItem* item : led_items)
            {
                item->Snap();
            }

            emit Changed();
        });


        connect(led_item, &LedItem::RectSelectionRequest, [=](){

            std::vector<LedPosition*> items = GetSelection();
            if(items.size() == 1)
            {

                LedPosition* start = items.front();
                LedPosition* end = led_item->GetLedPosition();
                QRect selection_rect(start->point, end->point);

                for(LedItem* item : led_items)
                {
                    item->setSelected(selection_rect.contains(item->GetLedPosition()->point));
                }

                emit SelectionChanged();
            }

        });

    }
}

void EditorGrid::Clear()
{
    scene->clear();
    led_items.clear();
}

void EditorGrid::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:  MoveSelection(-1,  0); break;
    case Qt::Key_Right: MoveSelection( 1,  0); break;
    case Qt::Key_Up:   MoveSelection( 0, -1); break;
    case Qt::Key_Down:  MoveSelection( 0,  1); break;

    default: QGraphicsView::keyPressEvent(event); break;
    }
}

void EditorGrid::MoveSelection(int delta_x, int delta_y)
{
    for(LedItem* led_item: led_items)
    {
        if(led_item->isSelected())
        {
            led_item->setX(led_item->x() + delta_x);
            led_item->setY(led_item->y() + delta_y);
            led_item->Snap();
        }
    }

    UpdateItems();
}

std::vector<LedPosition*> EditorGrid::GetSelection()
{
    std::vector<LedPosition*> selection;

    for(LedItem* led_item: led_items)
    {
        if(led_item->isSelected())
        {
            selection.push_back(led_item->GetLedPosition());
        }
    }

    return selection;
}




