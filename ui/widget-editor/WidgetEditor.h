#ifndef WIDGETEDITOR_H
#define WIDGETEDITOR_H

#include <QWidget>
#include "GridSettings.h"
#include "ControllerZone.h"

namespace Ui {
class WidgetEditor;
}

class WidgetEditor : public QWidget
{
    Q_OBJECT

public:
    static int Show(ControllerZone*, std::vector<ControllerZone*>);

signals:
    void Cancel();
    void Save();

private slots:
    void on_identify_button_clicked();
    void on_cancel_button_clicked();
    void on_save_button_clicked();
    void on_reset_button_clicked();
    void on_copy_shape_button_clicked();    
    void on_rotate_button_clicked();
    void on_v_flip_button_clicked();
    void on_h_flip_button_clicked();
    void on_w_spinBox_valueChanged(int);
    void on_h_spinBox_valueChanged(int);
    void on_auto_identify_stateChanged(int);
    void on_v_line_button_clicked();
    void on_h_line_button_clicked();
    void on_grow_button_clicked();
    void on_shrink_button_clicked();
    void on_circle_button_clicked();
    void on_square_button_clicked();
    void on_zigzag_button_clicked();
    void on_sawtooth_button_clicked();
    void on_undo_button_clicked();
    void on_auto_resize_button_clicked();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    explicit WidgetEditor(QWidget *parent = nullptr, ControllerZone* ctrl_zone = nullptr);
    ~WidgetEditor();

    CustomShape* temp_shape = nullptr;

    Ui::WidgetEditor *ui;
    ControllerZone* ctrl_zone;
    std::vector<ControllerZone*> other_zones;
    GridSettings* settings;

    std::vector<CustomShape*> states;

    void UpdateWidgetsValues();
    void IdentifySelected();
    void InitShape();
    void AutoResize();
    QPointF GetCenter(std::vector<LedPosition*>);

    bool StateChanged();
    void SaveState();
    void Undo();
    void RestoreState(CustomShape* shape);
};

#endif // WIDGETEDITOR_H
