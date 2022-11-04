#ifndef DEVICEWIDGET_H
#define DEVICEWIDGET_H

#include <QWidget>
#include "ControllerZone.h"

namespace Ui {
class DeviceWidget;
}

class DeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceWidget(QWidget *parent = nullptr, ControllerZone* controller_zone = nullptr);
    ~DeviceWidget();

    ControllerZone* getControllerZone();
    void setSelected(bool);
    bool isSelected();

    void setEnabled(bool);
    bool isEnabled();
    void updateName();

private slots:
    void on_enable_toggled(bool);
    void on_select_toggled(bool);
    void on_rename_clicked();

signals:
    void Enabled(bool);
    void Selected(bool);
    void Renamed(QString);

private:
    Ui::DeviceWidget *ui;
    ControllerZone* controller_zone;
};

#endif // DEVICEWIDGET_H
