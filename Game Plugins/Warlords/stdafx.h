// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"") 

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include "targetver.h"

// Windows Header Files:
#include <windows.h>

#ifdef _DEBUG
#define LIB_NAME_DECORATION(NAME) NAME ## "_d.lib"
#else
#define LIB_NAME_DECORATION(NAME) NAME ## ".lib"
#endif

// C RunTime Header Files
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <WindowsX.h>
#include <htmlhelp.h>



