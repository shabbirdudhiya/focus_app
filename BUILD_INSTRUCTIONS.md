# Build Instructions for Focus App

## Overview

This is a lightweight Windows focus app that helps you stay productive. It includes:
- Floating timer window (always on top)
- Pomodoro-style focus/break timer
- App blocking with warnings
- Task tracking

## Requirements

- **Windows 7 or later**
- **MinGW (g++)** - for compilation

## Installation Options

### Option 1: Quick Install with Chocolatey (Recommended)

1. Install Chocolatey (if you don't have it):
   ```powershell
   Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
   ```

2. Install MinGW:
   ```powershell
   choco install mingw -y
   ```

3. Compile the app:
   ```cmd
   compile.bat
   ```

### Option 2: Manual MinGW Installation

1. Download MinGW from: https://www.mingw-w64.org/
2. Install it (make sure to select g++ during installation)
3. Add MinGW's `bin` folder to your PATH environment variable
4. Open a new Command Prompt and run:
   ```cmd
   compile.bat
   ```

### Option 3: Using MSYS2

1. Download and install MSYS2 from: https://www.msys2.org/
2. Open MSYS2 terminal and update packages:
   ```bash
   pacman -Syu
   pacman -S mingw-w64-x86_64-gcc
   ```
3. Add MSYS2's mingw64/bin to your PATH
4. Run compile.bat

## Compiling

Once you have MinGW installed, simply run:

```cmd
compile.bat
```

This will create `focus_app.exe` in the same directory.

## Running

Double-click `focus_app.exe` or run from command line:

```cmd
focus_app.exe
```

## Alternative: Using CMake

If you prefer CMake:

```cmd
mkdir build
cd build
cmake ..
cmake --build .
```

The executable will be in the `build` directory.

## Troubleshooting

### "g++ not found" error

This means MinGW is not installed or not in your PATH. Please install MinGW and ensure the `bin` folder is in your PATH.

### Compilation errors

Make sure you're using a recent version of MinGW. If you get errors about missing libraries, try:

```cmd
g++ --version
```

If the version is very old, update MinGW.

### Missing Windows SDK

The app uses Windows API functions. If you get linker errors about missing functions, you may need to:
1. Install Windows SDK
2. Or use a newer version of MinGW that includes the necessary headers

## Files Created

After successful compilation:
- `focus_app.exe` - The main executable
- `compile.bat` - Build script
- `focus_app_simple.cpp` - Source code

## Notes

- The app is **completely standalone** - no installation required
- It's **very lightweight** - uses minimal memory and CPU
- **No dependencies** - just the executable
- **Portable** - can be run from any directory or USB drive

## Customization

You can customize the app by editing `focus_app_simple.cpp`:

- Change `focusMinutes` and `breakMinutes` for different timer durations
- Modify `blockedApps` vector to add/remove blocked applications
- Adjust `WARNING_COOLDOWN_SECONDS` to change how often warnings appear

## License

This is a personal project. Feel free to use and modify as needed.
