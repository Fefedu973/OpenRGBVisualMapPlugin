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
    static int Show(ControllerZone*);

signals:
    void Cancel();
    void Save();

private slots:
    void on_identify_button_clicked();
    void on_cancel_button_clicked();
    void on_save_button_clicked();
    void on_reset_button_clicked();

    void on_w_spinBox_valueChanged(int);
    void on_h_spinBox_valueChanged(int);
    void on_auto_identify_stateChanged(int);

private:
    explicit WidgetEditor(QWidget *parent = nullptr, ControllerZone* ctrl_zone = nullptr);
    ~WidgetEditor();

    void Update();
    void IdentifySelected();
    void ResetShape();

    Ui::WidgetEditor *ui;
    ControllerZone* ctrl_zone;
    GridSettings* settings;
};

#endif // WIDGETEDITOR_H
