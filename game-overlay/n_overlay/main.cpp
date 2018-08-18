#include "stable.h"
#include "./overlay/session.h"
#include "./overlay/hookapp.h"

HHOOK g_injectHook = nullptr;

extern "C" __declspec(dllexport) LRESULT CALLBACK msg_hook_proc_ov(int code,
                                                                   WPARAM wparam, LPARAM lparam)
{
    static bool hooking = true;
    MSG *msg = (MSG *)lparam;

    if (hooking && msg->message == (WM_USER + 432))
    {
        typedef BOOL(WINAPI * fn)(HHOOK);
        LOGGER("n_overlay") << "@trace threadId:" << ::GetCurrentThreadId();

        g_injectHook = (HHOOK)msg->lParam;

        session::setInjectWindow(msg->hwnd);
    }

    return CallNextHookEx(0, code, wparam, lparam);
}

HINSTANCE g_moduleHandle = nullptr;

HANDLE g_hookAppThread = nullptr;


INT WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        wchar_t name[MAX_PATH];
        GetModuleFileNameW(hModule, name, MAX_PATH);
        ::LoadLibraryW(name);

        MH_Initialize();

        g_moduleHandle = hModule;
        DisableThreadLibraryCalls((HMODULE)hModule);

        g_hookAppThread = HookApp::instance()->start();
    }
    if (dwReason == DLL_PROCESS_DETACH)
    {
        MH_Uninitialize();

        if (g_hookAppThread)
        {
            HookApp::instance()->quit();

            if (WaitForSingleObject(g_hookAppThread, 1000) != WAIT_OBJECT_0)
            {
                TerminateThread(g_hookAppThread, 0);
            }
        }
    }

    return TRUE;
}
