#include <windows.h>
#include <string>
#include <vector>
#include <cassert>

// Simple unit tests for Focus App
// These test the core functionality without GUI

// Test FormatTime function
std::wstring FormatTime(int seconds) {
    int minutes = seconds / 60;
    int secs = seconds % 60;
    std::wstring result;
    if (minutes < 10) result += L"0";
    result += std::to_wstring(minutes);
    result += L":";
    if (secs < 10) result += L"0";
    result += std::to_wstring(secs);
    return result;
}

// Test IsProcessBlocked function
bool IsProcessBlocked(const std::wstring& processName, const std::vector<std::wstring>& blockedApps) {
    for (const auto& blocked : blockedApps)
    {
        if (processName.find(blocked) != std::wstring::npos)
        {
            return true;
        }
    }
    return false;
}

int main() {
    // Test FormatTime
    assert(FormatTime(0) == L"00:00");
    assert(FormatTime(30) == L"00:30");
    assert(FormatTime(60) == L"01:00");
    assert(FormatTime(25 * 60) == L"25:00");
    assert(FormatTime(5 * 60 + 30) == L"05:30");
    
    // Define blocked apps for testing
    std::vector<std::wstring> blockedApps = {
        L"whatsapp", L"chrome", L"firefox", L"youtube"
    };
    
    // Test IsProcessBlocked
    assert(IsProcessBlocked(L"whatsapp.exe", blockedApps) == true);
    assert(IsProcessBlocked(L"chrome.exe", blockedApps) == true);
    assert(IsProcessBlocked(L"firefox.exe", blockedApps) == true);
    assert(IsProcessBlocked(L"youtube.exe", blockedApps) == true);
    assert(IsProcessBlocked(L"notepad.exe", blockedApps) == false);
    assert(IsProcessBlocked(L"explorer.exe", blockedApps) == false);
    
    // Test with full paths
    assert(IsProcessBlocked(L"C:\\Program Files\\WhatsApp\\whatsapp.exe", blockedApps) == true);
    assert(IsProcessBlocked(L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe", blockedApps) == true);
    
    return 0;
}
