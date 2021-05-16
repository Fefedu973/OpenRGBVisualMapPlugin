#include "OpenRGBVisualMapPlugin.h"
#include "TooltipProxy.h"
#include "VisualMapSettingsManager.h"
#include "ZoneManager.h"

bool OpenRGBVisualMapPlugin::DarkTheme = false;
ResourceManager* OpenRGBVisualMapPlugin::RMPointer = nullptr;

OpenRGBPluginInfo OpenRGBVisualMapPlugin::Initialize(bool Dt, ResourceManager *RM)
{
    PInfo.PluginName         = "VisualMap";
    PInfo.PluginDescription  = "Spatial configurator";
    PInfo.PluginLocation     = "TopTabBar";
    PInfo.HasCustom          = true;
    PInfo.PluginLabel        = new QLabel("VisualMap");

    RMPointer                = RM;
    DarkTheme                = Dt;

    return PInfo;
}

QWidget* OpenRGBVisualMapPlugin::CreateGUI(QWidget* parent)
{
    VisualMapSettingsManager::CreateSettingsDirectory();
    OpenRGBVisualMapPlugin::RMPointer->WaitForDeviceDetection();

    ui = new OpenRGBVisualMapTab(parent);

    ui->setStyle(new TooltipProxy(ui->style()));
    ui->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    RMPointer->RegisterDetectionStartCallback(DetectionStart, ui);
    RMPointer->RegisterDetectionEndCallback(DetectionEnd, ui);

    return ui;
}

void OpenRGBVisualMapPlugin::DetectionStart(void* o)
{
    printf("DetectionStart\n");
    ZoneManager::Get()->Clear();

    QMetaObject::invokeMethod((OpenRGBVisualMapTab *)o, "Clear", Qt::QueuedConnection);
}
void OpenRGBVisualMapPlugin::DetectionEnd(void* o)
{
    printf("DetectionEnd\n");
    ZoneManager::Get()->ResetControllerZones();
    QMetaObject::invokeMethod((OpenRGBVisualMapTab *)o, "DeviceListChanged", Qt::QueuedConnection);
}
