#include <Windows.h>
#include <Shellapi.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>

int g_waitCount = 20;

bool unsafeInjectDll(DWORD dwProcessId, PCWSTR pszLibFile)
{
    bool bOk = false; // Assume that the function fails
    HANDLE hProcess = NULL, hThread = NULL;
    PWSTR pszLibFileRemote = NULL;

    __try
    {
        // Get a handle for the target process.
        hProcess = OpenProcess(
            PROCESS_QUERY_INFORMATION | // Required by Alpha
                PROCESS_CREATE_THREAD | // For CreateRemoteThread
                PROCESS_VM_OPERATION |  // For VirtualAllocEx/VirtualFreeEx
                PROCESS_VM_WRITE,       // For WriteProcessMemory
            FALSE, dwProcessId);
        if (hProcess == NULL)
            __leave;

        // Calculate the number of bytes needed for the DLL's pathname
        int cch = 1 + lstrlenW(pszLibFile);
        int cb = cch * sizeof(wchar_t);

        // Allocate space in the remote process for the pathname
        pszLibFileRemote = (PWSTR)
            VirtualAllocEx(hProcess, NULL, cb, MEM_COMMIT, PAGE_READWRITE);
        if (pszLibFileRemote == NULL)
            __leave;

        // Copy the DLL's pathname to the remote process' address space
        if (!WriteProcessMemory(hProcess, pszLibFileRemote,
                                (PVOID)pszLibFile, cb, NULL))
            __leave;

        // Get the real address of LoadLibraryW in Kernel32.dll
        PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
            GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
        if (pfnThreadRtn == NULL)
            __leave;

        // Create a remote thread that calls LoadLibraryW(DLLPathname)
        hThread = CreateRemoteThread(hProcess, NULL, 0,
                                     pfnThreadRtn, pszLibFileRemote, 0, NULL);
        if (hThread == NULL)
            __leave;

        // Wait for the remote thread to terminate
        WaitForSingleObject(hThread, INFINITE);

        bOk = true; // Everything executed successfully
    }
    __finally
    { // Now, we can clean everything up

        // Free the remote memory that contained the DLL's pathname
        if (pszLibFileRemote != NULL)
            VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);

        if (hThread != NULL)
            CloseHandle(hThread);

        if (hProcess != NULL)
            CloseHandle(hProcess);
    }

    return (bOk);
}

#ifndef MAKEULONGLONG
#define MAKEULONGLONG(ldw, hdw) ((ULONGLONG(hdw) << 32) | ((ldw)&0xFFFFFFFF))
#endif

#ifndef MAXULONGLONG
#define MAXULONGLONG ((ULONGLONG) ~((ULONGLONG)0))
#endif

DWORD getProcMainThreadId(DWORD dwProcID)
{
    DWORD dwMainThreadID = 0;
    ULONGLONG ullMinCreateTime = MAXULONGLONG;

    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap != INVALID_HANDLE_VALUE)
    {
        THREADENTRY32 th32;
        th32.dwSize = sizeof(THREADENTRY32);
        BOOL bOK = TRUE;
        for (bOK = Thread32First(hThreadSnap, &th32); bOK;
             bOK = Thread32Next(hThreadSnap, &th32))
        {
            if (dwMainThreadID == 0)
            {
                dwMainThreadID = th32.th32ThreadID;
            }
            if (th32.th32OwnerProcessID == dwProcID)
            {
                HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION,
                                            TRUE, th32.th32ThreadID);

                if (hThread)
                {
                    FILETIME afTimes[4] = {0};
                    if (GetThreadTimes(hThread,
                                       &afTimes[0], &afTimes[1], &afTimes[2], &afTimes[3]))
                    {
                        ULONGLONG ullTest = MAKEULONGLONG(afTimes[0].dwLowDateTime,
                                                          afTimes[0].dwHighDateTime);
                        if (ullTest && ullTest < ullMinCreateTime)
                        {
                            ullMinCreateTime = ullTest;
                            dwMainThreadID = th32.th32ThreadID; // let it be main... :)
                        }
                    }
                    CloseHandle(hThread);
                }
            }
        }
#ifndef UNDER_CE
        CloseHandle(hThreadSnap);
#else
        CloseToolhelp32Snapshot(hThreadSnap);
#endif
    }
    return dwMainThreadID;
}

bool safeInjectDll(DWORD pid, DWORD threadId, const std::wstring &dll)
{
    typedef HHOOK(WINAPI * fn)(int, HOOKPROC, HINSTANCE, DWORD);
    HMODULE user32 = GetModuleHandleW(L"USER32");
    fn set_windows_hook_ex;
    HMODULE lib = LoadLibraryW(dll.c_str());
    LPVOID proc;
    HHOOK hook;

    if (!lib || !user32)
    {
        if (!lib)
        {
            std::wcout << L"LoadLibraryW failed:" << dll;
        }
        if (!user32)
        {
            std::wcout << L"USER32 module not found:" << dll;
        }
        return false;
    }

#ifdef _WIN64
    proc = GetProcAddress(lib, "msg_hook_proc_ov");
#else
    proc = GetProcAddress(lib, "_msg_hook_proc_ov@12");
#endif

    if (!proc)
    {
        std::wcout << L"GetProcAddress msg_hook_proc_ov failed";
        return false;
    }

    set_windows_hook_ex = (fn)GetProcAddress(user32, "SetWindowsHookExA");
    if (threadId == 0)
    {
        threadId = getProcMainThreadId(pid);
    }

    std::wcout << "hook "
               << "pid: " << pid << ", thread:" << threadId;

    hook = set_windows_hook_ex(WH_GETMESSAGE, (HOOKPROC)proc, lib, threadId);
    if (!hook)
    {
        DWORD err = GetLastError();
        std::wcout << L"SetWindowsHookEx failed: " << err;
        return false;
    }

    for (auto i = 0; i < g_waitCount; i++)
    {
        Sleep(500);
        std::wcout << L"PostThreadMessage to hook window";

        PostThreadMessage(threadId, WM_USER + 432, 0, (LPARAM)hook);
    }
    return true;
}

bool startInject(DWORD pid, DWORD threadId, const std::wstring &dll)
{
    bool succeed = false;

    if (pid > 0)
    {
        if (safeInjectDll(pid, threadId, dll))
        {
            succeed = true;
        }
        std::wcout << L"safeInject, pid:" << pid << L", result:" << succeed;
    }

    return succeed;
}

int main()
{
    LPWSTR cmdlineRaw = GetCommandLineW();
    int numArgs = 0;
    LPWSTR *cmdLine = CommandLineToArgvW(cmdlineRaw, &numArgs);

    if (numArgs == 4)
    {

        std::wstring pidStr = cmdLine[1];
        std::wstring injectThreadStr = cmdLine[2];
        std::wstring dll = cmdLine[3];

        DWORD pid = std::stoul(pidStr);
        DWORD threadId = std::stoul(injectThreadStr);

        bool succeed = startInject(pid, threadId, dll);

        return succeed ? 0 : 1;
    }
    else
    {
        return -1;
    }
}
