#include "OpenRGBVisualMapPlugin.h"
#include "TooltipProxy.h"
#include "VisualMapSettingsManager.h"

ResourceManagerInterface* OpenRGBVisualMapPlugin::RMPointer = nullptr;

OpenRGBPluginInfo OpenRGBVisualMapPlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info;

    info.Name           = "OpenRGB Visual Map Plugin";
    info.Description    = "Group and organize your devices on a spatial map";
    info.Version        = VERSION_STRING;
    info.Commit         = GIT_COMMIT_ID;
    info.URL            = "https://gitlab.com/OpenRGBDevelopers/OpenRGBVisualMapPlugin";

    info.Label          = "Visual Map";
    info.Location       = OPENRGB_PLUGIN_LOCATION_TOP;

    info.Icon.load(":/images/OpenRGBVisualMapPlugin.png");

    return(info);
}

unsigned int OpenRGBVisualMapPlugin::GetPluginAPIVersion()
{
    return(OPENRGB_PLUGIN_API_VERSION);
}

void OpenRGBVisualMapPlugin::Load(ResourceManagerInterface* RM)
{
    RMPointer = RM;
}

QWidget* OpenRGBVisualMapPlugin::GetWidget()
{
    printf("[OpenRGBVisualMapPlugin] version %s (%s), build date %s\n", VERSION_STRING, GIT_COMMIT_ID, GIT_COMMIT_DATE);

    VisualMapSettingsManager::CreateSettingsDirectory();
    OpenRGBVisualMapPlugin::RMPointer->WaitForDeviceDetection();

    ui = new OpenRGBVisualMapTab(nullptr);

    ui->setStyle(new TooltipProxy(ui->style()));
    ui->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    RMPointer->RegisterDetectionStartCallback(DetectionStart, ui);
    RMPointer->RegisterDetectionEndCallback(DetectionEnd, ui);

    return ui;
}

QMenu* OpenRGBVisualMapPlugin::GetTrayMenu()
{
    return(nullptr);
}

void OpenRGBVisualMapPlugin::Unload()
{    
    ui->UnregisterAll();
    ui->Clear();

    RMPointer->UnregisterDetectionStartCallback(DetectionStart, ui);
    RMPointer->UnregisterDetectionEndCallback(DetectionEnd, ui);
}

void OpenRGBVisualMapPlugin::DetectionStart(void* o)
{
    printf("[OpenRGBVisualMapPlugin] DetectionStart\n");

    // immediate backup, don't run this in a thread
    ((OpenRGBVisualMapTab *)o)->UnregisterAll();
    ((OpenRGBVisualMapTab *)o)->Backup();
    // clear the GUI on the GUI thread
    QMetaObject::invokeMethod((OpenRGBVisualMapTab *)o, "Clear", Qt::QueuedConnection);
}
void OpenRGBVisualMapPlugin::DetectionEnd(void* o)
{
    printf("[OpenRGBVisualMapPlugin] DetectionEnd\n");

    QMetaObject::invokeMethod((OpenRGBVisualMapTab *)o, "Recreate",  Qt::QueuedConnection);
}
