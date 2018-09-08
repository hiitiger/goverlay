#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class WinDll
{
    HMODULE hModule_ = nullptr;
    WinDll(const WinDll&) = delete;
    WinDll& operator=(const WinDll&) = delete;

public:
    WinDll(const wchar_t* libName) 
    { 
        hModule_ = (::LoadLibraryW(libName));
    }

    ~WinDll()
    {
        if (hModule_)
        {
            ::FreeLibrary(hModule_);
        }
    };

    WinDll(WinDll&& rhs)
        : hModule_(rhs.hModule_)
    {
        rhs.hModule_ = HMODULE(0);
    };

    WinDll& operator=(WinDll&& rhs)
    {
        hModule_ = rhs.hModule_;
        rhs.hModule_ = HMODULE(0);
        return *this;
    };

    template <typename FuncPtrType>
    FuncPtrType GetProcAddress(const char* procName) const
    {
        return (FuncPtrType)(::GetProcAddress(hModule_, procName));
    };

    bool loaded() const
    {
        return !!hModule_;
    }

    HMODULE module() const 
    {
        return hModule_;
    }
 };
