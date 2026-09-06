#ifndef GRID_H
#define GRID_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QImage>
#include <QWheelEvent>

#include "ControllerZoneItem.h"
#include "Scene.h"
#include "GridSettings.h"

/*---------------------------------------------------------*\
| Stop zooming out where the grid stops being drawn, so it  |
| never blinks out at the end of the range                  |
\*---------------------------------------------------------*/
#define GRID_MIN_ZOOM GRID_MIN_SPACING

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
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
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
    bool fitted = false;

    void FitToView();

    QImage preview_image;

};

#endif // GRID_H
