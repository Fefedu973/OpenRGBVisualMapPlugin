#ifndef DEVICELIST_H
#define DEVICELIST_H

#include <QWidget>
#include "ControllerZone.h"
#include "DeviceWidget.h"

namespace Ui {
class DeviceList;
}

class DeviceList : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceList(QWidget *parent = nullptr);
    ~DeviceList();

    void Clear();
    void Init(std::vector<ControllerZone*>);
    void SetSelection(std::vector<ControllerZone*>);
    void UpdateControllerState(ControllerZone*);

signals:
    void DeviceAdded(ControllerZone*);
    void DeviceRemoved(ControllerZone*);
    void SelectionChanged(std::vector<ControllerZone*>);

private:
    Ui::DeviceList *ui;
    std::vector<DeviceWidget*> device_widgets;
};

#endif // DEVICELIST_H
