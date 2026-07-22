# Focus App - Summary

## What I Created

I've created a **lightweight Windows focus app** that matches your requirements:

### Key Features

1. **Floating Timer Window**
   - Always on top (topmost window)
   - Semi-transparent (240 alpha)
   - Round corners
   - Draggable - click and drag to move anywhere on screen
   - Right-click for context menu
   - Shows: Task name, Timer, Current mode (FOCUS/BREAK/IDLE)

2. **Pomodoro Timer**
   - 25-minute focus sessions (configurable)
   - 5-minute break sessions (configurable)
   - Start Focus / Start Break / Pause / Stop buttons
   - Visual feedback with color coding (red for focus, green for break)

3. **App Blocking with Warnings**
   - Monitors active window every 5 seconds during focus
   - Checks against blocked apps list
   - Shows warning popup when blocked app is detected
   - Warning cooldown: 5 minutes (won't spam you)
   - Flashes timer window to get your attention

4. **Task Tracking**
   - Enter what you're working on
   - Displayed in both main window and floating timer

5. **Settings**
   - Configure focus duration
   - Configure break duration
   - View blocked apps list

### Blocked Apps (Default)
- WhatsApp
- Telegram
- Discord
- Instagram
- YouTube
- Chrome
- Firefox
- Edge
- Opera
- Brave
- MS Edge

### Technical Details

- **Language**: C++ with Win32 API
- **No Electron, no bloated frameworks**
- **Single executable** - completely standalone
- **Very lightweight** - minimal memory and CPU usage
- **Pure native Windows app**

### Files Created

1. **focus_app_simple.cpp** - Main source code (20KB)
2. **compile.bat** - Simple build script
3. **build.bat** - Alternative build script
4. **CMakeLists.txt** - For CMake builds
5. **README.md** - User documentation
6. **BUILD_INSTRUCTIONS.md** - Detailed build guide

## How It Works

### Architecture

- **Main Window**: Control panel with all buttons and settings
- **Timer Window**: Floating window that shows the timer
- **Settings Window**: Configure durations and view blocked apps
- **Background Thread**: Checks for blocked apps every 5 seconds
- **Timer Thread**: Counts down the time

### App Blocking Mechanism

1. Every 5 seconds, the app checks what window is active
2. Gets the process name of the active window
3. Compares against the blocked apps list
4. If blocked, shows a warning (but only once every 5 minutes per app)
5. Flashes the timer window to get your attention

### Why This Approach

- **Lightweight**: Uses Win32 API directly, no heavy frameworks
- **Simple**: Easy to understand and modify
- **Effective**: Warnings appear when you're distracted
- **Non-intrusive**: Only warns, doesn't force-close apps
- **Configurable**: Easy to add/remove blocked apps

## Comparison to HTML Version

Your original HTML file had:
- Clock view
- Focus view with timer
- Settings for focus/break duration
- Session tracking
- Dark/light mode
- Fullscreen mode

The new app has:
- ✅ Timer functionality
- ✅ Focus/break modes
- ✅ Settings for durations
- ✅ Floating window
- ✅ App blocking warnings
- ✅ Task input
- ✅ Always on top
- ✅ Draggable timer
- ❌ No clock view (can be added)
- ❌ No session tracking dots (can be added)
- ❌ No theme switching (can be added)

## How to Build

### Quick Start

1. Install MinGW (g++ compiler)
2. Run `compile.bat`
3. Double-click `focus_app.exe`

### Detailed Instructions

See `BUILD_INSTRUCTIONS.md` for complete setup guide.

## Customization

### Add More Blocked Apps

Edit `focus_app_simple.cpp`, find the `blockedApps` vector and add more:

```cpp
std::vector<std::wstring> blockedApps = {
    L"whatsapp", L"telegram", L"discord", L"instagram", L"youtube",
    L"chrome", L"firefox", L"edge", L"opera", L"brave", L"msedge",
    L"spotify", L"steam", L"origin"  // Add your own
};
```

### Change Timer Durations

Edit the default values:

```cpp
int focusMinutes = 25;  // Change to your preferred focus time
int breakMinutes = 5;   // Change to your preferred break time
```

### Change Warning Cooldown

```cpp
const int WARNING_COOLDOWN_SECONDS = 300; // 5 minutes - change as needed
```

## Future Enhancements

If you want to add more features, here are some ideas:

1. **Session Tracking**: Count completed focus sessions
2. **Statistics**: Track time spent in focus vs break
3. **Sound Notifications**: Play a sound when timer ends
4. **More Customization**: Fonts, colors, window size
5. **Persistent Settings**: Save settings to a file
6. **Tray Icon**: Minimize to system tray
7. **Hotkeys**: Global keyboard shortcuts
8. **Website Blocking**: More sophisticated URL detection

## Why C++ and Win32?

- **Performance**: Native code, very fast
- **Size**: Tiny executable (a few hundred KB)
- **Memory**: Uses minimal RAM
- **No Dependencies**: Just the executable
- **Control**: Full access to Windows API
- **Lightweight**: Exactly what you asked for

## Alternative Approaches Considered

1. **Electron**: Too heavy, uses Chromium (~100MB+)
2. **Python with Tkinter**: Requires Python runtime
3. **Python with PyQt**: Requires Qt libraries
4. **C# with WPF**: Requires .NET runtime
5. **AutoHotkey**: Limited UI capabilities

**C++ with Win32 API was the best choice** for a truly lightweight, standalone app.

## Questions?

If you have any questions about the code or want modifications, just ask!
