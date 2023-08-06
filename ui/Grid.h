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

    void Init();

    void ResetItems(std::vector<ControllerZone*>);
    void UpdateItems();
    void ClearSelection();
    void ApplySettings(GridSettings* settings);

    void SetSelection(std::vector<ControllerZone*>);
    std::vector<ControllerZone*> GetSelection();
    std::vector<ControllerZoneItem*> GetSelectedItems();
    void Clear();
    void MoveSelection(int, int);
    void UpdatePreview(QImage image);

signals:
    void SelectionChanged(std::vector<ControllerZone*>);
    void Changed();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    GridSettings* settings;
    std::vector<ControllerZoneItem*> ctrl_zone_items;
    Scene* scene = nullptr;
    bool left_button_pressed = false;
    bool right_button_pressed = false;

    QGraphicsPixmapItem* preview;
    QPixmap preview_pixmap;

};

#endif // GRID_H
