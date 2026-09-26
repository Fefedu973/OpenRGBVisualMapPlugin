@echo off
setlocal
if not defined QT_ROOT (
  echo Set QT_ROOT to a Qt MSVC kit before building.
  exit /b 2
)
if not defined OPENRGB_CORE_DIR set "OPENRGB_CORE_DIR=%~dp0..\OpenRGB-Room"
if not defined VCVARS set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
call "%VCVARS%" x64
if errorlevel 1 exit /b %errorlevel%
set "PATH=%QT_ROOT%\bin;%PATH%;C:\Program Files\Git\usr\bin"
if not exist "%~dp0.build" mkdir "%~dp0.build"
cd /d "%~dp0.build"
qmake ..\OpenRGBVisualMapPlugin.pro "OPENRGB_CORE_DIR=%OPENRGB_CORE_DIR%" QMAKE_STREAM_EDITOR=sed CONFIG+=release CONFIG-=debug CONFIG-=debug_and_release CONFIG-=build_all
if errorlevel 1 exit /b %errorlevel%
nmake /NOLOGO
exit /b %errorlevel%
