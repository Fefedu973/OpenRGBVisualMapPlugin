#ifndef EDITORGRID_H
#define EDITORGRID_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPoint>

#include "Scene.h"
#include "GridSettings.h"
#include "LedItem.h"
#include "ControllerZone.h"

class EditorGrid : public QGraphicsView
{
     Q_OBJECT

public:
    explicit EditorGrid(QWidget *parent);

    void ApplySettings(GridSettings*);
    void SetSelected(int);
    int GetSelected();
    void CreateLEDItems(CustomShape*);

    void UpdateItems();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

signals:
    void ItemSelected(int);
    void ItemMoved(int);

private:
    GridSettings* settings = nullptr;
    Scene* scene = nullptr;

    int selected = -1;

    std::vector<LedItem*> led_items;

    void Clear();
};

#endif // EDITORGRID_H
