#pragma once

#include <string>
#include <vector>
#include <unordered_set>

struct AppConfig {
    // Timer settings
    int focusDuration = 25; // minutes
    int breakDuration = 5; // minutes
    bool autoStart = true;
    
    // Monitoring settings
    int warningDelay = 1; // minutes before warning
    bool showWarnings = true;
    bool blockAfterWarning = false;
    int blockAfterMinutes = 5;
    
    // Whitelist (only these are allowed)
    std::unordered_set<std::wstring> whitelistedApps;
    std::unordered_set<std::wstring> whitelistedWebsites;
    
    // Blacklist (these trigger warnings)
    std::unordered_set<std::wstring> blacklistedApps;
    std::unordered_set<std::wstring> blacklistedWebsites;
    
    // Window settings
    bool alwaysOnTop = true;
    int widgetX = 100;
    int widgetY = 100;
    bool minimizeToTray = true;
    bool startMinimized = false;
    
    // State
    bool isPaused = false;
};

class ConfigManager {
public:
    static ConfigManager& getInstance();
    
    void loadConfig();
    void saveConfig();
    AppConfig& getConfig();
    const AppConfig& getConfig() const;
    
    void addToWhitelist(const std::wstring& app);
    void removeFromWhitelist(const std::wstring& app);
    void addToBlacklist(const std::wstring& app);
    void removeFromBlacklist(const std::wstring& app);
    
    bool isWhitelisted(const std::wstring& app) const;
    bool isBlacklisted(const std::wstring& app) const;
    
private:
    ConfigManager();
    ~ConfigManager();
    
    AppConfig config;
    std::wstring configPath;
    
    void ensureDefaultConfig();
};
