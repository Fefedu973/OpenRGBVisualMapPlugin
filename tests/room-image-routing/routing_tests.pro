QT += core gui widgets testlib
CONFIG += console c++17 release
CONFIG -= app_bundle debug debug_and_release
TEMPLATE = app
TARGET = routing-tests
isEmpty(OPENRGB_CORE_DIR): OPENRGB_CORE_DIR = $$PWD/../../../OpenRGB-Room
INCLUDEPATH += stubs ../.. ../../ui ../../ui/widget-editor $$OPENRGB_CORE_DIR $$OPENRGB_CORE_DIR/RGBController $$OPENRGB_CORE_DIR/dependencies/json
DEFINES += VERSION_STRING=\\\"test\\\"
SOURCES += test_routing.cpp ../../ImageRouting.cpp ../../LedRouting.cpp ../../VirtualController.cpp \
    $$OPENRGB_CORE_DIR/RGBController/RGBController.cpp \
    $$OPENRGB_CORE_DIR/RGBController/RGBController_Virtual.cpp \
    $$OPENRGB_CORE_DIR/RGBController/RGBControllerKeyNames.cpp $$OPENRGB_CORE_DIR/StringUtils.cpp
