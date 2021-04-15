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
# OpenRGB Plugin SDK                                                                            #
#-----------------------------------------------------------------------------------------------#
INCLUDEPATH +=                                                                                  \
    OpenRGB/                                                                                    \
    OpenRGB/i2c_smbus                                                                           \
    OpenRGB/net_port                                                                            \
    OpenRGB/RGBController                                                                       \
    OpenRGB/dependencies/json                                                                   \

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
    ui/ColorPicker.h \
    ui/ColorStop.h \
    ui/ControllerZoneItem.h \
    ui/Scene.h \
    ui/TooltipProxy.h

#-----------------------------------------------------------------------------------------------#
# GUI and misc                                                                                  #
#-----------------------------------------------------------------------------------------------#
INCLUDEPATH +=                                                                                  \
    ui/                                                                                         \       
    Dependencies/                                                                               \
    Dependencies/HSV                                                                            \
    Dependencies/ColorWheel                                                                     \

HEADERS +=                                                                                      \
    OpenRGBVisualMapPlugin.h                                                                    \
    ui/BackgroundApplier.h \
    ui/OpenRGBVisualMapTab.h \
    VisualMapSettingsManager.h \
    ZoneManager.h \
    ui/Grid.h \
    ui/GridOptions.h \
    ui/ItemOptions.h \
    VisualMapJsonDefinitions.h \
    Dependencies/HSV/hsv.h                                                                      \
    Dependencies/ColorWheel/ColorWheel.h


SOURCES +=                                                                                      \
    OpenRGBVisualMapPlugin.cpp                                                                  \
    VisualMapSettingsManager.cpp \
    ZoneManager.cpp \
    ui/BackgroundApplier.cpp \
    ui/ColorPicker.cpp \
    ui/ColorStop.cpp \
    ui/ControllerZoneItem.cpp \
    ui/Grid.cpp                                                                                 \
    ui/GridOptions.cpp \
    ui/ItemOptions.cpp \
    ui/OpenRGBVisualMapTab.cpp \
    Dependencies/HSV/hsv.cpp \
    Dependencies/ColorWheel/ColorWheel.cpp                                                      \
    ui/Scene.cpp


FORMS +=                                                                                        \
    ui/BackgroundApplier.ui \
    ui/ColorPicker.ui \
    ui/ColorStop.ui \
    ui/GridOptions.ui \
    ui/ItemOptions.ui \
    ui/OpenRGBVisualMapTab.ui

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
macx: {
}

RESOURCES += \
    images/res.qrc
