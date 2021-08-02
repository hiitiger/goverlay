#pragma once

#include "winheader.h"

#include "minhook/MinHook.h"


__pragma(warning(push))
__pragma(warning(disable:4005))
#include <dxgitype.h>

#include <d3d9.h>

#include <dxgi.h>
#include <d3d11.h>
#include <d3d10.h>
#include <D3DCompiler.h>

#include "dxgi1_2.h"
__pragma(warning(pop))


#include <Wincodec.h>

#include "corelib.h"
#include <assert.h>
#include "utils.hpp"

#include "share_mem.h"

#include "geometry.h"
#include "message/gmessage.hpp"

#include "common.hpp"

#include "hook/apihook.hpp"
#include "graphics/graphics.h"
#include "graphics/fpstimer.h"



#ifndef ReleaseCOM
#	define ReleaseCOM(p) {if (NULL != (p)) {(p)->Release(); (p) = NULL;}}
#endif	


#ifdef _DEBUG
#define __trace__ std::cout << std::endl << __FUNCTION__ 
#else
#define __trace__ LOGGER("n_overlay") 
#endif

#define AUTO_INPUT_INTERCEPT 1
