#include "stable.h"
#include "overlay.h"
#include "hookapp.h"
#include "session.h"

const char k_overlayIpcName[] = "n_overlay_1a1y2o8l0b";

OverlayConnector::OverlayConnector()
{
}

OverlayConnector::~OverlayConnector()
{
}

void OverlayConnector::start()
{
    CHECK_THREAD(Threads::HookApp);

    std::string ipcName = k_overlayIpcName;
    ipcName.append("-");
    ipcName.append(win_utils::toLocal8Bit(HookApp::instance()->procName()));
    ipcName.append("-");
    ipcName.append(std::to_string(::GetCurrentProcessId()));
    getIpcCenter()->init(ipcName);

    std::string mainIpcName = k_overlayIpcName;
    mainIpcName.append("-");
    ipcLink_ = getIpcCenter()->getLink(mainIpcName);
    ipcLink_->addClient(this);
    getIpcCenter()->connectToHost(ipcLink_, "", "", false);
}

void OverlayConnector::quit()
{
    CHECK_THREAD(Threads::HookApp);

    if (ipcLink_)
    {
        _sendOverlayExit();
        getIpcCenter()->closeLink(ipcLink_);
        ipcLink_ = nullptr;
    }
    getIpcCenter()->uninit();
}

void OverlayConnector::sendInputHookInfo()
{
    CHECK_THREAD(Threads::HookApp);

    _sendInputHookInfo();
}

void OverlayConnector::sendGraphicsHookInfo(const D3d9HookInfo &info)
{
    CHECK_THREAD(Threads::HookApp);

    HookApp::instance()->async([this, info]() {
        _sendGraphicsHookInfo(info);
    });
}

void OverlayConnector::sendGraphicsHookInfo(const DxgiHookInfo &info)
{
    CHECK_THREAD(Threads::HookApp);

    HookApp::instance()->async([this, info]() {
        _sendGraphicsHookInfo(info);
    });
}

void OverlayConnector::sendGraphicsWindowSetupInfo(HWND window, int width, int height, bool focus, bool hooked)
{
    CHECK_THREAD(Threads::HookApp);

    HookApp::instance()->async([this, window, width, height, focus, hooked]() {
        _sendGraphicsWindowSetupInfo(window, width, height, focus, hooked);
    });
}

void OverlayConnector::sendGraphicsWindowResizeEvent(HWND window, int width, int height)
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this, window, width, height]() {
        _sendGraphicsWindowResizeEvent(window, width, height);
    });
}

void OverlayConnector::sendGraphicsWindowFocusEvent(HWND window, bool focus)
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this, window, focus]() {
        _sendGraphicsWindowFocusEvent(window, focus);
    });
}

void OverlayConnector::sendInputIntercept()
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this]() {
        _sendInputIntercept(true);
    });
}

void OverlayConnector::sendInputStopIntercept()
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this]() {
        _sendInputIntercept(false);
    });
}

void OverlayConnector::sendGameWindowInput()
{
    CHECK_THREAD(Threads::Window);

    HookApp::instance()->async([this]() {
        _sendGameWindowInput();
    });
}


void OverlayConnector::_heartbeat()
{
    CHECK_THREAD(Threads::HookApp);
}

void OverlayConnector::_sendOverlayExit()
{
    CHECK_THREAD(Threads::HookApp);
}

void OverlayConnector::_sendGameProcessInfo()
{
    CHECK_THREAD(Threads::HookApp);
}

void OverlayConnector::_sendInputHookInfo()
{
    CHECK_THREAD(Threads::HookApp);
}

void OverlayConnector::_sendGraphicsHookInfo(const D3d9HookInfo &info)
{
    CHECK_THREAD(Threads::HookApp);

    overlay::D3d9HookInfo hookInfo;
    hookInfo.endSceneHooked = info.endSceneHooked;
    hookInfo.presentHooked = info.presentHooked;
    hookInfo.presentExHooked = info.presentExHooked;
    hookInfo.swapChainPresentHooked = info.swapChainPresentHooked;
    hookInfo.resetHooked = info.resetHooked;
    hookInfo.resetExHooked = info.resetExHooked;

    overlay::GraphicsHookInfo message;
    message.graphics = "d3d9";
    message.d3d9hookInfo = hookInfo;

    _sendMessage(&message);
}

void OverlayConnector::_sendGraphicsHookInfo(const DxgiHookInfo &info)
{
    CHECK_THREAD(Threads::HookApp);
    overlay::DxgiHookInfo hookInfo;

    overlay::GraphicsHookInfo message;
    message.graphics = "dxgi";
    message.dxgihookInfo = hookInfo;

    _sendMessage(&message);
}

void OverlayConnector::_sendGraphicsWindowSetupInfo(HWND window, int width, int height, bool focus, bool hooked)
{
    CHECK_THREAD(Threads::HookApp);

    overlay::GraphicsWindowSetup message;
    message.window = (std::uint32_t)window;
    message.width = width;
    message.height = height;
    message.focus = focus;
    message.hooked = hooked;
    _sendMessage(&message);
}

void OverlayConnector::_sendInputIntercept(bool v)
{
    CHECK_THREAD(Threads::HookApp);

    overlay::GameInputIntercept message;
    message.intercepting = v;

    _sendMessage(&message);
}

void OverlayConnector::_sendGameWindowInput()
{
    CHECK_THREAD(Threads::HookApp);
}

void OverlayConnector::_sendGraphicsWindowResizeEvent(HWND window, int width, int height)
{
    CHECK_THREAD(Threads::HookApp);
    overlay::GraphcisWindowRezizeEvent message;
    message.window = (std::uint32_t)window;
    message.width = width;
    message.height = height;

    _sendMessage(&message);
}

void OverlayConnector::_sendGraphicsWindowFocusEvent(HWND window, bool focus)
{
    CHECK_THREAD(Threads::HookApp);
    overlay::GraphcisWindowFocusEvent message;
    message.window = (std::uint32_t)window;
    message.focus = focus;

    _sendMessage(&message);
}

void OverlayConnector::_sendMessage(overlay::GMessage *message)
{
    overlay::OverlayIpc ipcMsg;

    overlay::json obj = message->toJson();

    ipcMsg.type = message->type;
    ipcMsg.message = obj.dump();

    getIpcCenter()->sendMessage(ipcLink_, ipcClientId_, 0, &ipcMsg);
}

void OverlayConnector::onIpcMessage()
{
}

void OverlayConnector::onFrameBuffer()
{
}

void OverlayConnector::onCommand()
{
}

void OverlayConnector::onGraphicsCommand()
{
}

void OverlayConnector::onLinkConnect(IIpcLink *link)
{
    DAssert(link == ipcLink_);

    LOGGER("n_overlay") << "@trace";

    this->_sendGameProcessInfo();
}

void OverlayConnector::onLinkClose(IIpcLink *link)
{
    DAssert(link == ipcLink_);
    ipcLink_ = nullptr;
}

void OverlayConnector::onMessage(IIpcLink * /*link*/, int /*hostPort*/, const std::string &message)
{
    int ipcMsgId = *(int*)message.c_str();
    if (ipcMsgId == overlay::OverlayIpc::MsgId)
    {
        overlay::OverlayIpc ipcMsg;
        ipcMsg.upack(message);

        if (ipcMsg.type == "window")
        {
            std::shared_ptr<overlay::Window> overlayMsg = std::make_shared<overlay::Window>() ;
            overlay::json json = overlay::json::parse(ipcMsg.message);
            overlayMsg->fromJson(json);

            {
                std::lock_guard<std::mutex> lock(windowsLock_);
                windows_.push_back(overlayMsg);
            }
            if (overlayMsg->transparent)
            {
                _updateFrameBuffer(overlayMsg->windowId, overlayMsg->bufferName);
            }

            this->windowEvent()(overlayMsg->windowId);

        }
        else if (ipcMsg.type == "window.framebuffer")
        {
            std::shared_ptr<overlay::FrameBuffer> overlayMsg = std::make_shared<overlay::FrameBuffer>();
            overlay::json json = overlay::json::parse(ipcMsg.message);
            overlayMsg->fromJson(json);

            std::lock_guard<std::mutex> lock(windowsLock_);

            auto it = std::find_if(windows_.begin(), windows_.end(), [&](const auto& window) {
                return overlayMsg->windowId == window->windowId;
            });

            if (it != windows_.end())
            {
                auto window = *it;
                if (window->transparent)
                {
                    _updateFrameBuffer(window->windowId, window->bufferName);
                }

                this->frameBufferEvent()(window->windowId);
            }
        }
    }
}

void OverlayConnector::saveClientId(IIpcLink * /*link*/, int clientId)
{
    ipcClientId_ = clientId;
}

void OverlayConnector::_updateFrameBuffer(std::uint32_t windowId, const std::string& bufferName)
{
    namespace share_mem = boost::interprocess;

    std::shared_ptr<share_mem::windows_shared_memory> windowBitmapMem;
    std::shared_ptr<share_mem::mapped_region> fullRegion;

    try
    {
        windowBitmapMem.reset(new boost::interprocess::windows_shared_memory(share_mem::open_only, bufferName.c_str(), share_mem::read_only));
        fullRegion.reset(new share_mem::mapped_region(*windowBitmapMem, share_mem::read_only));
    }
    catch (...)
    {
    }

    if (fullRegion)
    {
        Storm::ScopeLovkV1 lockShareMem(shareMemoryLock_);

        char *orgin = static_cast<char*>(fullRegion->get_address());
        overlay::ShareMemFrameBuffer* head = (overlay::ShareMemFrameBuffer*)orgin;
        int* mem = (int*)(orgin + sizeof(overlay::ShareMemFrameBuffer));

        std::shared_ptr<overlay_game::FrameBuffer> frameBuffer(new overlay_game::FrameBuffer(head->width, head->height, mem));

        std::lock_guard<std::mutex> lock(framesLock_);
        frameBuffers_[windowId] = frameBuffer;

    }
}
