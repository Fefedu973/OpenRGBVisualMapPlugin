QT += core gui widgets testlib
CONFIG += console c++17 release
CONFIG -= app_bundle debug debug_and_release
TEMPLATE = app
TARGET = persistence-tests
isEmpty(OPENRGB_CORE_DIR): OPENRGB_CORE_DIR = $$PWD/../../../OpenRGB-Room
INCLUDEPATH += $$OPENRGB_CORE_DIR/tests/room-plugin-images/stubs $$OPENRGB_CORE_DIR/tests/room-plugin-images \
    $$OPENRGB_CORE_DIR $$OPENRGB_CORE_DIR/RGBController $$OPENRGB_CORE_DIR/dependencies/json ../..
SOURCES += test_persistence.cpp $$OPENRGB_CORE_DIR/RGBController/RGBController.cpp \
    $$OPENRGB_CORE_DIR/RGBController/RGBController_Virtual.cpp $$OPENRGB_CORE_DIR/RGBController/RGBControllerKeyNames.cpp $$OPENRGB_CORE_DIR/StringUtils.cpp
