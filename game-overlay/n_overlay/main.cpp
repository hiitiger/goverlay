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



INT WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
         trace::DebugConsole::allocDebugConsole();

        wchar_t name[MAX_PATH];
        GetModuleFileNameW(hModule, name, MAX_PATH);
        ::LoadLibraryW(name);

        g_moduleHandle = hModule;
        DisableThreadLibraryCalls((HMODULE)hModule);

        HookApp::initialize();
    }
    if (dwReason == DLL_PROCESS_DETACH)
    {
        HookApp::uninitialize();
    }

    return TRUE;
}
