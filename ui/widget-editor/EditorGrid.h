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
    explicit EditorGrid(QWidget*);
    void ApplySettings(GridSettings*);
    std::vector<LedPosition*> GetSelection();
    void CreateLEDItems(CustomShape*);
    void UpdateItems();

protected:
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void SelectionChanged();
    void Changed();    

private:
    GridSettings* settings = nullptr;
    Scene* scene = nullptr;
    LedPosition* selected = nullptr;
    std::vector<LedItem*> led_items;    
    bool left_button_pressed = false;
    bool right_button_pressed = false;

    void Clear();
};

#endif // EDITORGRID_H
