
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <Mmsystem.h>

#ifdef min
#undef min
#endif // min

#ifdef max
#undef max
#endif // max

#include "corelib.h"


#include <dinput.h>

#include <d3d9.h>
#include "dxsdk/Include/d3dx9.h"

#include "D3DUtils.h"
#include "fpstimer.h"
#include "input.h"
#include "D9Game.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "dxsdk/Include/DxErr.h"
#pragma comment(lib, "dxerr.lib")
