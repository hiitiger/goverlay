#include "StdAfx.h"
#include "D9GameApp.h"


D9GameApp::D9GameApp(void)
{

}


D9GameApp::~D9GameApp(void)
{

}

void D9GameApp::initWindow()
{
    //Register class
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = D9GameApp::WindowProc;
    wc.hInstance     = GetModuleHandle(nullptr);
    wc.lpszClassName = L"D9GameWindow";
    wc.hCursor		 = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    RegisterClassEx(&wc);

    //CreateWindow
    _window = CreateWindowEx(
        NULL, L"D9GameWindow", L"D9GameWindow", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        1200, 800, NULL, NULL, GetModuleHandle(nullptr), this);

}

void D9GameApp::initD3d9()
{
    RECT rc;
    GetClientRect(_window, &rc);
    _graphics.reset(new D9Graphics());
    _graphics->resize(rc.right, rc.bottom);
    _graphics->initD3d9(_window);
    _graphics->setup();
}

void D9GameApp::showWindow()
{
    ShowWindow(_window, SW_SHOW);
}

void D9GameApp::runGameLoop()
{
    _input.reset(new Input);

    MSG msg = {0};
    double lastTime = (double)timeGetTime();
    _fps.start();

    while(msg.message != WM_QUIT)
    {
        if (::PeekMessage(&msg, NULL, 0,0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else
        {
            _fps.tick();

  
          
            
            {
                double curTime = (double)timeGetTime();
                double timeDelta = (curTime - lastTime)*0.001;
                _graphics->render((float)timeDelta);

                lastTime = curTime;

                std::wstring fps = std::to_wstring((long long)_fps.fps()) + L",D9GameWindow";
                SetWindowTextW(_window, fps.c_str());
            }
        }
    }

    _graphics->shutdown();
}

LRESULT CALLBACK D9GameApp::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    D9GameApp *pThis = nullptr;

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (D9GameApp*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

        pThis->_window = hwnd;
    }
    else
    {
        pThis = (D9GameApp*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    if (pThis)
    {
        return pThis->handleMessage(uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

static HCURSOR  handCursor = (HCURSOR)::LoadImageW(NULL, IDC_HAND, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);

LRESULT D9GameApp::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_CLOSE:
        ::DestroyWindow(_window);
        return 0;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        break;
    case WM_SIZE:
        {
            if (!IsIconic(_window))
            {
                int w = LOWORD(lParam);
                int h = HIWORD(lParam);
                _graphics->resize(w, h);
            }      
        }
        break;
    case WM_SETCURSOR:
        {
           // ::SetCursor(handCursor);
          //  return 1;
        }
        break;
    case WM_KEYDOWN:
        {
            if (wParam == VK_ESCAPE)
            {
                WCHAR szPath[MAX_PATH] = { 0 };
                GetModuleFileNameW(NULL, szPath, MAX_PATH);
                WCHAR achLongPath[MAX_PATH] = { 0 };
                TCHAR **lppPart = { NULL };
                ::GetFullPathNameW(szPath, MAX_PATH, achLongPath, lppPart);

                std::wstring dirPath = achLongPath;
                size_t lastSepartor = dirPath.find_last_of('\\');
                dirPath.erase(lastSepartor);

    #ifdef _WIN64
                dirPath.append(L"\\n_overlay.x64.dll");
    #else
                dirPath.append(L"\\n_overlay.dll");
    #endif
                ::LoadLibraryW(dirPath.c_str());
            }
        }
        break;
    }
    return ::DefWindowProcW(_window, uMsg, wParam, lParam);
}
