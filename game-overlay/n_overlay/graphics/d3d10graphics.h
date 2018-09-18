#pragma once

#include "dxgigraphics.h"


struct D3d10WindowSprite
{
    int windowId;
    std::string name;
    overlay::WindowRect rect;
    std::string bufferName;

    std::unique_ptr<boost::interprocess::windows_shared_memory> windowBitmapMem;
    std::unique_ptr<boost::interprocess::mapped_region> fullRegion;

    Windows::ComPtr<ID3D10Texture2D> texture;
};


class D3d10Graphics : public DxgiGraphics
{
    Windows::ComPtr<IDXGISwapChain> swap_;

public:
    D3d10Graphics();
    ~D3d10Graphics();

    Windows::ComPtr<IDXGISwapChain> swapChain() const override;
    void freeGraphics() override;

    bool _initGraphicsContext(IDXGISwapChain* swap);
    bool _initGraphicsState();
    void _initSpriteDrawer();

    void _createSprites();
    void _createWindowSprites();

    std::shared_ptr<D3d10WindowSprite> _createWindowSprite(const std::shared_ptr<overlay::Window>& window);
    void _updateSprite(std::shared_ptr<D3d10WindowSprite>& sprite, bool clear = false);

    void _checkAndResyncWindows() override;


    void _drawBlockSprite()override;
    void _drawWindowSprites() override;
    void _drawMainSprite()override;

    void _drawWindowSprite(std::shared_ptr<D3d10WindowSprite>&);

    void _saveStatus() override;
    void _prepareStatus() override;
    void _restoreStatus() override;
};