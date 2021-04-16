#ifndef SCENE_H
#define SCENE_H

#include <QGraphicsScene>
#include <QPainter>
#include "GridSettings.h"

class Scene: public QGraphicsScene
{
public:
    Scene(GridSettings* settings) : QGraphicsScene(0, 0, settings->w, settings->h), settings(settings) {};
    void OnSettingsChanged();

    protected:
        void drawBackground(QPainter *painter, const QRectF &rect);

    private:
        GridSettings* settings;
};

#endif // SCENE_H
