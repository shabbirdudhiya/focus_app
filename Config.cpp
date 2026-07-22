#include "Config.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <shlobj.h>

ConfigManager::ConfigManager() {
    PWSTR appDataPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath))) {
        configPath = std::wstring(appDataPath) + L"\\FocusApp\\config.txt";
        CoTaskMemFree(appDataPath);
    } else {
        configPath = L"config.txt";
    }
    
    ensureDefaultConfig();
    loadConfig();
}

ConfigManager::~ConfigManager() {
    saveConfig();
}

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::ensureDefaultConfig() {
    // Default blacklisted apps
    config.blacklistedApps.insert(L"whatsapp");
    config.blacklistedApps.insert(L"telegram");
    config.blacklistedApps.insert(L"discord");
    config.blacklistedApps.insert(L"facebook");
    config.blacklistedApps.insert(L"instagram");
    config.blacklistedApps.insert(L"twitter");
    config.blacklistedApps.insert(L"tiktok");
    
    // Default blacklisted websites
    config.blacklistedWebsites.insert(L"youtube.com");
    config.blacklistedWebsites.insert(L"facebook.com");
    config.blacklistedWebsites.insert(L"instagram.com");
    config.blacklistedWebsites.insert(L"twitter.com");
    config.blacklistedWebsites.insert(L"tiktok.com");
    config.blacklistedWebsites.insert(L"netflix.com");
    config.blacklistedWebsites.insert(L"amazon.com");
    config.blacklistedWebsites.insert(L"reddit.com");
    
    // Default whitelisted apps
    config.whitelistedApps.insert(L"focusapp");
    config.whitelistedApps.insert(L"notepad");
    config.whitelistedApps.insert(L"calc");
    config.whitelistedApps.insert(L"explorer");
}

void ConfigManager::loadConfig() {
    std::wifstream file(configPath);
    if (!file.is_open()) {
        return;
    }
    
    std::wstring line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == L'#') continue;
        
        size_t eqPos = line.find(L'=');
        if (eqPos == std::wstring::npos) continue;
        
        std::wstring key = line.substr(0, eqPos);
        std::wstring value = line.substr(eqPos + 1);
        
        // Trim whitespace
        auto trim = [](std::wstring& s) {
            s.erase(0, s.find_first_not_of(L" \t"));
            s.erase(s.find_last_not_of(L" \t") + 1);
        };
        trim(key);
        trim(value);
        
        if (key == L"focus_duration") config.focusDuration = std::stoi(value);
        else if (key == L"break_duration") config.breakDuration = std::stoi(value);
        else if (key == L"warning_delay") config.warningDelay = std::stoi(value);
        else if (key == L"show_warnings") config.showWarnings = (value == L"true");
        else if (key == L"always_on_top") config.alwaysOnTop = (value == L"true");
        else if (key == L"widget_x") config.widgetX = std::stoi(value);
        else if (key == L"widget_y") config.widgetY = std::stoi(value);
    }
    
    file.close();
}

void ConfigManager::saveConfig() {
    CreateDirectory(configPath.substr(0, configPath.find_last_of(L'\\')).c_str(), nullptr);
    
    std::wofstream file(configPath);
    if (!file.is_open()) {
        return;
    }
    
    file << L"# Focus App Configuration\n";
    file << L"focus_duration=" << config.focusDuration << L"\n";
    file << L"break_duration=" << config.breakDuration << L"\n";
    file << L"warning_delay=" << config.warningDelay << L"\n";
    file << L"show_warnings=" << (config.showWarnings ? L"true" : L"false") << L"\n";
    file << L"always_on_top=" << (config.alwaysOnTop ? L"true" : L"false") << L"\n";
    file << L"widget_x=" << config.widgetX << L"\n";
    file << L"widget_y=" << config.widgetY << L"\n";
    
    file << L"\n# Blacklisted Apps\n";
    for (const auto& app : config.blacklistedApps) {
        file << L"blacklist_app=" << app << L"\n";
    }
    
    file << L"\n# Blacklisted Websites\n";
    for (const auto& site : config.blacklistedWebsites) {
        file << L"blacklist_website=" << site << L"\n";
    }
    
    file << L"\n# Whitelisted Apps\n";
    for (const auto& app : config.whitelistedApps) {
        file << L"whitelist_app=" << app << L"\n";
    }
    
    file.close();
}

AppConfig& ConfigManager::getConfig() {
    return config;
}

const AppConfig& ConfigManager::getConfig() const {
    return config;
}

void ConfigManager::addToWhitelist(const std::wstring& app) {
    config.whitelistedApps.insert(app);
    saveConfig();
}

void ConfigManager::removeFromWhitelist(const std::wstring& app) {
    config.whitelistedApps.erase(app);
    saveConfig();
}

void ConfigManager::addToBlacklist(const std::wstring& app) {
    config.blacklistedApps.insert(app);
    saveConfig();
}

void ConfigManager::removeFromBlacklist(const std::wstring& app) {
    config.blacklistedApps.erase(app);
    saveConfig();
}

bool ConfigManager::isWhitelisted(const std::wstring& app) const {
    std::wstring lowerApp = app;
    std::transform(lowerApp.begin(), lowerApp.end(), lowerApp.begin(), ::towlower);
    
    for (const auto& wl : config.whitelistedApps) {
        std::wstring lowerWl = wl;
        std::transform(lowerWl.begin(), lowerWl.end(), lowerWl.begin(), ::towlower);
        if (lowerApp.find(lowerWl) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}

bool ConfigManager::isBlacklisted(const std::wstring& app) const {
    std::wstring lowerApp = app;
    std::transform(lowerApp.begin(), lowerApp.end(), lowerApp.begin(), ::towlower);
    
    for (const auto& bl : config.blacklistedApps) {
        std::wstring lowerBl = bl;
        std::transform(lowerBl.begin(), lowerBl.end(), lowerBl.begin(), ::towlower);
        if (lowerApp.find(lowerBl) != std::wstring::npos) {
            return true;
        }
    }
    return false;
}
