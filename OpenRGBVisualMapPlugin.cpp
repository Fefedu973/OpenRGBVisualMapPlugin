#include "OpenRGBVisualMapPlugin.h"
#include "OpenRGBVisualMapTab.h"
#include "TooltipProxy.h"

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
    OpenRGBVisualMapPlugin::RMPointer->WaitForDeviceDetection();

    OpenRGBVisualMapTab* pluginGUI = new OpenRGBVisualMapTab(parent);

    pluginGUI->setStyle(new TooltipProxy(pluginGUI->style()));
    pluginGUI->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    return pluginGUI;
}
