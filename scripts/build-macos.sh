#!/usr/bin/env bash

#-----------------------------------------------------------------------#
# OpenRGB Visual Map Plugin MacOS Build Script                          #
#-----------------------------------------------------------------------#

#-----------------------------------------------------------------------#
# Setup environment                                                     #
#-----------------------------------------------------------------------#
export APPIMAGE_EXTRACT_AND_RUN=1

if   [ "$1" = "qt6" ] && [ "$2" = "arm" ]; then
    export ARCH=""
    export BREW_PATH=/opt/homebrew
    export QT_PATH=bin
elif [ "$1" = "qt6" ] && [ "$2" = "intel" ]; then
    export ARCH="arch -x86_64"
    export BREW_PATH=/usr/local
    export QT_PATH=bin
elif [ "$1" = "qt5" ] && [ "$2" = "arm" ]; then
    export ARCH=""
    export BREW_PATH=/opt/homebrew
    export QT_PATH=opt/qt@5/bin
elif [ "$1" = "qt5" ] && [ "$2" = "intel" ]; then
    export ARCH="arch -x86_64"
    export BREW_PATH=/usr/local
    export QT_PATH=opt/qt@5/bin
else
    echo "Invalid arguments, specify qt5 or qt6 and arm or intel"
    echo "Example: ./build-macos.sh qt6 arm"
fi

#-----------------------------------------------------------------------#
# Build OpenRGB Visual Map Plugin                                       #
#-----------------------------------------------------------------------#
eval $($BREW_PATH/bin/brew shellenv)
$ARCH $BREW_PATH/$QT_PATH/qmake OpenRGBVisualMapPlugin.pro
$ARCH make -j$(sysctl -n hw.ncpu)

#-----------------------------------------------------------------------#
# Fix framework name linkage so that it uses bundled frameworks in the  #
# OpenRGB app bundle                                                    #
#-----------------------------------------------------------------------#
if [ "$1" = "qt5" ]; then
    install_name_tool -change $BREW_PATH/opt/qt@5/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore libOpenRGBVisualMapPlugin.dylib
    install_name_tool -change $BREW_PATH/opt/qt@5/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui libOpenRGBVisualMapPlugin.dylib
    install_name_tool -change $BREW_PATH/opt/qt@5/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets libOpenRGBVisualMapPlugin.dylib
fi

if [ "$1" = "qt6" ]; then
    install_name_tool -change $BREW_PATH/opt/qtbase/lib/QtCore.framework/Versions/A/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/A/QtCore libOpenRGBVisualMapPlugin.dylib
    install_name_tool -change $BREW_PATH/opt/qtbase/lib/QtGui.framework/Versions/A/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/A/QtGui libOpenRGBVisualMapPlugin.dylib
    install_name_tool -change $BREW_PATH/opt/qtbase/lib/QtWidgets.framework/Versions/A/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/A/QtWidgets libOpenRGBVisualMapPlugin.dylib
fi

#-----------------------------------------------------------------------#
# Sign the binary                                                       #
#-----------------------------------------------------------------------#
$ARCH codesign --force --verify -s OpenRGB libOpenRGBVisualMapPlugin.dylib