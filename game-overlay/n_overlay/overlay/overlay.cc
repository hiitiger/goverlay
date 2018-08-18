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

void OverlayConnector::sendGraphicsHookInfo(const std::map<std::string, std::string>& data)
{
    CHECK_THREAD(Threads::HookApp);

    _sendGraphicsHookInfo();
}

void OverlayConnector::sendGraphicsWindowSetupInfo()
{
    CHECK_THREAD(Threads::Graphics);

    HookApp::instance()->async([this]() {
        _sendGraphicsWindowSetupInfo();
    });
}

void OverlayConnector::sendInputBlocked()
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this]() {
        _sendInputBlocked();
    });
}

void OverlayConnector::sendInputUnBlocked()
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this]() {
        _sendInputUnBlocked();
    });
}

void OverlayConnector::sendGameWindowInput()
{
    CHECK_THREAD(Threads::Window);

    HookApp::instance()->async([this]() {
        _sendGameWindowInput();
    });
}

void OverlayConnector::sendGameWindowEvent()
{
    CHECK_THREAD(Threads::Window);
    HookApp::instance()->async([this]() {
        _sendGameWindowEvent();
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

void OverlayConnector::_sendGraphicsHookInfo()
{
    CHECK_THREAD(Threads::HookApp);
}

void OverlayConnector::_sendGraphicsWindowSetupInfo()
{
    CHECK_THREAD(Threads::HookApp);
}

void OverlayConnector::_sendInputBlocked()
{
    CHECK_THREAD(Threads::HookApp);
}

void OverlayConnector::_sendInputUnBlocked()
{
    CHECK_THREAD(Threads::HookApp);
}

void OverlayConnector::_sendGameWindowInput()
{
    CHECK_THREAD(Threads::HookApp);
}

void OverlayConnector::_sendGameWindowEvent()
{
    CHECK_THREAD(Threads::HookApp);
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
}

void OverlayConnector::saveClientId(IIpcLink * /*link*/, int clientId)
{
    ipcClientId_ = clientId;
}