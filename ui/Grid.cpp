#include "Grid.h"
#include "math.h"

#include "RGBController.h"
#include "ControllerZoneItem.h"

Grid::Grid(QWidget *parent) : QGraphicsView(parent)
{

    setStyleSheet("background-color: #534e52;");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resize(w, h);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scene = new Scene(0,0,w,h);
    scene->setSceneRect(0,0,w,h);

    preview = scene->addPixmap(preview_pixmap);

    setScene(scene);

    setFrameShadow(QFrame::Raised);
    setFrameStyle(QFrame::NoFrame);

    scale(scaleFactor, scaleFactor);
}

int Grid::GetWidth()
{
    return w;
}

int Grid::GetHeight()
{
    return h;
}

void Grid::SetWidth(int value)
{
    w = value;
}
void Grid::SetHeight(int value)
{
    h = value;
}

void Grid::SetSettings(GridSettings settings)
{
    SetHeight(settings.h);
    SetWidth(settings.w);

    scene->SetHeight(settings.h);
    scene->SetWidth(settings.w);
    scene->SetGrid(settings.show_grid);
    scene->SetBounds(settings.show_bounds);

    scene->invalidate(scene->sceneRect());

    scene->setSceneRect(0,0,w,h);
    scene->update();
}

void Grid::ResetItems()
{
    scene->clear();

    preview = scene->addPixmap(preview_pixmap);

    ctrl_zone_items.clear();

    std::vector<ControllerZone*> zones = ZoneManager::Get()->GetAvailableZones();

    for(unsigned int i = 0; i < zones.size(); i++)
    {        
        ControllerZoneItem* item = new ControllerZoneItem(zones[i]);
        ctrl_zone_items.push_back(item);

        item->setVisible(ZoneManager::Get()->HasZone(i));

        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

        scene->addItem(item);

        connect(item, &ControllerZoneItem::Selected, [=](){
            SetSelected(i);
            emit ItemSelected(i);
        });

        connect(item, &ControllerZoneItem::Moved, [=](){
            emit ItemMoved(i);
        });
    }
}

void Grid::SetSelected(int idx)
{
    for(unsigned int i = 0; i < ctrl_zone_items.size(); i++)
    {
        ctrl_zone_items[i]->SetSelected((int)i == idx);
        ctrl_zone_items[i]->update();
    }
}

void Grid::UpdateItems()
{
    for(unsigned int i = 0; i < ctrl_zone_items.size(); i++)
    {
        ctrl_zone_items[i]->update();
    }
}

void Grid::UpdatePreview(QImage* image)
{
    //QPixmap new_pixmap;
    preview_pixmap.convertFromImage(*image);
    preview->setPixmap(preview_pixmap);
    preview->update();
}

void Grid::wheelEvent(QWheelEvent *event)
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
