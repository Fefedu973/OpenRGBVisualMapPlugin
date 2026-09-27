#ifndef ITEMOPTIONS_H
#define ITEMOPTIONS_H

#include <QWidget>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include "ControllerZone.h"

namespace Ui {
class ItemOptions;
}

class ItemOptions : public QWidget
{
    Q_OBJECT

public:
    explicit ItemOptions(QWidget *parent = nullptr);
    ~ItemOptions();

    void SetControllerZone(ControllerZone*);
    void Update();

signals:
    void ItemOptionsChanged() const;
    void ShapeEditRequest(ControllerZone*);

private:
    Ui::ItemOptions *ui;
    ControllerZone* ctrl_zone = nullptr;
    QDoubleSpinBox* affine_values[4]{};
    QCheckBox* affine_flips[2]{};

    void UpdateWidgetsVisibility();

private slots:
    void on_x_spinBox_valueChanged(double);
    void on_y_spinBox_valueChanged(double);
    void on_scale_spinBox_valueChanged(double);
    void on_led_spacing_spinBox_valueChanged(int);
    void on_shape_comboBox_currentIndexChanged(int);
    void on_reverse_checkBox_stateChanged(int);
    void on_identifyButton_clicked();
    void on_edit_shape_button_clicked();

};

#endif // ITEMOPTIONS_H
