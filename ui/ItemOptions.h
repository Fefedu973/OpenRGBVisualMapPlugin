#ifndef ITEMOPTIONS_H
#define ITEMOPTIONS_H

#include <QWidget>
#include "ZoneManager.h"

namespace Ui {
class ItemOptions;
}

class ItemOptions : public QWidget
{
    Q_OBJECT

public:
    explicit ItemOptions(QWidget *parent = nullptr);
    ~ItemOptions();

    void SetControllerZone(int);
    void Update();

signals:
    void ItemOptionsChanged() const;

private:
    Ui::ItemOptions *ui;
    ControllerZone* zone = nullptr;

private slots:
    void on_x_spinBox_valueChanged(int);
    void on_y_spinBox_valueChanged(int);
    void on_led_spacing_spinBox_valueChanged(int);
    void on_shape_comboBox_currentIndexChanged(int);
    void on_reverse_checkBox_stateChanged(int);
};

#endif // ITEMOPTIONS_H
