#include "OpenRGBVisualMapPlugin.h"
#include "OpenRGBVisualMapTab.h"
#include "TooltipProxy.h"

bool OpenRGBVisualMapPlugin::DarkTheme = false;
ResourceManager* OpenRGBVisualMapPlugin::RMPointer = nullptr;

QLabel* TabLabel()
{
    QLabel* Label = new QLabel();
    Label->setText("VisualMap");
    return Label;
}

OpenRGBPluginInfo OpenRGBVisualMapPlugin::Initialize(bool Dt, ResourceManager *RM)
{
    OpenRGBVisualMapPlugin::PInfo.PluginName         = "VisualMap";
    OpenRGBVisualMapPlugin::PInfo.PluginDescription  = "Spatial configurator";
    OpenRGBVisualMapPlugin::PInfo.PluginLocation     = "TopTabBar";

    OpenRGBVisualMapPlugin::PInfo.HasCustom   = true;
    OpenRGBVisualMapPlugin::DarkTheme = Dt;
    OpenRGBVisualMapPlugin::PInfo.PluginLabel = TabLabel();
    OpenRGBVisualMapPlugin::RMPointer = RM;

    return OpenRGBVisualMapPlugin::PInfo;
}

QWidget* OpenRGBVisualMapPlugin::CreateGUI(QWidget* parent)
{
    OpenRGBVisualMapPlugin::RMPointer->WaitForDeviceDetection();        
    OpenRGBVisualMapTab* pluginGUI = new OpenRGBVisualMapTab(parent);
    pluginGUI->setStyle(new TooltipProxy(pluginGUI->style()));
    pluginGUI->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    return pluginGUI;
}

