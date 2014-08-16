#ifndef _MINIMIZE_TO_TRAY_H_
#define _MINIMIZE_TO_TRAY_H_

#include "resource.h"

#define TRAY_ICON				IDI_WARLORDSMAILER

#define WM_TRAYMESSAGE			WM_USER + 42

#define ANIMATE_TRAY_FRAMERATE		16
#define ANIMATE_TRAY_NUMFRAMES		35

void MinimizeWndToTray(HWND hWnd);
void RestoreWndFromTray(HWND hWnd);
BOOL CALLBACK MinimizeToTrayProc(HWND hWnd,UINT uiMsg,WPARAM wParam,LPARAM lParam);
void ShowNotifyIcon(HWND hWnd, BOOL bAdd);

#endif // _MINIMIZE_TO_TRAY_H_
