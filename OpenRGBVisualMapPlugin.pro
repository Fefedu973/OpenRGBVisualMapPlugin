QT +=                  \
    gui                \
    widgets            \
    core               \

win32:CONFIG += QTPLUGIN

TEMPLATE = lib
DEFINES += ORGBVISUALMAPPLUGIN_LIBRARY

win32:CONFIG += c++17

unix:!macx {
  QMAKE_CXXFLAGS += -std=c++17
}

#-----------------------------------------------------------------------------------------------#
# Application Configuration                                                                     #
#-----------------------------------------------------------------------------------------------#
PLUGIN_VERSION     = 0.1

#-----------------------------------------------------------------------------------------------#
# Automatically generated build information                                                     #
#-----------------------------------------------------------------------------------------------#
win32:BUILDDATE = $$system(date /t)
unix:BUILDDATE  = $$system(date -R -d "@${SOURCE_DATE_EPOCH:-$(date +%s)}")
GIT_COMMIT_ID   = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ rev-parse HEAD)
GIT_COMMIT_DATE = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ show -s --format=%ci HEAD)
GIT_BRANCH      = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ rev-parse --abbrev-ref HEAD)


win32:LATEST_BUILD_URL="https://gitlab.com/OpenRGBDevelopers/openrgbvisualmapplugin/-/jobs/artifacts/master/download?job=Windows 64"
unix:!macx:LATEST_BUILD_URL="https://gitlab.com/OpenRGBDevelopers/openrgbvisualmapplugin/-/jobs/artifacts/master/download?job=Linux 64"

DEFINES +=                                                                                      \
    VERSION_STRING=\\"\"\"$$PLUGIN_VERSION\\"\"\"                                               \
    BUILDDATE_STRING=\\"\"\"$$BUILDDATE\\"\"\"                                                  \
    GIT_COMMIT_ID=\\"\"\"$$GIT_COMMIT_ID\\"\"\"                                                 \
    GIT_COMMIT_DATE=\\"\"\"$$GIT_COMMIT_DATE\\"\"\"                                             \
    GIT_BRANCH=\\"\"\"$$GIT_BRANCH\\"\"\"                                                       \
    LATEST_BUILD_URL=\\"\"\"$$LATEST_BUILD_URL\\"\"\"                                           \

#-------------------------------------------------------------------#
# Includes                                                          #
#-------------------------------------------------------------------#
INCLUDEPATH +=                                                                                  \
    OpenRGB/                                                                                    \
    OpenRGB/i2c_smbus                                                                           \
    OpenRGB/net_port                                                                            \
    OpenRGB/RGBController                                                                       \
    OpenRGB/dependencies/json                                                                   \
    OpenRGB/qt                                                                                  \
    ui/                                                                                         \
    ui/widget-editor/                                                                           \

HEADERS +=                                                                                      \
    OpenRGB/NetworkClient.h                                                                     \
    OpenRGB/NetworkProtocol.h                                                                   \
    OpenRGB/NetworkServer.h                                                                     \
    OpenRGB/OpenRGBPluginInterface.h                                                            \
    OpenRGB/ProfileManager.h                                                                    \
    OpenRGB/ResourceManager.h                                                                   \
    OpenRGB/SettingsManager.h                                                                   \
    OpenRGB/dependencies/json/json.hpp                                                          \
    OpenRGB/i2c_smbus/i2c_smbus.h                                                               \
    OpenRGB/net_port/net_port.h                                                                 \
    OpenRGB/RGBController/RGBController.h                                                       \
    OpenRGBVisualMapPlugin.h                                                                    \
    ZoneManager.h                                                                               \
    VirtualController.h                                                                         \
    VisualMapSettingsManager.h                                                                  \
    VisualMapJsonDefinitions.h                                                                  \
    ControllerZone.h                                                                            \
    ui/ColorPicker.h                                                                            \
    ui/ColorStop.h                                                                              \
    ui/ControllerZoneItem.h                                                                     \
    ui/DeviceList.h                                                                             \
    ui/DeviceWidget.h                                                                           \
    ui/EditableLabel.h                                                                          \
    ui/GradientPresets.h                                                                        \
    ui/GridSettings.h                                                                           \
    ui/PluginInfo.h                                                                             \
    ui/Scene.h                                                                                  \
    ui/TabHeader.h                                                                              \
    ui/TooltipProxy.h                                                                           \
    ui/VirtualControllerTab.h                                                                   \
    ui/widget-editor/EditorGrid.h                                                               \
    ui/widget-editor/LedItem.h                                                                  \
    ui/BackgroundApplier.h                                                                      \
    ui/OpenRGBVisualMapTab.h                                                                    \
    ui/widget-editor/WidgetEditor.h                                                             \
    ui/Grid.h                                                                                   \
    ui/GridOptions.h                                                                            \
    ui/ItemOptions.h                                                                            \
    OpenRGB/qt/hsv.h                                                                            \

SOURCES +=                                                                                      \
    OpenRGB/RGBController/RGBController.cpp                                                     \
    OpenRGBVisualMapPlugin.cpp                                                                  \
    VirtualController.cpp                                                                       \
    VisualMapSettingsManager.cpp                                                                \
    ZoneManager.cpp                                                                             \
    ui/BackgroundApplier.cpp                                                                    \
    ui/ColorPicker.cpp                                                                          \
    ui/ColorStop.cpp                                                                            \
    ui/ControllerZoneItem.cpp                                                                   \
    ui/DeviceList.cpp                                                                           \
    ui/DeviceWidget.cpp                                                                         \
    ui/EditableLabel.cpp                                                                        \
    ui/Grid.cpp                                                                                 \
    ui/GridOptions.cpp                                                                          \
    ui/ItemOptions.cpp                                                                          \
    ui/OpenRGBVisualMapTab.cpp                                                                  \
    ui/PluginInfo.cpp                                                                           \
    ui/Scene.cpp                                                                                \
    ui/TabHeader.cpp                                                                            \
    ui/VirtualControllerTab.cpp                                                                 \
    ui/widget-editor/EditorGrid.cpp                                                             \
    ui/widget-editor/LedItem.cpp                                                                \
    ui/widget-editor/WidgetEditor.cpp                                                           \
    OpenRGB/qt/hsv.cpp                                                                          \

FORMS +=                                                                                        \
    ui/BackgroundApplier.ui                                                                     \
    ui/ColorPicker.ui                                                                           \
    ui/ColorStop.ui                                                                             \
    ui/DeviceList.ui \
    ui/DeviceWidget.ui \
    ui/GridOptions.ui                                                                           \
    ui/ItemOptions.ui                                                                           \
    ui/OpenRGBVisualMapTab.ui                                                                   \
    ui/PluginInfo.ui                                                                            \
    ui/TabHeader.ui                                                                             \
    ui/VirtualControllerTab.ui                                                                  \
    ui/widget-editor/WidgetEditor.ui                                                            \

#-------------------------------------------------------------------#
# Windows GitLab CI Configuration                                   #
#-------------------------------------------------------------------#
win32:CONFIG(debug, debug|release) {
    win32:DESTDIR = debug
}

win32:CONFIG(release, debug|release) {
    win32:DESTDIR = release
}

win32:OBJECTS_DIR = _intermediate_$$DESTDIR/.obj
win32:MOC_DIR     = _intermediate_$$DESTDIR/.moc
win32:RCC_DIR     = _intermediate_$$DESTDIR/.qrc
win32:UI_DIR      = _intermediate_$$DESTDIR/.ui

win32:contains(QMAKE_TARGET.arch, x86_64) {
    LIBS +=                                                             \
        -lws2_32                                                        \
        -lole32                                                         \
}

win32:contains(QMAKE_TARGET.arch, x86) {
    LIBS +=                                                             \
        -lws2_32                                                        \
        -lole32                                                         \
}

win32:DEFINES +=                                                        \
    _MBCS                                                               \
    WIN32                                                               \
    _CRT_SECURE_NO_WARNINGS                                             \
    _WINSOCK_DEPRECATED_NO_WARNINGS                                     \
    WIN32_LEAN_AND_MEAN                                                 \

#-----------------------------------------------------------------------#
# Linux-specific Configuration                                          #
#-----------------------------------------------------------------------#
unix:!macx {
}

#-----------------------------------------------------------------------#
# MacOS-specific Configuration                                          #
#-----------------------------------------------------------------------#
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

macx: {
    CONFIG += c++17
}

RESOURCES += \
    images/res.qrc
