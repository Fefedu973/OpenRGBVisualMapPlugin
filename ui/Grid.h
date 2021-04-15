#ifndef GRID_H
#define GRID_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QImage>
#include <QPixmap>
#include <QWheelEvent>

#include "ZoneManager.h"
#include "ControllerZoneItem.h"
#include "Scene.h"
#include "GridOptions.h"

class Grid : public QGraphicsView
{
    Q_OBJECT

public:
    explicit Grid(QWidget *parent = nullptr);

    int GetWidth();
    int GetHeight();

    void SetWidth(int);
    void SetHeight(int);

    void ResetItems();
    void UpdateItems();
    void SetSelected(int);

    void SetSettings(GridSettings);
    void UpdatePreview(QImage* image);

signals:
    void ItemSelected(int);
    void ItemMoved(int);

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    QGraphicsPixmapItem* preview;
    QPixmap preview_pixmap;

    float scaleFactor = 1.0f;

    int w = 128;
    int h = 128;

    std::vector<ControllerZoneItem*> ctrl_zone_items;
    int selected_idx = -1;

    Scene* scene;
};

#endif // GRID_H
