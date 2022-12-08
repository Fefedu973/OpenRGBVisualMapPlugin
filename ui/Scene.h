#ifndef SCENE_H
#define SCENE_H

#include <QGraphicsScene>
#include <QPainter>
#include "GridSettings.h"

#define GRID_LINE_WIDTH   0.1
#define GRID_LINE_ALPHA   0x40
#define BOUNDS_LINE_WIDTH 0.2

class Scene: public QGraphicsScene
{
public:
    Scene(GridSettings* settings) : QGraphicsScene(0, 0, settings->w, settings->h), settings(settings) {};
    void ApplySettings(GridSettings*);

    protected:
        void drawBackground(QPainter *painter, const QRectF &rect);

    private:
        GridSettings* settings;
};

#endif // SCENE_H
