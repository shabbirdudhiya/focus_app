# Focus App - Lightweight Windows Focus Timer

A simple, lightweight focus app for Windows that helps you stay productive by timing your work sessions and warning you when you use blocked apps.

## Features

- **Floating Timer Window**: Always-on-top timer that floats above other windows
- **Pomodoro Technique**: 25-minute focus sessions with 5-minute breaks (configurable)
- **App Blocking**: Warns you when using blocked apps during focus sessions
- **Task Tracking**: Set what you're working on
- **Lightweight**: Pure Win32 API, no Electron, no heavy frameworks
- **Draggable Timer**: Move the floating timer anywhere on screen

## Requirements

- Windows 7 or later
- MinGW (for building from source)

## Building

### Option 1: Using the build script
1. Install MinGW (make sure g++ is in your PATH)
2. Run `build.bat`
3. The executable `focus_app.exe` will be created

### Option 2: Using CMake
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Option 3: Manual compilation
```bash
g++ -static -static-libgcc -static-libstdc++ -D_UNICODE -DUNICODE -o focus_app.exe focus_app.cpp -luser32 -lgdi32 -lcomctl32 -lshell32 -lpsapi
```

## Usage

1. Run `focus_app.exe`
2. The main window will appear with controls
3. A floating timer window will also appear (can be hidden/shown)

### Controls

- **Main Window**:
  - Enter your task in the text field
  - Start Focus: Begin a focus session
  - Start Break: Begin a break session
  - Pause: Pause the current timer
  - Stop: Stop the current timer
  - Settings: Configure focus/break durations
  - Show Timer: Show the floating timer window
  - Hide Timer: Hide the floating timer window
  - Exit: Close the application

- **Floating Timer**:
  - Displays the current time remaining
  - Shows the current task
  - Shows the current mode (FOCUS, BREAK, IDLE)
  - Can be dragged by clicking and moving
  - Right-click for context menu

- **Settings**:
  - Configure focus duration (in minutes)
  - Configure break duration (in minutes)
  - View blocked apps list

## Blocked Apps

The app will warn you when you use these apps during a focus session:
- WhatsApp
- Telegram
- Discord
- Instagram
- YouTube
- Web browsers (Chrome, Firefox, Edge, Opera, Brave, Safari)

The warning appears every 5 minutes to avoid spamming.

## Customization

To customize the blocked apps list, edit the `blockedApps` vector in `focus_app.cpp`:

```cpp
std::vector<std::wstring> blockedApps = {
    L"whatsapp", L"telegram", L"discord", L"instagram", L"youtube",
    L"chrome", L"firefox", L"edge", L"opera", L"brave", L"safari"
};
```

Similarly, you can customize blocked websites in the `blockedSites` vector.

## Keyboard Shortcuts

- **Space**: Start focus / Toggle pause
- **Escape**: Stop timer

## Notes

- The app uses very little memory and CPU
- The floating timer is always on top
- Warnings appear as message boxes and by flashing the timer window
- The app checks for blocked apps every 5 seconds during focus sessions

## License

This is a personal project. Feel free to use and modify as needed.
