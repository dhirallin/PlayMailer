// MinimizeToTray
//
// A couple of routines to show how to make it produce a custom caption
// animation to make it look like we are minimizing to and maximizing 
// from the system tray
//
// These routines are public domain, but it would be nice if you dropped
// me a line if you use them!
//
// 1.0 29.06.2000 Initial version
// 1.1 01.07.2000 The window retains it's place in the Z-order of windows
//     when minimized/hidden. This means that when restored/shown, it doen't
//     always appear as the foreground window unless we call SetForegroundWindow
//
// Copyright 2000 Matthew Ellis <m.t.ellis@bigfoot.com>
#include "stdafx.h"
#include "MinimizeToTray.h"

// Odd. VC++6 winuser.h has IDANI_CAPTION defined (as well as IDANI_OPEN and
// IDANI_CLOSE), but the Platform SDK only has IDANI_OPEN...

// I don't know what IDANI_OPEN or IDANI_CLOSE do. Trying them in this code
// produces nothing. Perhaps they were intended for window opening and closing
// like the MAC provides...
#ifndef IDANI_OPEN
#define IDANI_OPEN 1
#endif
#ifndef IDANI_CLOSE
#define IDANI_CLOSE 2
#endif
#ifndef IDANI_CAPTION
#define IDANI_CAPTION 3
#endif

#define DEFAULT_RECT_WIDTH 150
#define DEFAULT_RECT_HEIGHT 30

// Returns the rect of where we think the system tray is. This will work for
// all current versions of the shell. If explorer isn't running, we try our
// best to work with a 3rd party shell. If we still can't find anything, we
// return a rect in the lower right hand corner of the screen
static VOID GetTrayWndRect(LPRECT lpTrayRect)
{
  // First, we'll use a quick hack method. We know that the taskbar is a window
  // of class Shell_TrayWnd, and the status tray is a child of this of class
  // TrayNotifyWnd. This provides us a window rect to minimize to. Note, however,
  // that this is not guaranteed to work on future versions of the shell. If we
  // use this method, make sure we have a backup!
  HWND hShellTrayWnd=FindWindowEx(NULL,NULL,TEXT("Shell_TrayWnd"),NULL);
  if(hShellTrayWnd)
  {
    HWND hTrayNotifyWnd=FindWindowEx(hShellTrayWnd,NULL,TEXT("TrayNotifyWnd"),NULL);
    if(hTrayNotifyWnd)
    {
      GetWindowRect(hTrayNotifyWnd,lpTrayRect);
      return;
    }
  }

  // OK, we failed to get the rect from the quick hack. Either explorer isn't
  // running or it's a new version of the shell with the window class names
  // changed (how dare Microsoft change these undocumented class names!) So, we
  // try to find out what side of the screen the taskbar is connected to. We
  // know that the system tray is either on the right or the bottom of the
  // taskbar, so we can make a good guess at where to minimize to
  APPBARDATA appBarData;
  appBarData.cbSize=sizeof(appBarData);
  if(SHAppBarMessage(ABM_GETTASKBARPOS,&appBarData))
  {
    // We know the edge the taskbar is connected to, so guess the rect of the
    // system tray. Use various fudge factor to make it look good
    switch(appBarData.uEdge)
    {
      case ABE_LEFT:
      case ABE_RIGHT:
	// We want to minimize to the bottom of the taskbar
	lpTrayRect->top=appBarData.rc.bottom-100;
	lpTrayRect->bottom=appBarData.rc.bottom-16;
	lpTrayRect->left=appBarData.rc.left;
	lpTrayRect->right=appBarData.rc.right;
	break;

      case ABE_TOP:
      case ABE_BOTTOM:
	// We want to minimize to the right of the taskbar
	lpTrayRect->top=appBarData.rc.top;
	lpTrayRect->bottom=appBarData.rc.bottom;
	lpTrayRect->left=appBarData.rc.right-100;
	lpTrayRect->right=appBarData.rc.right-16;
	break;
    }

    return;
  }

  // Blimey, we really aren't in luck. It's possible that a third party shell
  // is running instead of explorer. This shell might provide support for the
  // system tray, by providing a Shell_TrayWnd window (which receives the
  // messages for the icons) So, look for a Shell_TrayWnd window and work out
  // the rect from that. Remember that explorer's taskbar is the Shell_TrayWnd,
  // and stretches either the width or the height of the screen. We can't rely
  // on the 3rd party shell's Shell_TrayWnd doing the same, in fact, we can't
  // rely on it being any size. The best we can do is just blindly use the
  // window rect, perhaps limiting the width and height to, say 150 square.
  // Note that if the 3rd party shell supports the same configuraion as
  // explorer (the icons hosted in NotifyTrayWnd, which is a child window of
  // Shell_TrayWnd), we would already have caught it above
  hShellTrayWnd=FindWindowEx(NULL,NULL,TEXT("Shell_TrayWnd"),NULL);
  if(hShellTrayWnd)
  {
    GetWindowRect(hShellTrayWnd,lpTrayRect);
    if(lpTrayRect->right-lpTrayRect->left>DEFAULT_RECT_WIDTH)
      lpTrayRect->left=lpTrayRect->right-DEFAULT_RECT_WIDTH;
    if(lpTrayRect->bottom-lpTrayRect->top>DEFAULT_RECT_HEIGHT)
      lpTrayRect->top=lpTrayRect->bottom-DEFAULT_RECT_HEIGHT;

    return;
  }

  // OK. Haven't found a thing. Provide a default rect based on the current work
  // area
  SystemParametersInfo(SPI_GETWORKAREA,0,lpTrayRect,0);
  lpTrayRect->left=lpTrayRect->right-DEFAULT_RECT_WIDTH;
  lpTrayRect->top=lpTrayRect->bottom-DEFAULT_RECT_HEIGHT;
}

// Check to see if the animation has been disabled
static BOOL GetDoAnimateMinimize(VOID)
{
  ANIMATIONINFO ai;

  ai.cbSize=sizeof(ai);
  SystemParametersInfo(SPI_GETANIMATION,sizeof(ai),&ai,0);

  return ai.iMinAnimate?TRUE:FALSE;
}

LRESULT CALLBACK CaptionWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void AnimateToTray(HWND hWnd)
{
	HWND hCaptionWindow;
	RECT rcFrom, rcTo;
	TCHAR windowText[512];
	WNDCLASSEX wcex;
	HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	int xInc, yInc, captionWidth;

	if(GetDoAnimateMinimize())
	{
		// Get the rect of the tray and the window. Note that the window rect
		// is still valid even though the window is hidden
    
		GetTrayWndRect(&rcTo);
		
		GetWindowRect(hWnd,&rcFrom);
		GetWindowText(hWnd, windowText, sizeof(windowText));

		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= CaptionWndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= (HICON)SendMessage(hWnd, WM_GETICON, ICON_BIG, 0);
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= L"Caption Window";
		wcex.hIconSm		= (HICON)SendMessage(hWnd, WM_GETICON, ICON_SMALL, 0);
		
		if(!RegisterClassEx(&wcex))
			return;

		hCaptionWindow = CreateWindow(L"Caption Window", windowText, WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION, rcFrom.left,
			rcFrom.top, 0, 0, NULL, NULL, hInstance, NULL);
		
		captionWidth = GetSystemMetrics(SM_CYCAPTION) + 3;

		SetWindowPos(hCaptionWindow, NULL, 0, 0, 220, captionWidth, SWP_NOMOVE | SWP_NOZORDER);
		
		xInc = (rcTo.left - rcFrom.left) / ANIMATE_TRAY_NUMFRAMES;
		yInc = (rcTo.top + captionWidth - rcFrom.top) / ANIMATE_TRAY_NUMFRAMES; 

		for(int i = 0; i < ANIMATE_TRAY_NUMFRAMES; i++)
		{
			Sleep(ANIMATE_TRAY_FRAMERATE);
			SetWindowPos(hCaptionWindow, NULL, rcFrom.left+=xInc, rcFrom.top+=yInc, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}

		DestroyWindow(hCaptionWindow);
		UnregisterClass(L"Caption Window", hInstance);
	}
}

VOID MinimizeWndToTray(HWND hWnd)
{
  // Add the tray icon. If we add it before the call to DrawAnimatedRects,
  // the taskbar gets erased, but doesn't get redrawn until DAR finishes.
  // This looks untidy, so call the functions in this order

  // Hide the window
  ShowWindow(hWnd, SW_HIDE);
  ShowOwnedPopups(hWnd, FALSE);
  AnimateToTray(hWnd);
}

VOID RestoreWndFromTray(HWND hWnd)
{
  // Show the window, and make sure we're the foreground window
  ShowWindow(hWnd, SW_SHOW);
  if(IsIconic(hWnd))
	  ShowWindow(hWnd, SW_SHOWNORMAL);
	  
  SetActiveWindow(hWnd);
  SetForegroundWindow(hWnd);
  ShowOwnedPopups(hWnd, TRUE);

  // Remove the tray icon. As described above, remove the icon after the
  // call to DrawAnimatedRects, or the taskbar will not refresh itself
  // properly until DAR finished
}

VOID ShowNotifyIcon(HWND hWnd, BOOL bAdd)
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid,sizeof(nid));
	nid.cbSize=sizeof(NOTIFYICONDATA);
	nid.hWnd=hWnd;
	nid.uID=0;
	nid.uFlags=NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage=WM_TRAYMESSAGE;
	nid.hIcon = (HICON)GetClassLong(hWnd, GCL_HICONSM);
	nid.uVersion = 0;
	GetWindowText(hWnd, nid.szTip, sizeof(nid.szTip));
	wcscat_s(nid.szTip, sizeof(nid.szTip), L"\nDouble-click to maximize");

	if(bAdd)
	{
		for(int i = 0; i < 60; i++)
		{
			if(Shell_NotifyIcon(NIM_ADD, &nid))
				break;

			Sleep(1000);
		}
	}
	else
		Shell_NotifyIcon(NIM_DELETE, &nid);
}

BOOL CALLBACK MinimizeToTrayProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	static bool bHideIcon=false;
	static UINT s_uTaskbarRestart;

	switch(uiMsg)
	{
		case WM_CREATE:
			s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
			break;
		case WM_SYSCOMMAND:
			if(wParam==SC_MINIMIZE)
			{
				MinimizeWndToTray(hWnd);
				ShowNotifyIcon(hWnd, TRUE);

				// Return TRUE to tell DefDlgProc that we handled the message, and set
				// DWL_MSGRESULT to 0 to say that we handled WM_SYSCOMMAND
				SetWindowLong(hWnd,DWL_MSGRESULT,0);
				return TRUE;
			}
			break;
		case WM_TRAYMESSAGE:
			switch(lParam)
			{
				case WM_LBUTTONDBLCLK:
					RestoreWndFromTray(hWnd);

					// If we remove the icon here, the following WM_LBUTTONUP (the user
					// releasing the mouse button after a double click) can be sent to
					// the icon that occupies the position we were just using, which can
					// then activate, when the user doesn't want it to. So, set a flag
					// so that we remove the icon on the next WM_LBUTTONUP
					bHideIcon=true;
					return TRUE;
				case WM_LBUTTONUP:
					// The user has previously double-clicked the icon, remove it in
					// response to this second WM_LBUTTONUP
					if(bHideIcon)
					{
						ShowNotifyIcon(hWnd,FALSE);
						bHideIcon=false;
					}
					return TRUE;
			}
			break;
		case WM_DESTROY:
			ShowNotifyIcon(hWnd, FALSE);
			break;
		default:
			if(uiMsg == s_uTaskbarRestart)
				ShowNotifyIcon(hWnd, TRUE);
			break;
	}

	return FALSE;
}
