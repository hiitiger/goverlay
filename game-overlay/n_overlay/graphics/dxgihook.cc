#include "stable.h"
#include "dxgihook.h"

DXGIHook::DXGIHook()
{

}

DXGIHook::~DXGIHook()
{
    unhook();
}

bool DXGIHook::hook()
{
    return false;
}

void DXGIHook::unhook()
{

}
