@echo off
setlocal

<<<<<<< HEAD
:: Focus App Build Script for Windows
:: This script compiles the focus app using g++ (MinGW)

:: Check if g++ is available
where g++ >nul 2>&1
if errorlevel 1 (
    echo Error: g++ (MinGW) not found in PATH
    echo Please install MinGW and add it to your PATH
    echo Download from: https://www.mingw-w64.org/
=======
:: Focus App Build Script
:: Builds the lightweight Windows focus application

set TARGET=FocusApp.exe
set OBJS=main.obj TimerWidget.obj AppMonitor.obj Config.obj
set RES=resource.res

:: Check if we're in the right directory
if not exist "main.cpp" (
    echo Error: Please run this script from the project directory
>>>>>>> 3a88b4d236f09c65121fcab973945bc4a006c191
    pause
    exit /b 1
)

<<<<<<< HEAD
:: Get script directory
set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

:: Compile the application
echo Compiling Focus App...
g++ -static -static-libgcc -static-libstdc++ -D_UNICODE -DUNICODE -o focus_app.exe focus_app.cpp -luser32 -lgdi32 -lcomctl32 -lshell32 -lpsapi

if errorlevel 1 (
    echo Compilation failed!
=======
:: Create build directory
if not exist "build" mkdir build

:: Compile resource file
echo Compiling resources...
windres -O coff resource.rc %RES% || (
    echo Error: windres not found. Please install MinGW or Visual Studio.
>>>>>>> 3a88b4d236f09c65121fcab973945bc4a006c191
    pause
    exit /b 1
)

<<<<<<< HEAD
echo.
echo Build successful!
echo focus_app.exe created in: %CD%
echo.
echo To run: focus_app.exe
pause
=======
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
>>>>>>> 3a88b4d236f09c65121fcab973945bc4a006c191
