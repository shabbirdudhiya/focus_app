@echo off
setlocal

:: Simple Focus App Compiler for Windows
:: Uses g++ (MinGW) to compile the application

:: Check if g++ exists
where g++ >nul 2>&1
if errorlevel 1 (
    echo.
    echo ERROR: g++ compiler not found!
    echo.
    echo Please install MinGW and add it to your PATH.
    echo.
    echo Download MinGW from: https://www.mingw-w64.org/
    echo Or install via Chocolatey: choco install mingw
    echo.
    pause
    exit /b 1
)

:: Get the directory where this script is located
set "SCRIPT_DIR=%~dp0"
cd /d "%SCRIPT_DIR%"

echo.
echo ========================================
echo    Focus App - Compiling...
echo ========================================
echo.

:: Compile with static linking for a single executable
g++ -static -static-libgcc -static-libstdc++ -D_UNICODE -DUNICODE -o focus_app.exe focus_app_simple.cpp -luser32 -lgdi32 -lcomctl32 -lshell32 -lpsapi

if errorlevel 1 (
    echo.
    echo ERROR: Compilation failed!
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo    SUCCESS! focus_app.exe created
echo ========================================
echo.
echo To run the app, double-click focus_app.exe
echo Or run: focus_app.exe
necho.
pause
