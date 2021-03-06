// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"") 

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <stdint.h>

#ifdef _DEBUG
#define LIB_NAME_DECORATION(NAME) NAME ## "_d.lib"
#else
#define LIB_NAME_DECORATION(NAME) NAME ## ".lib"
#endif

// C RunTime and Windows Header Files
#include <Commdlg.h> 
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
#include <Richedit.h>
#include <errno.h>
#include <math.h>

// WinSock
#include <Winsock2.h>
#include <ws2tcpip.h>

// For ShellExecute()
#include <ShellAPI.h>
// Common Controls / XP Visual Styles
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
// For handling fonts
#include <wingdi.h> 
#pragma comment(lib, "gdi32.lib") 
// Used by SHBrowseForFolder() etc
#include <shlobj.h>
#include <objbase.h>
// Used by StrStrI
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
// Enable XP Visual Styles

// Calculate CRC for file
#include <ImageHlp.h>
#pragma comment(lib, "Imagehlp.lib")

// PlaySound support
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// Drawing images
#pragma comment(lib, "Msimg32.lib")

// HTML Help
#pragma comment(lib, "HtmlHelp.lib")

// For checking if internet is connected
#include <Wininet.h>
#pragma comment(lib, "Wininet.lib")

#include <Wincrypt.h>
#pragma comment(lib, "Crypt32.lib")

#pragma comment( lib, LIB_NAME_DECORATION("libconfig") )
#pragma comment( lib, LIB_NAME_DECORATION("libetpan") )

//#pragma comment( lib, "libeay32.lib")
