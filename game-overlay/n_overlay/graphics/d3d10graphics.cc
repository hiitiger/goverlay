#include "D3d10Graphics.h"

D3d10Graphics::D3d10Graphics()
{

}

D3d10Graphics::~D3d10Graphics()
{
    freeGraphics();
}


Windows::ComPtr<IDXGISwapChain> D3d10Graphics::swapChain() const
{
    return swap_;
}

void D3d10Graphics::freeGraphics()
{
    __super::freeGraphics();
}


bool D3d10Graphics::_initGraphicsContext(IDXGISwapChain* swap)
{
    return false;
}

bool D3d10Graphics::_initGraphicsState()
{
    return false;
}

void D3d10Graphics::_initSpriteDrawer()
{

}

void D3d10Graphics::_createSprites()
{

}

void D3d10Graphics::_createWindowSprites()
{

}

std::shared_ptr<D3d10WindowSprite> D3d10Graphics::_createWindowSprite(const std::shared_ptr<overlay::Window>& window)
{
    return nullptr;
}

void D3d10Graphics::_updateSprite(std::shared_ptr<D3d10WindowSprite>& sprite, bool clear /*= false*/)
{

}

void D3d10Graphics::_checkAndResyncWindows()
{

}

void D3d10Graphics::_drawBlockSprite()
{

}

void D3d10Graphics::_drawWindowSprites()
{

}

void D3d10Graphics::_drawMainSprite()
{

}

void D3d10Graphics::_drawWindowSprite(std::shared_ptr<D3d10WindowSprite>&)
{

}

void D3d10Graphics::_saveStatus()
{

}

void D3d10Graphics::_prepareStatus()
{

}

void D3d10Graphics::_restoreStatus()
{

}

