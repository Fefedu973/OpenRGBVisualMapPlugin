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

    void ResetItems(std::vector<ControllerZone*>);
    void UpdateItems();
    void ClearSelection();
    void ApplySettings(GridSettings* settings);
    void UpdatePreview(QImage image);

    void SetSelection(std::vector<ControllerZone*>);
    std::vector<ControllerZoneItem*> GetSelection();
    void Clear();
    void MoveSelection(int, int);

signals:
    void SelectionChanged();
    void Changed();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QGraphicsPixmapItem* preview;
    QPixmap preview_pixmap;
    GridSettings* settings;
    std::vector<ControllerZoneItem*> ctrl_zone_items;
    Scene* scene = nullptr;
    bool left_button_pressed = false;
    bool right_button_pressed = false;


};

#endif // GRID_H
