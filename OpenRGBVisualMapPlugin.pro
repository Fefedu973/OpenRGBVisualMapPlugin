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
MAJOR       = 1
MINOR       = 0
SUFFIX      = git

SHORTHASH   = $$system("git rev-parse --short=7 HEAD")
LASTTAG     = "release_"$$MAJOR"."$$MINOR
COMMAND     = "git rev-list --count "$$LASTTAG"..HEAD"
COMMITS     = $$system($$COMMAND)

VERSION_NUM = $$MAJOR"."$$MINOR"."$$COMMITS
VERSION_STR = $$MAJOR"."$$MINOR

VERSION_DEB = $$VERSION_NUM
VERSION_WIX = $$VERSION_NUM
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
# Metadata                                                                                      #
#-----------------------------------------------------------------------------------------------#
PROJECT_DESC                = "Group and organize your devices on a spatial map"
PROJECT_NAME                = "OpenRGB Visual Map Plugin"
PROJECT_URL                 = "https://gitlab.com/OpenRGBDevelopers/OpenRGBVisualMapPlugin"

#-----------------------------------------------------------------------------------------------#
# Inject vars in defines                                                                        #
#-----------------------------------------------------------------------------------------------#
DEFINES +=                                                                                      \
    VERSION_STRING=\\"\"\"$$VERSION_STR\\"\"\"                                                  \
    BUILDDATE_STRING=\\"\"\"$$BUILDDATE\\"\"\"                                                  \
    GIT_COMMIT_ID=\\"\"\"$$GIT_COMMIT_ID\\"\"\"                                                 \
    GIT_COMMIT_DATE=\\"\"\"$$GIT_COMMIT_DATE\\"\"\"                                             \
    GIT_BRANCH=\\"\"\"$$GIT_BRANCH\\"\"\"                                                       \
    PROJECT_DESC=\\"\"\"$$PROJECT_DESC\\"\"\"                                                   \
    PROJECT_NAME=\\"\"\"$$PROJECT_NAME\\"\"\"                                                   \
    PROJECT_URL=\\"\"\"$$PROJECT_URL\\"\"\"                                                     \
    LATEST_BUILD_URL=\\"\"\"$$LATEST_BUILD_URL\\"\"\"                                           \

#-----------------------------------------------------------------------------------------------#
# Update version in plugin metadata json                                                        #
#-----------------------------------------------------------------------------------------------#
JSON_FILE_IN            = $$PWD/OpenRGBVisualMapPlugin.json.in
JSON_FILE_OUT           = $$PWD/OpenRGBVisualMapPlugin.json

prebuild_json.target    = prebuild_json_target
prebuild_json.depends   = FORCE
prebuild_json.commands  = $$QMAKE_STREAM_EDITOR -e \"s|VERSION_NUM|$$VERSION_NUM|g\"            \
                                                -e \"s|VERSION_STR|$$VERSION_STR|g\"            \
                                                -e \"s|GIT_COMMIT_ID|$$GIT_COMMIT_ID|g\"        \
                                                -e \"s|PROJECT_DESC|$$PROJECT_DESC|g\"          \
                                                -e \"s|PROJECT_NAME|$$PROJECT_NAME|g\"          \
                                                -e \"s|PROJECT_URL|$$PROJECT_URL|g\"            \
                                                $$JSON_FILE_IN > $$JSON_FILE_OUT                \

QMAKE_EXTRA_TARGETS    += prebuild_json
PRE_TARGETDEPS         += prebuild_json_target

#-----------------------------------------------------------------------------------------------#
# OpenRGB Plugin SDK                                                                            #
#-----------------------------------------------------------------------------------------------#
isEmpty(OPENRGB_CORE_DIR):!isEmpty(OPENRGB_ROOM_ROOT): OPENRGB_CORE_DIR = $$OPENRGB_ROOM_ROOT
isEmpty(OPENRGB_CORE_DIR): OPENRGB_CORE_DIR = $$PWD/OpenRGB
!exists($$OPENRGB_CORE_DIR/FrameRouting/OpenRGBImagePluginAPI.h): error("Use OPENRGB_CORE_DIR pointing to an image-capable OpenRGB core")
INCLUDEPATH +=                                                                                  \
    $$OPENRGB_CORE_DIR/                                                                         \
    $$OPENRGB_CORE_DIR/dependencies/json                                                        \
    $$OPENRGB_CORE_DIR/RGBController                                                            \
    $$OPENRGB_CORE_DIR/qt                                                                       \

SOURCES +=                                                                                      \
    $$OPENRGB_CORE_DIR/qt/hsv.cpp

#-------------------------------------------------------------------#
# Includes                                                          #
#-------------------------------------------------------------------#
INCLUDEPATH +=                                                                                  \
    ui                                                                                          \
    ui/widget-editor                                                                            \

HEADERS +=                                                                                      \
    OpenRGBPluginsFont.h                                                                        \
    OpenRGBVisualMapPlugin.h                                                                    \
    ZoneManager.h                                                                               \
    LedRouting.h                                                                                \
    ImageRouting.h                                                                              \
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

SOURCES +=                                                                                      \
    OpenRGBPluginsFont.cpp                                                                      \
    OpenRGBVisualMapPlugin.cpp                                                                  \
    VirtualController.cpp                                                                       \
    VisualMapSettingsManager.cpp                                                                \
    ZoneManager.cpp                                                                             \
    LedRouting.cpp                                                                              \
    ImageRouting.cpp                                                                            \
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
    ui/Scene.cpp                                                                                \
    ui/TabHeader.cpp                                                                            \
    ui/VirtualControllerTab.cpp                                                                 \
    ui/widget-editor/EditorGrid.cpp                                                             \
    ui/widget-editor/LedItem.cpp                                                                \
    ui/widget-editor/WidgetEditor.cpp                                                           \

FORMS +=                                                                                        \
    ui/BackgroundApplier.ui                                                                     \
    ui/ColorPicker.ui                                                                           \
    ui/ColorStop.ui                                                                             \
    ui/DeviceList.ui                                                                            \
    ui/DeviceWidget.ui                                                                          \
    ui/GridOptions.ui                                                                           \
    ui/ItemOptions.ui                                                                           \
    ui/OpenRGBVisualMapTab.ui                                                                   \
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

#-----------------------------------------------------------------------------------------------#
# Linux-specific Configuration                                                                  #
#-----------------------------------------------------------------------------------------------#
unix:!macx {
    QMAKE_CXXFLAGS += -std=c++17 -Wno-psabi

    #-------------------------------------------------------------------------------------------#
    # Add static files to installation                                                          #
    #-------------------------------------------------------------------------------------------#
    target.path=$$PREFIX/lib/openrgb/plugins/
    metainfo.path=$$PREFIX/share/metainfo/
    metainfo.files+=org.openrgb.OpenRGB.Plugin.VisualMap.metainfo.xml
    INSTALLS += target metainfo
}

#-----------------------------------------------------------------------------------------------#
# MacOS-specific Configuration                                                                  #
#-----------------------------------------------------------------------------------------------#
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

macx: {
    CONFIG += c++17
}

RESOURCES +=                                                                                    \
    resources.qrc
