// DavidTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DavidTest.h"
#include <Shellapi.h>

#define MAX_LOADSTRING 100

// Global Variables:
HWND hMainWnd;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

TCHAR *trimWhiteSpace(TCHAR *inputStr)
{
	TCHAR *end;
	TCHAR *temp, *str;
	int inputSize;

	inputSize = wcslen(inputStr) + 1;

	temp = (TCHAR *)malloc(inputSize * sizeof(TCHAR));
	wcscpy_s(temp, inputSize, inputStr);

	str = temp;

	// Trim leading space
	while(iswspace(*str)) str++;

	if(*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	end = str + wcslen(str) - 1;
	while(end > str && iswspace(*end)) end--;

	// Write new null terminator
	*(end+1) = 0;

	wcscpy_s(inputStr, inputSize, str);

	free(temp);

	return inputStr;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	trimWhiteSpace(lpCmdLine);

	if(lpCmdLine[0] != L'\0')
	{
		MessageBox(NULL, lpCmdLine, L"Test", MB_OK);
		ShellExecute(NULL, L"open", L"schtasks", lpCmdLine, 0, SW_HIDE);
		exit(0);
	}

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DAVIDTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DAVIDTEST));

	RegisterHotKey(hMainWnd, 0, MOD_SHIFT | MOD_CONTROL, VK_A);

	//TCHAR testBuffer[2048];
	//swprintf(testBuffer, 2048, L"%d", ProcSpeedRead());
	//MessageBox(NULL, testBuffer, L"Test", MB_OK);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	UnregisterHotKey(hMainWnd, 0);
	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DAVIDTEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DAVIDTEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   hMainWnd = hWnd;

   return TRUE;
}

void PressKey(WORD vKey)
{
	PressKeyDown(vKey, TRUE);
	PressKeyUp(vKey, TRUE);
}

void PressKeyDown(WORD vKey, BOOL delay)
{
	INPUT key;

	memset(&key, 0, sizeof(INPUT));
	key.type = INPUT_KEYBOARD;
	key.ki.wVk = vKey;
	key.ki.dwExtraInfo = PLAYMAILER_TAG;
	SendInput(1, &key, sizeof(INPUT));
	
	if(delay)
		Sleep(50);
}

void PressKeyUp(WORD vKey, BOOL delay)
{
	INPUT key;

	memset(&key, 0, sizeof(INPUT));
	key.type = INPUT_KEYBOARD;
	key.ki.dwFlags = KEYEVENTF_KEYUP;
	key.ki.wVk = vKey;
	key.ki.dwExtraInfo = PLAYMAILER_TAG;
	SendInput(1, &key, sizeof(INPUT));
	
	if(delay)
		Sleep(50);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	int err;

	switch (message)
	{
	case WM_HOTKEY:
		//ShellExecute(hWnd, L"runas", L"F:\DavidTest.exe", L"/Create /TN \"Dude\\Dude\" /TR \"F:\\Projects\\VS\\PlayMailer\\PlayMailer\\bin\\PlayMailerDI64.exe\" /SC ONCE /ST 00:00 /RL HIGHEST /F", 0, SW_HIDE);
		// L"cmd /c schtasks /Create /TN \"PlayMailer Test\" /TR \"F:\Projects\VS\PlayMailer\PlayMailer\bin\disableinput64.exe /ENABLE\" /SC ONCE /ST 00:00 /RL HIGHEST /F"
		return 0;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
