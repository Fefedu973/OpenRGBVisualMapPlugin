/*---------------------------------------------------------*\
| EditorGrid.h                                              |
|                                                           |
|   OpenRGB Visual Map Plugin Editor Grid                   |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <QChar>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPoint>
#include <QWheelEvent>
#include "ControllerZone.h"
#include "GridSettings.h"
#include "LedItem.h"
#include "Scene.h"

class EditorGrid : public QGraphicsView
{
     Q_OBJECT

public:
    explicit                    EditorGrid(QWidget*);

    void                        ApplySettings(GridSettings*);
    std::vector<LedPosition*>   GetSelection();
    void                        CreateLEDItems(CustomShape*);
    void                        UpdateItems();

protected:
    void wheelEvent(QWheelEvent*)               override;
    void mousePressEvent(QMouseEvent *event)    override;
    void mouseReleaseEvent(QMouseEvent *event)  override;
    void keyPressEvent(QKeyEvent *event)        override;

signals:
    void SelectionChanged();
    void Changed();    

private:
    std::vector<LedItem*>   led_items;    
    GridSettings*           settings                = nullptr;
    Scene*                  scene                   = nullptr;

    bool                    left_button_pressed     = false;
    bool                    right_button_pressed    = false;

    void Clear();
    void MoveSelection(int, int);
};
