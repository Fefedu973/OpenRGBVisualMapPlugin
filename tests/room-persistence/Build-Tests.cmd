@echo off
setlocal
if not defined QT_ROOT exit /b 2
if not defined OPENRGB_CORE_DIR set "OPENRGB_CORE_DIR=%~dp0..\..\..\OpenRGB-Room"
if not defined VCVARS set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
call "%VCVARS%" x64
if errorlevel 1 exit /b %errorlevel%
set "PATH=%QT_ROOT%\bin;%PATH%"
if not exist "%~dp0.build" mkdir "%~dp0.build"
cd /d "%~dp0.build"
qmake ..\persistence_tests.pro "OPENRGB_CORE_DIR=%OPENRGB_CORE_DIR%"
if errorlevel 1 exit /b %errorlevel%
nmake /NOLOGO
if errorlevel 1 exit /b %errorlevel%
set "QT_QPA_PLATFORM=offscreen"
persistence-tests.exe "%~dp0..\..\.build\release\OpenRGBVisualMapPlugin.dll"
exit /b %errorlevel%
