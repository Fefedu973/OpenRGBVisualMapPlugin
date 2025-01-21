#-----------------------------------------------------------------------------------------------#
# OpenRGB Visual Map Plugin QMake Project                                                       #
#-----------------------------------------------------------------------------------------------#

#-----------------------------------------------------------------------------------------------#
# Qt Configuration                                                                              #
#-----------------------------------------------------------------------------------------------#
QT +=                                                                                           \
    core                                                                                        \
    gui                                                                                         \
    widgets

DEFINES += ORGBVISUALMAPPLUGIN_LIBRARY
TEMPLATE = lib

#-----------------------------------------------------------------------------------------------#
# Build Configuration                                                                           #
#-----------------------------------------------------------------------------------------------#
CONFIG +=                                                                                       \
    plugin                                                                                      \
    silent

#-----------------------------------------------------------------------------------------------#
# Application Configuration                                                                     #
#-----------------------------------------------------------------------------------------------#
MAJOR       = 0
MINOR       = 9
SUFFIX      = git

SHORTHASH   = $$system("git rev-parse --short=7 HEAD")
LASTTAG     = "release_"$$MAJOR"."$$MINOR
COMMAND     = "git rev-list --count "$$LASTTAG"..HEAD"
COMMITS     = $$system($$COMMAND)

VERSION_NUM = $$MAJOR"."$$MINOR"."$$COMMITS
VERSION_STR = $$MAJOR"."$$MINOR

VERSION_DEB = $$VERSION_NUM
VERSION_WIX = $$VERSION_NUM".0"
VERSION_AUR = $$VERSION_NUM
VERSION_RPM = $$VERSION_NUM

equals(SUFFIX, "git") {
VERSION_STR = $$VERSION_STR"+ ("$$SUFFIX$$COMMITS")"
VERSION_DEB = $$VERSION_DEB"~git"$$SHORTHASH
VERSION_AUR = $$VERSION_AUR".g"$$SHORTHASH
VERSION_RPM = $$VERSION_RPM"^git"$$SHORTHASH
} else {
    !isEmpty(SUFFIX) {
VERSION_STR = $$VERSION_STR"+ ("$$SUFFIX")"
VERSION_DEB = $$VERSION_DEB"~"$$SUFFIX
VERSION_AUR = $$VERSION_AUR"."$$SUFFIX
VERSION_RPM = $$VERSION_RPM"^"$$SUFFIX
    }
}

message("VERSION_NUM: "$$VERSION_NUM)
message("VERSION_STR: "$$VERSION_STR)
message("VERSION_DEB: "$$VERSION_DEB)
message("VERSION_WIX: "$$VERSION_WIX)
message("VERSION_AUR: "$$VERSION_AUR)
message("VERSION_RPM: "$$VERSION_RPM)

#-----------------------------------------------------------------------------------------------#
# Automatically generated build information                                                     #
#-----------------------------------------------------------------------------------------------#
win32:BUILDDATE = $$system(date /t)
unix:BUILDDATE  = $$system(date -R -d "@${SOURCE_DATE_EPOCH:-$(date +%s)}")
GIT_COMMIT_ID   = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ rev-parse HEAD)
GIT_COMMIT_DATE = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ show -s --format=%ci HEAD)
GIT_BRANCH      = $$system(git --git-dir $$_PRO_FILE_PWD_/.git --work-tree $$_PRO_FILE_PWD_ rev-parse --abbrev-ref HEAD)

#-----------------------------------------------------------------------------------------------#
# Download links                                                                                #
#-----------------------------------------------------------------------------------------------#
win32:LATEST_BUILD_URL="https://gitlab.com/OpenRGBDevelopers/openrgbvisualmapplugin/-/jobs/artifacts/master/download?job=Windows 64"
unix:!macx:LATEST_BUILD_URL="https://gitlab.com/OpenRGBDevelopers/openrgbvisualmapplugin/-/jobs/artifacts/master/download?job=Linux 64"

#-----------------------------------------------------------------------------------------------#
# Inject vars in defines                                                                        #
#-----------------------------------------------------------------------------------------------#
DEFINES +=                                                                                      \
    VERSION_STRING=\\"\"\"$$VERSION_STR\\"\"\"                                                  \
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
    OpenRGB/dependencies/hidapi/hidapi                                                          \
    OpenRGB/hidapi_wrapper/                                                                     \

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
    OpenRGBPluginsFont.h                                                                        \
    OpenRGBVisualMapPlugin.h                                                                    \
    ZoneManager.h                                                                               \
    VirtualController.h                                                                         \
    VisualMapSettingsManager.h                                                                  \
    VisualMapJsonDefinitions.h                                                                  \
    ControllerZone.h                                                                            \
    ui/ClickableLabel.h                                                                         \
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
    OpenRGB/hidapi_wrapper/hidapi_wrapper.h                                                     \

SOURCES +=                                                                                      \
    OpenRGB/RGBController/RGBController.cpp                                                     \
    OpenRGBPluginsFont.cpp                                                                      \
    OpenRGBVisualMapPlugin.cpp                                                                  \
    VirtualController.cpp                                                                       \
    VisualMapSettingsManager.cpp                                                                \
    ZoneManager.cpp                                                                             \
    ui/BackgroundApplier.cpp                                                                    \
    ui/ClickableLabel.cpp                                                                       \
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
    ui/DeviceList.ui                                                                            \
    ui/DeviceWidget.ui                                                                          \
    ui/GridOptions.ui                                                                           \
    ui/ItemOptions.ui                                                                           \
    ui/OpenRGBVisualMapTab.ui                                                                   \
    ui/PluginInfo.ui                                                                            \
    ui/TabHeader.ui                                                                             \
    ui/VirtualControllerTab.ui                                                                  \
    ui/widget-editor/WidgetEditor.ui                                                            \

#-----------------------------------------------------------------------------------------------#
# Windows-specific Configuration                                                                #
#-----------------------------------------------------------------------------------------------#
win32:CONFIG += QTPLUGIN c++17

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
    LIBS +=                                                                                     \
        -lws2_32                                                                                \
        -lole32                                                                                 \
}

win32:contains(QMAKE_TARGET.arch, x86) {
    LIBS +=                                                                                     \
        -lws2_32                                                                                \
        -lole32                                                                                 \
}

win32:DEFINES +=                                                                                \
    _MBCS                                                                                       \
    WIN32                                                                                       \
    _CRT_SECURE_NO_WARNINGS                                                                     \
    _WINSOCK_DEPRECATED_NO_WARNINGS                                                             \
    WIN32_LEAN_AND_MEAN                                                                         \

win32:INCLUDEPATH +=                                                                            \
    OpenRGB/dependencies/hidapi-win/include
#-----------------------------------------------------------------------------------------------#
# Linux-specific Configuration                                                                  #
#-----------------------------------------------------------------------------------------------#
unix:!macx {
    QMAKE_CXXFLAGS += -std=c++17 -Wno-psabi
    target.path=$$PREFIX/lib/openrgb/plugins/
    INSTALLS += target
}

contains(QMAKE_PLATFORM, linux) {
   INCLUDEPATH+=/usr/include/hidapi
}

#-----------------------------------------------------------------------------------------------#
# MacOS-specific Configuration                                                                  #
#-----------------------------------------------------------------------------------------------#
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

macx: {
    CONFIG += c++17 link_pkgconfig
    PKGCONFIG +=                                                                                \
    hidapi
}

RESOURCES +=                                                                                    \
    resources.qrc
