#pragma once

#include <windows.h>
#include <string>

class TimerWidget {
public:
    TimerWidget();
    ~TimerWidget();
    
    void create(HWND parent = nullptr);
    void show();
    void hide();
    void updateTimer(int minutes, int seconds);
    void setFocusMode(bool inFocusMode);
    void setPaused(bool paused);
    
    HWND getHandle() const;
    bool isVisible() const;
    
    void setPosition(int x, int y);
    void setAlwaysOnTop(bool onTop);
    
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
private:
    void registerWindowClass();
    void createWindow();
    void paintWindow();
    
    HWND hwnd = nullptr;
    HWND parentHwnd = nullptr;
    bool visible = false;
    bool focusMode = true;
    bool paused = false;
    int timerMinutes = 0;
    int timerSeconds = 0;
    
    HFONT hFont = nullptr;
    HFONT hSmallFont = nullptr;
    
    static TimerWidget* instance;
};
