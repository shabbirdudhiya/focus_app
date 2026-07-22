#pragma once

#include <windows.h>
#include <string>
#include <unordered_map>
#include <chrono>

class AppMonitor {
public:
    AppMonitor();
    ~AppMonitor();
    
    void startMonitoring();
    void stopMonitoring();
    void pauseMonitoring();
    void resumeMonitoring();
    
    void setWarningCallback(void (*callback)(const std::wstring&, int));
    void setBlockCallback(void (*callback)(const std::wstring&));
    
    std::wstring getActiveWindowTitle();
    std::wstring getActiveWindowClass();
    std::wstring getActiveWindowProcessName();
    
    bool isMonitoring() const;
    bool isPaused() const;
    
private:
    static LRESULT CALLBACK keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);
    
    void checkActiveWindow();
    void showWarning(const std::wstring& appName);
    
    HHOOK hKeyboardHook = nullptr;
    HHOOK hMouseHook = nullptr;
    bool monitoring = false;
    bool paused = false;
    
    std::unordered_map<std::wstring, std::chrono::system_clock::time_point> appUsageTimers;
    
    void (*warningCallback)(const std::wstring&, int) = nullptr;
    void (*blockCallback)(const std::wstring&) = nullptr;
    
    static AppMonitor* instance;
};
