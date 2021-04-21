#include "Scene.h"

void Scene::ApplySettings(GridSettings* settings)
{    
    this->settings = settings;
    invalidate(sceneRect());
    setSceneRect(0,0,settings->w * settings->grid_scale_factor, settings->h * settings->grid_scale_factor);
    update();
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{    
    painter->setRenderHints(QPainter::Antialiasing);

    qreal left = int(rect.left()) - (int(rect.left()) % (settings->grid_size * settings->grid_scale_factor));
    qreal top = int(rect.top()) - (int(rect.top()) % (settings->grid_size * settings->grid_scale_factor));

    if(settings->show_grid)
    {
        QVarLengthArray<QLineF, 64> grid_lines;

        for (qreal x = left; x < rect.right(); x += (settings->grid_size * settings->grid_scale_factor))
            grid_lines.append(QLineF(x, rect.top(), x, rect.bottom()));
        for (qreal y = top; y < rect.bottom(); y += (settings->grid_size * settings->grid_scale_factor))
            grid_lines.append(QLineF(rect.left(), y, rect.right(), y));

        painter->setPen(QPen(QColor(0xFF, 0xFF, 0xFF, 0x18), 0.1));
        painter->drawLines(grid_lines.data(), grid_lines.size());
    }

    // Bounds
    if(settings->show_bounds)
    {
        QVarLengthArray<QLineF, 64> bound_lines;

        bound_lines.append(QLineF(0, 0, settings->w * settings->grid_scale_factor, 0));
        bound_lines.append(QLineF(0, 0, 0, settings->h * settings->grid_scale_factor));
        bound_lines.append(QLineF(0, settings->h * settings->grid_scale_factor, settings->w * settings->grid_scale_factor, settings->h * settings->grid_scale_factor));
        bound_lines.append(QLineF(settings->w * settings->grid_scale_factor, 0, settings->w * settings->grid_scale_factor, settings->h * settings->grid_scale_factor));

        painter->setPen(QPen(QColor(0xC7, 0x95, 0x6D, 0xA0), 0.1));
        painter->drawLines(bound_lines.data(), bound_lines.size());
    }
}
