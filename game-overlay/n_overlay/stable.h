#pragma once

#include "winheader.h"

#include "minhook/MinHook.h"

#include <dxgitype.h>

__pragma(warning(push))
__pragma(warning(disable:4005))

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


#define BOOST_ALL_NO_LIB
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "geometry.h"
#include "common.hpp"
#include "message/gmessage.hpp"

#include "hook/apihook.hpp"
#include "graphics/graphics.h"
#include "graphics/fpstimer.h"


#ifndef ReleaseCOM
#	define ReleaseCOM(p) {if (NULL != (p)) {(p)->Release(); (p) = NULL;}}
#endif	


#define __trace__  std::cout << __FUNCTION__ << std::endl;