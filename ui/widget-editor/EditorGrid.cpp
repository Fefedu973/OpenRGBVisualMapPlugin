#include "EditorGrid.h"
#include "ControllerZone.h"
#include "LedItem.h"

void EditorGrid::Init(GridSettings* s)
{
    settings = s;

    setStyleSheet("background-color: #534e52;");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resize(settings->w * settings->grid_scale_factor, settings->h * settings->grid_scale_factor);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scene = new Scene(settings);
    scene->setSceneRect(0,
                        0,
                        settings->w * settings->grid_scale_factor,
                        settings->h * settings->grid_scale_factor);

    setSceneRect(- (settings->w * settings->grid_scale_factor) / 2,
                 - (settings->h * settings->grid_scale_factor) / 2,
                 settings->w * settings->grid_scale_factor * 2,
                 settings->h * settings->grid_scale_factor * 2);

    setScene(scene);

    setFrameShadow(QFrame::Raised);
    setFrameStyle(QFrame::NoFrame);

    scale(scaleFactor, scaleFactor);
}

void EditorGrid::OnSettingsChanged()
{
    scene->OnSettingsChanged();
}

void EditorGrid::wheelEvent(QWheelEvent *event)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    int angle = event->angleDelta().y();

    qreal factor;

    if (angle > 0) {
        factor = 1.05;
    } else {
        factor = 0.95;
    }

    scale(factor, factor);

    event->accept();
}

void EditorGrid::CreateLEDItems(CustomShape* shape)
{
    Clear();

    for(int unsigned led_num = 0; led_num < shape->led_positions.size(); led_num++)
    {
        LedItem* item = new LedItem(led_num, shape->led_positions[led_num], settings);

        led_items.push_back(item);

        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

        scene->addItem(item);

        connect(item, &LedItem::Selected, [=](){
            SetSelected(led_num);
            emit ItemSelected(led_num);
        });

        connect(item, &LedItem::Moving, [=](){
            emit ItemMoved(led_num);
        });

        connect(item, &LedItem::Released, [=](){
            item->Restrict(settings->w * settings->grid_scale_factor, settings->h * settings->grid_scale_factor);
            emit ItemMoved(led_num);
        });
    }
}

void EditorGrid::Clear()
{
    scene->clear();
    led_items.clear();
}

void EditorGrid::SetSelected(int idx)
{
    selected = idx;

    for(unsigned int i = 0; i < led_items.size(); i++)
    {
        led_items[i]->SetSelected((int)i == idx);
        led_items[i]->update();
    }
}

int EditorGrid::GetSelected()
{
    return selected;
}
