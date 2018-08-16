#include <napi.h>

namespace overlay
{
Napi::Value start(const Napi::CallbackInfo &info);

Napi::Value stop(const Napi::CallbackInfo &info);

Napi::Value setEventCallback(const Napi::CallbackInfo &info);

Napi::Value log(const Napi::CallbackInfo &info);

Napi::Value sendCommand(const Napi::CallbackInfo &info);

Napi::Value sendGraphicsCommand(const Napi::CallbackInfo &info);

Napi::Value sendFrameBuffer(const Napi::CallbackInfo &info);


} // namespace overlay
