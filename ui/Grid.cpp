#include "Grid.h"
#include "math.h"

#include "RGBController.h"
#include "ControllerZoneItem.h"

void Grid::Init(GridSettings* s)
{
    settings = s;

    scene = new Scene(settings);
    scene->setSceneRect(0,0, settings->w, settings->h);
    preview = scene->addPixmap(preview_pixmap);

    setStyleSheet("background-color: #534e52;");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resize(settings->w, settings->h );

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(true);
    setDragMode(QGraphicsView::ScrollHandDrag);

    setScene(scene);
    setSceneRect(QRect(-settings->w / 2,-settings->h / 2, settings->w *2, settings->h*2));

    setFrameShadow(QFrame::Raised);
    setFrameStyle(QFrame::NoFrame);

    scale(scaleFactor, scaleFactor);
}

void Grid::OnSettingsChanged()
{
    setSceneRect(QRect(-settings->w / 2,-settings->h / 2, settings->w *2, settings->h*2));
    scene->OnSettingsChanged();
}

void Grid::ResetItems(std::vector<ControllerZone*> ctrl_zones)
{
    scene->clear();

    preview = scene->addPixmap(preview_pixmap);
    UpdatePreview(new QImage(0, 0, QImage::Format_RGB32));

    ctrl_zone_items.clear();

    for(unsigned int i = 0; i < ctrl_zones.size(); i++)
    {        
        ControllerZoneItem* item = new ControllerZoneItem(ctrl_zones[i]);
        ctrl_zone_items.push_back(item);

        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

        scene->addItem(item);

        item->SetSelected(item->GetControllerZone() == selected_ctrl_zone);

        connect(item, &ControllerZoneItem::Selected, [=](){
            SetSelected(item->GetControllerZone());
            emit ItemSelected(item->GetControllerZone());
        });

        connect(item, &ControllerZoneItem::Moved, [=](){
            item->Restrict(settings->w,settings->h);
            emit ItemMoved(item->GetControllerZone());
        });
    }
}

void Grid::SetSelected(ControllerZone* ctrl_zone)
{
    selected_ctrl_zone = ctrl_zone;

    for(ControllerZoneItem* item: ctrl_zone_items)
    {
        item->SetSelected(item->GetControllerZone() == selected_ctrl_zone);
        item->update();
    }    
}

void Grid::ClearSelection()
{
    SetSelected(nullptr);
}

void Grid::UpdateItems()
{
    for(ControllerZoneItem* item: ctrl_zone_items)
    {
        item->update();
    }
    scene->update();
}

void Grid::UpdatePreview(QImage* image)
{
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
