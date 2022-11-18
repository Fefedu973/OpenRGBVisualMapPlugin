#include "Scene.h"
#include "OpenRGBVisualMapPlugin.h"

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
        QColor line_color = OpenRGBVisualMapPlugin::DarkTheme ?
                    QColor(0xFF, 0xFF, 0xFF, 0x3F) : QColor(0x00, 0x00, 0x00, 0x3F);

        QVarLengthArray<QLineF, 64> grid_lines;

        for (qreal x = left; x < rect.right(); x += (settings->grid_size))
            grid_lines.append(QLineF(x, rect.top(), x, rect.bottom()));
        for (qreal y = top; y < rect.bottom(); y += (settings->grid_size))
            grid_lines.append(QLineF(rect.left(), y, rect.right(), y));

        painter->setPen(QPen(line_color, 0.1));
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
        painter->setPen(QPen(QColor(0xFF, 0x63, 0x00, 0xFF), 0.1));
        painter->drawLines(bound_lines.data(), bound_lines.size());
    }
}
