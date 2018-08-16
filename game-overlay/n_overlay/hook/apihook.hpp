#pragma once
#include "winheader.h"

template <class Fn>
struct ApiHook
{
    Fn orginalFunc_ = nullptr;
};

inline DWORD_PTR * getVFunctionAddr(DWORD_PTR *object, int index)
{
    DWORD_PTR *vAddr = nullptr;
    if (object)
    {
        DWORD_PTR *vtblPointer = reinterpret_cast<DWORD_PTR *>(*object);
        if (vtblPointer)
        {
            vAddr = reinterpret_cast<DWORD_PTR *>(*(vtblPointer + index));
        }
    }

    return vAddr;
}
