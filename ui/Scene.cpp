#include "Scene.h"

void Scene::SetWidth(int value)
{
    w = value;
}
void Scene::SetHeight(int value)
{
    h = value;
}

void Scene::SetGrid(bool value)
{
    grid = value;
}

void Scene::SetBounds(bool value)
{
    bounds = value;
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{    
    painter->setRenderHints(QPainter::Antialiasing);

    qreal left = int(rect.left()) - (int(rect.left()) % grid_size);
    qreal top = int(rect.top()) - (int(rect.top()) % grid_size);

    if(grid)
    {
        QVarLengthArray<QLineF, 64> grid_lines;

        for (qreal x = left; x < rect.right(); x += grid_size)
            grid_lines.append(QLineF(x, rect.top(), x, rect.bottom()));
        for (qreal y = top; y < rect.bottom(); y += grid_size)
            grid_lines.append(QLineF(rect.left(), y, rect.right(), y));


        painter->setPen(QPen(QColor(0xFF, 0xFF, 0xFF, 0x18), 1));
        painter->drawLines(grid_lines.data(), grid_lines.size());
    }

    // Bounds
    if(bounds)
    {
        QVarLengthArray<QLineF, 64> bound_lines;

        bound_lines.append(QLineF(0, 0, w, 0));
        bound_lines.append(QLineF(0, 0, 0, h));
        bound_lines.append(QLineF(0, h, w, h));
        bound_lines.append(QLineF(w, 0, w, h));

        painter->setPen(QPen(QColor(0xC7, 0x95, 0x6D, 0x80), 1));
        painter->drawLines(bound_lines.data(), bound_lines.size());
    }

}
