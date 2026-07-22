#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <tlhelp32.h>
#include <psapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

// Global variables
HINSTANCE hInst;
HWND hMainWnd, hTimerWnd, hSettingsWnd;
HFONT hFont, hFontLarge, hFontDigital, hFontTitle;
HBRUSH hDarkBrush, hLightBrush, hBackgroundBrush;
COLORREF textColor, bgColor, borderColor;

// App state
enum class AppMode { IDLE, FOCUS, BREAK };
enum class ViewMode { CLOCK_VIEW, FOCUS_VIEW };

AppMode currentMode = AppMode::IDLE;
ViewMode currentView = ViewMode::CLOCK_VIEW;
bool isPaused = false;
bool isRunning = false;
bool isDarkMode = true;
bool isFullscreen = false;
int focusMinutes = 25;
int breakMinutes = 5;
int timeLeft = 25 * 60;
std::wstring currentTask = L"What are you working on?";
int sessionsCompleted = 0;

// Blocked apps
std::vector<std::wstring> blockedApps = {
    L"whatsapp", L"telegram", L"discord", L"instagram", L"youtube",
    L"chrome", L"firefox", L"edge", L"opera", L"brave", L"msedge"
};

// Last warning time for each process
std::map<std::wstring, std::chrono::system_clock::time_point> lastWarningTimes;
const int WARNING_COOLDOWN_SECONDS = 300; // 5 minutes

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TimerWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SettingsWndProc(HWND, UINT, WPARAM, LPARAM);

void CreateMainWindow();
void UpdateTheme();
void UpdateTimerDisplay();
void StartFocusTimer();
void StartBreakTimer();
void StopTimer();
void TogglePause();
void ToggleView();
void ToggleFullscreen();
void CheckBlockedApps();
std::wstring FormatTime(int seconds);
std::wstring GetCurrentTime();
std::wstring GetCurrentDate();
bool IsProcessBlocked(const std::wstring& processName);
void ShowWarning(const std::wstring& message);

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    hInst = hInstance;
    
    // Initialize common controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icc);
    
    // Create fonts
    hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    hFontLarge = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    hFontDigital = CreateFont(72, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, L"Consolas");
    hFontTitle = CreateFont(18, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    
    // Register window classes
    WNDCLASSEX wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInst;
    wcex.lpszClassName = L"FocusAppMain";
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    RegisterClassEx(&wcex);
    
    wcex.lpfnWndProc = TimerWndProc;
    wcex.lpszClassName = L"FocusAppTimer";
    RegisterClassEx(&wcex);
    
    wcex.lpfnWndProc = SettingsWndProc;
    wcex.lpszClassName = L"FocusAppSettings";
    RegisterClassEx(&wcex);
    
    // Create windows
    CreateMainWindow();
    
    hTimerWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        L"FocusAppTimer", L"Focus Timer",
        WS_POPUP | WS_VISIBLE, 100, 100, 300, 150,
        nullptr, nullptr, hInst, nullptr);
    
    hSettingsWnd = CreateWindowEx(0, L"FocusAppSettings", L"Settings",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        nullptr, nullptr, hInst, nullptr);
    
    // Make timer window semi-transparent
    SetLayeredWindowAttributes(hTimerWnd, 0, 240, LWA_ALPHA);
    
    // Make timer window round
    HRGN hRgn = CreateRoundRectRgn(0, 0, 300, 150, 20, 20);
    SetWindowRgn(hTimerWnd, hRgn, TRUE);
    DeleteObject(hRgn);
    
    // Set initial theme
    UpdateTheme();
    
    // Show main window
    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);
    
    // Start checking for blocked apps in a separate thread
    std::thread checkerThread([]() {
        while (true) {
            CheckBlockedApps();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });
    checkerThread.detach();
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

// Create main window
void CreateMainWindow()
{
    hMainWnd = CreateWindowEx(0, L"FocusAppMain", L"Focus App",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 700,
        nullptr, nullptr, hInst, nullptr);
}

// Update theme colors
void UpdateTheme()
{
    if (isDarkMode)
    {
        bgColor = RGB(15, 23, 42); // slate-950
        textColor = RGB(248, 250, 252); // slate-50
        borderColor = RGB(51, 65, 85); // slate-800
        
        if (hDarkBrush) DeleteObject(hDarkBrush);
        hDarkBrush = CreateSolidBrush(bgColor);
        hBackgroundBrush = hDarkBrush;
    }
    else
    {
        bgColor = RGB(248, 250, 252); // slate-50
        textColor = RGB(15, 23, 42); // slate-950
        borderColor = RGB(203, 213, 225); // slate-200
        
        if (hLightBrush) DeleteObject(hLightBrush);
        hLightBrush = CreateSolidBrush(bgColor);
        hBackgroundBrush = hLightBrush;
    }
    
    if (hMainWnd) InvalidateRect(hMainWnd, nullptr, TRUE);
    if (hTimerWnd) InvalidateRect(hTimerWnd, nullptr, TRUE);
    if (hSettingsWnd) InvalidateRect(hSettingsWnd, nullptr, TRUE);
}

// Format time as MM:SS
std::wstring FormatTime(int seconds)
{
    int minutes = seconds / 60;
    int secs = seconds % 60;
    std::wstringstream ss;
    ss << std::setw(2) << std::setfill(L'0') << minutes << L":" 
       << std::setw(2) << std::setfill(L'0') << secs;
    return ss.str();
}

// Get current time as HH:MM:SS
std::wstring GetCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_struct;
    localtime_s(&tm_struct, &in_time_t);
    
    std::wstringstream ss;
    ss << std::setw(2) << std::setfill(L'0') << tm_struct.tm_hour << L":"
       << std::setw(2) << std::setfill(L'0') << tm_struct.tm_min << L":"
       << std::setw(2) << std::setfill(L'0') << tm_struct.tm_sec;
    return ss.str();
}

// Get current date
std::wstring GetCurrentDate()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::tm tm_struct;
    localtime_s(&tm_struct, &in_time_t);
    
    const wchar_t* weekdays[] = {L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday"};
    const wchar_t* months[] = {L"January", L"February", L"March", L"April", L"May", L"June",
                               L"July", L"August", L"September", L"October", L"November", L"December"};
    
    std::wstringstream ss;
    ss << weekdays[tm_struct.tm_wday] << L", "
       << months[tm_struct.tm_mon] << L" "
       << tm_struct.tm_mday << L", "
       << (tm_struct.tm_year + 1900);
    return ss.str();
}

// Update timer display
void UpdateTimerDisplay()
{
    if (hTimerWnd) InvalidateRect(hTimerWnd, nullptr, TRUE);
    if (hMainWnd) InvalidateRect(hMainWnd, nullptr, TRUE);
}

// Start focus timer
void StartFocusTimer()
{
    if (isRunning) return;
    isRunning = true;
    isPaused = false;
    currentMode = AppMode::FOCUS;
    timeLeft = focusMinutes * 60;
    
    std::thread timerThread([]() {
        while (isRunning && !isPaused && timeLeft > 0 && currentMode == AppMode::FOCUS)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            timeLeft--;
            UpdateTimerDisplay();
            
            if (timeLeft <= 0)
            {
                MessageBeep(MB_ICONINFORMATION);
                sessionsCompleted++;
                StartBreakTimer();
                break;
            }
        }
    });
    timerThread.detach();
    UpdateTimerDisplay();
}

// Start break timer
void StartBreakTimer()
{
    if (isRunning) return;
    isRunning = true;
    isPaused = false;
    currentMode = AppMode::BREAK;
    timeLeft = breakMinutes * 60;
    
    std::thread timerThread([]() {
        while (isRunning && !isPaused && timeLeft > 0 && currentMode == AppMode::BREAK)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            timeLeft--;
            UpdateTimerDisplay();
            
            if (timeLeft <= 0)
            {
                MessageBeep(MB_ICONINFORMATION);
                StopTimer();
                break;
            }
        }
    });
    timerThread.detach();
    UpdateTimerDisplay();
}

// Stop timer
void StopTimer()
{
    isRunning = false;
    isPaused = false;
    currentMode = AppMode::IDLE;
    timeLeft = focusMinutes * 60;
    UpdateTimerDisplay();
}

// Toggle pause
void TogglePause()
{
    if (!isRunning) return;
    isPaused = !isPaused;
    UpdateTimerDisplay();
}

// Toggle view between clock and focus
void ToggleView()
{
    if (currentView == ViewMode::CLOCK_VIEW)
    {
        currentView = ViewMode::FOCUS_VIEW;
    }
    else
    {
        currentView = ViewMode::CLOCK_VIEW;
    }
    InvalidateRect(hMainWnd, nullptr, TRUE);
}

// Toggle fullscreen
void ToggleFullscreen()
{
    if (isFullscreen)
    {
        SetWindowLong(hMainWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        SetWindowPos(hMainWnd, nullptr, 0, 0, 600, 700, SWP_FRAMECHANGED);
        isFullscreen = false;
    }
    else
    {
        MONITORINFO mi = {sizeof(MONITORINFO)};
        GetMonitorInfo(MonitorFromWindow(hMainWnd, MONITOR_DEFAULTTOPRIMARY), &mi);
        SetWindowLong(hMainWnd, GWL_STYLE, WS_POPUP);
        SetWindowPos(hMainWnd, nullptr, mi.rcMonitor.left, mi.rcMonitor.top, 
                     mi.rcMonitor.right - mi.rcMonitor.left, 
                     mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_FRAMECHANGED);
        isFullscreen = true;
    }
}

// Check for blocked apps
void CheckBlockedApps()
{
    if (currentMode != AppMode::FOCUS || isPaused) return;
    
    HWND hForeground = GetForegroundWindow();
    if (!hForeground) return;
    
    DWORD pid;
    GetWindowThreadProcessId(hForeground, &pid);
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return;
    
    wchar_t processName[MAX_PATH] = {0};
    if (GetModuleFileNameEx(hProcess, nullptr, processName, MAX_PATH))
    {
        std::wstring procName = processName;
        // Extract just the filename
        size_t pos = procName.find_last_of(L"\\/");
        if (pos != std::wstring::npos)
        {
            procName = procName.substr(pos + 1);
        }
        // Remove .exe extension
        pos = procName.find_last_of(L".");
        if (pos != std::wstring::npos)
        {
            procName = procName.substr(0, pos);
        }
        
        // Convert to lowercase for comparison
        std::wstring lowerProcName = procName;
        for (auto& c : lowerProcName) c = towlower(c);
        
        // Check if blocked
        if (IsProcessBlocked(lowerProcName))
        {
            // Check cooldown
            auto now = std::chrono::system_clock::now();
            auto last = lastWarningTimes.find(lowerProcName);
            
            if (last == lastWarningTimes.end() || 
                std::chrono::duration_cast<std::chrono::seconds>(now - last->second).count() > WARNING_COOLDOWN_SECONDS)
            {
                lastWarningTimes[lowerProcName] = now;
                ShowWarning(L"You're using a blocked app: " + procName + L"\n\nGet back to work!");
            }
        }
    }
    
    CloseHandle(hProcess);
}

// Check if process is blocked
bool IsProcessBlocked(const std::wstring& processName)
{
    for (const auto& blocked : blockedApps)
    {
        if (processName.find(blocked) != std::wstring::npos)
        {
            return true;
        }
    }
    return false;
}

// Show warning message
void ShowWarning(const std::wstring& message)
{
    // Bring timer window to front
    if (hTimerWnd)
    {
        SetForegroundWindow(hTimerWnd);
        FLASHWINFO flash = {0};
        flash.cbSize = sizeof(FLASHWINFO);
        flash.hwnd = hTimerWnd;
        flash.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
        flash.uCount = 3;
        flash.dwTimeout = 500;
        FlashWindowEx(&flash);
    }
    
    MessageBox(nullptr, message.c_str(), L"Focus App Warning", MB_ICONWARNING | MB_OK);
}

// Draw a rounded rectangle
void DrawRoundedRect(HDC hdc, int x, int y, int width, int height, int radius, COLORREF color)
{
    HRGN hRgn = CreateRoundRectRgn(x, y, x + width, y + height, radius, radius);
    HBRUSH hBrush = CreateSolidBrush(color);
    FillRgn(hdc, hRgn, hBrush);
    DeleteObject(hRgn);
    DeleteObject(hBrush);
}

// Draw session dots
void DrawSessionDots(HDC hdc, int x, int y, int count, int active)
{
    for (int i = 0; i < count; i++)
    {
        COLORREF color = (i < active) ? RGB(255, 255, 255) : RGB(100, 100, 100);
        HBRUSH hBrush = CreateSolidBrush(color);
        Ellipse(hdc, x + i * 20, y, x + i * 20 + 12, y + 12);
        DeleteObject(hBrush);
    }
}

// Main window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
            // Store window handle
            hMainWnd = hWnd;
            
            // Create buttons - Clock View
            CreateWindow(L"BUTTON", L"Focus", WS_VISIBLE | WS_CHILD | BS_FLAT,
                20, 20, 80, 30, hWnd, (HMENU)10, hInst, nullptr);
            
            // Theme toggle button
            CreateWindow(L"BUTTON", isDarkMode ? L"☀️" : L"🌙", WS_VISIBLE | WS_CHILD | BS_FLAT,
                560, 20, 40, 30, hWnd, (HMENU)11, hInst, nullptr);
            
            // Fullscreen toggle button
            CreateWindow(L"BUTTON", L"⛶", WS_VISIBLE | WS_CHILD | BS_FLAT,
                520, 20, 40, 30, hWnd, (HMENU)12, hInst, nullptr);
            
            // Focus View buttons (initially hidden)
            CreateWindow(L"EDIT", currentTask.c_str(), 
                WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                50, 100, 400, 30, hWnd, (HMENU)100, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Start Focus", WS_CHILD,
                50, 150, 120, 40, hWnd, (HMENU)1, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Start Break", WS_CHILD,
                180, 150, 120, 40, hWnd, (HMENU)2, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Pause", WS_CHILD,
                310, 150, 120, 40, hWnd, (HMENU)3, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Stop", WS_CHILD,
                440, 150, 120, 40, hWnd, (HMENU)4, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Settings", WS_CHILD,
                250, 200, 120, 40, hWnd, (HMENU)5, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Show Timer", WS_CHILD,
                250, 250, 120, 40, hWnd, (HMENU)6, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Hide Timer", WS_CHILD,
                250, 300, 120, 40, hWnd, (HMENU)7, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Exit", WS_CHILD,
                250, 350, 120, 40, hWnd, (HMENU)8, hInst, nullptr);
        }
        break;
        
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case 1: StartFocusTimer(); break;
            case 2: StartBreakTimer(); break;
            case 3: TogglePause(); break;
            case 4: StopTimer(); break;
            case 5: ShowWindow(hSettingsWnd, SW_SHOW); break;
            case 6: ShowWindow(hTimerWnd, SW_SHOW); break;
            case 7: ShowWindow(hTimerWnd, SW_HIDE); break;
            case 8: PostQuitMessage(0); break;
            case 10: ToggleView(); break;
            case 11: 
                isDarkMode = !isDarkMode;
                UpdateTheme();
                InvalidateRect(hWnd, nullptr, TRUE);
                break;
            case 12: ToggleFullscreen(); break;
            case 100: // Task edit
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    HWND hEdit = (HWND)lParam;
                    wchar_t text[256] = {0};
                    GetWindowText(hEdit, text, 256);
                    currentTask = text;
                    UpdateTimerDisplay();
                }
                break;
            }
        }
        break;
        
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            RECT rc;
            GetClientRect(hWnd, &rc);
            
            // Fill background
            HBRUSH hBrush = CreateSolidBrush(bgColor);
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
            
            SetTextColor(hdc, textColor);
            SetBkMode(hdc, TRANSPARENT);
            
            if (currentView == ViewMode::CLOCK_VIEW)
            {
                // Clock View
                SelectObject(hdc, hFontTitle);
                TextOut(hdc, rc.left + 20, rc.top + 80, L"Current Time", 12);
                
                SelectObject(hdc, hFontDigital);
                std::wstring currentTime = GetCurrentTime();
                TextOut(hdc, rc.left + 50, rc.top + 120, currentTime.c_str(), currentTime.length());
                
                SelectObject(hdc, hFont);
                std::wstring currentDate = GetCurrentDate();
                TextOut(hdc, rc.left + 50, rc.top + 200, currentDate.c_str(), currentDate.length());
                
                // Session tracking
                SelectObject(hdc, hFont);
                std::wstring sessionText = L"Progress: " + std::to_wstring(sessionsCompleted);
                TextOut(hdc, rc.left + 50, rc.top + 250, sessionText.c_str(), sessionText.length());
                
                // Draw session dots
                DrawSessionDots(hdc, rc.left + 50, rc.top + 280, 5, sessionsCompleted);
            }
            else
            {
                // Focus View
                SelectObject(hdc, hFontTitle);
                TextOut(hdc, rc.left + 20, rc.top + 80, L"What are you working on?", 22);
                
                // Task input (visual only, actual edit is a separate control)
                SelectObject(hdc, hFontLarge);
                TextOut(hdc, rc.left + 50, rc.top + 120, currentTask.c_str(), currentTask.length());
                
                // Timer display
                std::wstring timeStr = FormatTime(timeLeft);
                
                COLORREF timerColor;
                switch (currentMode)
                {
                case AppMode::FOCUS: timerColor = RGB(255, 100, 100); break;
                case AppMode::BREAK: timerColor = RGB(100, 255, 100); break;
                default: timerColor = textColor; break;
                }
                
                SetTextColor(hdc, timerColor);
                SelectObject(hdc, hFontDigital);
                TextOut(hdc, rc.left + 150, rc.top + 180, timeStr.c_str(), timeStr.length());
                
                SetTextColor(hdc, textColor);
                SelectObject(hdc, hFont);
                
                std::wstring status;
                switch (currentMode)
                {
                case AppMode::FOCUS: status = L"Deep Work"; break;
                case AppMode::BREAK: status = L"Take a Break"; break;
                default: status = L"Ready to Focus?"; break;
                }
                if (isPaused) status += L" - PAUSED";
                
                TextOut(hdc, rc.left + 200, rc.top + 280, status.c_str(), status.length());
                
                // Session tracking
                std::wstring sessionText = L"Progress: " + std::to_wstring(sessionsCompleted);
                TextOut(hdc, rc.left + 50, rc.top + 320, sessionText.c_str(), sessionText.length());
                DrawSessionDots(hdc, rc.left + 50, rc.top + 350, 5, sessionsCompleted);
            }
            
            EndPaint(hWnd, &ps);
        }
        break;
        
    case WM_SIZE:
        InvalidateRect(hWnd, nullptr, TRUE);
        break;
        
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
        
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Timer window procedure
LRESULT CALLBACK TimerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static POINT dragStart = {0, 0};
    static bool isDragging = false;
    
    switch (message)
    {
    case WM_LBUTTONDOWN:
        dragStart.x = LOWORD(lParam);
        dragStart.y = HIWORD(lParam);
        isDragging = true;
        SetCapture(hWnd);
        break;
        
    case WM_MOUSEMOVE:
        if (isDragging)
        {
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            RECT rc;
            GetWindowRect(hWnd, &rc);
            
            int dx = pt.x - dragStart.x;
            int dy = pt.y - dragStart.y;
            
            SetWindowPos(hWnd, nullptr, rc.left + dx, rc.top + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        break;
        
    case WM_LBUTTONUP:
        isDragging = false;
        ReleaseCapture();
        break;
        
    case WM_RBUTTONDOWN:
        {
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1, L"Close");
            AppendMenu(hMenu, MF_STRING, 2, L"Settings");
            
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            ClientToScreen(hWnd, &pt);
            
            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, nullptr);
            
            if (cmd == 1) ShowWindow(hWnd, SW_HIDE);
            else if (cmd == 2) ShowWindow(hSettingsWnd, SW_SHOW);
            
            DestroyMenu(hMenu);
        }
        break;
        
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            RECT rc;
            GetClientRect(hWnd, &rc);
            
            // Fill background
            HBRUSH hBrush = CreateSolidBrush(RGB(45, 45, 48));
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
            
            // Draw border
            HPEN hPen = CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
            SelectObject(hdc, hPen);
            RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 20, 20);
            DeleteObject(hPen);
            
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            
            // Draw task
            SelectObject(hdc, hFont);
            TextOut(hdc, 20, 20, currentTask.c_str(), currentTask.length());
            
            // Draw timer
            std::wstring timeStr = FormatTime(timeLeft);
            
            COLORREF timerColor;
            switch (currentMode)
            {
            case AppMode::FOCUS: timerColor = RGB(255, 100, 100); break;
            case AppMode::BREAK: timerColor = RGB(100, 255, 100); break;
            default: timerColor = RGB(255, 255, 255); break;
            }
            
            SetTextColor(hdc, timerColor);
            SelectObject(hdc, hFontDigital);
            TextOut(hdc, 80, 50, timeStr.c_str(), timeStr.length());
            
            // Draw status
            SetTextColor(hdc, RGB(150, 150, 150));
            SelectObject(hdc, hFont);
            
            std::wstring status;
            switch (currentMode)
            {
            case AppMode::FOCUS: status = L"FOCUS"; break;
            case AppMode::BREAK: status = L"BREAK"; break;
            default: status = L"IDLE"; break;
            }
            if (isPaused) status += L" - PAUSED";
            
            TextOut(hdc, 20, 100, status.c_str(), status.length());
            
            EndPaint(hWnd, &ps);
        }
        break;
        
    case WM_DESTROY:
        break;
        
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Settings window procedure
LRESULT CALLBACK SettingsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
            CreateWindow(L"STATIC", L"Focus Duration (minutes):", WS_VISIBLE | WS_CHILD,
                20, 20, 180, 20, hWnd, nullptr, hInst, nullptr);
            
            HWND hFocusEdit = CreateWindow(L"EDIT", std::to_wstring(focusMinutes).c_str(),
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
                210, 20, 50, 25, hWnd, (HMENU)100, hInst, nullptr);
            SendMessage(hFocusEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            CreateWindow(L"STATIC", L"Break Duration (minutes):", WS_VISIBLE | WS_CHILD,
                20, 60, 180, 20, hWnd, nullptr, hInst, nullptr);
            
            HWND hBreakEdit = CreateWindow(L"EDIT", std::to_wstring(breakMinutes).c_str(),
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
                210, 60, 50, 25, hWnd, (HMENU)101, hInst, nullptr);
            SendMessage(hBreakEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            CreateWindow(L"BUTTON", L"Save", WS_VISIBLE | WS_CHILD,
                100, 120, 80, 30, hWnd, (HMENU)1, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Close", WS_VISIBLE | WS_CHILD,
                200, 120, 80, 30, hWnd, (HMENU)2, hInst, nullptr);
            
            CreateWindow(L"STATIC", L"Blocked Apps:", WS_VISIBLE | WS_CHILD,
                20, 170, 100, 20, hWnd, nullptr, hInst, nullptr);
            
            HWND hAppsList = CreateWindow(L"LISTBOX", L"",
                WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_STANDARD,
                20, 200, 350, 80, hWnd, (HMENU)102, hInst, nullptr);
            SendMessage(hAppsList, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            for (const auto& app : blockedApps)
            {
                SendMessage(hAppsList, LB_ADDSTRING, 0, (LPARAM)app.c_str());
            }
        }
        break;
        
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case 1: // Save
                {
                    HWND hFocusEdit = GetDlgItem(hWnd, 100);
                    HWND hBreakEdit = GetDlgItem(hWnd, 101);
                    
                    wchar_t text[16] = {0};
                    GetWindowText(hFocusEdit, text, 16);
                    focusMinutes = _wtoi(text);
                    
                    GetWindowText(hBreakEdit, text, 16);
                    breakMinutes = _wtoi(text);
                    
                    if (currentMode == AppMode::IDLE)
                    {
                        timeLeft = focusMinutes * 60;
                    }
                    
                    UpdateTimerDisplay();
                    MessageBox(hWnd, L"Settings saved!", L"Success", MB_ICONINFORMATION | MB_OK);
                }
                break;
            case 2: // Close
                ShowWindow(hWnd, SW_HIDE);
                break;
            }
        }
        break;
        
    case WM_DESTROY:
        break;
        
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
