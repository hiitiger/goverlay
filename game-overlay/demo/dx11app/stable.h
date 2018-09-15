#pragma once

#include <Windows.h>

#ifdef min
#undef min
#endif // min

#ifdef max
#undef max
#endif // max

#include "corelib.h"


#include <string>
#include <sstream>
#include <memory>
#include <assert.h>
#include <vector>
#include <math.h>

#pragma warning(disable:4005)

#include <d3d11.h>
#include <D3DCompiler.h>

#include "dxsdk/Include/d3dx11.h"
#include "dxsdk/Include/d3dx10.h"
#include <dxsdk/Include/DxErr.h>
#include <dxsdk/Include/xnamath.h>


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "dxerr.lib")
#pragma comment(lib, "D3DCompiler.lib")

#ifdef _WIN64
#pragma comment(lib, "Effects11.x64.lib")
#pragma comment(lib, "FW1FontWrapper.x64.lib")

#else
#pragma comment(lib, "Effects11.lib")
#pragma comment(lib, "FW1FontWrapper.lib")
#endif

#include "d3dx11effect.h"
#include "FW1FontWrapper.h"

#include "./src/d3dutil.h"
#include "./src/GameTimer.h"
#include "./src/camera.h"

#include "./src/dx11sprite.h"
#include "./src/DxAppBase.h"

#include "./src/geometrygenerator.h"
#include "common/mathhelper.h"
#include "common/d3d11savestatus.h"
