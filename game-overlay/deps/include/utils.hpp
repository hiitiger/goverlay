#include <Windows.h>
#include <string>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;


namespace win_utils
{
inline std::string toUtf8(const wchar_t *wstr, int size = -1)
{
    std::string str;
    if (size < 0)
    {
        size = (int)wcslen(wstr);
    }
    int bytesNeed = WideCharToMultiByte(CP_UTF8, NULL, wstr, size, NULL, 0, NULL, FALSE);
    str.resize(bytesNeed);
    WideCharToMultiByte(CP_UTF8, NULL, wstr, size, const_cast<char *>(str.c_str()), bytesNeed, NULL, FALSE);
    return str;
}

inline std::string toUtf8(const std::wstring &wstr)
{
    return toUtf8(wstr.c_str(), wstr.size());
}

inline bool createProcess(const std::wstring& path, const std::wstring& argument)
{
    PROCESS_INFORMATION ProcInfo = { 0 };
    STARTUPINFO StartupInfo = { 0 };

    std::wstring dir = path;
    dir.erase(path.find_last_of('\\'));
    std::wstring cmdLine = L"\"";
    cmdLine.append(path).append(L"\" ").append(argument);

    BOOL ret = CreateProcessW(path.c_str(), (LPWSTR)(cmdLine.c_str()), NULL, NULL, FALSE, 0, NULL, dir.c_str(), &StartupInfo, &ProcInfo);

    if (ProcInfo.hProcess != NULL)
    {
        CloseHandle(ProcInfo.hProcess);
    }
    if (ProcInfo.hThread != NULL)
    {
        CloseHandle(ProcInfo.hThread);
    }

    return ret == TRUE;
}

inline std::wstring applicationDirPath()
{
    WCHAR szPath[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, szPath, MAX_PATH);
    WCHAR achLongPath[MAX_PATH] = { 0 };
    TCHAR** lppPart = { NULL };
    ::GetFullPathNameW(szPath, MAX_PATH, achLongPath, lppPart);

    std::wstring dirPath = achLongPath;
    size_t lastSepartor = dirPath.find_last_of('\\');
    dirPath.erase(lastSepartor);
    return dirPath;
}

inline std::wstring moduleDirPath()
{
    WCHAR   szPath[MAX_PATH] = {0};
    GetModuleFileNameW((HINSTANCE)&__ImageBase, szPath, _countof(szPath));

    WCHAR achLongPath[MAX_PATH] = { 0 };
    TCHAR** lppPart = { NULL };
    ::GetFullPathNameW(szPath, MAX_PATH, achLongPath, lppPart);

    std::wstring dirPath = achLongPath;
    size_t lastSepartor = dirPath.find_last_of('\\');
    dirPath.erase(lastSepartor);
    return dirPath;
}

inline bool customizeUIPIPolicy(HWND window, UINT message, bool add)
{
    typedef BOOL (WINAPI* fnExType)(
        HWND                hWnd,
        UINT                message,
        DWORD               action,
        PCHANGEFILTERSTRUCT pChangeFilterStruct
        );

    typedef BOOL(WINAPI *fnType)(UINT, DWORD);


    HMODULE hModule = LoadLibrary(L"user32.dll");
    if (!hModule)
    {
        return false;
    }

    fnExType pfnEx = (fnExType)::GetProcAddress(hModule, "ChangeWindowMessageFilterEx");
    if (pfnEx)
    {
        pfnEx(window, message, add ? MSGFLT_ALLOW : MSGFLT_DISALLOW, nullptr);
    }
    else
    {
        fnType pfn = (fnType)::GetProcAddress(hModule, "ChangeWindowMessageFilter");
        if (!pfn)
        {
            FreeLibrary(hModule);
            return false;
        }

        pfn(message, add ? MSGFLT_ADD : MSGFLT_REMOVE);
    }

    FreeLibrary(hModule);
    return true;
}

} // namespace win_utils
