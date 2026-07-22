#include "TimerWidget.h"
#include "Config.h"
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

TimerWidget* TimerWidget::instance = nullptr;

TimerWidget::TimerWidget() {
    instance = this;
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

TimerWidget::~TimerWidget() {
    if (hFont) DeleteObject(hFont);
    if (hSmallFont) DeleteObject(hSmallFont);
    if (hwnd) DestroyWindow(hwnd);
    
    GdiplusShutdown(nullptr);
    instance = nullptr;
}

void TimerWidget::registerWindowClass() {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = windowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"FocusAppTimerWidget";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
    RegisterClassEx(&wc);
}

void TimerWidget::create(HWND parent) {
    parentHwnd = parent;
    registerWindowClass();
    createWindow();
}

void TimerWidget::createWindow() {
    auto& config = ConfigManager::getInstance().getConfig();
    
    hwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED,
        L"FocusAppTimerWidget",
        L"Focus Timer",
        WS_POPUP | WS_VISIBLE,
        config.widgetX, config.widgetY, 250, 120,
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    
    if (hwnd) {
        // Create fonts
        hFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        
        hSmallFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        
        setAlwaysOnTop(config.alwaysOnTop);
        visible = true;
        
        // Make window click-through for the background
        SetLayeredWindowAttributes(hwnd, 0, 200, LWA_ALPHA);
    }
}

void TimerWidget::show() {
    if (hwnd && !visible) {
        ShowWindow(hwnd, SW_SHOW);
        visible = true;
    }
}

void TimerWidget::hide() {
    if (hwnd && visible) {
        ShowWindow(hwnd, SW_HIDE);
        visible = false;
    }
}

void TimerWidget::updateTimer(int minutes, int seconds) {
    timerMinutes = minutes;
    timerSeconds = seconds;
    if (hwnd) {
        InvalidateRect(hwnd, nullptr, TRUE);
    }
}

void TimerWidget::setFocusMode(bool inFocusMode) {
    focusMode = inFocusMode;
    if (hwnd) {
        InvalidateRect(hwnd, nullptr, TRUE);
    }
}

void TimerWidget::setPaused(bool isPaused) {
    paused = isPaused;
    if (hwnd) {
        InvalidateRect(hwnd, nullptr, TRUE);
    }
}

void TimerWidget::setPosition(int x, int y) {
    if (hwnd) {
        SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        auto& config = ConfigManager::getInstance().getConfig();
        config.widgetX = x;
        config.widgetY = y;
        ConfigManager::getInstance().saveConfig();
    }
}

void TimerWidget::setAlwaysOnTop(bool onTop) {
    if (hwnd) {
        SetWindowPos(hwnd, onTop ? HWND_TOPMOST : HWND_NOTOPMOST, 
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

HWND TimerWidget::getHandle() const {
    return hwnd;
}

bool TimerWidget::isVisible() const {
    return visible;
}

void TimerWidget::paintWindow() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    
    // Double buffering
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, ps.rcPaint.right - ps.rcPaint.left, 
                                               ps.rcPaint.bottom - ps.rcPaint.top);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
    
    // Fill background
    Graphics graphics(memDC);
    
    int width = ps.rcPaint.right - ps.rcPaint.left;
    int height = ps.rcPaint.bottom - ps.rcPaint.top;
    
    // Semi-transparent background
    Color bgColor(40, 40, 40, 200);
    SolidBrush bgBrush(bgColor);
    graphics.FillRectangle(&bgBrush, 0, 0, width, height);
    
    // Draw border
    Pen borderPen(Color(255, 100, 100, 100), 2);
    graphics.DrawRectangle(&borderPen, 0, 0, width - 1, height - 1);
    
    // Draw timer text
    std::wstring timeText;
    if (paused) {
        timeText = L"PAUSED";
    } else {
        wchar_t buffer[64];
        swprintf_s(buffer, L"%02d:%02d", timerMinutes, timerSeconds);
        timeText = buffer;
    }
    
    // Create GDI+ objects for text
    Font timerFont(L"Segoe UI", 48, FontStyleBold, UnitPixel);
    SolidBrush textBrush(Color(255, 255, 255, 255));
    
    RectF textRect(20, 20, width - 40, 60);
    graphics.DrawString(timeText.c_str(), timeText.length(), &timerFont, textRect, nullptr, &textBrush);
    
    // Draw status text
    Font statusFont(L"Segoe UI", 14, FontStyleRegular, UnitPixel);
    SolidBrush statusBrush(Color(200, 255, 255, 255));
    
    std::wstring statusText = focusMode ? L"Focus Mode" : L"Break Time";
    RectF statusRect(20, 70, width - 40, 40);
    graphics.DrawString(statusText.c_str(), statusText.length(), &statusFont, statusRect, nullptr, &statusBrush);
    
    // Copy to screen
    BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, 
           ps.rcPaint.right - ps.rcPaint.left, 
           ps.rcPaint.bottom - ps.rcPaint.top,
           memDC, 0, 0, SRCCOPY);
    
    // Cleanup
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
    
    EndPaint(hwnd, &ps);
}

LRESULT CALLBACK TimerWidget::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (instance && instance->hwnd == hwnd) {
        switch (uMsg) {
            case WM_PAINT:
                instance->paintWindow();
                return 0;
                
            case WM_LBUTTONDOWN:
                // Allow dragging
                PostMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                return 0;
                
            case WM_RBUTTONDOWN:
                // Show context menu
                {
                    POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                    ClientToScreen(hwnd, &pt);
                    
                    HMENU hMenu = CreatePopupMenu();
                    AppendMenu(hMenu, MF_STRING, 1, L"Settings");
                    AppendMenu(hMenu, MF_STRING, 2, L"Pause");
                    AppendMenu(hMenu, MF_STRING, 3, L"Exit");
                    
                    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, nullptr);
                    DestroyMenu(hMenu);
                    
                    if (cmd == 3) {
                        PostQuitMessage(0);
                    }
                }
                return 0;
                
            case WM_DESTROY:
                instance->hwnd = nullptr;
                return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
