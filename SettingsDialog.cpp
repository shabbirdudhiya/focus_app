#include "SettingsDialog.h"
#include "Config.h"
#include <windows.h>
#include <commctrl.h>
#include <sstream>

SettingsDialog* SettingsDialog::instance = nullptr;

SettingsDialog::SettingsDialog(HWND parent) : parentHwnd(parent), hDialog(nullptr) {
    instance = this;
}

SettingsDialog::~SettingsDialog() {
    if (hDialog) {
        DestroyWindow(hDialog);
    }
    instance = nullptr;
}

void SettingsDialog::show() {
    hDialog = CreateDialogParam(GetModuleHandle(nullptr), 
                               MAKEINTRESOURCE(1000), 
                               parentHwnd, 
                               dialogProc, 
                               (LPARAM)this);
    
    if (hDialog) {
        ShowWindow(hDialog, SW_SHOW);
    }
}

LRESULT CALLBACK SettingsDialog::dialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (instance && instance->hDialog == hwnd) {
        switch (uMsg) {
            case WM_INITDIALOG:
                instance->initDialog(hwnd);
                return TRUE;
                
            case WM_COMMAND:
                switch (LOWORD(wParam)) {
                    case IDOK:
                        instance->saveSettings();
                        EndDialog(hwnd, IDOK);
                        return TRUE;
                        
                    case IDCANCEL:
                        EndDialog(hwnd, IDCANCEL);
                        return TRUE;
                }
                return FALSE;
                
            case WM_CLOSE:
                EndDialog(hwnd, IDCANCEL);
                return TRUE;
        }
    }
    return FALSE;
}

void SettingsDialog::initDialog(HWND hwnd) {
    auto& config = ConfigManager::getInstance().getConfig();
    
    // Get controls
    hFocusDuration = GetDlgItem(hwnd, 1001);
    hBreakDuration = GetDlgItem(hwnd, 1002);
    hWarningDelay = GetDlgItem(hwnd, 1003);
    hShowWarnings = GetDlgItem(hwnd, 1004);
    hAlwaysOnTop = GetDlgItem(hwnd, 1005);
    hAutoStart = GetDlgItem(hwnd, 1006);
    hBlacklistApps = GetDlgItem(hwnd, 1007);
    hWhitelistApps = GetDlgItem(hwnd, 1008);
    
    // Set values
    SetDlgItemInt(hwnd, 1001, config.focusDuration, FALSE);
    SetDlgItemInt(hwnd, 1002, config.breakDuration, FALSE);
    SetDlgItemInt(hwnd, 1003, config.warningDelay, FALSE);
    
    Button_SetCheck(hShowWarnings, config.showWarnings ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(hAlwaysOnTop, config.alwaysOnTop ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(hAutoStart, config.autoStart ? BST_CHECKED : BST_UNCHECKED);
    
    // Populate blacklist
    std::wstring blacklistText;
    for (const auto& app : config.blacklistedApps) {
        if (!blacklistText.empty()) blacklistText += L", ";
        blacklistText += app;
    }
    SetDlgItemText(hwnd, 1007, blacklistText.c_str());
    
    // Populate whitelist
    std::wstring whitelistText;
    for (const auto& app : config.whitelistedApps) {
        if (!whitelistText.empty()) whitelistText += L", ";
        whitelistText += app;
    }
    SetDlgItemText(hwnd, 1008, whitelistText.c_str());
}

void SettingsDialog::saveSettings() {
    auto& config = ConfigManager::getInstance().getConfig();
    
    // Get values
    config.focusDuration = GetDlgItemInt(hDialog, 1001);
    config.breakDuration = GetDlgItemInt(hDialog, 1002);
    config.warningDelay = GetDlgItemInt(hDialog, 1003);
    
    config.showWarnings = (Button_GetCheck(hShowWarnings) == BST_CHECKED);
    config.alwaysOnTop = (Button_GetCheck(hAlwaysOnTop) == BST_CHECKED);
    config.autoStart = (Button_GetCheck(hAutoStart) == BST_CHECKED);
    
    // Get blacklist
    WCHAR blacklist[4096] = {0};
    GetDlgItemText(hDialog, 1007, blacklist, 4096);
    config.blacklistedApps.clear();
    std::wstring blStr(blacklist);
    size_t pos = 0;
    while (pos < blStr.length()) {
        size_t comma = blStr.find(L',', pos);
        if (comma == std::wstring::npos) comma = blStr.length();
        std::wstring app = blStr.substr(pos, comma - pos);
        // Trim whitespace
        app.erase(0, app.find_first_not_of(L" \t"));
        app.erase(app.find_last_not_of(L" \t") + 1);
        if (!app.empty()) {
            config.blacklistedApps.insert(app);
        }
        pos = comma + 1;
    }
    
    // Get whitelist
    WCHAR whitelist[4096] = {0};
    GetDlgItemText(hDialog, 1008, whitelist, 4096);
    config.whitelistedApps.clear();
    std::wstring wlStr(whitelist);
    pos = 0;
    while (pos < wlStr.length()) {
        size_t comma = wlStr.find(L',', pos);
        if (comma == std::wstring::npos) comma = wlStr.length();
        std::wstring app = wlStr.substr(pos, comma - pos);
        // Trim whitespace
        app.erase(0, app.find_first_not_of(L" \t"));
        app.erase(app.find_last_not_of(L" \t") + 1);
        if (!app.empty()) {
            config.whitelistedApps.insert(app);
        }
        pos = comma + 1;
    }
    
    ConfigManager::getInstance().saveConfig();
}
