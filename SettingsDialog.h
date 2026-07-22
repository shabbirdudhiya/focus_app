#pragma once

#include <windows.h>
#include <string>
#include <vector>

class SettingsDialog {
public:
    SettingsDialog(HWND parent);
    ~SettingsDialog();
    
    void show();
    HWND getHandle() const { return hDialog; }
    
private:
    static LRESULT CALLBACK dialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    void initDialog(HWND hwnd);
    void saveSettings();
    
    HWND parentHwnd;
    HWND hDialog;
    
    // Controls
    HWND hFocusDuration;
    HWND hBreakDuration;
    HWND hWarningDelay;
    HWND hShowWarnings;
    HWND hAlwaysOnTop;
    HWND hAutoStart;
    HWND hBlacklistApps;
    HWND hWhitelistApps;
    
    static SettingsDialog* instance;
};
