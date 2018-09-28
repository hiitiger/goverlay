
#include "stdafx.h"
#include "D9Game.h"

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HINSTANCE g_instance = nullptr;

INT WINAPI wWinMain(HINSTANCE  instance, HINSTANCE , LPTSTR , int )
{
    g_instance = instance;

    D9GameApp app;
    app.initWindow();
    app.initD3d9();
    app.showWindow();
    app.runGameLoop();
    return 0;

}


