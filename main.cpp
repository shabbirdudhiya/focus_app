#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <psapi.h>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

#include "TimerWidget.h"
#include "AppMonitor.h"
#include "Config.h"
#include "SettingsDialog.h"
#include "resource.h"

// Global variables
HINSTANCE hInst;
TimerWidget* g_timerWidget = nullptr;
AppMonitor* g_appMonitor = nullptr;
SettingsDialog* g_settingsDialog = nullptr;
HWND g_mainWnd = nullptr;
NOTIFYICONDATA g_trayIcon = {0};

// Timer state
std::atomic<bool> g_focusMode = {true};
std::atomic<int> g_remainingSeconds = {0};
std::atomic<bool> g_timerRunning = {false};
std::atomic<bool> g_timerPaused = {false};

// Forward declarations
LRESULT CALLBACK mainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void startFocusTimer();
void stopFocusTimer();
void toggleTimer();
void showWarningPopup(const std::wstring& appName, int minutesUsed);
void showBlockPopup(const std::wstring& appName);
void updateTrayIcon();
void createTrayIcon(HWND hwnd);
void removeTrayIcon();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    
    // Initialize COM
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    
    // Initialize Common Controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);
    
    // Load config
    ConfigManager::getInstance();
    
    // Create main window (hidden)
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = mainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"FocusAppMainWindow";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
    RegisterClassEx(&wc);
    
    g_mainWnd = CreateWindowEx(0, L"FocusAppMainWindow", L"Focus App", 
                               WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                               400, 300, nullptr, nullptr, hInstance, nullptr);
    
    if (!g_mainWnd) {
        return 1;
    }
    
    // Create timer widget
    g_timerWidget = new TimerWidget();
    g_timerWidget->create(g_mainWnd);
    
    // Create app monitor
    g_appMonitor = new AppMonitor();
    g_appMonitor->setWarningCallback(showWarningPopup);
    g_appMonitor->setBlockCallback(showBlockPopup);
    g_appMonitor->startMonitoring();
    
    // Create settings dialog
    g_settingsDialog = new SettingsDialog(g_mainWnd);
    
    // Create tray icon
    createTrayIcon(g_mainWnd);
    
    // Auto-start timer if configured
    auto& config = ConfigManager::getInstance().getConfig();
    if (config.autoStart) {
        startFocusTimer();
    }
    
    // Main message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (g_settingsDialog && IsDialogMessage(g_settingsDialog->getHandle(), &msg)) {
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    stopFocusTimer();
    g_appMonitor->stopMonitoring();
    
    delete g_settingsDialog;
    delete g_appMonitor;
    delete g_timerWidget;
    
    removeTrayIcon();
    
    CoUninitialize();
    
    return (int)msg.wParam;
}

LRESULT CALLBACK mainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            // Start with window hidden if configured
            {
                auto& config = ConfigManager::getInstance().getConfig();
                if (config.startMinimized) {
                    ShowWindow(hWnd, SW_HIDE);
                }
            }
            return 0;
            
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONDOWN) {
                POINT pt;
                GetCursorPos(&pt);
                
                HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MAIN_MENU));
                HMENU hSubMenu = GetSubMenu(hMenu, 0);
                
                // Update menu items based on state
                auto& config = ConfigManager::getInstance().getConfig();
                EnableMenuItem(hSubMenu, ID_TRAY_PAUSE, g_timerPaused ? MF_GRAYED : MF_ENABLED);
                EnableMenuItem(hSubMenu, ID_TRAY_RESUME, g_timerPaused ? MF_ENABLED : MF_GRAYED);
                
                SetForegroundWindow(hWnd);
                TrackPopupMenu(hSubMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, nullptr);
                DestroyMenu(hMenu);
            }
            return 0;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_TRAY_SHOW:
                    if (g_timerWidget) {
                        g_timerWidget->show();
                    }
                    ShowWindow(hWnd, SW_RESTORE);
                    SetForegroundWindow(hWnd);
                    return 0;
                    
                case ID_TRAY_SETTINGS:
                    if (g_settingsDialog) {
                        g_settingsDialog->show();
                    }
                    return 0;
                    
                case ID_TRAY_PAUSE:
                    g_timerPaused = true;
                    if (g_timerWidget) {
                        g_timerWidget->setPaused(true);
                    }
                    g_appMonitor->pauseMonitoring();
                    updateTrayIcon();
                    return 0;
                    
                case ID_TRAY_RESUME:
                    g_timerPaused = false;
                    if (g_timerWidget) {
                        g_timerWidget->setPaused(false);
                    }
                    g_appMonitor->resumeMonitoring();
                    updateTrayIcon();
                    return 0;
                    
                case ID_TRAY_EXIT:
                    PostQuitMessage(0);
                    return 0;
            }
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        case WM_CLOSE:
            // Minimize to tray instead of closing
            ShowWindow(hWnd, SW_HIDE);
            return 0;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

void createTrayIcon(HWND hwnd) {
    g_trayIcon.cbSize = sizeof(NOTIFYICONDATA);
    g_trayIcon.hWnd = hwnd;
    g_trayIcon.uID = ID_TRAY_ICON;
    g_trayIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_trayIcon.uCallbackMessage = WM_TRAYICON;
    g_trayIcon.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP_ICON));
    wcscpy_s(g_trayIcon.szTip, L"Focus App - Stay Productive");
    
    Shell_NotifyIcon(NIM_ADD, &g_trayIcon);
    updateTrayIcon();
}

void removeTrayIcon() {
    g_trayIcon.cbSize = sizeof(NOTIFYICONDATA);
    Shell_NotifyIcon(NIM_DELETE, &g_trayIcon);
}

void updateTrayIcon() {
    HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP_ICON));
    
    if (g_timerRunning) {
        if (g_focusMode) {
            // Focus mode icon - maybe change to a different icon
        } else {
            // Break mode icon
        }
    }
    
    if (g_timerPaused) {
        // Paused icon
    }
    
    g_trayIcon.hIcon = hIcon;
    Shell_NotifyIcon(NIM_MODIFY, &g_trayIcon);
    DestroyIcon(hIcon);
}

void startFocusTimer() {
    if (g_timerRunning) return;
    
    auto& config = ConfigManager::getInstance().getConfig();
    g_remainingSeconds = config.focusDuration * 60;
    g_focusMode = true;
    g_timerRunning = true;
    g_timerPaused = false;
    
    if (g_timerWidget) {
        g_timerWidget->setFocusMode(true);
        g_timerWidget->setPaused(false);
        g_timerWidget->updateTimer(g_remainingSeconds / 60, g_remainingSeconds % 60);
    }
    
    updateTrayIcon();
    
    // Start timer thread
    std::thread([&]() {
        while (g_timerRunning && g_remainingSeconds > 0) {
            if (!g_timerPaused) {
                g_remainingSeconds--;
                if (g_timerWidget) {
                    g_timerWidget->updateTimer(g_remainingSeconds / 60, g_remainingSeconds % 60);
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        if (g_remainingSeconds <= 0 && g_timerRunning) {
            // Timer expired - switch to break mode
            g_focusMode = false;
            g_remainingSeconds = config.breakDuration * 60;
            
            if (g_timerWidget) {
                g_timerWidget->setFocusMode(false);
                g_timerWidget->updateTimer(g_remainingSeconds / 60, g_remainingSeconds % 60);
            }
            
            // Show notification
            MessageBox(nullptr, L"Focus session complete! Take a break.", 
                      L"Focus App", MB_OK | MB_ICONINFORMATION);
            
            // Continue with break timer
            while (g_timerRunning && g_remainingSeconds > 0) {
                if (!g_timerPaused) {
                    g_remainingSeconds--;
                    if (g_timerWidget) {
                        g_timerWidget->updateTimer(g_remainingSeconds / 60, g_remainingSeconds % 60);
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            
            if (g_remainingSeconds <= 0 && g_timerRunning) {
                // Break expired - back to focus
                startFocusTimer();
            }
        }
    }).detach();
}

void stopFocusTimer() {
    g_timerRunning = false;
    g_timerPaused = false;
    
    if (g_timerWidget) {
        g_timerWidget->setPaused(false);
        g_timerWidget->updateTimer(0, 0);
    }
    
    updateTrayIcon();
}

void toggleTimer() {
    if (g_timerRunning) {
        stopFocusTimer();
    } else {
        startFocusTimer();
    }
}

void showWarningPopup(const std::wstring& appName, int minutesUsed) {
    auto& config = ConfigManager::getInstance().getConfig();
    
    std::wstring message = L"You've been using " + appName + L" for " + 
                          std::to_wstring(minutesUsed) + L" minute(s).\n\n" +
                          L"Get back to work!";
    
    MessageBox(nullptr, message.c_str(), L"Focus App Warning", 
               MB_OK | MB_ICONWARNING | MB_TOPMOST);
    
    if (config.blockAfterWarning && minutesUsed >= config.blockAfterMinutes) {
        // In a real implementation, we could force minimize the window
        // For now, just show a stronger warning
        MessageBox(nullptr, L"You should really get back to work now!", 
                  L"Focus App - Time's Up!", MB_OK | MB_ICONERROR | MB_TOPMOST);
    }
}

void showBlockPopup(const std::wstring& appName) {
    std::wstring message = L"The application '" + appName + L"' is blocked during focus time.";
    MessageBox(nullptr, message.c_str(), L"Focus App - Blocked", 
               MB_OK | MB_ICONERROR | MB_TOPMOST);
}
