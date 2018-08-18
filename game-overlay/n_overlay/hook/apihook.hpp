#pragma once
template <class I, class O>
struct force_cast
{
    union {
        I i;
        O o;
    } mm;

    force_cast(I i)
    {
        mm.i = i;
    }

    operator O()
    {
        return cast();
    }

    O cast()
    {
        return mm.o;
    }
};

template <class Fn>
struct ApiHook
{
    Fn ppOriginal_ = nullptr;

    std::wstring name_;
    DWORD_PTR *pTarget_ = nullptr;
    DWORD_PTR *pHooked_ = nullptr;
    bool actived_ = false;

  public:
    ApiHook(const std::wstring &name, DWORD_PTR *pTarget, DWORD_PTR *pHooked)
        : name_(name), pTarget_(pTarget), pHooked_(pHooked), actived_(false)
    {
    }

    ~ApiHook()
    {
        removeHook();
    }

    bool activeHook()
    {
        if (!actived_)
        {

            MH_STATUS r = MH_CreateHook(pTarget_, pHooked_, (LPVOID *)&ppOriginal_);
            if (MH_OK == r)
            {
                r = MH_EnableHook(pTarget_);
                if (MH_OK == r)
                {
                    actived_ = true;
                }
                else
                {
                    MH_RemoveHook(pTarget_);
                    ppOriginal_ = nullptr;
                }
            }
        }

        return succeed();
    }

    void removeHook()
    {
        if (actived_)
        {
            MH_RemoveHook(pTarget_);
        }
        actived_ = false;
    }

    template <class ReturnType, class... A>
    ReturnType callOrginal(A... args)
    {
        return ppOriginal_(std::forward<A>(args)...);
    }

    Fn orignal()
    {
        return force_cast<DWORD_PTR *, Fn>(pTarget_);
    }

    bool succeed() { return actived_; }
};

inline DWORD_PTR *getVFunctionAddr(DWORD_PTR *object, int index)
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
