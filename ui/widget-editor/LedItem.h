/*---------------------------------------------------------*\
| LedItem.h                                                 |
|                                                           |
|   OpenRGB Visual Map Plugin LED Item                      |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QPainter>
#include <QPen>
#include "ControllerZone.h"
#include "GridSettings.h"

class LedItem: public QObject, public QGraphicsItem
{
    Q_OBJECT;
    Q_INTERFACES(QGraphicsItem);

public:
    LedItem(LedPosition*, GridSettings*);

    QRectF          boundingRect() const;

    void            paint(QPainter*, const QStyleOptionGraphicsItem*,QWidget*);

    LedPosition*    GetLedPosition();

    void            Snap();

signals:
      void Released();
      void RectSelectionRequest();

private:
    LedPosition*    led_position;
    GridSettings*   settings;

    bool            hover           = false;
    bool            pressed         = false;

    const QBrush    default_brush   =  QBrush(QColor("#f2d974"), Qt::BrushStyle::NoBrush);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent*);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*);
    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*) ;
};
