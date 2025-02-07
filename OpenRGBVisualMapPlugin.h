#ifndef OPENRGBVISUALMAPPLUGIN_H
#define OPENRGBVISUALMAPPLUGIN_H

#include "OpenRGBPluginInterface.h"
#include "ResourceManagerInterface.h"
#include "OpenRGBVisualMapTab.h"

#include <QObject>
#include <QString>
#include <QtPlugin>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QAction>

class OpenRGBVisualMapPlugin : public QObject, public OpenRGBPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OpenRGBPluginInterface_IID)
    Q_INTERFACES(OpenRGBPluginInterface)

public:
    ~OpenRGBVisualMapPlugin() {};

    /*-------------------------------------------------------------------------------------------------*\
    | Plugin Information                                                                                |
    \*-------------------------------------------------------------------------------------------------*/
    virtual OpenRGBPluginInfo   GetPluginInfo()                                                 override;
    virtual unsigned int        GetPluginAPIVersion()                                           override;

    /*-------------------------------------------------------------------------------------------------*\
    | Plugin Functionality                                                                              |
    \*-------------------------------------------------------------------------------------------------*/
    virtual void                Load(ResourceManagerInterface* resource_manager_ptr)            override;
    virtual QWidget*            GetWidget()                                                     override;
    virtual QMenu*              GetTrayMenu()                                                   override;
    virtual void                Unload()                                                        override;

    static ResourceManagerInterface* RMPointer;

    OpenRGBVisualMapTab* ui;

private:
    static void DetectionStart(void*);
    static void DetectionEnd(void*);
};

#endif // OPENRGBVISUALMAPPLUGIN_H
