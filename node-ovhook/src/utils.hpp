#include <Windows.h>
#include <string>

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

} // namespace win_utils
