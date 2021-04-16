#ifndef GRID_H
#define GRID_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QImage>
#include <QPixmap>
#include <QWheelEvent>

#include "ControllerZoneItem.h"
#include "Scene.h"
#include "GridSettings.h"

class Grid : public QGraphicsView
{
    Q_OBJECT

public:
    explicit Grid(QWidget *parent) : QGraphicsView(parent){}

    void Init(GridSettings*);

    void ResetItems();
    void UpdateItems();
    void SetSelected(int);

    void OnSettingsChanged();
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

    GridSettings* settings;

    std::vector<ControllerZoneItem*> ctrl_zone_items;
    int selected_idx = -1;

    Scene* scene;
};

#endif // GRID_H
