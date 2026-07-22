@echo off
setlocal

:: Focus App Build Script for Windows
:: This script compiles the focus app using g++ (MinGW)

:: Check if g++ is available
where g++ >nul 2>&1
if errorlevel 1 (
    echo Error: g++ (MinGW) not found in PATH
    echo Please install MinGW and add it to your PATH
    echo Download from: https://www.mingw-w64.org/
    pause
    exit /b 1
)

:: Get script directory
set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

:: Compile the application
echo Compiling Focus App...
g++ -static -static-libgcc -static-libstdc++ -D_UNICODE -DUNICODE -o focus_app.exe focus_app.cpp -luser32 -lgdi32 -lcomctl32 -lshell32 -lpsapi

if errorlevel 1 (
    echo Compilation failed!
    pause
    exit /b 1
)

echo.
echo Build successful!
echo focus_app.exe created in: %CD%
echo.
echo To run: focus_app.exe
pause
