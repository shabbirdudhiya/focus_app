#include "AppMonitor.h"
#include "Config.h"
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <sstream>
#include <iomanip>

AppMonitor* AppMonitor::instance = nullptr;

AppMonitor::AppMonitor() {
    instance = this;
}

AppMonitor::~AppMonitor() {
    stopMonitoring();
    instance = nullptr;
}

void AppMonitor::startMonitoring() {
    if (monitoring) return;
    
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHookProc, nullptr, 0);
    hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseHookProc, nullptr, 0);
    
    if (hKeyboardHook && hMouseHook) {
        monitoring = true;
        paused = false;
    }
}

void AppMonitor::stopMonitoring() {
    if (!monitoring) return;
    
    if (hKeyboardHook) {
        UnhookWindowsHookEx(hKeyboardHook);
        hKeyboardHook = nullptr;
    }
    if (hMouseHook) {
        UnhookWindowsHookEx(hMouseHook);
        hMouseHook = nullptr;
    }
    
    monitoring = false;
    appUsageTimers.clear();
}

void AppMonitor::pauseMonitoring() {
    paused = true;
}

void AppMonitor::resumeMonitoring() {
    paused = false;
}

void AppMonitor::setWarningCallback(void (*callback)(const std::wstring&, int)) {
    warningCallback = callback;
}

void AppMonitor::setBlockCallback(void (*callback)(const std::wstring&)) {
    blockCallback = callback;
}

bool AppMonitor::isMonitoring() const {
    return monitoring;
}

bool AppMonitor::isPaused() const {
    return paused;
}

LRESULT CALLBACK AppMonitor::keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && instance && !instance->paused) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            instance->checkActiveWindow();
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK AppMonitor::mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && instance && !instance->paused) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN) {
            instance->checkActiveWindow();
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void AppMonitor::checkActiveWindow() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return;
    
    std::wstring processName = getActiveWindowProcessName();
    if (processName.empty()) return;
    
    // Skip our own app
    if (processName.find(L"focusapp") != std::wstring::npos) return;
    
    auto& config = ConfigManager::getInstance().getConfig();
    
    // Check if whitelisted
    if (config.whitelistedApps.size() > 0) {
        if (ConfigManager::getInstance().isWhitelisted(processName)) {
            return; // Allowed
        }
    }
    
    // Check if blacklisted
    if (ConfigManager::getInstance().isBlacklisted(processName)) {
        auto now = std::chrono::system_clock::now();
        
        // Track usage time
        if (appUsageTimers.find(processName) == appUsageTimers.end()) {
            appUsageTimers[processName] = now;
        } else {
            auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
                now - appUsageTimers[processName]).count();
            
            if (elapsed >= config.warningDelay && config.showWarnings) {
                if (warningCallback) {
                    warningCallback(processName, static_cast<int>(elapsed));
                }
                appUsageTimers[processName] = now; // Reset timer
            }
        }
    }
}

std::wstring AppMonitor::getActiveWindowTitle() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return L"";
    
    int length = GetWindowTextLength(hwnd);
    if (length == 0) return L"";
    
    std::wstring title(length + 1, L'\0');
    GetWindowText(hwnd, &title[0], length + 1);
    title.resize(length);
    return title;
}

std::wstring AppMonitor::getActiveWindowClass() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return L"";
    
    WCHAR className[256] = {0};
    GetClassName(hwnd, className, 256);
    return std::wstring(className);
}

std::wstring AppMonitor::getActiveWindowProcessName() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return L"";
    
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess) return L"";
    
    WCHAR processName[MAX_PATH] = {0};
    if (GetModuleFileNameEx(hProcess, nullptr, processName, MAX_PATH)) {
        std::wstring fullPath(processName);
        size_t lastSlash = fullPath.find_last_of(L'\\/');
        if (lastSlash != std::wstring::npos) {
            std::wstring name = fullPath.substr(lastSlash + 1);
            // Remove .exe extension
            size_t dotPos = name.find_last_of(L'.');
            if (dotPos != std::wstring::npos) {
                name = name.substr(0, dotPos);
            }
            CloseHandle(hProcess);
            return name;
        }
    }
    
    CloseHandle(hProcess);
    return L"";
}
