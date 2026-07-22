@echo off
setlocal

:: Focus App - Full Version Compiler
:: Uses g++ (MinGW) to compile the application with all features

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
echo    Focus App - Full Version
    Compiling with all features...
echo ========================================
echo.

:: Compile with static linking for a single executable
g++ -static -static-libgcc -static-libstdc++ -D_UNICODE -DUNICODE -o focus_app.exe focus_app_full.cpp -luser32 -lgdi32 -lcomctl32 -lshell32 -lpsapi

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
    with all features:
echo ========================================
echo.
echo Features included:
echo   - Modern Clock View
echo   - Focus View with Pomodoro timer
echo   - Theme Toggle (Dark/Light)
echo   - Fullscreen mode
echo   - Floating timer window
echo   - App blocking warnings
echo   - Session tracking
echo   - Configurable settings
echo.
echo To run: focus_app.exe
echo.
pause
