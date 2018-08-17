#include "stable.h"
#include "overlay.h"

namespace overlay
{

OverlayConnector::OverlayConnector()
{
}

OverlayConnector::~OverlayConnector()
{
}


void OverlayConnector::start()
{

}

void OverlayConnector::stop()
{

}

void OverlayConnector::onIpcMessage()
{

}

void OverlayConnector::onCommand()
{

}

void OverlayConnector::onGraphicsCommand()
{

}

void OverlayConnector::onLinkConnect(IIpcLink* link)
{
}

void OverlayConnector::onLinkClose(IIpcLink* /*link*/)
{

}

void OverlayConnector::onMessage(IIpcLink* /*link*/, int /*hostPort*/, const std::string& message)
{

}

void OverlayConnector::saveClientId(IIpcLink* /*link*/, int clientId)
{
    ipcClientId_ = clientId;
}

} // namespace overlay