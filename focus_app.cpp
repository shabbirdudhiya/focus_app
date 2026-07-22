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
#include <shellapi.h>
#include <tlhelp32.h>
#include <psapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

// Global variables
HINSTANCE hInst;
HWND hMainWnd, hTimerWnd, hSettingsWnd;
HFONT hFont, hFontLarge, hFontDigital;

// App state
enum class AppMode { IDLE, FOCUS, BREAK };
AppMode currentMode = AppMode::IDLE;
bool isPaused = false;
bool isRunning = false;
int focusMinutes = 25;
int breakMinutes = 5;
int timeLeft = focusMinutes * 60;
std::wstring currentTask = L"What are you working on?";

// Blocked apps/websites
std::vector<std::wstring> blockedApps = {
    L"whatsapp", L"telegram", L"discord", L"instagram", L"youtube",
    L"chrome", L"firefox", L"edge", L"opera", L"brave", L"safari"
};
std::vector<std::wstring> blockedSites = {
    L"youtube.com", L"instagram.com", L"facebook.com", L"twitter.com",
    L"x.com", L"reddit.com", L"tiktok.com", L"netflix.com"
};

// Last warning time for each process
std::map<std::wstring, std::chrono::system_clock::time_point> lastWarningTimes;
const int WARNING_COOLDOWN_SECONDS = 300; // 5 minutes

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TimerWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SettingsWndProc(HWND, UINT, WPARAM, LPARAM);

void CreateMainWindow();
void CreateTimerWindow();
void CreateSettingsWindow();
void UpdateTimerDisplay();
void StartFocusTimer();
void StartBreakTimer();
void StopTimer();
void TogglePause();
void CheckBlockedApps();
std::wstring FormatTime(int seconds);
std::wstring GetWindowTitle(HWND hwnd);
std::wstring GetProcessName(DWORD pid);
bool IsProcessBlocked(const std::wstring& processName);
bool IsUrlBlocked(const std::wstring& url);
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
    hFontDigital = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, L"Consolas");
    
    // Create windows
    CreateMainWindow();
    CreateTimerWindow();
    CreateSettingsWindow();
    
    // Show main window
    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);
    
    // Start checking for blocked apps
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
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"FocusAppMain";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    
    RegisterClassEx(&wcex);
    
    hMainWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"FocusAppMain", L"Focus App",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 600,
        nullptr, nullptr, hInst, nullptr);
}

// Create timer window (floating)
void CreateTimerWindow()
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = TimerWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"FocusAppTimer";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    
    RegisterClassEx(&wcex);
    
    hTimerWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        L"FocusAppTimer", L"Focus Timer",
        WS_POPUP | WS_VISIBLE, 100, 100, 300, 150,
        nullptr, nullptr, hInst, nullptr);
    
    // Make window semi-transparent
    SetLayeredWindowAttributes(hTimerWnd, 0, 240, LWA_ALPHA);
    
    // Make it draggable
    // We'll handle this in the window procedure
}

// Create settings window
void CreateSettingsWindow()
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = SettingsWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"FocusAppSettings";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    
    RegisterClassEx(&wcex);
    
    hSettingsWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"FocusAppSettings", L"Settings",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        nullptr, nullptr, hInst, nullptr);
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

// Update timer display
void UpdateTimerDisplay()
{
    if (hTimerWnd)
    {
        InvalidateRect(hTimerWnd, nullptr, TRUE);
        UpdateWindow(hTimerWnd);
    }
    if (hMainWnd)
    {
        InvalidateRect(hMainWnd, nullptr, TRUE);
        UpdateWindow(hMainWnd);
    }
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
                // Focus time ended, start break
                MessageBeep(MB_ICONINFORMATION);
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
                // Break time ended
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
        std::transform(lowerProcName.begin(), lowerProcName.end(), lowerProcName.begin(), ::towlower);
        
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

// Check if URL is blocked
bool IsUrlBlocked(const std::wstring& url)
{
    std::wstring lowerUrl = url;
    std::transform(lowerUrl.begin(), lowerUrl.end(), lowerUrl.begin(), ::towlower);
    
    for (const auto& blocked : blockedSites)
    {
        if (lowerUrl.find(blocked) != std::wstring::npos)
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
        // Flash window
        FLASHWINFO flash;
        flash.cbSize = sizeof(FLASHWINFO);
        flash.hwnd = hTimerWnd;
        flash.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
        flash.uCount = 3;
        flash.dwTimeout = 500;
        FlashWindowEx(&flash);
    }
    
    // Show message box
    MessageBox(nullptr, message.c_str(), L"Focus App Warning", MB_ICONWARNING | MB_OK);
}

// Get window title
std::wstring GetWindowTitle(HWND hwnd)
{
    wchar_t title[256] = {0};
    GetWindowText(hwnd, title, 256);
    return std::wstring(title);
}

// Get process name from PID
std::wstring GetProcessName(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return L"";
    
    wchar_t processName[MAX_PATH] = {0};
    if (GetModuleFileNameEx(hProcess, nullptr, processName, MAX_PATH))
    {
        CloseHandle(hProcess);
        return std::wstring(processName);
    }
    
    CloseHandle(hProcess);
    return L"";
}

// Main window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
            // Create controls
            CreateWindow(L"STATIC", L"Focus App", WS_VISIBLE | WS_CHILD | SS_CENTER,
                10, 10, 380, 30, hWnd, nullptr, hInst, nullptr);
            
            CreateWindow(L"STATIC", L"Task:", WS_VISIBLE | WS_CHILD,
                20, 50, 50, 20, hWnd, nullptr, hInst, nullptr);
            
            HWND hTaskEdit = CreateWindow(L"EDIT", currentTask.c_str(), 
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                80, 50, 280, 25, hWnd, (HMENU)100, hInst, nullptr);
            SendMessage(hTaskEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            CreateWindow(L"STATIC", L"Timer:", WS_VISIBLE | WS_CHILD,
                20, 90, 50, 20, hWnd, nullptr, hInst, nullptr);
            
            HWND hTimerDisplay = CreateWindow(L"STATIC", FormatTime(timeLeft).c_str(),
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                80, 90, 280, 40, hWnd, (HMENU)101, hInst, nullptr);
            SendMessage(hTimerDisplay, WM_SETFONT, (WPARAM)hFontDigital, TRUE);
            
            CreateWindow(L"BUTTON", L"Start Focus", WS_VISIBLE | WS_CHILD,
                50, 150, 120, 40, hWnd, (HMENU)1, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Start Break", WS_VISIBLE | WS_CHILD,
                180, 150, 120, 40, hWnd, (HMENU)2, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Pause", WS_VISIBLE | WS_CHILD,
                50, 200, 120, 40, hWnd, (HMENU)3, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Stop", WS_VISIBLE | WS_CHILD,
                180, 200, 120, 40, hWnd, (HMENU)4, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Settings", WS_VISIBLE | WS_CHILD,
                115, 250, 120, 40, hWnd, (HMENU)5, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Show Timer", WS_VISIBLE | WS_CHILD,
                115, 300, 120, 40, hWnd, (HMENU)6, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Hide Timer", WS_VISIBLE | WS_CHILD,
                115, 350, 120, 40, hWnd, (HMENU)7, hInst, nullptr);
            
            CreateWindow(L"BUTTON", L"Exit", WS_VISIBLE | WS_CHILD,
                115, 400, 120, 40, hWnd, (HMENU)8, hInst, nullptr);
        }
        break;
        
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case 1: // Start Focus
                StartFocusTimer();
                break;
            case 2: // Start Break
                StartBreakTimer();
                break;
            case 3: // Pause
                TogglePause();
                break;
            case 4: // Stop
                StopTimer();
                break;
            case 5: // Settings
                ShowWindow(hSettingsWnd, SW_SHOW);
                break;
            case 6: // Show Timer
                ShowWindow(hTimerWnd, SW_SHOW);
                break;
            case 7: // Hide Timer
                ShowWindow(hTimerWnd, SW_HIDE);
                break;
            case 8: // Exit
                PostQuitMessage(0);
                break;
            case 100: // Task edit
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    HWND hEdit = (HWND)lParam;
                    wchar_t text[256] = {0};
                    GetWindowText(hEdit, text, 256);
                    currentTask = text;
                }
                break;
            }
        }
        break;
        
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            // Draw status
            std::wstring status;
            switch (currentMode)
            {
            case AppMode::FOCUS:
                status = L"Status: FOCUS";
                break;
            case AppMode::BREAK:
                status = L"Status: BREAK";
                break;
            default:
                status = L"Status: IDLE";
                break;
            }
            if (isPaused) status += L" (PAUSED)";
            
            TextOut(hdc, 20, 400, status.c_str(), status.length());
            
            EndPaint(hWnd, &ps);
        }
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
    case WM_CREATE:
        // Make window round cornered
        {
            HRGN hRgn = CreateRoundRectRgn(0, 0, 300, 150, 20, 20);
            SetWindowRgn(hWnd, hRgn, TRUE);
            DeleteObject(hRgn);
        }
        break;
        
    case WM_LBUTTONDOWN:
        // Start dragging
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
        // Show context menu
        {
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1, L"Close");
            AppendMenu(hMenu, MF_STRING, 2, L"Settings");
            
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            ClientToScreen(hWnd, &pt);
            
            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, nullptr);
            
            if (cmd == 1)
            {
                ShowWindow(hWnd, SW_HIDE);
            }
            else if (cmd == 2)
            {
                ShowWindow(hSettingsWnd, SW_SHOW);
            }
            
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
            
            // Draw timer
            std::wstring timeStr = FormatTime(timeLeft);
            std::wstring status;
            
            switch (currentMode)
            {
            case AppMode::FOCUS:
                SetTextColor(hdc, RGB(255, 100, 100));
                status = L"FOCUS";
                break;
            case AppMode::BREAK:
                SetTextColor(hdc, RGB(100, 255, 100));
                status = L"BREAK";
                break;
            default:
                SetTextColor(hdc, RGB(255, 255, 255));
                status = L"IDLE";
                break;
            }
            
            if (isPaused) status += L" - PAUSED";
            
            // Draw task
            SetTextColor(hdc, RGB(200, 200, 200));
            SelectObject(hdc, hFont);
            TextOut(hdc, 20, 20, currentTask.c_str(), currentTask.length());
            
            // Draw timer
            SetTextColor(hdc, RGB(255, 255, 255));
            SelectObject(hdc, hFontDigital);
            TextOut(hdc, 80, 50, timeStr.c_str(), timeStr.length());
            
            // Draw status
            SetTextColor(hdc, RGB(150, 150, 150));
            SelectObject(hdc, hFont);
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
