# Focus App - Lightweight Windows Productivity Tool

A minimal, native Windows application that helps you stay focused by monitoring your app usage and showing warnings when you use blacklisted applications.

## Features

- **Floating Timer Widget**: Always-on-top timer that shows your focus/break time
- **App Monitoring**: Tracks which applications you're using
- **Blacklist System**: Warns you when using blacklisted apps (WhatsApp, social media, etc.)
- **Whitelist Mode**: Only allows apps you explicitly whitelist
- **Configurable Warnings**: Set warning delay (1-5 minutes)
- **Tray Icon**: Minimize to system tray
- **Lightweight**: Pure Win32 API, no Electron, no bloat

## Requirements

- Windows 7 or later
- Visual Studio 2022 (for building)
- CMake 3.15+ (optional)

## Building

### Using Visual Studio

1. Open `FocusApp.sln` in Visual Studio
2. Build the solution (F7)
3. Run `FocusApp.exe`

### Using CMake

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Using MinGW

```bash
make
```

## Configuration

The app creates a configuration file at:
`%APPDATA%\FocusApp\config.txt`

You can edit this file to customize:
- Focus and break durations
- Warning delay
- Blacklisted/whitelisted apps
- Window position

### Default Blacklisted Apps
- whatsapp
- telegram
- discord
- facebook
- instagram
- twitter
- tiktok

### Default Blacklisted Websites
- youtube.com
- facebook.com
- instagram.com
- twitter.com
- tiktok.com
- netflix.com
- amazon.com
- reddit.com

### Default Whitelisted Apps
- focusapp
- notepad
- calc
- explorer

## Usage

1. Run `FocusApp.exe`
2. The floating timer widget will appear
3. Drag the widget to position it
4. Right-click the widget for options
5. Right-click the tray icon to pause/resume monitoring

### Controls

- **Tray Icon Menu**:
  - Show Timer: Show the floating widget
  - Settings: (Coming soon)
  - Pause Monitoring: Temporarily pause app monitoring
  - Resume Monitoring: Resume app monitoring
  - Exit: Quit the application

## Customization

### Adding to Blacklist

Edit `config.txt` and add lines like:
```
blacklist_app=spotify
blacklist_website=twitch.tv
```

### Adding to Whitelist

Edit `config.txt` and add lines like:
```
whitelist_app=visualstudio
whitelist_app=chrome
```

### Changing Timer Settings

Edit `config.txt`:
```
focus_duration=25
break_duration=5
warning_delay=2
```

## Implementation Details

- **Pure Win32 API**: No frameworks, no Electron, minimal dependencies
- **Low Memory Usage**: Uses ~5-10MB RAM
- **Low CPU Usage**: Uses hooks efficiently, minimal polling
- **Portable**: Single executable, no installation required

## Architecture

- `main.cpp`: Main application entry point, message loop
- `TimerWidget.cpp`: Floating timer window implementation
- `AppMonitor.cpp`: Application usage monitoring via Windows hooks
- `Config.cpp`: Configuration management

## License

MIT License - Feel free to use, modify, and distribute.

## Future Features

- [ ] Settings dialog
- [ ] Add/remove apps from blacklist/whitelist via UI
- [ ] Website monitoring (browser tab detection)
- [ ] Hard blocking mode (force minimize windows)
- [ ] Statistics and usage reports
- [ ] Multiple focus profiles
- [ ] Sound notifications
