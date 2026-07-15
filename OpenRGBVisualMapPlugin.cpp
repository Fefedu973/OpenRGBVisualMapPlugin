#include "OpenRGBVisualMapPlugin.h"
#include "ResourceManagerCallback.h"
#include "TooltipProxy.h"
#include "VisualMapSettingsManager.h"
#include "ZoneManager.h"

/*---------------------------------------------------------*\
| Plugin Global Variables                                   |
\*---------------------------------------------------------*/
std::atomic<bool>               OpenRGBVisualMapPlugin::controllers_updating;
std::vector<ControllerZone*>    OpenRGBVisualMapPlugin::controller_zones;
std::shared_mutex               OpenRGBVisualMapPlugin::controller_zones_mutex;
OpenRGBPluginAPIInterface*      OpenRGBVisualMapPlugin::api = nullptr;

OpenRGBPluginInfo OpenRGBVisualMapPlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info;

    info.Name           = PROJECT_NAME;
    info.Description    = PROJECT_DESC;
    info.Version        = VERSION_STRING;
    info.Commit         = GIT_COMMIT_ID;
    info.URL            = PROJECT_URL;

    info.Label          = "Visual Map";
    info.Location       = OPENRGB_PLUGIN_LOCATION_TOP;

    info.Icon.load(":/images/OpenRGBVisualMapPlugin.png");

    return(info);
}

unsigned int OpenRGBVisualMapPlugin::GetPluginAPIVersion()
{
    return(OPENRGB_PLUGIN_API_VERSION);
}

/*---------------------------------------------------------*\
| Plugin Functionality                                      |
\*---------------------------------------------------------*/
void OpenRGBVisualMapPlugin::Load(OpenRGBPluginAPIInterface* plugin_api_ptr)
{
    /*-----------------------------------------------------*\
    | Store API interface pointer                           |
    \*-----------------------------------------------------*/
    api = plugin_api_ptr;

    /*-----------------------------------------------------*\
    | Log initial messages                                  |
    \*-----------------------------------------------------*/
    LOG_INFO("[OpenRGBVisualMapPlugin] version %s (%s), build date %s\n", VERSION_STRING, GIT_COMMIT_ID, GIT_COMMIT_DATE);

    /*-----------------------------------------------------*\
    | Create settings directory                             |
    \*-----------------------------------------------------*/
    VisualMapSettingsManager::CreateSettingsDirectory();

    /*-----------------------------------------------------*\
    | Initialize the controller zone list                   |
    \*-----------------------------------------------------*/
    ZoneManager::Get()->UpdateControllerZones();

    /*-----------------------------------------------------*\
    | Create the main UI widget                             |
    \*-----------------------------------------------------*/
    ui = new OpenRGBVisualMapTab();

    ui->setStyle(new TooltipProxy(ui->style()));
    ui->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

QWidget* OpenRGBVisualMapPlugin::GetWidget()
{
    return ui;
}

QMenu* OpenRGBVisualMapPlugin::GetTrayMenu()
{
    return(nullptr);
}

void OpenRGBVisualMapPlugin::Unload()
{    
    ui->HideAll();
    ui->Clear();
}

void OpenRGBVisualMapPlugin::OnProfileAboutToLoad()
{

}

void OpenRGBVisualMapPlugin::OnProfileLoad(nlohmann::json /*profile_data*/)
{

}

nlohmann::json OpenRGBVisualMapPlugin::OnProfileSave()
{
    nlohmann::json profile_json;
    return(profile_json);
}

unsigned char* OpenRGBVisualMapPlugin::OnSDKCommand(unsigned int /*pkt_id*/, unsigned char * /*pkt_data*/, unsigned int * /*pkt_size*/)
{
    return(NULL);
}

/*---------------------------------------------------------*\
| Update Signals                                            |
\*---------------------------------------------------------*/
void OpenRGBVisualMapPlugin::ProfileManagerUpdated(unsigned int /*update_reason*/)
{

}

void OpenRGBVisualMapPlugin::ResourceManagerUpdated(unsigned int update_reason)
{
    switch(update_reason)
    {
        case RESOURCEMANAGER_UPDATE_REASON_DEVICE_LIST_UPDATED:
            ZoneManager::Get()->UpdateControllerZones();
            QMetaObject::invokeMethod(ui, "Recreate", Qt::BlockingQueuedConnection);
            break;
    }
}

void OpenRGBVisualMapPlugin::SettingsManagerUpdated(unsigned int /*update_reason*/)
{

}