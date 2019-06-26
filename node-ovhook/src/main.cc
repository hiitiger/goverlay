#include <napi.h>
#include <Windows.h>
#include <psapi.h>
#include <string>
#include <vector>
#include <iostream>
#include "utils.hpp"

const WCHAR k_inject_helper[] = L"n_ovhelper.exe";
const WCHAR k_inject_helper_x64[] = L"n_ovhelper.x64.exe";

const WCHAR k_inject_dll[] = L"n_overlay.dll";
const WCHAR k_inject_dll_x64[] = L"n_overlay.x64.dll";

struct win_scope_handle
{
  HANDLE handle = nullptr;

  win_scope_handle(HANDLE h = nullptr)
      : handle(h)
  {
  }

  ~win_scope_handle()
  {
    ::CloseHandle(this->handle);
  }

  win_scope_handle &operator=(HANDLE h)
  {
    if (this->handle)
    {
      ::CloseHandle(this->handle);
    }
    this->handle = h;
    return *this;
  }

  operator bool() const
  {
    return !!handle;
  }
};

enum class window_search_mode
{
  INCLUDE_MINIMIZED = 0,
  EXCLUDE_MINIMIZED = 1
};

struct window_hook_info
{
  HWND hwnd;
  std::wstring title;
  DWORD processId;
  DWORD threadId;
};

static inline bool file_exists(const std::wstring& file)
{
    WIN32_FILE_ATTRIBUTE_DATA findData;
    return 0 != ::GetFileAttributesEx(file.c_str(), GetFileExInfoStandard, &findData);
}


static bool check_window_valid(HWND window, enum window_search_mode mode)
{
  DWORD styles, ex_styles;
  RECT rect;

  if (!IsWindowVisible(window) || (mode == window_search_mode::EXCLUDE_MINIMIZED && IsIconic(window)))
    return false;

  GetClientRect(window, &rect);
  styles = (DWORD)GetWindowLongPtr(window, GWL_STYLE);
  ex_styles = (DWORD)GetWindowLongPtr(window, GWL_EXSTYLE);

  if (ex_styles & WS_EX_TOOLWINDOW)
    return false;
  if (styles & WS_CHILD)
    return false;
  if (mode == window_search_mode::EXCLUDE_MINIMIZED && (rect.bottom == 0 || rect.right == 0))
    return false;

  return true;
}

static inline HWND next_window(HWND window, enum window_search_mode mode)
{
  while (true)
  {
    window = GetNextWindow(window, GW_HWNDNEXT);
    if (!window || check_window_valid(window, mode))
      break;
  }

  return window;
}

static inline HWND first_window(enum window_search_mode mode)
{
  HWND window = GetWindow(GetDesktopWindow(), GW_CHILD);
  if (!check_window_valid(window, mode))
    window = next_window(window, mode);
  return window;
}

static void get_window_title(std::wstring &name, HWND hwnd)
{
  int len = GetWindowTextLengthW(hwnd);
  if (!len)
    return;
  name.resize(len);
  GetWindowTextW(hwnd, const_cast<wchar_t *>(name.c_str()), len + 1);
}

static bool fill_window_info(window_hook_info &info, HWND hwnd)
{
  wchar_t wname[MAX_PATH];
  win_scope_handle process;
  DWORD processId = 0;
  DWORD threadId = GetWindowThreadProcessId(hwnd, &processId);
  if (!threadId)
    return false;

  if (threadId == GetCurrentProcessId())
    return false;

  process = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
  if (!process)
  {
    // std::cout << "err:" << GetLastError() << std::endl;
    return false;
  }

  if (!GetProcessImageFileNameW(process.handle, wname, MAX_PATH))
    return false;

  info.hwnd = hwnd;
  info.processId = processId;
  info.threadId = threadId;

  get_window_title(info.title, hwnd);

  return true;
}

static inline bool is_64bit_windows()
{
#ifdef _WIN64
  return true;
#else
  BOOL x86 = false;
  bool success = !!IsWow64Process(GetCurrentProcess(), &x86);
  return success && !!x86;
#endif
}

static inline bool is_64bit_process(HANDLE process)
{
  BOOL x86 = true;
  if (is_64bit_windows())
  {
    bool success = !!IsWow64Process(process, &x86);
    if (!success)
    {
      return false;
    }
  }

  return !x86;
}

static bool is_64bit_process(DWORD processId)
{
  win_scope_handle process = OpenProcess(
      PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
      false, processId);
  if (!process)
  {
    return false;
  }
  return is_64bit_process(process.handle);
}

static std::wstring get_inject_helper_path(bool x64)
{

  std::wstring dir = win_utils::moduleDirPath();

  std::wstring helper;
  if (x64)
  {
    helper = dir + L"\\" + k_inject_helper_x64;
  }
  else
  {
    helper = dir + L"\\" + k_inject_helper;
  }

  return helper;
}

static std::wstring get_inject_dll_path(bool x64)
{
  std::wstring dir = win_utils::moduleDirPath();
  std::wstring dll;
  if (x64)
  {
    dll = dir + L"\\" + k_inject_dll_x64;
  }
  else
  {
    dll = dir + L"\\" + k_inject_dll;
  }
  return dll;
}

static bool inject_process(DWORD processId, DWORD threadId)
{
  win_scope_handle process = OpenProcess(
      PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
      false, processId);
  if (!process)
  {
    return false;
  }

  std::wstring dir = win_utils::moduleDirPath();

  std::wstring helper;
  std::wstring dll;
  if (is_64bit_process(process.handle))
  {
    helper = dir + L"\\" + k_inject_helper_x64;
    dll = dir + L"\\" + k_inject_dll_x64;
  }
  else
  {
    helper = dir + L"\\" + k_inject_helper;
    dll = dir + L"\\" + k_inject_dll;
  }

  std::wstring args = std::to_wstring(processId) + L" " + std::to_wstring(threadId) + L" \"" + dll + L"\"";
  return win_utils::createProcess(helper, args);
}

Napi::Value getTopWindows(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  bool include_minimized = false;
  if (info.Length() == 1)
  {
    include_minimized = info[0].As<Napi::Boolean>();
  }

  std::vector<window_hook_info> windows;
  const auto mode = include_minimized ? window_search_mode::INCLUDE_MINIMIZED : window_search_mode::EXCLUDE_MINIMIZED;
  auto window = first_window(mode);
  while (window)
  {
    window_hook_info info = {0};
    if (fill_window_info(info, window))
    {
      windows.push_back(info);
    }
    window = next_window(window, mode);
  }

  auto arr = Napi::Array::New(env, windows.size());
  for (auto i = 0; i != windows.size(); ++i)
  {
    const auto &info = windows[i];
    auto infoObject = Napi::Object::New(env);

    infoObject.Set("windowId", Napi::Value::From(env, (std::uint32_t)info.hwnd));
    infoObject.Set("processId", Napi::Value::From(env, (std::uint32_t)info.processId));
    infoObject.Set("threadId", Napi::Value::From(env, (std::uint32_t)info.threadId));
    infoObject.Set("title", Napi::Value::From(env, win_utils::toUtf8(info.title)));

    arr.Set(i, infoObject);
  }

  return arr;
}

Napi::Value injectProcess(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  if (info.Length() != 1)
  {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Object object = info[0].ToObject();
  const std::uint32_t processId = object.Get("processId").ToNumber().Uint32Value();
  const std::uint32_t threadId = object.Get("threadId").ToNumber().Uint32Value();

  const bool x64 = is_64bit_process(processId);
  std::wstring helper_path = get_inject_helper_path(x64);
  std::wstring dll_path = get_inject_dll_path(x64);
  const bool inject_helper_exist = file_exists(helper_path);
  const bool inject_dll_exist = file_exists(dll_path);
  const bool injected = inject_process(processId, threadId);

  auto result = Napi::Object::New(env);

  result.Set("injectHelper", Napi::Value::From(env, win_utils::toUtf8(helper_path)));
  result.Set("injectDll", Napi::Value::From(env, win_utils::toUtf8(dll_path)));
  result.Set("injectSucceed", Napi::Value::From(env, injected));
  return Napi::Value::From(env, result);
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
  exports.Set(Napi::String::New(env, "getTopWindows"),
              Napi::Function::New(env, getTopWindows));

  exports.Set(Napi::String::New(env, "injectProcess"),
              Napi::Function::New(env, injectProcess));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)