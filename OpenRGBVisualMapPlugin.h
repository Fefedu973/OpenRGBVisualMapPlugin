#ifndef OPENRGBVISUALMAPPLUGIN_H
#define OPENRGBVISUALMAPPLUGIN_H

#include "OpenRGBPluginInterface.h"
#include "ResourceManager.h"
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

    OpenRGBPluginInfo       PInfo;
    OpenRGBPluginInfo       Initialize(bool, ResourceManager*)   override;
    QWidget*                CreateGUI(QWidget *Parent)           override;
    static bool             DarkTheme;
    static ResourceManager* RMPointer;

     OpenRGBVisualMapTab* ui;

private:
    static void DetectionStart(void*);
    static void DetectionEnd(void* );
};

#endif // OPENRGBVISUALMAPPLUGIN_H
