#pragma  once
#include "graphics.h"


struct DXGIHookData
{
    HANDLE dxgiModule_ = nullptr;
    HANDLE d3d10Module_ = nullptr;
    HANDLE d3d11Module_ = nullptr;
};

class DXGIHook : public IHookModule, public DXGIHookData
{
    
public:
    DXGIHook();
    ~DXGIHook();

    bool hook() override;
    void unhook() override;


};