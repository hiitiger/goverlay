#include <napi.h>
#include "overlay.h"


Napi::String Method(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::String::New(env, "hello world");
}


Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "hello"), Napi::Function::New(env, Method));

  exports.Set(Napi::String::New(env, "start"), Napi::Function::New(env, overlay::start));
  exports.Set(Napi::String::New(env, "stop"), Napi::Function::New(env, overlay::stop));


  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)