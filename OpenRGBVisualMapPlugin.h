/*---------------------------------------------------------*\
| OpenRGBVisualMapPlugin.h                                  |
|                                                           |
|   OpenRGB Visual Map Plugin                               |
|                                                           |
|   This file is part of the OpenRGB Visual Map Plugin      |
|   project                                                 |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <shared_mutex>
#include <vector>
#include <QAction>
#include <QDialog>
#include <QLabel>
#include <QObject>
#include <QtPlugin>
#include <QPushButton>
#include <QString>
#include <QWidget>
#include "LogManager.h"
#include "OpenRGBPluginInterface.h"
#include "OpenRGBVisualMapTab.h"
#include "ResourceManagerCallback.h"

class OpenRGBVisualMapPlugin : public QObject, public OpenRGBPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OpenRGBPluginInterface_IID FILE "OpenRGBVisualMapPlugin.json")
    Q_INTERFACES(OpenRGBPluginInterface)

public:
    ~OpenRGBVisualMapPlugin() {};

    /*-----------------------------------------------------*\
    | Plugin Information                                    |
    \*-----------------------------------------------------*/
    virtual OpenRGBPluginInfo   GetPluginInfo()                                                                     override;
    virtual unsigned int        GetPluginAPIVersion()                                                               override;

    /*-----------------------------------------------------*\
    | Plugin Functionality                                  |
    \*-----------------------------------------------------*/
    void                        Load(OpenRGBPluginAPIInterface* plugin_api_ptr)                                     override;
    QWidget*                    GetWidget()                                                                         override;
    QMenu*                      GetTrayMenu()                                                                       override;
    void                        Unload()                                                                            override;
    void                        OnProfileAboutToLoad()                                                              override;
    void                        OnProfileLoad(nlohmann::json profile_data)                                          override;
    nlohmann::json              OnProfileSave()                                                                     override;
    unsigned char*              OnSDKCommand(unsigned int pkt_id, unsigned char * pkt_data, unsigned int *pkt_size) override;

    /*-----------------------------------------------------*\
    | Update Signals                                        |
    \*-----------------------------------------------------*/
    void                        ProfileManagerUpdated(unsigned int update_reason)                                   override;
    void                        ResourceManagerUpdated(unsigned int update_reason)                                  override;
    void                        SettingsManagerUpdated(unsigned int update_reason)                                  override;

private:
    /*-----------------------------------------------------*\
    | User interface widget                                 |
    \*-----------------------------------------------------*/
    OpenRGBVisualMapTab*        ui;

private:
    static void DetectionStart(void*);
    static void DetectionEnd(void*);

public:
    /*-----------------------------------------------------*\
    | Plugin Global Variables                               |
    \*-----------------------------------------------------*/
    static std::atomic<bool>            controllers_updating;
    static std::vector<ControllerZone*> controller_zones;
    static std::shared_mutex            controller_zones_mutex;
    static OpenRGBPluginAPIInterface *  api;
};

/*---------------------------------------------------------*\
| LogManager logging macros                                 |
\*---------------------------------------------------------*/
#undef  LogAppend
#define LogAppend(level, ...)   OpenRGBVisualMapPlugin::api->LogEntry(__FILE__, __LINE__, level, __VA_ARGS__)