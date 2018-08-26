#pragma once


class UiApp 
{
    std::mutex uilock_;
    HHOOK msgHook_ = nullptr;
    HHOOK wndProcHook_ = nullptr;
    HHOOK wndRetProcHook_ = nullptr;
    std::atomic<HWND> graphicsWindow_ = false;
    std::atomic<bool> windowFocus_ = false;
    RECT windowClientRect_ = {};

    bool isIntercepting_ = false;

    std::mutex taskLock_;
    std::deque<std::function<void()>> tasks_;

public:
    UiApp();
    ~UiApp();

    bool trySetupGraphicsWindow(HWND window);

    bool setup(HWND window);

    bool windowSetted() const;

    void async(const std::function<void()>& task);

    void startInputIntercept();
    void stopInputIntercept();

    bool isInterceptingInput();

    bool hookWindow(HWND window);
    void unhookWindow();

    void updateWindowState(HWND window);
    void clearWindowState();

private:

    static LRESULT CALLBACK GetMsgProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);
    static LRESULT CALLBACK CallWndProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);
    static LRESULT CALLBACK CallWndRetProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);

    LRESULT hookGetMsgProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);
    LRESULT hookCallWndProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);
    LRESULT hookCallWndRetProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam);

    void checkHotkey();
};
