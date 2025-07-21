#include "Grid.h"
#include "math.h"
#include "ControllerZoneItem.h"
#include "OpenRGBVisualMapPlugin.h"

void Grid::Init()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(true);
}

void Grid::ApplySettings(GridSettings* s)
{
    settings = s;

    if(!scene)
    {
        scene = new Scene(settings);
        setScene(scene);
        resize(settings->w, settings->h);

        preview = scene->addPixmap(preview_pixmap);
    }

    setSceneRect(- (settings->w) / 2,
                 - (settings->h) / 2,
                 settings->w * 2,
                 settings->h * 2);

    QRect view(- (settings->w) ,
                 - (settings->h),
                 settings->w*2 ,
                 settings->h*2);

    scene->ApplySettings(settings);
    fitInView(view,Qt::KeepAspectRatio);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    update();
}

void Grid::Clear()
{
    scene->clear();
    ctrl_zone_items.clear();
}

void Grid::ResetItems(std::vector<ControllerZone*> ctrl_zones)
{
    Clear();

    preview = scene->addPixmap(preview_pixmap);
    UpdatePreview(QImage(0, 0, QImage::Format_RGB32));

    for(ControllerZone* ctrl_zone: ctrl_zones)
    {
        ControllerZoneItem* ctrl_zone_item = new ControllerZoneItem(ctrl_zone, settings);
        ctrl_zone_items.push_back(ctrl_zone_item);

        scene->addItem(ctrl_zone_item);

        connect(ctrl_zone_item, &ControllerZoneItem::Released, [=](){
            for(ControllerZoneItem* item : ctrl_zone_items)
            {
                item->Snap();
            }

            emit Changed();
        });

        connect(ctrl_zone_item, &ControllerZoneItem::RectSelectionRequest, [=](){

            std::vector<ControllerZoneItem*> items = GetSelectedItems();

            if(items.size() == 1)
            {
                ControllerZoneItem* start = items.front();
                ControllerZoneItem* end = ctrl_zone_item;

                QRect selection_rect(start->point(), end->point());

                for(ControllerZoneItem* item : ctrl_zone_items)
                {
                    item->setSelected(selection_rect.contains(item->point()));
                }

                emit SelectionChanged(GetSelection());
            }

        });

    }
}

void Grid::SetSelection(std::vector<ControllerZone*> ctrl_zones)
{
    for(ControllerZoneItem* item: ctrl_zone_items)
    {
        bool selected = std::find(ctrl_zones.begin(), ctrl_zones.end(), item->GetControllerZone()) != ctrl_zones.end();
        item->setSelected(selected);
        item->update();
    }
}

void Grid::ClearSelection()
{
    for(ControllerZoneItem* item: ctrl_zone_items)
    {
        item->setSelected(false);
        item->update();
    }
}

void Grid::UpdateItems()
{
    for(ControllerZoneItem* item: ctrl_zone_items)
    {
        item->setX(item->GetControllerZone()->settings.x);
        item->setY(item->GetControllerZone()->settings.y);
        item->update();
    }

    scene->update();

    update();
}

void Grid::UpdatePreview(QImage image)
{
    preview_pixmap.convertFromImage(image);
    preview->setPixmap(preview_pixmap);
    preview->update();
}


void Grid::wheelEvent(QWheelEvent *event)
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

void Grid::mousePressEvent(QMouseEvent *event)
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

void Grid::mouseReleaseEvent(QMouseEvent *event)
{
    setDragMode(QGraphicsView::DragMode::NoDrag);

    QGraphicsView::mouseReleaseEvent(event);

    if(left_button_pressed)
    {
        emit SelectionChanged(GetSelection());
    }

    left_button_pressed = false;
    right_button_pressed = false;
}


void Grid::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:  MoveSelection(-1,  0); break;
    case Qt::Key_Right: MoveSelection( 1,  0); break;
    case Qt::Key_Up:   MoveSelection( 0, -1); break;
    case Qt::Key_Down:  MoveSelection( 0,  1); break;

    default: QGraphicsView::keyPressEvent(event); break;
    }
}

void Grid::MoveSelection(int delta_x, int delta_y)
{
    for(ControllerZoneItem* ctrl_zone_item: ctrl_zone_items)
    {
        if(ctrl_zone_item->isSelected())
        {
            ctrl_zone_item->setX(ctrl_zone_item->x() + delta_x);
            ctrl_zone_item->setY(ctrl_zone_item->y() + delta_y);
            ctrl_zone_item->Snap();
        }
    }

    UpdateItems();
}

std::vector<ControllerZone*> Grid::GetSelection()
{
    std::vector<ControllerZone*> selection;

    for(ControllerZoneItem* ctrl_zone_item: ctrl_zone_items)
    {
        if(ctrl_zone_item->isSelected())
        {
            selection.push_back(ctrl_zone_item->GetControllerZone());
        }
    }

    return selection;
}

std::vector<ControllerZoneItem*> Grid::GetSelectedItems()
{
    std::vector<ControllerZoneItem*> selection;

    for(ControllerZoneItem* ctrl_zone_item: ctrl_zone_items)
    {
        if(ctrl_zone_item->isSelected())
        {
            selection.push_back(ctrl_zone_item);
        }
    }

    return selection;
}
