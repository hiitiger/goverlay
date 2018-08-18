#pragma once


class UiApp 
{
    volatile bool windowFocus_ = false;
    RECT windowClientRect_ = {};

    std::mutex taskLock_;
    std::deque<std::function<void()>> tasks_;

public:
    UiApp();
    ~UiApp();

    void trySetupGraphicsWindow(HWND window);

    void setup(HWND window);

    void async(const std::function<void()>& task);


    void startInputBlock();
    void stopInputBlock();

    void hookWindow(HWND window);
    void unhookWindow();

    void getWindowState(HWND window);
    void clearWindowState();
};
