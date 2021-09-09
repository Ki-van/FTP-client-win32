// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <windowsx.h>

#include <combaseapi.h>
#include <shobjidl_core.h>
#include <shlwapi.h>

#include <commctrl.h>
#include <tchar.h>

#include <iostream>

#include "Ws2tcpip.h"

//my own
#include <logger.h>
#include <FTPClient.h>
