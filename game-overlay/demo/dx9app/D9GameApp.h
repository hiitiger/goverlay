#pragma once
#include "D9Graphics.h"

class D9GameApp
{
public:
    D9GameApp(void);
    ~D9GameApp(void);

    void initWindow();
    void initD3d9();

    void showWindow();

    void runGameLoop();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND _window;
    std::unique_ptr<D9Graphics> _graphics;
    std::unique_ptr<Input> _input;

    FpsTimer _fps;

};

