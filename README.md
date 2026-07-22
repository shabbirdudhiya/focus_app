# Focus App - Lightweight Windows Focus Timer

A modern, lightweight focus app for Windows that helps you stay productive with a beautiful design matching your HTML app.

## Features

### ✅ Core Features 
- **Modern Clock View** - Displays current time and date with elegant design
- **Focus View** - Pomodoro-style timer with task tracking
- **Theme Toggle** - Switch between Dark and Light modes
- **Fullscreen Mode** - Immersive fullscreen experience
- **Floating Timer Window** - Always-on-top timer that floats above other windows
- **Pomodoro Timer** - 25-minute focus sessions with 5-minute breaks (configurable)
- **App Blocking** - Warns you when using blocked apps during focus sessions
- **Task Tracking** - Set what you're working on
- **Session Tracking** - Visual progress dots showing completed sessions

### 🎨 Design Features
- **Modern UI** - Clean, minimalist design similar to your HTML app
- **Dark/Light Theme** - Toggle with a single click
- **Digital Font** - Monospace timer display (Consolas)
- **Smooth Layout** - Well-organized controls and information
- **Responsive** - Adapts to window resizing

### 🚀 Productivity Features
- **Draggable Timer** - Move the floating timer anywhere on screen
- **Right-click Menu** - Context menu on timer window
- **Warning Cooldown** - Only warns every 5 minutes per blocked app
- **Sound Notifications** - Beep when timer completes
- **Visual Feedback** - Color-coded status (Red=Focus, Green=Break)

## Screenshots

### Clock View
- Current time in large digital format (HH:MM:SS)
- Current date (Weekday, Month Day, Year)
- Progress dots showing session count
- Clean, modern layout

### Focus View
- Task input field
- Large timer display
- Start Focus / Start Break / Pause / Stop buttons
- Session progress tracking
- Status indicator

## Requirements

- **Windows 7 or later**
- **MinGW (g++)** - for compilation (only needed if building from source)

## Quick Start

### Download Pre-built Executable

1. Go to: [Releases](https://github.com/shabbirdudhiya/focus_app/releases)
2. Download the latest `focus_app.exe`
3. Double-click to run (no installation needed!)

### Or Build from Source

1. Install MinGW (make sure g++ is in your PATH)
2. Run `compile.bat`
3. Double-click `focus_app.exe`

## Usage

### Main Window

The main window has two views:

#### Clock View (Default)
- Shows current time and date
- Click "Focus" button to switch to Focus View
- Theme toggle button (☀️/🌙) in top-right
- Fullscreen button (⛶) in top-right

#### Focus View
- Enter your task in the text field
- Large timer display showing time remaining
- **Start Focus** - Begin a 25-minute focus session
- **Start Break** - Begin a 5-minute break session
- **Pause** - Pause the current timer
- **Stop** - Stop the current timer and reset
- **Settings** - Configure focus/break durations
- **Show Timer** - Show the floating timer window
- **Hide Timer** - Hide the floating timer window
- **Exit** - Close the application

### Floating Timer Window

- Always stays on top of other windows
- Semi-transparent with round corners
- Shows: Task name, Timer, Current mode
- **Draggable** - Click and drag to move anywhere
- **Right-click** - Context menu with Close and Settings options
- Color-coded: Red (Focus), Green (Break), White (Idle)

### Settings Window

- **Focus Duration** - Set focus session length in minutes
- **Break Duration** - Set break session length in minutes
- **Blocked Apps List** - View which apps trigger warnings
- **Save** - Apply your settings
- **Close** - Close the settings window

### Blocked Apps

The app will warn you when using these apps during a focus session:
- WhatsApp, Telegram, Discord
- Instagram, YouTube
- Chrome, Firefox, Edge, Opera, Brave, MS Edge

**Warning appears every 5 minutes** to avoid spamming you.

## Keyboard Shortcuts

- **Space** - Start focus / Toggle pause
- **Escape** - Stop timer

## Customization

### Add More Blocked Apps

Edit `focus_app_full.cpp`, find the `blockedApps` vector:

```cpp
std::vector<std::wstring> blockedApps = {
    L"whatsapp", L"telegram", L"discord", L"instagram", L"youtube",
    L"chrome", L"firefox", L"edge", L"opera", L"brave", L"msedge",
    L"spotify", L"steam"  // Add your own
};
```

### Change Default Timer Durations

Edit these values:

```cpp
int focusMinutes = 25;  // Default focus duration
int breakMinutes = 5;   // Default break duration
```

### Change Warning Cooldown

```cpp
const int WARNING_COOLDOWN_SECONDS = 300; // 5 minutes
```

## Building from Source

### Using the build script

```cmd
compile.bat
```

This will create `focus_app.exe` with all features.

### Manual compilation

```cmd
g++ -static -static-libgcc -static-libstdc++ -D_UNICODE -DUNICODE -o focus_app.exe focus_app_full.cpp -luser32 -lgdi32 -lcomctl32 -lshell32 -lpsapi
```

### Using CMake

```cmd
mkdir build
cd build
cmake ..
cmake --build .
```

## Technical Details

- **Language**: C++ with Win32 API
- **No Electron, no bloated frameworks**
- **Single executable** - completely standalone
- **Very lightweight** - minimal memory and CPU usage
- **Pure native Windows app**
- **No dependencies** - just the executable

## Comparison to HTML Version

| Feature | HTML App | Windows App |
|---------|----------|--------------|
| Clock View | ✅ | ✅ |
| Focus View | ✅ | ✅ |
| Theme Toggle | ✅ | ✅ |
| Fullscreen Mode | ✅ | ✅ |
| Settings | ✅ | ✅ |
| Floating Window | ❌ | ✅ |
| App Blocking | ❌ | ✅ |
| Session Tracking | ✅ | ✅ |
| Digital Font | ✅ | ✅ |
| Modern Design | ✅ | ✅ |

**The Windows app has ALL the features of your HTML app PLUS app blocking and floating window!**

## Troubleshooting

### "g++ not found" error

Install MinGW and add it to your PATH:
- Download from: https://www.mingw-w64.org/
- Or use Chocolatey: `choco install mingw`

### Compilation errors

Make sure you're using a recent version of MinGW:
```cmd
g++ --version
```

### Missing Windows SDK

The app uses Windows API functions. If you get linker errors, install Windows SDK or use a newer MinGW version.

## License

This is a personal project. Feel free to use and modify as needed.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Push to your branch
5. Open a pull request

## Support

If you have any questions or issues, please open an issue on GitHub.
