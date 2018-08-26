#include "stable.h"
#include "hook/apihook.hpp"
#include "overlay/session.h"
#include "overlay/hookapp.h"
#include "inputhook.h"
 

namespace {

typedef SHORT(WINAPI *pfnGetAsyncKeyState)(int vKey);
typedef SHORT(WINAPI *pfnGetKeyState)(int vKey);
typedef BOOL(WINAPI *pfnGetKeyboardState)(__out_ecount(256) PBYTE lpKeyState);
typedef UINT(WINAPI *pfnGetRawInputData)(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);
typedef UINT(WINAPI *pfnGetRawInputBuffer)(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader);

typedef int (WINAPI *pfnShowCursor)(__in BOOL bShow);
typedef BOOL(WINAPI *pfnGetCursorPos)(LPPOINT lpPoint);
typedef BOOL(WINAPI *pfnSetCursorPos)(int X, int Y);
typedef HCURSOR(WINAPI *pfnSetCursor)(HCURSOR hCursor);
typedef HCURSOR(WINAPI *pfnGetCursor)();

pfnGetAsyncKeyState T_GetAsyncKeyState = nullptr;
pfnGetKeyState T_GetKeyState = nullptr;
pfnGetKeyboardState T_GetKeyboardState = nullptr;
pfnGetRawInputData T_GetRawInputData = nullptr;
pfnGetRawInputBuffer T_GetRawInputBuffer = nullptr;

pfnShowCursor T_ShowCursor = nullptr;
pfnGetCursorPos T_GetCursorPos = nullptr;
pfnSetCursorPos T_SetCursorPos = nullptr;
pfnSetCursor T_SetCursor = nullptr;
pfnGetCursor T_GetCursor = nullptr;


struct InputStatus
{

    std::atomic<bool> inputStateSaved = false;

    std::atomic<int> cursorCount = 0;

    std::atomic<bool> cursorVisible = false;

    std::atomic<POINT> cursorPos = POINT{ 0, 0 };

    std::atomic<HCURSOR> cursor = nullptr;
};

InputStatus g_savedInputStatus;

}

SHORT WINAPI H_GetAsyncKeyState(__in int vKey)
{
    if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        return 0;
    }
    else
    {
        return Windows::OrginalApi::GetAsyncKeyState(vKey);
    }
}


SHORT WINAPI H_GetKeyState(__in int vKey)
{
    if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        return 0;
    }
    else
    {
        return Windows::OrginalApi::GetKeyState(vKey);
    }
}

BOOL WINAPI H_GetKeyboardState(__out_ecount(256) PBYTE lpKeyState)
{
    if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        memset(lpKeyState, 0, 256);
        return TRUE;
    }
    else
    {
        return Windows::OrginalApi::GetKeyboardState(lpKeyState);
    }
}

int WINAPI H_ShowCursor(__in BOOL bShow)
{
    if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        int saveCount = g_savedInputStatus.cursorCount;
        g_savedInputStatus.cursorCount += bShow ? 1 : -1;
        g_savedInputStatus.cursorVisible = !!bShow;
        return saveCount;
    }
    else
    {
        return Windows::OrginalApi::ShowCursor(bShow);
    }
}

BOOL WINAPI H_GetCursorPos(LPPOINT lpPoint)
{
    if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        if (lpPoint)
        {
            *lpPoint = g_savedInputStatus.cursorPos;
        }
        return TRUE;
    }
    else
    {
        return Windows::OrginalApi::GetCursorPos(lpPoint);
    }
}

BOOL WINAPI H_SetCursorPos(int x, int y)
{
    if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        g_savedInputStatus.cursorPos = POINT{x, y};
        return TRUE;
    }
    else
    {
        return Windows::OrginalApi::SetCursorPos(x, y);
    }
}

HCURSOR WINAPI H_SetCursor(HCURSOR cursor)
{
    if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        cursor = cursor;
        return NULL;
    }
    else
    {
        return Windows::OrginalApi::SetCursor(cursor);
    }
}

HCURSOR WINAPI H_GetCursor()
{
    if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        return g_savedInputStatus.cursor;
    }
    else
    {
        return Windows::OrginalApi::GetCursor();
    }
}

UINT WINAPI H_GetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader)
{
    UINT ret = 0;
    if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        if (pcbSize)
        {
            if (pData == nullptr)
            {
                Windows::OrginalApi::GetRawInputData(hRawInput, uiCommand, nullptr, pcbSize, cbSizeHeader);
            }
            if (*pcbSize > 0)
            {
                LPBYTE lpb = new BYTE[*pcbSize];
                Windows::OrginalApi::GetRawInputData(hRawInput, uiCommand, lpb, pcbSize, cbSizeHeader);
                delete[] lpb;
                *pcbSize = 0;
            }
        }
        ret = 0;
    }
    else
    {
        ret = Windows::OrginalApi::GetRawInputData(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);
    }
    return ret;
}

UINT WINAPI H_GetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader)
{
    if (HookApp::instance()->uiapp()->isInterceptingInput())
    {
        if (pcbSize)
        {
            if (pData == nullptr)
            {
                Windows::OrginalApi::GetRawInputBuffer(NULL, pcbSize, sizeof(RAWINPUTHEADER));
            }
            if (*pcbSize > 0)
            {
                UINT AllocatedBufferByteCount = *pcbSize * 16;
                RAWINPUT* RawInputBuffer = reinterpret_cast<RAWINPUT*>(malloc(AllocatedBufferByteCount));

                UINT AllocatedBufferByteCountTwo = AllocatedBufferByteCount;
                UINT Result = Windows::OrginalApi::GetRawInputBuffer(RawInputBuffer, &(AllocatedBufferByteCountTwo), sizeof(RAWINPUTHEADER));
                if (Result == -1)
                {
                    LOGGER("n_overlay") << "err :" << GetLastError();
                }
                else if (Result != 0)
                {
                    UINT RawInputCount = Result;
                    DefRawInputProc(&(RawInputBuffer), RawInputCount, sizeof(RAWINPUTHEADER));
                }

                free(RawInputBuffer);
            }
            *pcbSize = 0;
        }
        return 0;
    }
    else
    {
        return Windows::OrginalApi::GetRawInputBuffer(pData, pcbSize, cbSizeHeader);
    }
}

struct InputHooks
{
    std::unique_ptr<ApiHook<pfnGetAsyncKeyState> >  m_GetAsyncKeyStateHook;
    std::unique_ptr<ApiHook<pfnGetKeyState> >  m_GetKeyStateHook;
    std::unique_ptr<ApiHook<pfnGetKeyboardState> >  m_GetKeyboardStateHook;

    std::unique_ptr<ApiHook<pfnShowCursor> >  m_ShowCursorHook;
    std::unique_ptr<ApiHook<pfnGetCursorPos> >  m_GetCursorPosHook;
    std::unique_ptr<ApiHook<pfnSetCursorPos> >  m_SetCursorPosHook;

    std::unique_ptr<ApiHook<pfnSetCursor> >  m_SetCursorHook;
    std::unique_ptr<ApiHook<pfnGetCursor> >  m_GetCursorHook;

    std::unique_ptr<ApiHook<pfnGetRawInputData> >  m_GetRawInputDataHook;
    std::unique_ptr<ApiHook<pfnGetRawInputBuffer> >  m_GetRawInputBufferHook;

};

static InputHooks g_inputHooks;


#define DoInputHook(hModule, Function) \
T_##Function = (pfn##Function)GetProcAddress(hModule, #Function);\
g_inputHooks.m_##Function##Hook.reset(new ApiHook<pfn##Function>(L#Function, (DWORD_PTR*)T_##Function, (DWORD_PTR*)H_##Function));\
result &= g_inputHooks.m_##Function##Hook->activeHook();\

bool InputHook::hook()
{
    bool result = true;
    HMODULE hUser32 = LoadLibraryA("user32.dll");

    DoInputHook(hUser32, GetAsyncKeyState);
    DoInputHook(hUser32, GetKeyState);
    DoInputHook(hUser32, GetKeyboardState);

    DoInputHook(hUser32, ShowCursor);
    DoInputHook(hUser32, GetCursorPos);
    DoInputHook(hUser32, SetCursorPos);
    DoInputHook(hUser32, SetCursor);
    DoInputHook(hUser32, GetCursor);

    DoInputHook(hUser32, GetRawInputData);
    DoInputHook(hUser32, GetRawInputBuffer);

    this->hooked_ = result;
    return result;
}

void InputHook::unhook()
{
    g_inputHooks.m_GetAsyncKeyStateHook.reset();
    g_inputHooks.m_GetKeyStateHook.reset();
    g_inputHooks.m_GetKeyboardStateHook.reset();

    g_inputHooks.m_ShowCursorHook.reset();
    g_inputHooks.m_GetCursorPosHook.reset();
    g_inputHooks.m_SetCursorPosHook.reset();

    g_inputHooks.m_SetCursorHook.reset();
    g_inputHooks.m_GetCursorHook.reset();

    g_inputHooks.m_GetRawInputDataHook.reset();
    g_inputHooks.m_GetRawInputBufferHook.reset();
}

void InputHook::saveInputState()
{
    if (!g_savedInputStatus.inputStateSaved)
    {
        g_savedInputStatus.cursorCount = Windows::OrginalApi::ShowCursor(TRUE);
        g_savedInputStatus.cursorCount -= 1;

        int showCursorCounter = Windows::OrginalApi::ShowCursor(TRUE);

        while (showCursorCounter  < 0)
        {
            auto nextCounter = Windows::OrginalApi::ShowCursor(TRUE);

            if (nextCounter == showCursorCounter)
            {
                LOGGER("n_overlay") << "oops!";
            }

            showCursorCounter = nextCounter;
        }

        POINT cursorPos = { 0 };
        Windows::OrginalApi::GetCursorPos(&cursorPos);
        g_savedInputStatus.cursorPos.store(cursorPos);

        g_savedInputStatus.cursor = Windows::OrginalApi::GetCursor();

        g_savedInputStatus.inputStateSaved = true;
    }
}

void InputHook::restoreInputState()
{
    if (g_savedInputStatus.inputStateSaved)
    {
        int curCursorCount = Windows::OrginalApi::ShowCursor(FALSE);

        if (g_savedInputStatus.cursorCount != curCursorCount)
        {
            BOOL showOrHide = g_savedInputStatus.cursorCount > curCursorCount ? TRUE : FALSE;

            int showCursorCounter = Windows::OrginalApi::ShowCursor(showOrHide);

            while (showCursorCounter != g_savedInputStatus.cursorCount)
            {

                auto nextCounter = Windows::OrginalApi::ShowCursor(showOrHide);

                if (nextCounter == showCursorCounter)
                {
                    LOGGER("n_overlay") << "oops!";
                }

                showCursorCounter = nextCounter;
            }
        }

        Windows::OrginalApi::SetCursor(g_savedInputStatus.cursor);
        if (g_savedInputStatus.cursorVisible)
        {
            Windows::OrginalApi::ShowCursor(TRUE);
        }

        g_savedInputStatus.inputStateSaved = false;
    }
}


SHORT Windows::OrginalApi::GetAsyncKeyState(_In_ int vKey)
{
    if (g_inputHooks.m_GetAsyncKeyStateHook)
    {
        return g_inputHooks.m_GetAsyncKeyStateHook->callOrginal<SHORT>(vKey);
    }
    else
    {
        return ::GetAsyncKeyState(vKey);
    }
}

SHORT Windows::OrginalApi::GetKeyState(_In_ int vKey)
{
    if (g_inputHooks.m_GetKeyStateHook)
    {
        return g_inputHooks.m_GetKeyStateHook->callOrginal<SHORT>(vKey);
    }
    else
    {
        return ::GetKeyState(vKey);
    }
}

BOOL Windows::OrginalApi::GetKeyboardState(__out_ecount(256) PBYTE lpKeyState)
{
    if (g_inputHooks.m_GetKeyboardStateHook)
    {
        return g_inputHooks.m_GetKeyboardStateHook->callOrginal<BOOL>(lpKeyState);
    }
    else
    {
        return ::GetKeyboardState(lpKeyState);
    }
}

int Windows::OrginalApi::ShowCursor(__in BOOL bShow)
{
    if (g_inputHooks.m_ShowCursorHook)
    {
        return g_inputHooks.m_ShowCursorHook->callOrginal<int>(bShow);
    }
    else
    {
        return ::ShowCursor(bShow);
    }
}

BOOL Windows::OrginalApi::GetCursorPos(LPPOINT lpPoint)
{
    if (g_inputHooks.m_GetCursorPosHook)
    {
        return g_inputHooks.m_GetCursorPosHook->callOrginal<BOOL>(lpPoint);
    }
    else
    {
        return ::GetCursorPos(lpPoint);
    }
}

BOOL Windows::OrginalApi::SetCursorPos(int X, int Y)
{
    if (g_inputHooks.m_SetCursorPosHook)
    {
        return g_inputHooks.m_SetCursorPosHook->callOrginal<BOOL>(X, Y);
    }
    else
    {
        return ::SetCursorPos(X, Y);
    }
}

HCURSOR Windows::OrginalApi::GetCursor()
{
    if (g_inputHooks.m_GetCursorHook)
    {
        return g_inputHooks.m_GetCursorHook->callOrginal<HCURSOR>();
    }
    else
    {
        return ::GetCursor();
    }
}

HCURSOR Windows::OrginalApi::SetCursor(HCURSOR cursor)
{
    if (g_inputHooks.m_SetCursorHook)
    {
        return g_inputHooks.m_SetCursorHook->callOrginal<HCURSOR>(cursor);
    }
    else
    {
        return ::SetCursor(cursor);
    }
}

UINT Windows::OrginalApi::GetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader)
{
    if (g_inputHooks.m_GetRawInputDataHook)
    {
        return g_inputHooks.m_GetRawInputDataHook->callOrginal<UINT>(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);
    }
    else
    {
        return ::GetRawInputData(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);
    }
}

UINT Windows::OrginalApi::GetRawInputBuffer(PRAWINPUT pData, PUINT pcbSize, UINT cbSizeHeader)
{
    if (g_inputHooks.m_GetRawInputBufferHook)
    {
        return g_inputHooks.m_GetRawInputBufferHook->callOrginal<UINT>(pData, pcbSize, cbSizeHeader);
    }
    else
    {
        return ::GetRawInputBuffer(pData, pcbSize, cbSizeHeader);
    }
}
