@echo off
setlocal

:: Focus App Build Script
:: Builds the lightweight Windows focus application

set TARGET=FocusApp.exe
set OBJS=main.obj TimerWidget.obj AppMonitor.obj Config.obj
set RES=resource.res

:: Check if we're in the right directory
if not exist "main.cpp" (
    echo Error: Please run this script from the project directory
    pause
    exit /b 1
)

:: Create build directory
if not exist "build" mkdir build

:: Compile resource file
echo Compiling resources...
windres -O coff resource.rc %RES% || (
    echo Error: windres not found. Please install MinGW or Visual Studio.
    pause
    exit /b 1
)

:: Compile C++ files
echo Compiling C++ files...
cl /EHsc /DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN /D_CRT_SECURE_NO_WARNINGS \
    /I. main.cpp TimerWidget.cpp AppMonitor.cpp Config.cpp \
    /link user32.lib gdi32.lib shell32.lib comctl32.lib psapi.lib \
    /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup /OUT:%TARGET% %RES% || (
    echo Error: cl.exe not found. Please run from Visual Studio Developer Command Prompt.
    pause
    exit /b 1
)

:: Check if build succeeded
if exist "%TARGET%" (
    echo Build successful!
    echo Running %TARGET%...
    start %TARGET%
) else (
    echo Build failed!
    pause
)

endlocal
