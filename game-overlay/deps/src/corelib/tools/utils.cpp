#include "stable.h"
#include "utils.h"
#include <string.h>
#include <ShlObj.h>


namespace Storm {

    std::wstring Utils::fromLocal8Bit(const char* str, int size /*= -1*/)
    {
        if (size < 0)
        {
            size = (int)strlen(str);
        }
        std::wstring wstr;
        int bytesNeed = MultiByteToWideChar(CP_ACP, 0, str, size, 0, 0);
        wstr.resize(bytesNeed);
        MultiByteToWideChar(CP_ACP, 0, str, size, const_cast<wchar_t*>(wstr.c_str()), bytesNeed);
        return wstr;
    }

    std::wstring Utils::fromLocal8Bit(const std::string& str)
    {
        return fromLocal8Bit(str.c_str(), str.size());
    }

    std::string Utils::toLocal8Bit(const wchar_t* wstr, int size /*= -1*/)
    {
        std::string str;
        if (size < 0)
        {
            size = (int)wcslen(wstr);
        }
        int bytesNeed = WideCharToMultiByte(CP_ACP, NULL, wstr, size, NULL, 0, NULL, FALSE);
        str.resize(bytesNeed);
        WideCharToMultiByte(CP_ACP, NULL, wstr, size, const_cast<char*>(str.c_str()), bytesNeed, NULL, FALSE);
        return str;
    }

    std::string Utils::toLocal8Bit(const std::wstring& wstr)
    {
        return toLocal8Bit(wstr.c_str(), wstr.size());
    }

    std::wstring Utils::fromUtf8(const char* str, int size /*= -1*/)
    {
        std::wstring wstr;
        if (size < 0)
        {
            size = (int)strlen(str);
        }
        int bytesNeed = MultiByteToWideChar(CP_UTF8, 0, str, size, 0, 0);
        wstr.resize(bytesNeed);
        MultiByteToWideChar(CP_UTF8, 0, str, size, const_cast<wchar_t*>(wstr.c_str()), bytesNeed);
        return wstr;
    }

    std::wstring Utils::fromUtf8(const std::string& str)
    {
        return fromUtf8(str.c_str(), str.size());
    }

    std::string Utils::toUtf8(const wchar_t* wstr, int size /*= -1*/)
    {
        std::string str;
        if (size < 0)
        {
            size = (int)wcslen(wstr);
        }
        int bytesNeed = WideCharToMultiByte(CP_UTF8, NULL, wstr, size, NULL, 0, NULL, FALSE);
        str.resize(bytesNeed);
        WideCharToMultiByte(CP_UTF8, NULL, wstr, size, const_cast<char*>(str.c_str()), bytesNeed, NULL, FALSE);
        return str;
    }

    std::string Utils::toUtf8(const std::wstring& str)
    {
        return toUtf8(str.c_str(), str.size());
    }

    char _toHex(unsigned int d)
    {
        return "0123456789abcdef"[(d & 0x0f)];
    }

    int _fromHex(char c)
    {
        if (c >= '0' && c <= '9')
        {
            return c - '0';
        }
        else if (c >= 'a' && c <= 'f')
        {
            return c - 'a' + 10;
        }
        else if (c >= 'A' && c <= 'F')
        {
            return c - 'A' + 10;
        }
        return 0;
    }

    std::string Utils::toHex(unsigned int d)
    {
        std::deque<char> hex;
        while (d)
        {
            unsigned int rem = d % 16;
            hex.push_front(_toHex(rem));
            d /= 16;
        }
        return std::string(hex.begin(), hex.end());
    }

    unsigned int Utils::fromHex(const std::string& h)
    {
        unsigned int d = 0;
        unsigned int p = 1;
        for (int i = h.size() - 1; i >= 0; --i)
        {
            d += p * _fromHex(h[i]);
            p *= 16;
        }

        return d;
    }

    std::string Utils::toHexCode(const std::string& d)
    {
        std::string hex(d.size() * 2, 0);
        for (int i = 0; i != d.size(); ++i)
        {
            unsigned int j = d[i] >> 4 & 0x0f;
            hex[i * 2] = _toHex(j);

            j = d[i]  & 0x0f;
            hex[i * 2 + 1] = _toHex(j);
        }

        return hex;
    }

    std::string Utils::fromHexCode(const std::string& h)
    {
        std::string str((h.size() + 1)/ 2, 0);
        char* data = const_cast<char*>(str.data());
        bool low_part = false;
        for (int i = 0; i != h.size(); ++i)
        {
            char j = h[i];
            int d = _fromHex(j);
            if (low_part)
            {
                *data |= d & 0x0f;
                ++data;
            }
            else
            {
                *data |= (d & 0x0f) << 4;
            }

            low_part = !low_part;
        }

        return str;
    }

    std::string Utils::simpleEncode(std::string text, char x)
    {
        auto  xor_enc = [=](std::string str) {
            for (int i = 0; i != str.size(); ++i)
            {
                str[i] = ((str[i]) ^ x);
            }

            return str;
        };

        return toHexCode(xor_enc(text));
    }

    std::string Utils::simpleDecode(std::string text, char x)
    {
        auto  xor_enc = [=](std::string str) {
            for (int i = 0; i != str.size(); ++i)
            {
                str[i] = ((str[i]) ^ x);
            }

            return str;
        };

        return xor_enc(fromHexCode(text));
    }

    std::wstring Utils::appDataPath()
    {
        WCHAR szPath[MAX_PATH];
        SHGetSpecialFolderPathW(NULL, szPath, CSIDL_LOCAL_APPDATA, TRUE);
        return szPath;
    }

    void Utils::makeSureDirExist(const std::wstring& dir)
    {
        if (!File::exists(dir))
        {
            SHCreateDirectory(NULL, dir.c_str());
        }
    }

    std::wstring Utils::appProcessName()
    {
        std::wstring procssName;
        WCHAR exePath[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, exePath, sizeof(exePath) / sizeof(WCHAR));
        WCHAR* name = wcsrchr(exePath, '\\');
        if (name != 0)
        {
            procssName = name + 1;
        }
        else
        {
            procssName = exePath;
        }

        return procssName;
    }

    std::wstring Utils::applicationDirPath()
    {
        WCHAR szPath[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, szPath, MAX_PATH);
        WCHAR achLongPath[MAX_PATH] = { 0 };
        TCHAR** lppPart = { NULL };
        ::GetFullPathNameW(szPath, MAX_PATH, achLongPath, lppPart);

        std::wstring dirPath = achLongPath;
        size_t lastSepartor = dirPath.find_last_of('\\');
        dirPath.erase(lastSepartor);
        return dirPath;
    }

    void Utils::usleep(std::int64_t microsecs_to_sleep)
    {
#if defined(_WIN32)
        LARGE_INTEGER ft;
        ft.QuadPart = -(10 * microsecs_to_sleep);
        HANDLE timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
        SetWaitableTimer(timer, &ft, 0, nullptr, nullptr, 0);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
#else
        std::this_thread::sleep_for(std::chrono::microseconds(microsecs_to_sleep));
#endif
    }
}