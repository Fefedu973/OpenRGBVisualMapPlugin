#include "Scene.h"
#include <QApplication>
#include <QPalette>

void Scene::ApplySettings(GridSettings* settings)
{    
    this->settings = settings;
    invalidate(sceneRect());
    setSceneRect(0,0,settings->w, settings->h);
    update();
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{    
    painter->setRenderHints(QPainter::Antialiasing);

    qreal left = int(rect.left()) - (int(rect.left()) % (settings->grid_size));
    qreal top = int(rect.top()) - (int(rect.top()) % (settings->grid_size));

    if(settings->show_grid)
    {
        QPalette pal = QApplication::palette();
        QColor col = pal.color(QPalette::Text);
        col.setAlpha(GRID_LINE_ALPHA);

        QVarLengthArray<QLineF, 64> grid_lines;

        for (qreal x = left; x < rect.right(); x += (settings->grid_size))
            grid_lines.append(QLineF(x, rect.top(), x, rect.bottom()));
        for (qreal y = top; y < rect.bottom(); y += (settings->grid_size))
            grid_lines.append(QLineF(rect.left(), y, rect.right(), y));

        painter->setPen(QPen(col, GRID_LINE_WIDTH));
        painter->drawLines(grid_lines.data(), grid_lines.size());
    }

    // Bounds
    if(settings->show_bounds)
    {
        QVarLengthArray<QLineF, 64> bound_lines;

        bound_lines.append(QLineF(0, 0, settings->w, 0));
        bound_lines.append(QLineF(0, 0, 0, settings->h));
        bound_lines.append(QLineF(0, settings->h, settings->w, settings->h));
        bound_lines.append(QLineF(settings->w, 0, settings->w, settings->h));
        QPalette pal = QApplication::palette();
        QColor col = pal.color(QPalette::Highlight);
        QPen pen(col, BOUNDS_LINE_WIDTH);
        painter->setPen(pen);
        painter->drawLines(bound_lines.data(), bound_lines.size());
    }
}
