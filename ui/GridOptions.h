#ifndef GRIDOPTIONS_H
#define GRIDOPTIONS_H

#include <QWidget>
#include "GridSettings.h"

namespace Ui {
class GridOptions;
}

class GridOptions : public QWidget
{
    Q_OBJECT

public:
    explicit GridOptions(QWidget *parent = nullptr);
    ~GridOptions();

    void Init(GridSettings* settings);

    void SetSettings(GridSettings*);

signals:
    void SettingsChanged();
    void AutoResizeRequest();

private slots:
    void on_w_spinBox_valueChanged(int);
    void on_h_spinBox_valueChanged(int);
    void on_grid_checkBox_stateChanged(int);
    void on_bounds_checkBox_stateChanged(int);
    void on_auto_load_checkBox_stateChanged(int);
    void on_auto_register_checkBox_stateChanged(int);
    void on_auto_resize_clicked();

    void on_unregister_members_checkBox_stateChanged(int arg1);

private:
    Ui::GridOptions *ui;
    GridSettings* settings;

    void Update();
};

#endif // GRIDOPTIONS_H
