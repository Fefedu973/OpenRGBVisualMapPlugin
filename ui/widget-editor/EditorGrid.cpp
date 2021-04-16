#include "EditorGrid.h"
#include "ControllerZone.h"
#include "LedItem.h"

void EditorGrid::Init(GridSettings* s)
{
    settings = s;

    setStyleSheet("background-color: #534e52;");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resize(settings->w, settings->h);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scene = new Scene(settings);
    scene->setSceneRect(0, 0, settings->w, settings->h);

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
    for(int unsigned led_num = 0; led_num < shape->led_positions.size(); led_num++)
    {
        LedItem* item = new LedItem(led_num, shape->led_positions[led_num]);

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
            item->Restrict(settings->w, settings->h);
            emit ItemMoved(led_num);
        });

        printf("LED Item #%d added at [%d,%d]\n", led_num, shape->led_positions[led_num]->x(), shape->led_positions[led_num]->y());
    }
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
