#include "stable.h"

HHOOK g_injectHook = nullptr;

extern "C" __declspec(dllexport) LRESULT CALLBACK msg_hook_proc_ov(int code,
                                                                   WPARAM wparam, LPARAM lparam)
{
    static bool hooking = true;
    MSG *msg = (MSG *)lparam;

    if (hooking && msg->message == (WM_USER + 432))
    {
        typedef BOOL(WINAPI * fn)(HHOOK);
        std::cout << "@trace msg_hook_proc_ov:" << std::this_thread::get_id() << std::endl;

        g_injectHook = (HHOOK)msg->lParam;
    }

    return CallNextHookEx(0, code, wparam, lparam);
}

HINSTANCE g_module_handle = nullptr;

INT WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        wchar_t name[MAX_PATH];
        GetModuleFileNameW(hModule, name, MAX_PATH);
        ::LoadLibraryW(name);

        MH_Initialize();

        g_module_handle = hModule;
        DisableThreadLibraryCalls((HMODULE)hModule);
    }
    if (dwReason == DLL_PROCESS_DETACH)
    {
        MH_Uninitialize();
    }

    return TRUE;
}
