#ifndef EDITORGRID_H
#define EDITORGRID_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QPoint>

#include "Scene.h"
#include "GridSettings.h"
#include "LedItem.h"
#include "ControllerZone.h"

class EditorGrid : public QGraphicsView
{
     Q_OBJECT

public:
    explicit EditorGrid(QWidget *parent) : QGraphicsView(parent){}

    void Init(GridSettings*);
    void OnSettingsChanged();

    void SetSelected(int);
    int GetSelected();

    void CreateLEDItems(CustomShape*);

protected:
    void wheelEvent(QWheelEvent *event) override;

signals:
    void ItemSelected(int);
    void ItemMoved(int);

private:
    GridSettings* settings;
    Scene* scene;
    float scaleFactor = 1.0f;
    int selected = -1;

    std::vector<LedItem*> led_items;

    void Clear();
};

#endif // EDITORGRID_H
