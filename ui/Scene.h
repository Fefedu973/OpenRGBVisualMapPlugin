#ifndef SCENE_H
#define SCENE_H

#include <QGraphicsScene>
#include <QPainter>

class Scene: public QGraphicsScene
{
public:
    Scene(qreal x, qreal y, qreal w, qreal h) : QGraphicsScene(x, y, w, h), w(w), h(h), grid(false), bounds(false) {};

    void SetWidth(int);
    void SetHeight(int);
    void SetGrid(bool);
    void SetBounds(bool);

    protected:
        void drawBackground(QPainter *painter, const QRectF &rect);

    private:
        int w;
        int h;
        bool grid;
        bool bounds;
        inline static const int grid_size = 8;
};

#endif // SCENE_H
