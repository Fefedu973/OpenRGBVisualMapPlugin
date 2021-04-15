#ifndef GRIDOPTIONS_H
#define GRIDOPTIONS_H

#include <QWidget>

namespace Ui {
class GridOptions;
}

struct GridSettings
{
    int w;
    int h;
    bool show_grid;
    bool show_bounds;
};

class GridOptions : public QWidget
{
    Q_OBJECT

public:
    explicit GridOptions(QWidget *parent = nullptr);
    ~GridOptions();

    GridSettings GetSettings();
    void SetSettings(GridSettings);

signals:
    void OptionsChanged(GridSettings);

private slots:
    void on_w_spinBox_valueChanged(int);
    void on_h_spinBox_valueChanged(int);
    void on_grid_checkBox_stateChanged(int);
    void on_bounds_checkBox_stateChanged(int);

private:
    Ui::GridOptions *ui;
    GridSettings settings = {128,128,false,false};

    void Update();
};

#endif // GRIDOPTIONS_H
