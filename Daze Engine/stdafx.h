// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here

#define VK_PROTOTYPES 1

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#include <vulkan\vulkan.h>
#include <vulkan\vk_platform.h>
#pragma comment(lib, "vulkan-1.lib")


#define DAZE_ENGINE_NAME "Daze Engine"
#define DAZE_ENGINE_VER_MAJOR 0
#define DAZE_ENGINE_VER_MINOR 1
#define DAZE_ENGINE_VER_COMB (DAZE_ENGINE_VER_MAJOR << 8 | DAZE_ENGINE_VER_MINOR)