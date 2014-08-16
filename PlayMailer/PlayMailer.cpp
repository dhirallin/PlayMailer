// PlayMailer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "PlayMailer.h"
#include "MinimizeToTray.h"
#include <Shellapi.h>
#include <Winsock2.h>
#include <errno.h>

#define MAX_LOADSTRING 100

// Global Variables:
DWORD ProcSpeed = 0, BaseProcSpeed = 0;

TCHAR mbBuffer[MBBUFFER_SIZE];

HINSTANCE hInst;								// current instance
HANDLE hAppMutex;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR AppFolderPath[MAX_PATH], AppPath[MAX_PATH], HelpPath[MAX_PATH], PluginsPath[MAX_PATH];

HHOOK hMsgBoxHook;

MailSettings *mail;
GeneralSettings *settings;

TCHAR tempFolderPath[MAX_PATH];
TCHAR tempRunCommand[MAX_PATH]; 

const TCHAR *mailProtocolNames[] = {L"IMAP", L"POP3"};
const TCHAR *securityProtocolNames[] = {L"SSL", L"TLS", L"None"};
const TCHAR *mailSettingLabels[] = {L"Your Name", L"Your Email", L"Incoming Mail Server", L"Incoming Mail Settings - Login Name", L"Incoming Mail Settings - Password", L"Outgoing Mail Server", L"Outgoing Mail Settings - Login Name", L"Outgoing Mail Settings - Password"};

TCHAR *settingsPageNames[] = {L"General Settings", L"Mail Settings", L"Game Settings", L"Notify Settings"};
#define NUM_SETTINGS_PAGES (sizeof(settingsPageNames) / sizeof(TCHAR *))
HWND hSettingsPages[NUM_SETTINGS_PAGES];
HTREEITEM hSettingsTreeItems[NUM_SETTINGS_PAGES];

TCHAR *columnHeaders[] = {L"Session Name", L"Session Type", L"Current Player's Turn", L"Actions"};
const int columnWidths[4] = {20, 20, 30, 30};
#define MAINLV_NUM_COLUMNS (sizeof(columnWidths) / sizeof(int))

const int Hotkeys[] = {VK_L, VK_S, VK_M, VK_P, VK_O, VK_H};
#define NUM_HOTKEYS	(sizeof(Hotkeys) / sizeof(int))

#ifdef _DEBUG	
const int DebugHotkeys[] = {VK_Z, VK_V, VK_D, VK_W, VK_U, VK_T, VK_R, VK_MINUS, VK_0, VK_1, VK_2, VK_3, VK_4, VK_5, VK_6, VK_7, VK_8, VK_9};
#define NUM_DEBUGHOTKEYS (sizeof(DebugHotkeys) / sizeof(int))
#endif

UINT HotkeyModifier;

TCHAR *EditPlayersLVHeaders[] = {L"No.", L"Player Email", L"Player Name (Optional)", L"Team"};
const int EditPlayersLVColumnWidths[4] = {9, 36, 33, 22};
#define EDITPLAYERSLV_NUM_COLUMNS (sizeof(EditPlayersLVColumnWidths) / sizeof(int))
HWND hEditPlayersLV;
HWND hFactionComboBox, hPlayerEmailComboBox, hPlayerNameComboBox;
HWND hPlayerEmailEdit, hPlayerNameEdit;
HWND hEditPlayersDialog;
int LastFactionSelected, BaseFaction;

TCHAR *PlayerListLVHeaders[] = {L"", L"", L"Player Email", L"Player Name", L"Team"};
const int PlayerListLVColumnWidths[5] = {5, 3, 38, 33, 21};
#define PLAYERLISTLV_NUM_COLUMNS (sizeof(PlayerListLVColumnWidths) / sizeof(int))
HWND hPlayerListLV;
BOOL *RecipientsMask = NULL;

int numSessions = 0;
LinkedList llSessions = {0};
LVSessionButtons lvButtons[MAX_BUTTON_ROWS];
const TCHAR *lvButtonLabels[] = { L"Load", L"Re-Send", L"Players", L"Settings", L"Delete" };

WNDPROC DefaultLVProc, DefaultStatusBarProc, DefaultEditProc, DefaultComboProc;

int selectedSession = -1;

LinkedList llEmailHistory = {0}, llNameHistory = {0};
LinkedList llGlobalGameSettings = {0};

int FailedFetches = 0;

HANDLE hRecvEmailThread = NULL;
SOCKET IMAPSocket = -1;
HANDLE hRecvEmailEvents[NUM_RECVEMAIL_EVENTS], hDoneEvent;

HANDLE hSendEmailThread = NULL;
HANDLE hSendEmailEvents[NUM_SENDEMAIL_EVENTS];
CRITICAL_SECTION Crit; 
HWND hSendEmailDialog = NULL;

BOOL IncomingSettingsChanged = FALSE;
BOOL LoadGameYourTurn;

// Main Window Controls
HWND hMainWnd, hMainListView;
HWND hSettingsButton, hNewGameButton, hViewHotkeysButton, hViewHelpButton, hContactAuthorButton; 
HWND hMuteSWCheck, hMuteAllCheck;
HWND hStatusBar;

HFONT hSmallFont, hDefaultFont, hLargeFont, hExtraLargeFont, hBoldFont;

HWND hCurrentDialog;
HWND hSessionSettingsDialog;
HWND hSettingsDialog;
HWND hFetchMailDialog;
int SelectedGGPage = CB_ERR;

int HotKeyPressed = 0;
BOOL HotKeysEnabled = FALSE;
BOOL ParseMailEnabled = FALSE;
BOOL CheckYourTurnEnabled = FALSE;
BOOL MailCheckedFirstTime = FALSE;
BOOL AlertSoundEnabled = TRUE;
BOOL ChatSoundEnabled = TRUE;
BOOL GameInProgress = FALSE;
BOOL SettingsLoaded = FALSE;
BOOL GUILoaded = FALSE;
BOOL IsNewSession = FALSE;
BOOL InternetConnectedState = FALSE;

int NewCurrentPlayer = -1;
int YourPlayerNumber = 1;
int NumSessionWideChatEmails = 0;
int NumPrivateChatEmails = 0;

HICON hLargeIcon, hSmallIcon;

int MajorVersion;

Wow64DisableWow64FsRedirectionFn *pWow64DisableWow64FsRedirection;
Wow64RevertWow64FsRedirectionFn *pWow64RevertWow64FsRedirection;

#include "callback_assigns.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	MSG msg;
	HACCEL hAccelTable;
	INITCOMMONCONTROLSEX icex;
	HINSTANCE hRichEditLib;
	BOOL globalGameSettingsOK, mailSettingsOK, hotkeysOK = TRUE;
	int len;
	TCHAR hotkeyModString[1024];
	LPWSTR *szArglist;
	BOOL start = FALSE, minimized = FALSE, createSchTask = FALSE;
	int nArgs;
	OSVERSIONINFO versionInfo;
	
	versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);	
	GetVersionEx(&versionInfo);
	
	MajorVersion = versionInfo.dwMajorVersion;

	SetAppPaths();

	szArglist = CommandLineToArgvW(lpCmdLine, &nArgs);

	for(int i = 0; i < nArgs; i++)
	{	
		if(!_wcsicmp(szArglist[i], L"/START"))
			start = TRUE;
		else if(!_wcsicmp(szArglist[i], L"/INSTALLER"))
			Sleep(2500);
		else if(!_wcsicmp(szArglist[i], L"/CREATE"))
			RemoveProgramFromStartupRegistry();
		else if(!_wcsicmp(szArglist[i], L"/CREATE_SCHTASK"))
			createSchTask = TRUE;
		else if(!_wcsicmp(szArglist[i], L"/MINIMIZED"))
			minimized = TRUE;
	}	

#ifndef _DEBUG
	if(MajorVersion >= 6 && !start) 
	{
		if(createSchTask)
			CreateSchTask(L"PlayMailer", AppPath, L"/START", SCHTASK_ONRUN);
		
		RunSchTask();
		exit(0);
	}
#endif

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PLAYMAILER, szWindowClass, MAX_LOADSTRING);

	hAppMutex = CreateMutex(NULL, FALSE, L"PlayMailer");
#ifndef _DEBUG
	HWND otherInstWnd;
	
	if(WaitForSingleObject(hAppMutex, 0) != WAIT_OBJECT_0)
	{
		if(otherInstWnd = FindWindow(szWindowClass, szTitle))
		{
			if(!IsWindowVisible(otherInstWnd))
				SendMessage(otherInstWnd, WM_BRING_TO_FRONT, 0, 0);

			SetForegroundWindow(otherInstWnd);
							
			return 0;
		}
		else
		{
			MessageBox(NULL, L"PlayMailer is already running.", L"Error running application", MB_OK);
			return 1;
		}
	}
#endif

	BaseProcSpeed = GetBaseProcSpeed();
	ProcSpeed = GetProcSpeed();
	
	HMODULE hkernel = GetModuleHandle(L"kernel32.dll");
	if(hkernel)
	{
		pWow64DisableWow64FsRedirection = reinterpret_cast<Wow64DisableWow64FsRedirectionFn*>(GetProcAddress(hkernel, "Wow64DisableWow64FsRedirection"));
		pWow64RevertWow64FsRedirection = reinterpret_cast<Wow64RevertWow64FsRedirectionFn*>(GetProcAddress(hkernel, "Wow64RevertWow64FsRedirection"));
	}
	
	// Ensure that the common control DLL is loaded. 
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex); 

	InitializeCriticalSection(&Crit);
	CoInitialize(NULL);
	hRichEditLib = LoadLibrary(TEXT("Riched20.dll"));
	 
	//LoadIconMetric(hInstance, MAKEINTRESOURCE(IDI_PLAYMAILER), LIM_LARGE, &hLargeIcon);
	//LoadIconMetric(hInstance, MAKEINTRESOURCE(IDI_PLAYMAILER), LIM_SMALL, &hSmallIcon);
	hLargeIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_PLAYMAILER), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_SHARED);
	hSmallIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_PLAYMAILER), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);

	if(!CreateSendEmailThread() || !CreateRecvEmailThread())
		goto END;

	MyRegisterClass(hInstance);

	if(minimized)
		nCmdShow = SW_SHOWMINNOACTIVE;

	// Perform application initialization:
	if(!InitInstance (hInstance, nCmdShow))
	{
		msg.wParam = 1;
		goto END;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PLAYMAILER));

	ForceSetForegroundWindow(hMainWnd);

	mail = (MailSettings *)malloc(sizeof(MailSettings));
	memset(mail, 0, sizeof(MailSettings));
	
	mail->checkMailInterval = 0;
	mail->inMailProtocol = DEFAULT_IN_MAIL_PROTOCOL;
	mail->inSecurityProtocol = DEFAULT_IN_SECURITY_PROTOCOL;
	mail->outSecurityProtocol = DEFAULT_OUT_SECURITY_PROTOCOL;

	settings = (GeneralSettings *)malloc(sizeof(GeneralSettings));
	memset(settings, 0, sizeof(GeneralSettings));

	settings->DOSBoxPath[0] = L'\0';
	settings->enterSendsMessage = TRUE;
	settings->runOnStartup = TRUE;
	settings->startMinimized = TRUE;
	settings->muteAllChat = FALSE;
	settings->muteSessionWideChat = FALSE;
	settings->windowsTopMost = TOPMOST_ALWAYS;
	settings->disableSounds = FALSE;
	settings->disableChatSound = FALSE;
	settings->disableYourTurnSound = FALSE;

	mailSettingsOK = LoadMailSettings();

	LoadGamePlugins();

	globalGameSettingsOK = LoadGlobalGameSettings();

	LoadPlayerHistory();
	LoadSessionList();

	ValidateGGSettingsListTopLevel();

	if(!mailSettingsOK)
		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hMainWnd, SettingsDialogProc, MAIL_SETTINGS);
	else
		SettingsLoaded = TRUE;

	srand( (unsigned)time( NULL ) ); // Seed random numbers

	wcscpy_s(mbBuffer, MBBUFFER_SIZE, L"Could not register global hot keys: ");
	
	if(WaitForSingleObject(hAppMutex, 0) != WAIT_OBJECT_0)
	{
		HotkeyModifier = DEBUG_HOTKEY_MODIFIER;
		wcscpy_s(hotkeyModString, 1024, DEBUG_HOTKEY_MODIFIER_STRING);
	}
	else
	{
		HotkeyModifier = HOTKEY_MODIFIER;
		wcscpy_s(hotkeyModString, 1024, HOTKEY_MODIFIER_STRING);
	}

	for(int i = 0; i < NUM_HOTKEYS; i++)
	{
		if(!RegisterHotKey(hMainWnd, i, HotkeyModifier, Hotkeys[i]))
		{
			wcscat_s(mbBuffer, MBBUFFER_SIZE, hotkeyModString);
			len = wcslen(mbBuffer);
			mbBuffer[len] = MapVirtualKey(Hotkeys[i], MAPVK_VK_TO_CHAR);
			mbBuffer[len + 1] = L'\0';
			wcscat_s(mbBuffer, MBBUFFER_SIZE, L", ");
			hotkeysOK = FALSE;
		}
	}
#ifdef _DEBUG
	for(int i = 0; i < NUM_DEBUGHOTKEYS; i++)
	{
		if(!RegisterHotKey(hMainWnd, i, HOTKEY_MODIFIER, DebugHotkeys[i]))
		{
			wcscat_s(mbBuffer, MBBUFFER_SIZE, HOTKEY_MODIFIER_STRING);
			len = wcslen(mbBuffer);
			mbBuffer[len] = MapVirtualKey(DebugHotkeys[i], MAPVK_VK_TO_CHAR);
			mbBuffer[len + 1] = L'\0';
			wcscat_s(mbBuffer, MBBUFFER_SIZE, L", ");
			hotkeysOK = FALSE;
		}
	}
#endif
	if(!hotkeysOK)
	{
		mbBuffer[wcslen(mbBuffer) - 2] = L'.';
		MessageBox(hMainWnd, mbBuffer, L"Warning!", MB_OK | MB_ICONEXCLAMATION);
	}

	if(SettingsLoaded) 
		ActivateMailer();
	else
		SetWindowText(hStatusBar, L"Mail Settings not configured.");

	
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{	
		if (!TranslateAccelerator(hMainWnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		ParseEmails();
		CheckYourTurn();
		CheckHotKeys();
	}

	FreeSessionList();
	LL_FreeAll(&llEmailHistory);
	LL_FreeAll(&llNameHistory);

END:
	ShowTaskBar(TRUE);

	if(hSendEmailThread)
		TerminateSendEmailThread();
	if(hRecvEmailThread)
		TerminateRecvEmailThread();

	FreeGamePlugins();
	DeleteCriticalSection(&Crit);
	CoUninitialize();
	FreeLibrary(hRichEditLib);
	ReleaseMutex(hAppMutex);

	return (int) msg.wParam;
}

BOOL EncryptString(TCHAR *decStr, uint8_t *encStr, int *encStrSize)
{
	DATA_BLOB DataIn;
	DATA_BLOB DataOut;
	DATA_BLOB DataEntropy;
	DataIn.pbData = (BYTE *)decStr;    
	DataIn.cbData = (wcslen(decStr) + 1) * sizeof(TCHAR); 
	BOOL ret = FALSE;
	long long entropy = 4266810088317944567LL;

	if(decStr[0] == L'\0')
	{
		*encStrSize = 0;
		return FALSE;
	}

	DataEntropy.pbData = (BYTE *)&entropy;
	DataEntropy.cbData = sizeof(long long);

	if(CryptProtectData(&DataIn, NULL, &DataEntropy, NULL, NULL, 0, &DataOut))
		ret = TRUE;
	
	memcpy(encStr, DataOut.pbData, DataOut.cbData);
	*encStrSize = DataOut.cbData;
	LocalFree(DataOut.pbData);

	return ret;
}

BOOL DecryptString(TCHAR *decStr, uint8_t *encStr, int encStrSize)
{
	DATA_BLOB DataIn;
	DATA_BLOB DataOut;
	DATA_BLOB DataEntropy;
	BOOL ret = FALSE;
	long long entropy = 4266810088317944567LL;

	if(encStrSize == 0)
	{
		decStr[0] = '\0';
		return FALSE;
	}

	DataIn.pbData = encStr;    
	DataIn.cbData = encStrSize; 
	DataEntropy.pbData = (BYTE *)&entropy;
	DataEntropy.cbData = sizeof(long long);

	if(CryptUnprotectData(&DataIn, NULL, &DataEntropy, NULL, NULL, 0, &DataOut))
		ret = TRUE;

	memcpy(decStr, DataOut.pbData, DataOut.cbData);
	LocalFree(DataOut.pbData);

	return ret;
}

/*void WaitUntilWindowDrawn()
{
	MSG msg;
	HACCEL hAccelTable;

	while(PeekMessage(&msg, NULL, 0, 0, TRUE))
	{	
		if (!TranslateAccelerator(hMainWnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}*/

void CreateSchTask(TCHAR *taskName, TCHAR *taskPath, TCHAR *taskArgs, int trigger)
{
	SearchReplace srStrings[] = {	
		{L"<DisallowStartIfOnBatteries>true</DisallowStartIfOnBatteries>", L"    <DisallowStartIfOnBatteries>false</DisallowStartIfOnBatteries>"},
		{L"<StopIfGoingOnBatteries>true</StopIfGoingOnBatteries>", L"    <StopIfGoingOnBatteries>false</StopIfGoingOnBatteries>"}
	};

	TCHAR cmd[3 * MAX_PATH];
	TCHAR triggerOnRun[] = L"/SC ONCE /ST 00:00"; 
	TCHAR triggerOnLogon[] = L"/SC ONLOGON";
	TCHAR *triggerStr;

	if(trigger == SCHTASK_ONRUN)
		triggerStr = triggerOnRun;
	else if(trigger == SCHTASK_ONLOGON)
		triggerStr = triggerOnLogon;

	swprintf(cmd, 3 * MAX_PATH, L"schtasks /Create /TN \"%s\" /TR \"\\\"%s\\\" %s\" %s /RL HIGHEST /F", taskName, taskPath, taskArgs, triggerStr);
	ExecuteCmd(cmd);
	swprintf(cmd, 3 * MAX_PATH, L"cmd /c schtasks /Query /XML /TN \"%s\" >temp.xml", taskName);
	ExecuteCmd(cmd);
	
	ReplaceLinesInFile(L"temp.xml", L"temp2.xml", srStrings, 2);
	
	swprintf(cmd, 3 * MAX_PATH, L"schtasks /Create /XML temp2.xml /TN \"%s\" /F", taskName); 
	ExecuteCmd(cmd);

	DeleteFile(L"temp.xml");
	DeleteFile(L"temp2.xml");
}

/*
void CreateSchTask(TCHAR *taskName, TCHAR *taskPath)
{
	TCHAR args[2 * MAX_PATH], appName[MAX_PATH];
	OSVERSIONINFO versionInfo;

	versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);	
	GetVersionEx(&versionInfo);
	
	if(versionInfo.dwMajorVersion < 6) // Only create scheduled task in Vista and above
		return;

	GetModuleFileName(NULL, appName, MAX_PATH);

	swprintf(args, 2 * MAX_PATH, L"/CREATE_SCHTASK \"%s\" %s", taskName, taskPath);
	ShellExecute(NULL, L"runas", appName, args, 0, SW_HIDE);
}
*/

void DisableWow64Redirection(BOOL disable)
{
	static PVOID OldValue;

	if(pWow64DisableWow64FsRedirection == NULL)
		return;

	if(disable)
		pWow64DisableWow64FsRedirection(&OldValue);
	else
		pWow64RevertWow64FsRedirection(OldValue);
}

TCHAR *GetFirstOutgoingEmail(TCHAR *filePath)
{
	HANDLE hFile;
	WIN32_FIND_DATA fileData;
	uint32_t lowestFileNumber = 0xFFFFFFFF, fileNumber;
	int len;
	
	DisableWow64Redirection(TRUE);

	swprintf(filePath, MAX_PATH, L"%s\\*.eml", OUTGOING_EMAIL_FOLDER);
	if((hFile = FindFirstFile(filePath, &fileData)) == INVALID_HANDLE_VALUE)
	{
		DisableWow64Redirection(FALSE);
		return NULL;
	}  

	do 
	{
		len = wcscspn(fileData.cFileName, L".");
		wcsncpy_s(filePath, MAX_PATH, fileData.cFileName, len);
		filePath[len] = L'\0';
		fileNumber = _wtoi(filePath);
		if(fileNumber < lowestFileNumber)
			lowestFileNumber = fileNumber;
	}
	while(FindNextFile(hFile, &fileData));

	FindClose(hFile);

	swprintf(filePath, MAX_PATH, L"%s\\%u.eml", OUTGOING_EMAIL_FOLDER, lowestFileNumber);

	DisableWow64Redirection(FALSE);
	return filePath;
}

void ActivateMailer()
{
	SetTimer(hMainWnd, CHECK_INTERNET_TIMER, CHECK_INTERNET_INTERVAL, NULL);

	if(IsInternetConnected())
	{
		hFetchMailDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_INPROGRESS), hMainWnd, FetchMailDialogProc);

		SetEvent( hSendEmailEvents[EVENT_SENDEMAIL_CHECK] );
		SetEvent( hRecvEmailEvents[EVENT_RECVEMAIL_CHECK] );
	}
}

void CopySession(SessionInfo *out, SessionInfo *in)
{
	Player **tempPlayers;

	tempPlayers = out->players;
	FreePlayers(out);
	FreeTeams(out);

	in->Clone(out);

	out->players = tempPlayers; 
	CopyPlayers(out, in);

	out->teams = AllocTeams();
	CopyTeams(out, in);

	out->pGameSettings = out->GetGameSettings();
}

void SleepC(DWORD benchMilliSeconds)
{
	DWORD adjustedTime;

	if(ProcSpeed < BENCH_CLOCK)
		adjustedTime = ((BENCH_CLOCK * benchMilliSeconds / ProcSpeed) - benchMilliSeconds) / 2 + benchMilliSeconds;
	else
		adjustedTime = benchMilliSeconds;
	
	Sleep(adjustedTime);
}

uint8_t GetCPULoad()
{
	FILETIME idleTime2, idleTime1;
	FILETIME kernelTime2, kernelTime1;
	FILETIME userTime2, userTime1;
	ULARGE_INTEGER idl, ker, usr, sys;
	uint8_t cpu;

	GetSystemTimes( &idleTime1, &kernelTime1, &userTime1 );
	Sleep(100);
	GetSystemTimes( &idleTime2, &kernelTime2, &userTime2 );

	usr.HighPart = userTime2.dwHighDateTime - userTime1.dwHighDateTime;
	usr.LowPart = userTime2.dwLowDateTime - userTime1.dwLowDateTime;
	ker.HighPart = kernelTime2.dwHighDateTime - kernelTime1.dwHighDateTime;
	ker.LowPart = kernelTime2.dwLowDateTime - kernelTime1.dwLowDateTime;
	idl.HighPart = idleTime2.dwHighDateTime - idleTime1.dwHighDateTime;
	idl.LowPart = idleTime2.dwLowDateTime - idleTime1.dwLowDateTime;
	
	sys.QuadPart = ker.QuadPart + usr.QuadPart;
	cpu = int( (sys.QuadPart - idl.QuadPart) * 100 / sys.QuadPart );

	return cpu;
}

uint64_t GetLoopSpeed()
{
	uint64_t i = 0;
	int64_t partFreq; 
	LARGE_INTEGER counts, freq, start;
	int FREQ_FRACTION = 10;

	QueryPerformanceFrequency(&freq);
	partFreq = freq.QuadPart / FREQ_FRACTION;

	QueryPerformanceCounter(&start);

	for(i = 0;;i++)
	{
		QueryPerformanceCounter(&counts);
		if(counts.QuadPart - start.QuadPart >= partFreq) // Count for a fraction of a second
			break;
	}

	return i * FREQ_FRACTION;
}

DWORD GetProcSpeed()
{
	/*
	DWORD speed1 = (DWORD)BaseProcSpeed * (100 - GetCPULoad()) / 100;
	DWORD speed2 = (DWORD)(GetLoopSpeed() / (BENCH_CPULOAD / BENCH_CLOCK));

	return MIN(speed1, speed2);
	*/

	return (DWORD)BaseProcSpeed * (100 - GetCPULoad()) / 100;
}

DWORD GetBaseProcSpeed()
{
	TCHAR Buffer[_MAX_PATH];
	DWORD BufSize = _MAX_PATH;
	DWORD dwMHz = _MAX_PATH;
	HKEY hKey;

	// open the key where the proc speed is hidden:
	long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey);
    
    if(lError != ERROR_SUCCESS)
	{// if the key is not found, tell the user why:
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, lError, 0, Buffer, _MAX_PATH, 0);
		MessageBox(NULL, Buffer, L"PlayMailer Error", MB_OK);	
		return 0;
    }

    // query the key:
    RegQueryValueEx(hKey, L"~MHz", NULL, NULL, (LPBYTE) &dwMHz, &BufSize);

	return dwMHz;
}

BOOL IsInternetConnected()
{
	DWORD internetFlags;

	InternetConnectedState = InternetGetConnectedState(&internetFlags, 0);

	if(!InternetConnectedState)
		SetWindowText(hStatusBar, L"Internet not connected.");

	return InternetConnectedState;
}

void CheckAllPlayerTeams()
{
	LinkedList *iter;
	SessionInfo *session;
	int sessionNum = 0;

	iter = &llSessions;

	while(iter->next != NULL)
	{
		iter = iter->next;

		session = (SessionInfo *)iter->item;

		if(session->selectingTeams)
		{
			RequestTeamSettings(session);
			CheckNewGame(session);
		}

		sessionNum++;
	}
}

void CentreWindow(HWND hWnd)
{
	RECT rc;
	int screenWidth, screenHeight, windowWidth, windowHeight;

	GetWindowRect(GetDesktopWindow(), &rc);
	screenWidth = rc.right - rc.left;
	screenHeight = rc.bottom - rc.top;
	GetWindowRect(hCurrentDialog, &rc);
	windowWidth = rc.right - rc.left;
	windowHeight = rc.bottom - rc.top;
	SetWindowPos(hCurrentDialog, NULL, screenWidth / 2 - (windowWidth / 2), screenHeight / 2 - (windowHeight / 2), 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	NMLVDISPINFO* plvdi;
	LPNMITEMACTIVATE lpnmitem; 
	SessionInfo *session;
	POINT pt = {};
	static int i = 0;
	int ret;
	RECT rc;
	BOOL tempConnectedState;
	MINMAXINFO* pmmi;

	if(MinimizeToTrayProc(hWnd, message, wParam, lParam))
		return 0;

	switch (message)
	{
	case WM_CREATE:
		hSmallFont = CreateDialogFont(FONT_SMALL, FONT_SIZE_SMALL, FW_NORMAL);
		hDefaultFont = CreateDialogFont(FONT_NORMAL, FONT_SIZE_NORMAL, FW_NORMAL);
		hLargeFont = CreateDialogFont(FONT_LARGE, FONT_SIZE_LARGE, FW_NORMAL);
		hBoldFont = CreateDialogFont(FONT_NORMAL, FONT_SIZE_NORMAL, FW_BOLD);
		hExtraLargeFont = CreateDialogFont(FONT_EXTRALARGE, FONT_SIZE_EXTRALARGE, FW_NORMAL);
		
		hStatusBar = CreateWindowEx ( 
			0, STATUSCLASSNAME, (LPCTSTR) NULL, WS_CHILD | WS_VISIBLE, 
			0, 0, 0, 0, hWnd, (HMENU)IDC_STATUSBAR, hInst, NULL	
		);      
		DefaultStatusBarProc = (WNDPROC)SetWindowLong (hStatusBar, GWL_WNDPROC, (LONG)StatusBarProc);
		SendMessage(hStatusBar, WM_SETFONT, (WPARAM)hLargeFont, (LPARAM)MAKELONG(TRUE, 0));
		GetClientRect(hWnd, &rc);
		SetWindowPos(hStatusBar, NULL, 0, rc.bottom - STATUSBAR_HEIGHT, rc.right, STATUSBAR_HEIGHT, SWP_NOZORDER);
			
		hNewGameButton = CreateWindow ( 
			L"BUTTON", L"New Session", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  
			10, 10,	130, 30, hWnd, (HMENU)IDC_NEWGAME_BUTTON, hInst, NULL	
		);      
		hSettingsButton = CreateWindow ( 
			L"BUTTON", L"Settings", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  
			155, 10, 100, 30, hWnd, (HMENU)IDC_SETTINGS_BUTTON, hInst, NULL	
		);
		hViewHotkeysButton = CreateWindow ( 
			L"BUTTON", L"View Hotkeys", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  
			270, 10, 150, 30, hWnd, (HMENU)IDC_HOTKEYS_BUTTON, hInst, NULL	
		);
		hViewHelpButton = CreateWindow ( 
			L"BUTTON", L"Help", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  
			435, 10, 80, 30, hWnd, (HMENU)IDC_HELP_BUTTON, hInst, NULL	
		);
		hContactAuthorButton = CreateWindow ( 
			L"BUTTON", L"Contact Author", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  
			530, 10, 150, 30, hWnd, (HMENU)IDC_CONTACT_AUTHOR_BUTTON, hInst, NULL	
		);

		SendMessage(hNewGameButton, WM_SETFONT, (WPARAM)hExtraLargeFont, (LPARAM)MAKELONG(TRUE, 0));
		SendMessage(hSettingsButton, WM_SETFONT, (WPARAM)hExtraLargeFont, (LPARAM)MAKELONG(TRUE, 0));
		SendMessage(hViewHotkeysButton, WM_SETFONT, (WPARAM)hExtraLargeFont, (LPARAM)MAKELONG(TRUE, 0));
		SendMessage(hViewHelpButton, WM_SETFONT, (WPARAM)hExtraLargeFont, (LPARAM)MAKELONG(TRUE, 0));
		SendMessage(hContactAuthorButton, WM_SETFONT, (WPARAM)hExtraLargeFont, (LPARAM)MAKELONG(TRUE, 0));

		hMuteAllCheck = CreateWindow ( 
			L"BUTTON", L"Mute all chat messages", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
			820, 4, 186, 20, hWnd, (HMENU)IDC_MUTEALL_CHECK, hInst, NULL	
		);
		hMuteSWCheck = CreateWindow ( 
			L"BUTTON", L"Mute session-wide chat messages", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
			820, 26, 265, 20, hWnd, (HMENU)IDC_MUTESW_CHECK, hInst, NULL	
		);
		SendMessage(hMuteAllCheck, WM_SETFONT, (WPARAM)hLargeFont, (LPARAM)MAKELONG(TRUE, 0));
		SendMessage(hMuteSWCheck, WM_SETFONT, (WPARAM)hLargeFont, (LPARAM)MAKELONG(TRUE, 0));
		
		hMainListView = CreateWindowEx ( WS_EX_CLIENTEDGE,
			WC_LISTVIEW, L"", WS_CLIPCHILDREN | WS_VISIBLE | WS_CHILD | LVS_SHOWSELALWAYS | LVS_REPORT | LVS_NOSORTHEADER, 
			10, 50, MAINLV_WIDTH, MAINLV_HEIGHT, hWnd, (HMENU)IDC_MAIN_LV, hInst, NULL	
		);
		ListView_SetExtendedListViewStyle(hMainListView, LVS_EX_FULLROWSELECT);
		InitMainListView(hMainListView); 

		break;
	case WM_GETMINMAXINFO: 
		DefWindowProc(hWnd, message, wParam, lParam);
		pmmi = (MINMAXINFO*)lParam;
		pmmi->ptMaxTrackSize.x = 2000;
		pmmi->ptMaxTrackSize.y = 2000;
		return 0;
	case WM_TRAYMESSAGE:
		switch (LOWORD(lParam))
		{
			case WM_CONTEXTMENU: // XP and later
			case WM_RBUTTONDOWN: 
				ShowPopMenu(hWnd);
				return 0;
			default:
				break;
        }
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDC_SETTINGS_BUTTON:
		case IDM_SETTINGS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, SettingsDialogProc);
			return 0;
		case IDC_NEWGAME_BUTTON:
		case IDM_NEW_GAME:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SESSION_SETTINGS), hWnd, SessionSettingsDialogProc, -1);
			return 0;
		case IDC_HOTKEYS_BUTTON:
		case IDM_HOTKEYS:
			ViewHotkeysHelp();
			return 0;
		case IDC_HELP_BUTTON:
		case IDM_HELP:
			HtmlHelp(hMainWnd, HelpPath, HH_DISPLAY_TOPIC, (DWORD)L"Quick Start.htm");
			return 0;
		case IDC_CONTACT_AUTHOR_BUTTON:
		case IDM_CONTACT_AUTHOR:
			EditPlainEmail(AUTHOR_NAME, AUTHOR_EMAIL);
			return 0;
		case IDC_MUTEALL_CHECK:
			if(settings->muteAllChat != !!Button_GetCheck(hMuteAllCheck))
			{
				settings->muteAllChat = !!Button_GetCheck(hMuteAllCheck);
				EnableWindow(hMuteSWCheck, !settings->muteAllChat);
				SaveMailSettings();
			}
			return 0;
		case IDC_MUTESW_CHECK:
			if(settings->muteSessionWideChat != !!Button_GetCheck(hMuteSWCheck))
			{
				settings->muteSessionWideChat = !!Button_GetCheck(hMuteSWCheck);
				SaveMailSettings();	
			}
			return 0;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			return 0;
		case IDM_EXIT:
		case IDM_POPUP_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_POPUP_OPEN:
			RestoreWndFromTray(hWnd);
			ShowNotifyIcon(hWnd, FALSE);
			return 0;
		default:
			break;
		}
		break;
	case WM_NOTIFY:
		wmId    = ((LPNMHDR)lParam)->idFrom;
		wmEvent = ((LPNMHDR)lParam)->code;

		if( ((LPNMHDR)lParam)->code == NM_CUSTOMDRAW )
		{
			switch( ((LPNMLVCUSTOMDRAW)lParam)->nmcd.dwDrawStage )
			{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
				{
					LPNMLVCUSTOMDRAW customDraw = (LPNMLVCUSTOMDRAW)lParam;

					int index = customDraw->nmcd.dwItemSpec;
					if(index == selectedSession)
					{
						customDraw->clrTextBk = RGB( 200, 200, 200 ); //COLORREF here.
						customDraw->clrText = RGB( 0, 0, 0 ); //COLORREF here.
					}
					return CDRF_NEWFONT;
				}
				default:
					return CDRF_DODEFAULT;
			}
			break;
		}
		
		switch(wmId) 
		{
		case IDC_MAIN_LV:
			switch(wmEvent) 
			{
			case NM_CLICK:
				lpnmitem = (LPNMITEMACTIVATE) lParam;
				if(lpnmitem->iItem != -1 && lpnmitem->iItem != selectedSession)
				{
					swprintf(mbBuffer, MBBUFFER_SIZE, L"Are you sure you want to change the selected session to \"%s\" ?", ((SessionInfo *)LL_GetItem(&llSessions, lpnmitem->iItem))->sessionName);
					if(selectedSession == -1 || IDOK == MessageBox(hMainWnd, mbBuffer, L"Change selected session?", MB_ICONQUESTION | MB_OKCANCEL))
						selectSession(lpnmitem->iItem);
				}
				break;
			case LVN_COLUMNCLICK:
			case LVN_ITEMCHANGING:
			case LVN_ITEMCHANGED:
				return TRUE;
			case LVN_GETDISPINFO:
				plvdi = (NMLVDISPINFO*)lParam; 
				session = (SessionInfo *)LL_GetItem(&llSessions, plvdi->item.iItem); 
				switch (plvdi->item.iSubItem)
				{	
					case 0:
						plvdi->item.pszText = session->sessionName;
						break;
					case 1:
						plvdi->item.pszText = session->ggSettings->gameID; 
						break;
                    case 2:
						if(session->selectingTeams)
							plvdi->item.pszText = L"Choosing Team Settings  .  .  .";					
						else if(isYourTurn(session) || session->actingCurrentPlayer)	
							swprintf(plvdi->item.pszText, MAX_SETTING + 20, L"YOU  (%s)", getYourPlayerName(session));
						else 
							BuildNameAddressStr(plvdi->item.pszText, getCurrentPlayerName(session), getCurrentPlayerEmail(session));
						break;
					case 3:
				        plvdi->item.pszText = L"";
					    break;
                    default:
						break;
				}
				return 0;
    		default:
				break;
			}
		default:
			break;
		}
	case WM_HOTKEY:
		if(HotKeysEnabled && LOWORD(lParam) == HotkeyModifier)
		{
			WaitHotKeyRelease(lParam);
			HotKeyPressed = HIWORD(lParam);
			return 0;
		}
		break;
	case WM_PARSE_MAIL:
		ParseMailEnabled = TRUE;
		return 0;
	case WM_BRING_TO_FRONT:
		if(!IsWindowVisible(hWnd))
		{
			RestoreWndFromTray(hWnd);
			ShowNotifyIcon(hWnd, FALSE);
		}
		else
			SetForegroundWindow(hWnd);
		break;
	case WM_TIMER:
		switch(wParam) 
		{
		case GAME_IN_PROGRESS_TIMER:
			GameInProgress = FALSE; 
			CheckYourTurnEnabled = TRUE;
			return 0;
		case CHECK_INTERNET_TIMER:
			tempConnectedState = InternetConnectedState;
			if(IsInternetConnected() != tempConnectedState)
			{
				if(InternetConnectedState)
				{
					SetEvent( hRecvEmailEvents[EVENT_RECVEMAIL_CHECK] );
					SetEvent( hSendEmailEvents[EVENT_SENDEMAIL_CHECK] );
				}
				SetTimer(hMainWnd, CHECK_INTERNET_TIMER, CHECK_INTERNET_INTERVAL, NULL);
			}
			return 0;
		case CHECK_TOPMOST_TIMER:
			if(IsGameWindowForeground())
			{
				if(IsGameWindowTopMost())
				{
					KillTimer(hWnd, CHECK_TOPMOST_TIMER);
					SuspendResumeGame(TRUE);
					CentreWindow(hCurrentDialog);
				}
				else
					SetTimer(hWnd, CHECK_TOPMOST_TIMER, CHECK_TOPMOST_INTERVAL, NULL);

				SetWindowPos(hCurrentDialog, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_FRAMECHANGED);
				ForceSetForegroundWindow(hCurrentDialog);
			}

			return 0;
		default:
			break;
		}
		return 0;
	case WM_POWERBROADCAST:
		if(wParam == PBT_APMRESUMEAUTOMATIC)
		{
			SetEvent( hRecvEmailEvents[EVENT_RECVEMAIL_CHECK] );
			SetEvent( hSendEmailEvents[EVENT_SENDEMAIL_CHECK] );
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_CLOSE:
		ret = DialogBox(hInst, MAKEINTRESOURCE(IDD_QUIT), NULL, QuitDialogProc);	
		if(ret == IDCANCEL)
			return 0;
		else if(ret == IDC_MINIMIZE_BUTTON)
		{
			MinimizeWndToTray(hWnd);
			ShowNotifyIcon(hWnd,TRUE);
			return 0;
		}
	case WM_DESTROY:
		for(int i = 0; i < NUM_HOTKEYS; i++)
			UnregisterHotKey(hMainWnd, i);	
		KillTimer(hWnd, GAME_IN_PROGRESS_TIMER);
		DeleteObject(hDefaultFont);
		DeleteObject(hLargeFont);
		DeleteObject(hExtraLargeFont);
		DeleteObject(hBoldFont);
		DeleteObject(hSmallFont);
		
		SetWindowLong (hMainListView, GWL_WNDPROC, (LONG)DefaultLVProc);
		SetWindowLong (hStatusBar, GWL_WNDPROC, (LONG)DefaultStatusBarProc);
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ForceSetForegroundWindow(HWND hWnd)
{
	PressKeyFast(VK_LMENU);
	PressKeyFast(VK_LMENU);
	SetForegroundWindow(hWnd);
}

LRESULT CALLBACK StatusBarProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) 
	{
	case WM_SIZE:
		return 0;
	}

	return CallWindowProc (DefaultStatusBarProc, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK MainListViewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int sIndex, ret;
	SessionInfo *session;
	int wmId, wmEvent;
	
	switch(message) 
	{
	case WM_NOTIFY:
		wmId    = ((LPNMHDR)lParam)->idFrom;
		wmEvent = ((LPNMHDR)lParam)->code;
		switch(wmEvent)
		{
		case HDN_BEGINTRACKW:
		case HDN_BEGINTRACKA: 
			return TRUE;
		}
		break;
	case WM_COMMAND:
		if(HIWORD(wParam) == BN_CLICKED)
		{
			for(int i = 0; i < numSessions; i++)
			{
				sIndex = i + ListView_GetTopIndex(hWnd);
				session = (SessionInfo *)LL_GetItem(&llSessions, sIndex);

				switch(LOWORD(wParam) - i * NUM_LVBUTTONS)
				{
				case LVBUTTON_LOAD:
					LoadGameRequest(session, FALSE);
					goto END;
				case LVBUTTON_SEND:
					SendEmailYourTurn(session);
					goto END;
				case LVBUTTON_PLAYERS:
					ret = DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_PLAYER_LIST), NULL, PlayerListDialogProc, sIndex);	
					if(ret == IDC_SENDMESSAGE_ALL || ret == IDC_SENDMESSAGE_SELECTED)
					{
						EditChatMessage(sIndex, NULL, RecipientsMask, NULL, NULL);
						if(RecipientsMask) 
							free(RecipientsMask);
					}
					goto END;
				case LVBUTTON_SETTINGS:
					DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SESSION_SETTINGS), hWnd, SessionSettingsDialogProc, sIndex);
					goto END;
				case LVBUTTON_DELETE:
					DeleteSession(sIndex);
					goto END;
				default: break;
				}
			}
		}
		break;
	default:
		break;
	}
END:
	return CallWindowProc (DefaultLVProc, hWnd, message, wParam, lParam);
}

INT_PTR CALLBACK QuitDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_COMMAND:
		if(HIWORD(wParam) == BN_CLICKED)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		swprintf(mbBuffer, MBBUFFER_SIZE, L"%s, Version %s", APP_TITLE, APP_VERSION);
		SetDlgItemText(hDlg, IDC_NAME_VERSION_STATIC, mbBuffer);

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

HWND InitDialogNotifySettings()
{
	HWND hDialog;

	hDialog = CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_NOTIFY), hSettingsDialog, NotifySettingsDialogProc, NULL);
	SetWindowPos(hDialog, NULL, DLUToPixelsX(hSettingsDialog, SETTINGS_PAGE_X), 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	SendDlgItemMessage(hDialog, IDC_NOTIFY_HEADER, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));
	SendDlgItemMessage(hDialog, IDC_TOPMOST_GB, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));
	SendDlgItemMessage(hDialog, IDC_MUTE_CHAT_GB, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));
	SendDlgItemMessage(hDialog, IDC_SOUNDS_GB, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));

	hSettingsPages[NOTIFY_SETTINGS] = hDialog;

	if(settings->windowsTopMost == TOPMOST_ALWAYS)
		CheckDlgButton(hDialog, IDC_TOPMOST_ALWAYS_RADIO, BST_CHECKED);
	else
		CheckDlgButton(hDialog, IDC_TOPMOST_GAME_RADIO, BST_CHECKED);

	CheckDlgButton(hDialog, IDC_MUTE_ALL_CHECK, settings->muteAllChat);
	CheckDlgButton(hDialog, IDC_MUTE_SW_CHECK, settings->muteSessionWideChat);
	if(settings->muteAllChat)
		EnableWindow(GetDlgItem(hDialog, IDC_MUTE_SW_CHECK), FALSE);

	CheckDlgButton(hDialog, IDC_DISABLE_SOUNDS_CHECK, settings->disableSounds);
	CheckDlgButton(hDialog, IDC_DISABLE_CHAT_SOUND_CHECK, settings->disableChatSound);
	CheckDlgButton(hDialog, IDC_DISABLE_YOURTURN_SOUND_CHECK, settings->disableYourTurnSound);
	if(settings->disableSounds)
	{
		EnableWindow(GetDlgItem(hDialog, IDC_DISABLE_CHAT_SOUND_CHECK), FALSE);
		EnableWindow(GetDlgItem(hDialog, IDC_DISABLE_YOURTURN_SOUND_CHECK), FALSE);
	}

	return hDialog;
}

HWND InitDialogGGSettings(LinkedList *llGGSettings)
{
	HWND hDialog;
	LinkedList *iter;
	GlobalGameSettings *game;

	hDialog = CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_GAME), hSettingsDialog, GGSettingsDialogProc, (LPARAM)llGGSettings);
	SetWindowPos(hDialog, NULL, DLUToPixelsX(hSettingsDialog, SETTINGS_PAGE_X), 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	SendDlgItemMessage(hDialog, IDC_GAME_HEADER, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));
	
	iter = llGGSettings;
	while(iter->next)
	{
		iter = iter->next;
		game = (GlobalGameSettings *)iter->item;
		swprintf(mbBuffer, MBBUFFER_SIZE, L"%s  (%s)", game->gameID, game->gameDetails);
		SendDlgItemMessage(hDialog, IDC_GAME_SELECTER_COMBO, CB_ADDSTRING, 0, (LPARAM)mbBuffer); 
	}
	
	hSettingsPages[GAME_SETTINGS] = hDialog;

	if(SelectedGGPage != CB_ERR)
	{
		SendDlgItemMessage(hDialog, IDC_GAME_SELECTER_COMBO, CB_SETCURSEL, SelectedGGPage, 0);
		
		CreateGGChildDialog(SelectedGGPage, hDialog);
	}
	else
	{
		SendDlgItemMessage(hDialog, IDC_GAME_SELECTER_COMBO, CB_INSERTSTRING, 0, (LPARAM)L"[-- Select Game --]");
		SendDlgItemMessage(hDialog, IDC_GAME_SELECTER_COMBO, CB_SETCURSEL, 0, 0);
	}

	return hDialog;
}

HWND InitDialogMailSettings(MailSettings *ms)
{
	TCHAR checkMailStr[10];
	HWND hDialog;
	TCHAR decInPassword[MAX_SETTING], decOutPassword[MAX_SETTING];

	hDialog = CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_MAIL), hSettingsDialog, MailSettingsDialogProc, NULL);
	SetWindowPos(hDialog, NULL, DLUToPixelsX(hSettingsDialog, SETTINGS_PAGE_X), 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	DecryptString(decInPassword, ms->inPassword, ms->inPasswordSize);
	DecryptString(decOutPassword, ms->outPassword, ms->outPasswordSize);

	// Make the headers bold font
	SendDlgItemMessage(hDialog, IDC_MAIL_HEADER, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));
	SendDlgItemMessage(hDialog, IDC_GB1, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));
	SendDlgItemMessage(hDialog, IDC_GB2, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));		
	SendDlgItemMessage(hDialog, IDC_NAME_EDIT, EM_SETLIMITTEXT, MAX_SHORTSETTING, 0);

	// Fill mail settings controls with saved values
	SendDlgItemMessage(hDialog, IDC_NAME_EDIT, WM_SETTEXT, 0, (LPARAM)ms->name);
	SendDlgItemMessage(hDialog, IDC_EMAIL_EDIT, WM_SETTEXT, 0, (LPARAM)ms->email);
	SendDlgItemMessage(hDialog, IDC_INSERVER_EDIT, WM_SETTEXT, 0, (LPARAM)ms->inServer);
	SendDlgItemMessage(hDialog, IDC_INSERVER_EDIT, WM_SETTEXT, 0, (LPARAM)ms->inServer);
	SendDlgItemMessage(hDialog, IDC_INLOGIN_EDIT, WM_SETTEXT, 0, (LPARAM)ms->inLogin);
	SendDlgItemMessage(hDialog, IDC_INPASSWORD_EDIT, WM_SETTEXT, 0, (LPARAM)decInPassword);
	SendDlgItemMessage(hDialog, IDC_OUTLOGIN_EDIT, WM_SETTEXT, 0, (LPARAM)ms->outLogin);
	SendDlgItemMessage(hDialog, IDC_OUTPASSWORD_EDIT, WM_SETTEXT, 0, (LPARAM)decOutPassword);
	SendDlgItemMessage(hDialog, IDC_OUTSERVER_EDIT, WM_SETTEXT, 0, (LPARAM)ms->outServer);
		
	swprintf(checkMailStr, 10, L"%d", ms->checkMailInterval);
	SendDlgItemMessage(hDialog, IDC_CHECKMAIL_EDIT, WM_SETTEXT, 0, (LPARAM)checkMailStr);

	if(!wcscmp(ms->inLogin, ms->outLogin) && !wcscmp(decInPassword, decOutPassword))
	{	
		CheckDlgButton(hDialog, IDC_OUTMATCHIN_CHECK, BST_CHECKED);
		Edit_Enable(GetDlgItem(hDialog, IDC_OUTLOGIN_EDIT), FALSE);
		Edit_Enable(GetDlgItem(hDialog, IDC_OUTPASSWORD_EDIT), FALSE);
	}

	for(int i = 0; i < sizeof(mailProtocolNames)/sizeof(TCHAR *); i++)
		SendDlgItemMessage(hDialog, IDC_INPROTOCOL_COMBO, CB_ADDSTRING, 0, (LPARAM)mailProtocolNames[i]); 
	SendDlgItemMessage(hDialog, IDC_INPROTOCOL_COMBO, CB_SETCURSEL, ms->inMailProtocol, 0);

	for(int i = 0; i < sizeof(securityProtocolNames)/sizeof(TCHAR *); i++)
	{
		SendDlgItemMessage(hDialog, IDC_INSECURITY_COMBO, CB_ADDSTRING, 0, (LPARAM)securityProtocolNames[i]); 
		SendDlgItemMessage(hDialog, IDC_OUTSECURITY_COMBO, CB_ADDSTRING, 0, (LPARAM)securityProtocolNames[i]); 
	}
	SendDlgItemMessage(hDialog, IDC_INSECURITY_COMBO, CB_SETCURSEL, ms->inSecurityProtocol, 0);
	SendDlgItemMessage(hDialog, IDC_OUTSECURITY_COMBO, CB_SETCURSEL, ms->outSecurityProtocol, 0);

	hSettingsPages[MAIL_SETTINGS] = hDialog;

	memset(decInPassword, 0, sizeof(TCHAR) * MAX_SETTING);
	memset(decOutPassword, 0, sizeof(TCHAR) * MAX_SETTING);

	return hDialog;
}

HWND InitDialogGeneralSettings()
{
	HWND hDialog;

	hDialog = CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_GENERAL), hSettingsDialog, GeneralSettingsDialogProc, NULL);
	SetWindowPos(hDialog, NULL, DLUToPixelsX(hSettingsDialog, SETTINGS_PAGE_X), 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	SendDlgItemMessage(hDialog, IDC_GENERAL_HEADER, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));

	CheckDlgButton(hDialog, IDC_RUN_ON_STARTUP_CHECK, settings->runOnStartup);
	CheckDlgButton(hDialog, IDC_START_MINIMIZED_CHECK, settings->startMinimized);

	if(!settings->runOnStartup) 
		EnableWindow(GetDlgItem(hDialog, IDC_START_MINIMIZED_CHECK), FALSE);

	hSettingsPages[GENERAL_SETTINGS] = hDialog;

	return hDialog;
}

INT_PTR CALLBACK GGSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{	
	int iItem;
	static LinkedList *llGGSettings;
	GlobalGameSettings *game;
	TCHAR oldRunCommand[MAX_PATH];

	switch(message)
	{
	case WM_INITDIALOG:
		llGGSettings = (LinkedList *)lParam;

		return TRUE;
	case WM_COMMAND:
		switch(HIWORD(wParam)) 
		{	
			case CBN_DROPDOWN:
				if(SelectedGGPage == CB_ERR)
					SendDlgItemMessage(hDialog, IDC_GAME_SELECTER_COMBO, CB_DELETESTRING, 0, 0);
				break;
			case CBN_CLOSEUP:
				if(ComboBox_GetCurSel((HWND)lParam) == CB_ERR)
				{
					SendDlgItemMessage(hDialog, IDC_GAME_SELECTER_COMBO, CB_INSERTSTRING, 0, (LPARAM)L"[-- Select Game --]");
					SendDlgItemMessage(hDialog, IDC_GAME_SELECTER_COMBO, CB_SETCURSEL, 0, 0);
				}
				break;
			case CBN_SELCHANGE:
				iItem = SendDlgItemMessage(hDialog, IDC_GAME_SELECTER_COMBO, CB_GETCURSEL, 0, 0);
				
				if(iItem != CB_ERR && iItem != SelectedGGPage)
				{
					if(SelectedGGPage != CB_ERR)
					{	
						game = (GlobalGameSettings *)LL_GetItem(llGGSettings, SelectedGGPage);
						wcscpy_s(oldRunCommand, MAX_PATH, game->runCommand);
						game->ParseDialogGG();
						if(wcscmp(oldRunCommand, game->runCommand))
							game->runCommandChanged = TRUE;
					}

					SelectedGGPage = iItem;
					
					DestroyWindow(hGGChildDialog);

					CreateGGChildDialog(SelectedGGPage, hDialog);
				}
				break;
			default:
				break;
		}
		break;
	default:
		break;
	}

	return FALSE;
}

HWND CreateGGChildDialog(int gameIndex, HWND hParent)
{
	GlobalGameSettings *game;

	game = (GlobalGameSettings *)LL_GetItem(&llGlobalGameSettings, gameIndex);
	
	hGGChildDialog = game->CreateGGDialog(hParent);
	game->InitDialogGG();
	SetWindowPos(hGGChildDialog, NULL, DLUToPixelsX(hGGChildDialog, GG_SETTINGS_CHILD_PANE_X), DLUToPixelsY(hGGChildDialog, GG_SETTINGS_CHILD_PANE_Y), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	ShowWindow(hGGChildDialog, SW_SHOW);
	
	return hGGChildDialog;
}

INT_PTR CALLBACK GGTopLevelDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static GlobalGameSettings *game;
	GlobalGameSettings *tempGame;

	switch(message)
	{
	case WM_INITDIALOG:
		game = (GlobalGameSettings *)lParam;
		swprintf(mbBuffer, MBBUFFER_SIZE, L"%s Global Settings", game->gameID);
		SetWindowText(hDialog, mbBuffer);
		hGGChildDialog = game->CreateGGDialog(hDialog);
		game->InitDialogGG();
		SetWindowPos(hGGChildDialog, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		ShowWindow(hGGChildDialog, SW_SHOW);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{	
			case IDOK:
				tempGame = game->Clone();
				tempGame->ParseDialogGG();

				if(wcscmp(tempGame->runCommand, game->runCommand))
					tempGame->runCommandChanged = TRUE;

				if(tempGame->ValidateGlobalGameSettings(VALIDATE_GUI, 0))
				{
					tempGame->Clone(game);
					SaveGlobalGameSettings();
					EndDialog(hDialog, TRUE);
				}
				break;
			case IDCANCEL:
				EndDialog(hDialog, FALSE);
				break;
		}
		break;
	case WM_DESTROY:
		break;
	default:
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK MailSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{	
			case IDC_OUTMATCHIN_CHECK:
				Edit_Enable(GetDlgItem(hDialog, IDC_OUTLOGIN_EDIT), !IsDlgButtonChecked(hDialog, IDC_OUTMATCHIN_CHECK));
				Edit_Enable(GetDlgItem(hDialog, IDC_OUTPASSWORD_EDIT), !IsDlgButtonChecked(hDialog, IDC_OUTMATCHIN_CHECK));
				break;
		}
		break;
	default:
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK GeneralSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{	
			case IDC_RUN_ON_STARTUP_CHECK:
				EnableWindow(GetDlgItem(hDialog, IDC_START_MINIMIZED_CHECK), IsDlgButtonChecked(hDialog, IDC_RUN_ON_STARTUP_CHECK));
				break;
		}
		break;
	default:
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK NotifySettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{	
			case IDC_MUTE_ALL_CHECK:
				EnableWindow(GetDlgItem(hDialog, IDC_MUTE_SW_CHECK), !IsDlgButtonChecked(hDialog, IDC_MUTE_ALL_CHECK));
				break;
			case IDC_DISABLE_SOUNDS_CHECK:
				EnableWindow(GetDlgItem(hDialog, IDC_DISABLE_CHAT_SOUND_CHECK), !IsDlgButtonChecked(hDialog, IDC_DISABLE_SOUNDS_CHECK));
				EnableWindow(GetDlgItem(hDialog, IDC_DISABLE_YOURTURN_SOUND_CHECK), !IsDlgButtonChecked(hDialog, IDC_DISABLE_SOUNDS_CHECK));
				break;
		}
		break;
	default:
		break;
	}

	return FALSE;
}

BOOL ParseDialogGGSettings(LinkedList *llGGSettings)
{
	HWND hDialog = hSettingsPages[GAME_SETTINGS];
	GlobalGameSettings *game;
	TCHAR oldRunCommand[MAX_PATH];

	if(SelectedGGPage != CB_ERR)
	{
		game = (GlobalGameSettings *)LL_GetItem(llGGSettings, SelectedGGPage);
		wcscpy_s(oldRunCommand, MAX_PATH, game->runCommand);
		game->ParseDialogGG();
		if(wcscmp(oldRunCommand, game->runCommand))
			game->runCommandChanged = TRUE;
	}

	if(!ValidateGGSettingsList(llGGSettings))
		return FALSE;

	return TRUE;	
}

BOOL ValidateGGSettingsListTopLevel()
{
	BOOL ret = TRUE;
	GlobalGameSettings *gg;
	LinkedList *iter;

	iter = &llGlobalGameSettings;

	while(iter->next)
	{
		iter = iter->next;

		gg = (GlobalGameSettings *)iter->item;

		if(!FindSessionByGameID(gg->gameID))
			continue;

		if(!gg->ValidateGlobalGameSettings(VALIDATE_FILE, 0))
		{	
			if(IDCANCEL == DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_GG_TOPLEVEL), hMainWnd, GGTopLevelDialogProc, (LPARAM)gg))
				ret = FALSE;
		}
		else
			SaveGlobalGameSettings();
	}

	return ret;
}

BOOL ValidateGGSettingsList(LinkedList *llGGSettings)
{
	GlobalGameSettings *gg;
	LinkedList *iter;
	HWND hGameSettingsDialog;
	int index;

	iter = llGGSettings;
	hGameSettingsDialog = hSettingsPages[GAME_SETTINGS];

	while(iter->next)
	{
		iter = iter->next;

		gg = (GlobalGameSettings *)iter->item;

		if(!FindSessionByGameID(gg->gameID))
			continue;

		if(!gg->ValidateGlobalGameSettings(VALIDATE_GUI, 0))
		{	
			index = LL_GetItemIndex(llGGSettings, gg);

			if(SelectedGGPage != index)
			{
				if(SelectedGGPage != CB_ERR)
					DestroyWindow(hGGChildDialog);
		
				SelectedGGPage = index;

				CreateGGChildDialog(SelectedGGPage, hGameSettingsDialog);
			}
		
			return FALSE;
		}
	}

	return TRUE;
}

int FindGGIndexByID(LinkedList *llGGSettings, TCHAR *gameID)
{
	LinkedList *iter;
	GlobalGameSettings *gg;
	int index = 0;

	iter = llGGSettings;
	
	while(iter->next)
	{
		iter = iter->next;
		gg = (GlobalGameSettings *)iter->item;

		if(!_wcsicmp(gg->gameID, gameID))
			return index;

		index++;
	}

	return -1;
}

BOOL ParseDialogGeneralSettings()
{
	HWND hDialog = hSettingsPages[GENERAL_SETTINGS];

	settings->runOnStartup = IsDlgButtonChecked(hDialog, IDC_RUN_ON_STARTUP_CHECK);
	settings->startMinimized = IsDlgButtonChecked(hDialog, IDC_START_MINIMIZED_CHECK);

	return TRUE;
}

BOOL ParseDialogNotifySettings()
{
	HWND hDialog = hSettingsPages[NOTIFY_SETTINGS];

	if(IsDlgButtonChecked(hDialog, IDC_TOPMOST_ALWAYS_RADIO))
		settings->windowsTopMost = TOPMOST_ALWAYS;
	else
		settings->windowsTopMost = TOPMOST_GAME_ACTIVE;

	settings->muteAllChat = IsDlgButtonChecked(hDialog, IDC_MUTE_ALL_CHECK);
	settings->muteSessionWideChat = IsDlgButtonChecked(hDialog, IDC_MUTE_SW_CHECK);
	Button_SetCheck(hMuteAllCheck, settings->muteAllChat);
	Button_SetCheck(hMuteSWCheck, settings->muteSessionWideChat);
	EnableWindow(hMuteSWCheck, !settings->muteAllChat); 		

	settings->disableSounds = IsDlgButtonChecked(hDialog, IDC_DISABLE_SOUNDS_CHECK);
	settings->disableChatSound = IsDlgButtonChecked(hDialog, IDC_DISABLE_CHAT_SOUND_CHECK);
	settings->disableYourTurnSound = IsDlgButtonChecked(hDialog, IDC_DISABLE_YOURTURN_SOUND_CHECK);

	return TRUE;
}

int ChangeSettingsPage(int settingsPage)
{
	for(int i = 0; i < NUM_SETTINGS_PAGES; i++)
		ShowWindow(hSettingsPages[i], SW_HIDE);

	ShowWindow(hSettingsPages[settingsPage], SW_SHOW);
	
	SetFocus(GetDlgItem(hSettingsDialog, IDC_PAGELIST_TV));

	return settingsPage;
}

INT_PTR CALLBACK SettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int settingsPage = GENERAL_SETTINGS;
	BOOL settingsValid = TRUE;
	static MailSettings tempMailSettings;
	static LinkedList llTempGGSettings;
	TVINSERTSTRUCT tvInsertItem;
	int wmEvent, wmId;
	LPNMTREEVIEW pnmtv;

	switch (message)
	{
	case WM_INITDIALOG:
		hSettingsDialog = hDialog;
		
		tvInsertItem.hParent = NULL;
		tvInsertItem.hInsertAfter = TVI_LAST;
		tvInsertItem.item.mask = TVIF_TEXT;

		for(int i = 0; i < NUM_SETTINGS_PAGES; i++)
		{
			tvInsertItem.item.pszText = settingsPageNames[i];
			hSettingsTreeItems[i] = (HTREEITEM)SendDlgItemMessage(hDialog, IDC_PAGELIST_TV, TVM_INSERTITEM, 0, (LPARAM)&tvInsertItem);
		}

		memcpy(&tempMailSettings, mail, sizeof(MailSettings)); 

		CopyGGSettingsList(&llTempGGSettings, &llGlobalGameSettings);

		InitDialogMailSettings(&tempMailSettings);
		InitDialogGGSettings(&llTempGGSettings);
		InitDialogGeneralSettings();
		InitDialogNotifySettings();

		if(lParam)
			settingsPage = lParam;
		
		SendDlgItemMessage(hSettingsDialog, IDC_PAGELIST_TV, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hSettingsTreeItems[settingsPage]);
		settingsPage = ChangeSettingsPage(settingsPage);

		if(_waccess(MAIL_CONFIG_FILE, 0))
			HtmlHelp(NULL, HelpPath, HH_DISPLAY_TOPIC, (DWORD)L"Quick Start.htm");
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{
		case IDOK:
			if(!(settingsValid &= ParseDialogMailSettings(&tempMailSettings)))
				ChangeSettingsPage(MAIL_SETTINGS);
			else if(!(settingsValid &= ParseDialogGGSettings(&llTempGGSettings)))
				ChangeSettingsPage(GAME_SETTINGS);
			else if(!(settingsValid &= ParseDialogGeneralSettings()))
				ChangeSettingsPage(GENERAL_SETTINGS);
			else if(!(settingsValid &= ParseDialogNotifySettings()))
				ChangeSettingsPage(NOTIFY_SETTINGS);

			if(settingsValid)
			{
				UpdateSettings(&tempMailSettings, &llTempGGSettings);
				EndDialog(hDialog, LOWORD(wParam));
			}
			break;
		case IDCANCEL:
			EndDialog(hDialog, LOWORD(wParam));
			break;
		default:
			break;
		}
		break;
	case WM_NOTIFY:
		wmId    = ((LPNMHDR)lParam)->idFrom;
		wmEvent = ((LPNMHDR)lParam)->code;
		switch(wmEvent)
		{
		case TVN_SELCHANGED:
			pnmtv = (LPNMTREEVIEW)lParam;
			for(int i = 0; i < NUM_SETTINGS_PAGES; i++)
			{
				if(pnmtv->itemNew.hItem == hSettingsTreeItems[i])
				{
					if(i != settingsPage)
						settingsPage = ChangeSettingsPage(i);
					
					break;
				}
			}
			break;
		}
		break;
	case WM_DESTROY:
		LL_FreeAll(&llTempGGSettings);
		EndDialog(hDialog, IDCANCEL);
		break;
	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;
}

void CopyGGSettingsList(LinkedList *out, LinkedList *in)
{
	GlobalGameSettings *gg;
	LinkedList *iter;

	iter = in;

	while(iter->next)
	{	
		iter = iter->next;

		gg = (GlobalGameSettings *)malloc(sizeof(GlobalGameSettings));
		memcpy(gg, (GlobalGameSettings *)iter->item, sizeof(GlobalGameSettings));
		LL_Add(out, gg);
	}
}

INT_PTR CALLBACK SessionSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SessionInfo *session, *tempSession;
	static int sessionIndex = -1, selectedGameIndex = CB_ERR;
	static TCHAR *selectedGameID = NULL;
	int iItem;
	BOOL saveSession = FALSE;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR playerName[MAX_SETTING];
	GlobalGameSettings *game;
	LinkedList *iter;

	switch (message)
	{
	case WM_INITDIALOG:	
		SetTopMost(hDialog);
		EnableWindow(hMainWnd, FALSE);

		selectedGameIndex = CB_ERR;
		selectedGameID = NULL;

		sessionIndex = lParam;
		tempSession = NULL;

		hSessionSettingsDialog = hDialog;
		
		if(sessionIndex != -1)
			session = (SessionInfo *)LL_GetItem(&llSessions, sessionIndex);
		else 
		{
			session = NULL;
			IsNewSession = TRUE;
		}

		if(session)
		{
			selectedGameIndex = LL_GetItemIndex(&llGlobalGameSettings, session->ggSettings);
			SendDlgItemMessage(hDialog, IDC_SELECTGAME_COMBO, CB_SETCURSEL, selectedGameIndex, 0);
			selectedGameID = ((GlobalGameSettings *)LL_GetItem(&llGlobalGameSettings, selectedGameIndex))->gameID;

			SendDlgItemMessage(hDialog, IDC_SELECTEDGAME_STATIC, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));
			SetDlgItemText(hDialog, IDC_SELECTEDGAME_STATIC, session->ggSettings->gameID);
			MoveDlgItemRelative(hDialog, IDC_SESSIONTYPE_STATIC, 0, 2);
			MoveDlgItemRelative(hDialog, IDC_SELECTEDGAME_STATIC, 0, 2);
			ShowWindow(GetDlgItem(hDialog, IDC_SELECTEDGAME_STATIC), SW_SHOW);		
		}
		else
		{
			iter = &llGlobalGameSettings;
			while(iter->next)
			{
				iter = iter->next;
				game = (GlobalGameSettings *)iter->item;
				swprintf(mbBuffer, MBBUFFER_SIZE, L"%s  (%s)", game->gameID, game->gameDetails);
				SendDlgItemMessage(hDialog, IDC_SELECTGAME_COMBO, CB_ADDSTRING, 0, (LPARAM)mbBuffer); 
			}

			SendDlgItemMessage(hDialog, IDC_SELECTGAME_COMBO, CB_INSERTSTRING, 0, (LPARAM)L"[-- Select Game --]");
			SendDlgItemMessage(hDialog, IDC_SELECTGAME_COMBO, CB_SETCURSEL, 0, 0);
		
			ShowWindow(GetDlgItem(hDialog, IDC_SELECTGAME_COMBO), SW_SHOW);
		}
		
		if(session)
		{
			SendDlgItemMessage(hDialog, IDC_GAMENAME_EDIT, WM_SETTEXT, 0, (LPARAM)session->sessionName);

			if(isYourTurn(session) && !IsNewSession) 
				SendDlgItemMessage(hDialog, IDC_PLAYERNAME_EDIT, WM_SETTEXT, 0, (LPARAM)getYourPlayerName(session));
			
			tempSession = AllocSession(session->pGameSettings->gameID);
			CopySession(tempSession, session);

			tempSession->InitGameSettingsDialog();
		}
	
		if(sessionIndex == -1 || !isYourTurn(session) || IsNewSession)
		{
			SetWindowPosDialogRelative(hDialog, hDialog, NULL, 0, 0, 0, -PLAYERNAME_EDIT_HEIGHT, SWP_NOMOVE | SWP_NOZORDER);
			ShowWindow(GetDlgItem(hDialog, IDC_PLAYERNAME_EDIT), SW_HIDE);
			ShowWindow(GetDlgItem(hDialog, IDC_PLAYERNAME_STATIC), SW_HIDE);
			ShowWindow(GetDlgItem(hDialog, IDC_PLAYERHELP_STATIC), SW_HIDE);

			MoveDlgItemRelative(hDialog, IDC_DIVIDER_STATIC, 0, -PLAYERNAME_EDIT_HEIGHT);
			MoveDlgItemRelative(hDialog, IDC_EDITPLAYERS_BUTTON, 0, -PLAYERNAME_EDIT_HEIGHT);
			MoveDlgItemRelative(hDialog, IDC_EDITPLAYERS_HELP_STATIC, 0, -PLAYERNAME_EDIT_HEIGHT);
			MoveDlgItemRelative(hDialog, IDOK, 0, -PLAYERNAME_EDIT_HEIGHT);
			MoveDlgItemRelative(hDialog, IDCANCEL, 0, -PLAYERNAME_EDIT_HEIGHT);
			
			if(selectedGameIndex != CB_ERR)
				SetWindowPosDialogRelative(hSessionSettingsDialog, hGameSettingsDialog, NULL, 0, -PLAYERNAME_EDIT_HEIGHT, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}

		if(selectedGameIndex != CB_ERR)
			ShowWindow(hGameSettingsDialog, SW_SHOW);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{
		case IDC_SELECTGAME_COMBO:
			switch(HIWORD(wParam))
			{
				case CBN_DROPDOWN:
					if(selectedGameIndex == CB_ERR)
						SendDlgItemMessage(hDialog, IDC_SELECTGAME_COMBO, CB_DELETESTRING, 0, 0);
					break;
				case CBN_CLOSEUP:
					if(ComboBox_GetCurSel((HWND)lParam) == CB_ERR)
					{
						SendDlgItemMessage(hDialog, IDC_SELECTGAME_COMBO, CB_INSERTSTRING, 0, (LPARAM)L"[-- Select Game --]");
						SendDlgItemMessage(hDialog, IDC_SELECTGAME_COMBO, CB_SETCURSEL, 0, 0);
					}
					break;
				case CBN_SELCHANGE:
					iItem = SendDlgItemMessage(hDialog, IDC_SELECTGAME_COMBO, CB_GETCURSEL, 0, 0);
			
					if(iItem != CB_ERR && iItem != selectedGameIndex)
					{
						if(selectedGameIndex != CB_ERR)
						{
							if(tempSession)
							{
								FreeSession(tempSession);
								tempSession = NULL;
							}

							DestroyWindow(hGameSettingsDialog);
						}

						selectedGameIndex = iItem;
						selectedGameID = ((GlobalGameSettings *)LL_GetItem(&llGlobalGameSettings, selectedGameIndex))->gameID;
						
						if(!tempSession)
							tempSession = AllocSession(selectedGameID);

						tempSession->InitGameSettingsDialog();
						SetWindowPosDialogRelative(hSessionSettingsDialog, hGameSettingsDialog, NULL, 0, -PLAYERNAME_EDIT_HEIGHT, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
						ShowWindow(hGameSettingsDialog, SW_SHOW);
					}
					break;
			}
			break;
		case IDC_EDITPLAYERS_BUTTON:
			if(selectedGameIndex == CB_ERR)
			{
				MessageBox(hDialog, L"First select a Session Type.", L"No session type selected", MB_OK | MB_ICONSTOP); 
				break;
			}

			if(!IsNewSession && isYourTurn(tempSession))
			{
				SendDlgItemMessage(hDialog, IDC_PLAYERNAME_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)playerName);
				trimWhiteSpace(playerName);
				ChangeYourPlayerName(tempSession, playerName);
				AddHistoryString(&llNameHistory, playerName, TRUE);
			}
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EDIT_PLAYERS), hDialog, EditPlayersDialogProc, (LPARAM)tempSession);
			if(!IsNewSession && isYourTurn(tempSession))
				SendDlgItemMessage(hDialog, IDC_PLAYERNAME_EDIT, WM_SETTEXT, 0, (LPARAM)getYourPlayerName(tempSession));
	
			else Edit_Enable(GetDlgItem(hDialog, IDC_PLAYERNAME_EDIT), FALSE);
			break;
		case IDOK:
			if(selectedGameIndex == CB_ERR)
			{
				MessageBox(hDialog, L"First select a Session Type.", L"No session type selected", MB_OK | MB_ICONSTOP); 
				break;
			}
			else if(!tempSession || tempSession->numPlayers == 0)
			{
				MessageBox(hDialog, L"Go to 'Add / Edit Players' and add some players.", L"Not enough players", MB_OK | MB_ICONSTOP);
				break;
			}

			if(sessionIndex == -1) 
			{
				if(session = CreateSession(tempSession))
					saveSession = TRUE;
			}
			else
			{
				if(UpdateSession(tempSession))
					saveSession = TRUE;
			}
			if(saveSession)
			{
				SyncPlayerLists(session, tempSession, FALSE);
				ApplyGlobalNamesToSession(tempSession, NULL, NULL);
				if(!IsNewSession && isYourTurn(tempSession))
				{
					SendDlgItemMessage(hDialog, IDC_PLAYERNAME_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)playerName);
					trimWhiteSpace(playerName);
					ChangeYourPlayerName(tempSession, playerName);
					AddHistoryString(&llNameHistory, playerName, TRUE);
				}

				session->CheckGameSettingsChanged(tempSession);

				CopySession(session, tempSession);
				
				CheckPlayerListChanged(session);
			
				ListView_Update(hMainListView, sessionIndex);
				
				CheckYourTurnEnabled = TRUE;

				if(!session->ggSettings->ValidateGlobalGameSettings(VALIDATE_QUIET, 0))
					DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_GG_TOPLEVEL), hSessionSettingsDialog, GGTopLevelDialogProc, (LPARAM)session->ggSettings);

				EnableWindow(hMainWnd, TRUE);
				EndDialog(hDialog, LOWORD(wParam));

				if(NewCurrentPlayer != -1)
					SendEmailYourTurn(session);
				else
					CheckNewGame(session);
				
				SaveSessionList();
			}
			break; 
		case IDCANCEL:
			if(session)
			{
				if(!session->ggSettings->ValidateGlobalGameSettings(VALIDATE_QUIET, 0))
					DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_GG_TOPLEVEL), hSessionSettingsDialog, GGTopLevelDialogProc, (LPARAM)session->ggSettings);
			}

			EnableWindow(hMainWnd, TRUE);
			EndDialog(hDialog, LOWORD(wParam));

			if(session)
			{
				CheckPlayerListChanged(session);
				CheckYourTurnEnabled = TRUE;

				CheckNewGame(session);
			
				SaveSessionList();
			}
			break;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		FreeSession(tempSession);
		EnableWindow(hMainWnd, TRUE);
		EndDialog(hDialog, IDCANCEL);
		break;
	case WM_PAINT:	
		hdc = BeginPaint(hDialog, &ps);

		EndPaint(hDialog, &ps);
		break;
	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;
}

void MoveDlgItemRelative(HWND hDlg, int nIDDlgItem, int xOffset, int yOffset)
{
	SetWindowPosDialogRelative(hDlg, GetDlgItem(hDlg, nIDDlgItem), NULL, xOffset, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void MoveWindowRelative(HWND hWnd, int xOffset, int yOffset)
{
	WINDOWPLACEMENT wndpl;

	GetWindowPlacement(hWnd, &wndpl);
	wndpl.rcNormalPosition.left += xOffset;
	wndpl.rcNormalPosition.top += yOffset;
	SetWindowPlacement(hWnd, &wndpl);
}

void SetWindowPosDialogRelative(HWND hDialog, HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	RECT rc;
	int width, height;

	GetWindowRect(hWnd, &rc);
	height = rc.bottom - rc.top;
	width = rc.right - rc.left;

	rc.left = X;
	rc.top = Y;
	rc.right = cx;
	rc.bottom = cy;
	MapDialogRect(hDialog, &rc);

	SetWindowPos(hWnd, NULL, GetWindowX(hWnd) + rc.left, GetWindowY(hWnd) + rc.top, width + rc.right, height + rc.bottom, uFlags);
}

int GetLargestFactionSize(SessionInfo *session)
{
	int largestNumPlayers, *factionTable, factionTableSize;

	factionTableSize = sizeof(int) * session->NUM_FACTIONS;
	factionTable = (int *)malloc(factionTableSize);

	memset(factionTable, 0, factionTableSize);

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->faction >= 0)
			factionTable[session->players[i]->faction]++;
	}

	// Count the number of equally smallest teams
	largestNumPlayers = 0;
	for(int j = 0; j < session->NUM_FACTIONS; j++)
	{
		if(factionTable[j] > largestNumPlayers)
			largestNumPlayers = factionTable[j];
	}

	free(factionTable);

	return largestNumPlayers;
}

INT_PTR CALLBACK EditPlayersDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SessionInfo *session = NULL, *oldSession;
	NMLVDISPINFO* plvdi;
	int playerNum;
	static int selectedPlayer;
	DWORD wmId, wmEvent;
	LPNMLISTVIEW pnmv;
	LPNMITEMACTIVATE lpnmitem;
						
	switch (message)
	{
		case WM_INITDIALOG:
			selectedPlayer = -1;

			hEditPlayersDialog = hDialog;

			oldSession = (SessionInfo *)lParam;
			
			session = AllocSession(oldSession->pGameSettings->gameID);
			CopySession(session, oldSession);

			hEditPlayersLV = GetDlgItem(hDialog, IDC_EDITPLAYERS_LV);
			ListView_SetExtendedListViewStyle(hEditPlayersLV, LVS_EX_FULLROWSELECT);
			SetWindowLong(hEditPlayersLV, GWL_STYLE, GetWindowLong(hEditPlayersLV, GWL_STYLE) | WS_CLIPCHILDREN);
			
			InitEditPlayersListView(hEditPlayersLV); 
			AddPlayerListToLV(hEditPlayersLV, session);
			InitPlayerEditControls(session);

			swprintf(mbBuffer, MBBUFFER_SIZE, L"Maximum number of players:  %d", session->MAX_TEAMS);
			SetDlgItemText(hDialog, IDC_MAX_PLAYERS_STATIC, mbBuffer);
			
			if(session->numPlayers > session->MAX_TEAMS || (!session->ggSettings->duplicateFactions && GetLargestFactionSize(session) > 1))
				session->enableRelayTeams = TRUE;
			
			if(session->enableRelayTeams)
			{
				swprintf(mbBuffer, MBBUFFER_SIZE, L"Maximum number of teams:  %d", session->MAX_TEAMS);
				SetDlgItemText(hDialog, IDC_MAX_PLAYERS_STATIC, mbBuffer);
				CheckDlgButton(hDialog, IDC_RELAY_TEAMS_CHECK, BST_CHECKED);
			}

			SetTopMost(hDialog);
			EnableWindow(hMainWnd, FALSE);
			break;
		case WM_KEYUP:
			if(GetFocus() == hPlayerNameEdit || GetFocus() == hPlayerEmailEdit)
			{
				SetFocus(hEditPlayersLV);
				ParsePlayerEditControls(session, selectedPlayer);
				return TRUE;
			}
			break;
		case WM_NOTIFY:
			wmId    = ((LPNMHDR)lParam)->idFrom;
			wmEvent = ((LPNMHDR)lParam)->code;

			switch(wmEvent)
			{
				case LVN_ITEMCHANGED:
					pnmv = (LPNMLISTVIEW) lParam; 
					if(pnmv->uChanged & LVIF_STATE)
					{
						if(!(pnmv->uOldState & LVIS_SELECTED) && pnmv->uNewState & LVIS_SELECTED)
						{	
							selectedPlayer = pnmv->iItem;
							ShowPlayerEditControls(session, pnmv->iItem);
						}
						else if(pnmv->uOldState & LVIS_SELECTED && !(pnmv->uNewState & LVIS_SELECTED))
						{
							ParsePlayerEditControls(session, pnmv->iItem);
							ShowPlayerEditControls(session, -1);
							if(ListView_GetNextItem(hEditPlayersLV, -1, LVNI_SELECTED) == -1)
								ListView_SetItemState(hEditPlayersLV, pnmv->iItem, 0, LVIS_FOCUSED);
						}
					}
					break;
				case NM_CLICK:
					lpnmitem = (LPNMITEMACTIVATE) lParam;
					if(lpnmitem->iItem != -1)
					{
						switch(lpnmitem->iSubItem)
						{
							case 1:
								if(IsWindowVisible(hPlayerEmailComboBox))
									SetFocus(hPlayerEmailComboBox);
								break;
							case 2:
								if(IsWindowVisible(hPlayerNameComboBox))
									SetFocus(hPlayerNameComboBox);
								break;
							case 3:
								if(IsWindowVisible(hFactionComboBox))
									SetFocus(hFactionComboBox);
								break;
							default:
								break;
						}
					}
					break;
				case LVN_GETDISPINFO:
					plvdi = (NMLVDISPINFO*)lParam; 
					playerNum = plvdi->item.iItem; 
					switch (plvdi->item.iSubItem)
					{	
						case 0:
							swprintf(plvdi->item.pszText, MAX_SETTING, L"%u", playerNum + 1); 
							break;
						case 1:
							plvdi->item.pszText = session->players[playerNum]->email; 
							break;
						case 2:
							plvdi->item.pszText = session->players[playerNum]->name;
							break;
						case 3:
							if(session->players[playerNum]->faction == FACTION_RANDOM)
								plvdi->item.pszText = L"Random";
							else if(session->players[playerNum]->faction == FACTION_PLAYERS_CHOICE)
								plvdi->item.pszText = L"Player's Choice";
							else
								plvdi->item.pszText = session->GetFactionNames()[session->players[playerNum]->faction];
							break;
						default:
							break;
					}
				default:
					break;
			}
			break;
		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				switch(LOWORD(wParam)) 
				{
					case IDC_ADDPLAYER_BUTTON:
						if(AddPlayer(session, NULL, NULL, -1))
							SetFocus(hPlayerEmailComboBox);
						break;
					case IDC_ADDYOU_BUTTON:
						if(AddPlayer(session, mail->email, NULL, -1))
							SetFocus(hPlayerNameComboBox);		
						break;
					case IDC_DELETE_BUTTON:
						DeletePlayerFromLV(session, selectedPlayer);
						break;
					case IDC_RELAY_TEAMS_CHECK:
						if(BST_CHECKED == IsDlgButtonChecked(hDialog, IDC_RELAY_TEAMS_CHECK))
						{
							session->enableRelayTeams = TRUE;
							swprintf(mbBuffer, MBBUFFER_SIZE, L"Maximum number of teams:  %d", session->MAX_TEAMS);
						}
						else
						{
							session->enableRelayTeams = FALSE;
							swprintf(mbBuffer, MBBUFFER_SIZE, L"Maximum number of players:  %d", session->MAX_TEAMS);
						}
						SetDlgItemText(hDialog, IDC_MAX_PLAYERS_STATIC, mbBuffer);
						break;
					case IDC_OK_BUTTON:
						ParsePlayerEditControls(session, ListView_GetNextItem(hEditPlayersLV, -1, LVNI_SELECTED));
						if(!ValidatePlayerList(session, VALIDATE_GUI))
							break;
						SortPlayerList(session);
						AssignTeams(session);
						CopySession(oldSession, session);
					case IDCANCEL:
						EnableWindow(hMainWnd, TRUE);
						EndDialog(hDialog, LOWORD(wParam));
						break;
					default:
						break;
				}
			}
			break;
		case WM_DESTROY:
			if(session) FreeSession(session);
			SetWindowLong (hPlayerEmailEdit, GWL_WNDPROC, (LONG)DefaultEditProc);
			SetWindowLong (hPlayerNameEdit, GWL_WNDPROC, (LONG)DefaultEditProc);
			SetWindowLong (hFactionComboBox, GWL_WNDPROC, (LONG)DefaultComboProc);
			SetWindowLong (hEditPlayersLV, GWL_WNDPROC, (LONG)DefaultLVProc);
			EnableWindow(hMainWnd, TRUE);
			EndDialog(hDialog, IDCANCEL);
			break;
		default:
			return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;		
}

LRESULT CALLBACK PlayerListLVProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DWORD wmId, wmEvent;
	
	if(message == WM_NOTIFY)
	{
		wmId    = ((LPNMHDR)lParam)->idFrom;
		wmEvent = ((LPNMHDR)lParam)->code;
		switch(wmEvent)
		{
		case HDN_BEGINTRACKW:
		case HDN_BEGINTRACKA: 
			return TRUE;
		}
	}

	return CallWindowProc(DefaultLVProc, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK EditPlayersListViewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int currSel;
	DWORD wmId, wmEvent;

	if(message == WM_KEYUP)
	{
		if(wParam == VK_RETURN && ListView_GetNextItem(hEditPlayersLV, -1, LVNI_SELECTED) != -1)
		{
			if(IsWindowVisible(hPlayerEmailComboBox))
				SetFocus(hPlayerEmailComboBox);
			else if(IsWindowVisible(hPlayerNameComboBox))
				SetFocus(hPlayerNameComboBox);
			else 
				SetFocus(hFactionComboBox);
		}
	}

	else if(message == WM_COMMAND)
	{
		if( ( HIWORD(wParam) == CBN_SELENDCANCEL
			|| HIWORD(wParam) == CBN_SELENDOK ) 
			&& (HWND)lParam == hFactionComboBox )
		{
			if((currSel = SendMessage(hFactionComboBox, CB_GETCURSEL, 0, 0)) == BaseFaction - 1)
				ComboBox_SetCurSel(hFactionComboBox, LastFactionSelected);
			else
				LastFactionSelected = currSel;
		}		
	}

	else if(message == WM_NOTIFY)
	{
		wmId    = ((LPNMHDR)lParam)->idFrom;
		wmEvent = ((LPNMHDR)lParam)->code;
		switch(wmEvent)
		{
		case HDN_BEGINTRACKW:
		case HDN_BEGINTRACKA: 
			return TRUE;
		}
	}

	return CallWindowProc(DefaultLVProc, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK EditPlayersEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int lastCurrSel = -1;
	int currSel;

	switch(message)
	{
	case WM_GETDLGCODE:
		if(wParam == VK_TAB)
			return DLGC_WANTTAB;
		break;
	case WM_KEYUP:
		if(wParam == VK_RETURN)
		{
			if(hWnd == hFactionComboBox)
				SetFocus(GetParent(hFactionComboBox));
			else
				SendMessage(GetParent(hEditPlayersLV), message, wParam, lParam);
		}
		break;
	case WM_KEYDOWN:
		if(hWnd == hFactionComboBox) 
		{
			if( (LastFactionSelected == BaseFaction && wParam == VK_UP) 
					|| (LastFactionSelected == BaseFaction - 2 && wParam == VK_DOWN) )
				ComboBox_SetCurSel(hFactionComboBox, BaseFaction - 1);
		}
		break;
	case WM_CHAR:
		if(wParam == 0x09)
		{
			if(hWnd == hFactionComboBox) 
				SetFocus(GetParent(hFactionComboBox));
			else
				SetFocus(GetNextDlgTabItem(GetParent(GetParent(hWnd)), GetParent(hWnd), FALSE));
			return 0;
		}
		break;
	case WM_COMMAND:
		if(HIWORD(wParam) == LBN_SELCHANGE && hWnd == hFactionComboBox)
		{
			currSel = SendMessage(hFactionComboBox, CB_GETCURSEL, 0, 0);

			if(currSel == BaseFaction - 1) 
			{
				ComboBox_GetLBText(hFactionComboBox, LastFactionSelected, mbBuffer);
				ComboBox_SetText(hFactionComboBox, mbBuffer);
				return 0;
			}
		}
		break;
	default:
		break;
	}	
	
	if(hWnd == hFactionComboBox)
		return CallWindowProc(DefaultComboProc, hWnd, message, wParam, lParam);
	return CallWindowProc(DefaultEditProc, hWnd, message, wParam, lParam);
}

INT_PTR CALLBACK NewPlayerFactionDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SessionInfo *session;
	static int *factionTable, factionTableSize;
	int selected, longestFactionLength, smallestFactionSize;
	TCHAR playerFactionNames[1024], playerList[1024];
	static HFONT hFixedFont;

	switch (message)
	{
		case WM_INITDIALOG:
			session = (SessionInfo *)lParam;

			factionTableSize = sizeof(int) * session->NUM_FACTIONS;
			factionTable = (int *)malloc(factionTableSize);

			hFixedFont = CreateFixedFont();
			SendDlgItemMessage(hDialog, IDC_TEAM_COMBO, WM_SETFONT, (WPARAM)hFixedFont, (LPARAM)MAKELONG(TRUE, 0));

			swprintf(mbBuffer, MBBUFFER_SIZE, L"Choose Team - Session: %s", session->sessionName);
			SetWindowText(hDialog, mbBuffer);
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Choose a Team for %s (Player %d)", getYourPlayerName(session), YourPlayerNumber);
			SetDlgItemText(hDialog, IDC_TEXT_STATIC, mbBuffer);

			memset(factionTable, 0, factionTableSize);

			for(int i = 0; i < session->numPlayers; i++)
			{
				if(session->players[i]->faction != -1)
					factionTable[session->players[i]->faction]++;
			}
			
			SendDlgItemMessage(hDialog, IDC_TEAM_COMBO, CB_ADDSTRING, 0, (LPARAM)L"Random");
			
			longestFactionLength = 0;
			for(int i = 0; i < session->NUM_FACTIONS; i++)
			{
				if((int)wcslen(session->GetFactionNames()[i]) > longestFactionLength)
					longestFactionLength = wcslen(session->GetFactionNames()[i]);
			}

			for(int i = 0; i < session->NUM_FACTIONS; i++)	
			{
				if(factionTable[i] == 0)
					wcscpy_s(playerList, 1024, L"<free>");
				else 
				{
					BuildFactionPlayerList(session, i, playerFactionNames);
					
					if(factionTable[i] == 1)
						swprintf(playerList, 1024, L"%s", playerFactionNames);
					else
						swprintf(playerList, 1024, L"(%d players) %s", factionTable[i], playerFactionNames);
				}
				
				swprintf(mbBuffer, MBBUFFER_SIZE, L"%-*s   : %s", longestFactionLength, session->GetFactionNames()[i], playerList);
				SendDlgItemMessage(hDialog, IDC_TEAM_COMBO, CB_ADDSTRING, 0, (LPARAM)mbBuffer);
			}	
			
			SendDlgItemMessage(hDialog, IDC_TEAM_COMBO, CB_SETCURSEL, 0, 0);

			SetTopMost(hDialog);
			EnableWindow(hMainWnd, FALSE);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDOK:
				case IDCANCEL:
					selected = SendDlgItemMessage(hDialog, IDC_TEAM_COMBO, CB_GETCURSEL, 0, 0) - 1;
					if(selected == -1)
					{
						session->players[session->currentPlayer]->faction = AssignRandomFaction(session);
					}
					else 
					{
						smallestFactionSize = GetSmallestFactionSize(session);
						if((!session->ggSettings->duplicateFactions || session->MAX_TEAMS < session->numPlayers) && factionTable[selected] > smallestFactionSize)
						{
							swprintf(mbBuffer, MBBUFFER_SIZE, L"Are you sure you want to join a %d player team? There are %s available.", factionTable[selected] + 1, (smallestFactionSize == 0 ? L"<free> teams" : L"smaller teams"));
							if(IDNO == MessageBox(hDialog, mbBuffer, L"Confirm Team Selection", MB_YESNO))
								break;
						}

						session->players[session->currentPlayer]->faction = selected;
					}
					
					AssignTeam(session, session->currentPlayer);

					if(!session->ggSettings->duplicateFactions && factionTable[selected] > 0)
						session->enableRelayTeams = TRUE;

					session->players[session->currentPlayer]->state |= PLAYER_MODIFY_FACTION;
					EnableWindow(hMainWnd, TRUE);
					EndDialog(hDialog, LOWORD(wParam));
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			free(factionTable);

			EnableWindow(hMainWnd, TRUE);
			EndDialog(hDialog, IDCANCEL);
			DeleteObject(hFixedFont);
			break;
		default:
			return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;		
}

int GetFactionSize(SessionInfo *session, int faction)
{
	int factionSize = 0;

	if(faction < 0) return 0;

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->faction == faction)
			factionSize++;
	}

	return factionSize;
}

int GetNumFactions(SessionInfo *session)
{
	int numFactions = 0;
	int factionTableSize;
	BOOL *factionTable;

	factionTableSize = sizeof(BOOL) * session->NUM_FACTIONS;
	factionTable = (BOOL *)malloc(factionTableSize);
	memset(factionTable, 0, factionTableSize);

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->faction >= 0 && !factionTable[session->players[i]->faction])
		{
			numFactions++;
			factionTable[session->players[i]->faction] = TRUE;
		}
	}
	
	free(factionTable);

	return numFactions;
}

int GetSmallestFactionSize(SessionInfo *session)
{
	int smallestNumPlayers, *factionTable, factionTableSize;

	factionTableSize = sizeof(int) * session->NUM_FACTIONS;
	factionTable = (int *)malloc(factionTableSize);

	memset(factionTable, 0, factionTableSize);

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->faction >= 0)
			factionTable[session->players[i]->faction]++;
	}

	// Count the number of equally smallest teams
	smallestNumPlayers = INT_MAX;
	for(int j = 0; j < session->NUM_FACTIONS; j++)
	{
		if(factionTable[j] < smallestNumPlayers)
			smallestNumPlayers = factionTable[j];
	}

	free(factionTable);

	return smallestNumPlayers;
}

int GetLargesstFactionSize(SessionInfo *session)
{
	int largestNumPlayers, *factionTable, factionTableSize;

	factionTableSize = sizeof(int) * session->NUM_FACTIONS;
	factionTable = (int *)malloc(factionTableSize);

	memset(factionTable, 0, sizeof(factionTable));

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->faction >= 0)
			factionTable[session->players[i]->faction]++;
	}

	// Count the number of equally smallest factions
	largestNumPlayers = 0;
	for(int j = 0; j < session->NUM_FACTIONS; j++)
	{
		if(factionTable[j] > largestNumPlayers)
			largestNumPlayers = factionTable[j];
	}

	free(factionTable);

	return largestNumPlayers;
}

TCHAR *BuildTeamList(SessionInfo *session, int team, TCHAR *playerNames)
{
	TCHAR nameAddr[MAX_EMAIL_FIELD];

	playerNames[0] = L'\0';

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->team == team)
		{
			BuildNameAddressStr(nameAddr, session->players[i]->name, session->players[i]->email);
			wcscat_s(playerNames, 1024, nameAddr);
			wcscat_s(playerNames, 1024, L", "); 
		}
	}
	playerNames[wcslen(playerNames) - 2] = L'\0';

	return playerNames;
}

TCHAR *BuildFactionPlayerList(SessionInfo *session, int faction, TCHAR *playerNames)
{
	TCHAR nameAddr[MAX_EMAIL_FIELD];

	playerNames[0] = L'\0';

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->faction == faction)
		{
			BuildNameAddressStr(nameAddr, session->players[i]->name, session->players[i]->email);
			wcscat_s(playerNames, 1024, nameAddr);
			wcscat_s(playerNames, 1024, L", "); 
		}
	}
	playerNames[wcslen(playerNames) - 2] = L'\0';

	return playerNames;
}

INT_PTR CALLBACK NewPlayerNameDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SessionInfo *session;
	TCHAR tempPlayerName[MAX_SETTING];

	switch (message)
	{
		case WM_INITDIALOG:
			session = (SessionInfo *)lParam;

			swprintf(mbBuffer, MBBUFFER_SIZE, L"New Player (%s) - Session: %s", (getCurrentPlayerFaction(session) == -1 ? L"Team unassigned" : getCurrentPlayerFactionName(session)), session->sessionName);
			SetWindowText(hDialog, mbBuffer);
			SetDlgItemText(hDialog, IDC_PLAYERNAME_EDIT, getYourPlayerName(session));
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Enter a name for Player %d (%s)", YourPlayerNumber, (getCurrentPlayerFaction(session) == -1 ? L"Team unassigned" : getCurrentPlayerFactionName(session)));
			SetDlgItemText(hDialog, IDC_TEXT_STATIC, mbBuffer);

			SetTopMost(hDialog);
			EnableWindow(hMainWnd, FALSE);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDOK:
				case IDCANCEL:
					GetDlgItemText(hDialog, IDC_PLAYERNAME_EDIT, tempPlayerName, MAX_SETTING);
					if(tempPlayerName[0] == L'\0')
						wcscpy_s(tempPlayerName, MAX_SETTING, mail->name);

					if(wcscmp(tempPlayerName, getYourPlayerName(session)))
					{
						session->state |= SESSION_MODIFY_PLAYERLIST;
						wcscpy_s(getCurrentPlayerName(session), MAX_SETTING, tempPlayerName);
						session->players[session->currentPlayer]->state |= PLAYER_MODIFY_NAME;
					}
					
					EnableWindow(hMainWnd, TRUE);
					EndDialog(hDialog, LOWORD(wParam));
				break;
			default:
				break;
			}
			break;
		case WM_DESTROY:
			EnableWindow(hMainWnd, TRUE);
			EndDialog(hDialog, IDCANCEL);
			break;
		default:
			return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;		
}

INT_PTR CALLBACK GameTipDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SessionInfo *session;

	switch (message)
	{
	case WM_INITDIALOG:
		session = (SessionInfo *)lParam;

		SetDlgItemText(hDlg, IDC_TEXT_STATIC, mbBuffer);
		
		SetTopMost(hDlg);
		EnableWindow(hMainWnd, FALSE);
		break;
	case WM_COMMAND:
		if(LOWORD(wParam) != IDC_DONTSHOW_CHECK)
		{
			if(IsDlgButtonChecked(hDlg, IDC_DONTSHOW_CHECK))
			{
				session->ggSettings->displayGameTip = !IsDlgButtonChecked(hDlg, IDC_DONTSHOW_CHECK);
				SaveGlobalGameSettings();
			}
			EnableWindow(hMainWnd, TRUE);
			EndDialog(hDlg, LOWORD(wParam));
		}
		break;
	case WM_DESTROY:
		EnableWindow(hMainWnd, TRUE);
		EndDialog(hDlg, IDCANCEL);
		break;
	default:
		return(INT_PTR)FALSE;
	}

	return (INT_PTR)TRUE;
}

INT_PTR CALLBACK LoadGameDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SessionInfo *session;

	switch (message)
	{
	case WM_INITDIALOG:
		session = (SessionInfo *)lParam;

		if(LoadGameYourTurn)
		{
			SetWindowText(hDlg, L"Your Turn!");
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%s (%s), it is your turn in \'%s\' !", getCurrentPlayerName(session), getCurrentPlayerFactionName(session), session->sessionName);
		}
		else
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Please choose one of the following options.");
		
		SetDlgItemText(hDlg, IDC_TEXT_STATIC, mbBuffer);
		
		if(session->ggSettings->fullScreen) CheckDlgButton(hDlg, IDC_FULLSCREEN_CHECK, BST_CHECKED);
		
		if(!session->ggSettings->fullScreenToggle) 
			ShowWindow(GetDlgItem(hDlg, IDC_FULLSCREEN_CHECK), SW_HIDE);

		SetTopMost(hDlg);
		EnableWindow(hMainWnd, FALSE);
		break;
	case WM_COMMAND:
		if(LOWORD(wParam) != IDC_FULLSCREEN_CHECK)
		{
			if(IsDlgButtonChecked(hDlg, IDC_FULLSCREEN_CHECK) != session->ggSettings->fullScreen)
			{
				session->ggSettings->fullScreen = IsDlgButtonChecked(hDlg, IDC_FULLSCREEN_CHECK);
				SaveGlobalGameSettings();
			}
			EnableWindow(hMainWnd, TRUE);
			EndDialog(hDlg, LOWORD(wParam));
		}
		break;
	case WM_DESTROY:
		EnableWindow(hMainWnd, TRUE);
		EndDialog(hDlg, IDCANCEL);
		break;
	default:
		return(INT_PTR)FALSE;
	}

	return (INT_PTR)TRUE;
}

INT_PTR CALLBACK RunGameDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SessionInfo *session;

	switch (message)
	{
	case WM_INITDIALOG:
		session = (SessionInfo *)lParam;

		if(LoadGameYourTurn)
		{	
			SetWindowText(hDlg, L"Your turn in new game!");
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%s (%s), it is your turn in new game \'%s.\'\n\nYou have the very first turn !\n\nWould you like to load %s now?\n(Warning: all currently running copies of %s will be closed first.)", getCurrentPlayerName(session), getCurrentPlayerFactionName(session), session->sessionName, session->pGameSettings->gameID, session->pGameSettings->gameID);
		}
		else
		{
			SetWindowText(hDlg, L"Load new game");
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%s (%s), you have the very first turn in game \'%s\'. Would you like to load %s now?\n\n(Warning: all currently running copies of %s will be closed first.)", getCurrentPlayerName(session), getCurrentPlayerFactionName(session), session->sessionName, session->pGameSettings->gameID, session->pGameSettings->gameID);
		}
		SetDlgItemText(hDlg, IDC_TEXT_STATIC, mbBuffer);
		
		if(session->ggSettings->fullScreen) 
			CheckDlgButton(hDlg, IDC_FULLSCREEN_CHECK, BST_CHECKED);
		
		if(!session->ggSettings->fullScreenToggle) 
			ShowWindow(GetDlgItem(hDlg, IDC_FULLSCREEN_CHECK), SW_HIDE);

		SetTopMost(hDlg);
		EnableWindow(hMainWnd, FALSE);
		break;
	case WM_COMMAND:
		if(LOWORD(wParam) != IDC_FULLSCREEN_CHECK)
		{
			if(IsDlgButtonChecked(hDlg, IDC_FULLSCREEN_CHECK) != session->ggSettings->fullScreen)
			{
				session->ggSettings->fullScreen = IsDlgButtonChecked(hDlg, IDC_FULLSCREEN_CHECK);
				SaveGlobalGameSettings();
			}
			EnableWindow(hMainWnd, TRUE);
			EndDialog(hDlg, LOWORD(wParam));
		}
		break;
	case WM_DESTROY:
		EnableWindow(hMainWnd, TRUE);
		EndDialog(hDlg, IDCANCEL);
		break;
	default:
		return(INT_PTR)FALSE;
	}

	return (INT_PTR)TRUE;
}

INT_PTR CALLBACK SendStatusUpdateDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{ 
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowText(hDlg, L"Sending Email");
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Sending player list update to other players . . .");
		SetDlgItemText(hDlg, IDC_INPROGRESS_STATIC, mbBuffer);
		return (INT_PTR)TRUE;
	default:
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK FetchMailDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{ 
	switch (message)
	{
	case WM_INITDIALOG:
		if(IsWindowVisible(hMainWnd))
			ShowWindow(hDlg, SW_SHOW);

		ShowWindow(GetDlgItem(hDlg, IDOK), SW_HIDE);
		SetWindowText(hDlg, L"Checking Email");
		SetDlgItemText(hDlg, IDC_INPROGRESS_STATIC, L"Checking email . . .");
	
		return (INT_PTR)TRUE;
	default:
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK SendEmailDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{ 
	TCHAR buffer[MAX_EMAIL_FIELD + 30];

	switch (message)
	{
	case WM_INITDIALOG:
		hSendEmailDialog = hDlg;
		SetWindowText(hDlg, L"Sending Email");
		swprintf(buffer, sizeof(buffer), L"Sending turn notice to %s . . .", (TCHAR *)lParam);
		SetDlgItemText(hDlg, IDC_INPROGRESS_STATIC, buffer);

		SetTimer(hDlg, SENDEMAIL_DIALOG_TIMER, SENDEMAIL_DIALOG_INTERVAL, NULL);
		
		SetTopMost(hDlg);
		EnableWindow(hMainWnd, FALSE);
		ShowWindow(hDlg, SW_SHOW);
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{
			case IDCANCEL:
			case IDOK:
				EnableWindow(hMainWnd, TRUE);
				EndDialog(hDlg, LOWORD(wParam));
				break;
		}
		break;
	case WM_TIMER:
		if(wParam == SENDEMAIL_DIALOG_TIMER)
		{
			EnableWindow(hMainWnd, TRUE);
			EndDialog(hDlg, LOWORD(wParam));
		}
		break;
	case WM_DESTROY:
		KillTimer(hDlg, SENDEMAIL_DIALOG_TIMER);
		EnableWindow(hMainWnd, TRUE);
		EndDialog(hDlg, IDCANCEL);
		hSendEmailDialog = NULL;
		break;
	default:
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK EditPlainEmailDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hEditBox;
	static PlainEmail *email;

	switch (message)
	{
		case WM_INITDIALOG:
			email = (PlainEmail *)lParam;
			
			SetWindowText(hDialog, L"Thanks for your feedback !");
			SendDlgItemMessage(hDialog, IDC_RECIPIENT_STATIC, WM_SETTEXT, 0, (LPARAM)email->to); 
			SendDlgItemMessage(hDialog, IDC_TO_STATIC, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));
			SendDlgItemMessage(hDialog, IDC_SUBJECT_STATIC, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));
			
			return (INT_PTR)TRUE;		
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDC_SEND_BUTTON:
					hEditBox = GetDlgItem(hDialog, IDC_MESSAGE_EDIT);
					email->message = (TCHAR *)malloc((Edit_GetTextLength(hEditBox) + 1) * sizeof(TCHAR));
					Edit_GetText(hEditBox, email->message, Edit_GetTextLength(hEditBox) + 1);					
					Edit_GetText(GetDlgItem(hDialog, IDC_SUBJECT_EDIT), email->subject, MAX_EMAIL_FIELD);
				case IDCANCEL:
					EndDialog(hDialog, LOWORD(wParam));
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;		
}


INT_PTR CALLBACK EditMessageDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hEditBox;
	static ChatMessage *chatMsg;

	switch (message)
	{
		case WM_INITDIALOG:
			chatMsg = (ChatMessage *)lParam;
			
			if(chatMsg->broadcast)
			{
				swprintf(mbBuffer, MBBUFFER_SIZE, L"ALL (%s)", chatMsg->sessionName);
				SendDlgItemMessage(hDialog, IDC_RECIPIENTS_EDIT, WM_SETTEXT, 0, (LPARAM)mbBuffer); 
			}
			else	
				SendDlgItemMessage(hDialog, IDC_RECIPIENTS_EDIT, WM_SETTEXT, 0, (LPARAM)chatMsg->to); 

			SendDlgItemMessage(hDialog, IDC_TO_EDIT, WM_SETTEXT, 0, (LPARAM)L"To:"); 
			SendDlgItemMessage(hDialog, IDC_TO_EDIT, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));

			if(settings->enterSendsMessage) 
			{
				CheckDlgButton(hDialog, IDC_RETURN_SENDS_MESSAGE_CHECK, BST_CHECKED);
				
				hEditBox = GetDlgItem(hDialog, IDC_MESSAGE_EDIT);
				SetWindowLong(hEditBox, GWL_STYLE, GetWindowLong(hEditBox, GWL_STYLE) & ~ES_WANTRETURN);
			}

			SetTopMost(hDialog);
			EnableWindow(hMainWnd, FALSE);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDC_RETURN_SENDS_MESSAGE_CHECK:
					hEditBox = GetDlgItem(hDialog, IDC_MESSAGE_EDIT);
					SetWindowLong(hEditBox, GWL_STYLE, GetWindowLong(hEditBox, GWL_STYLE) ^ ES_WANTRETURN);
					SetFocus(hEditBox);
					break;
				case IDC_SEND_BUTTON:
					hEditBox = GetDlgItem(hDialog, IDC_MESSAGE_EDIT);
					chatMsg->message = (TCHAR *)malloc((Edit_GetTextLength(hEditBox) + 1) * sizeof(TCHAR));
					Edit_GetText(hEditBox, chatMsg->message, Edit_GetTextLength(hEditBox) + 1);					
				case IDCANCEL:
					if(IsDlgButtonChecked(hDialog, IDC_RETURN_SENDS_MESSAGE_CHECK) != settings->enterSendsMessage)
					{
						settings->enterSendsMessage = IsDlgButtonChecked(hDialog, IDC_RETURN_SENDS_MESSAGE_CHECK);
						SaveMailSettings();
					}

					EnableWindow(hMainWnd, TRUE);
					EndDialog(hDialog, LOWORD(wParam));
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			EnableWindow(hMainWnd, TRUE);
			EndDialog(hDialog, IDCANCEL);
			break;
		default:
			return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;		
}

INT_PTR CALLBACK ReceiveMessageDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rc;
	static HBRUSH bkBrush;
	static COLORREF bkColor = RGB(200, 200, 200);
	static int headerHeight, headerWidth;
	ChatMessage *chatMsg;
	TCHAR *recipients;
	HWND hButton;
	
	switch (message)
	{
		case WM_INITDIALOG:
			chatMsg = (ChatMessage *)lParam;

			swprintf(mbBuffer, MBBUFFER_SIZE, L"PlayMailer - Message Received - %s", chatMsg->sessionName);
			SetWindowText(hDialog, mbBuffer);

			SendDlgItemMessage(hDialog, IDC_SENDER_EDIT, WM_SETTEXT, 0, (LPARAM)chatMsg->from);
			
			if(chatMsg->broadcast) 
			{
				swprintf(mbBuffer, MBBUFFER_SIZE, L"ALL (%s)", chatMsg->sessionName);
				recipients = mbBuffer;

				ShowWindow(GetDlgItem(hDialog, IDC_REPLYALL_BUTTON), SW_SHOW);
			
				NumSessionWideChatEmails--;
			}		
			else
			{
				if(wcschr(chatMsg->to, L','))
					ShowWindow(GetDlgItem(hDialog, IDC_REPLYREC_BUTTON), SW_SHOW);
				else
				{
					hButton = GetDlgItem(hDialog, IDC_REPLYSENDER_BUTTON);
					Button_SetStyle(hButton, GetWindowLong(hButton, GWL_STYLE) | BS_DEFPUSHBUTTON, TRUE);
				}
				
				recipients = chatMsg->to;

				NumPrivateChatEmails--;
			}

			if(NumSessionWideChatEmails == 0)
				ShowWindow(GetDlgItem(hDialog, IDC_SKIPSESSIONWIDE_BUTTON), SW_HIDE);

			if(NumSessionWideChatEmails + NumPrivateChatEmails == 0)
			{
				ShowWindow(GetDlgItem(hDialog, IDC_SKIPALL_BUTTON), SW_HIDE);
				Button_SetText(GetDlgItem(hDialog, IDCANCEL), L"Close");
			}

			SendDlgItemMessage(hDialog, IDC_RECIPIENTS_EDIT, WM_SETTEXT, 0, (LPARAM)recipients);
			SendDlgItemMessage(hDialog, IDC_MESSAGE_EDIT, WM_SETTEXT, 0, (LPARAM)chatMsg->message);

			SendDlgItemMessage(hDialog, IDC_FROM_EDIT, WM_SETTEXT, 0, (LPARAM)L"From:");
			SendDlgItemMessage(hDialog, IDC_FROM_EDIT, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));
			SendDlgItemMessage(hDialog, IDC_FROM_EDIT, EM_SETBKGNDCOLOR, FALSE, bkColor);
			
			SendDlgItemMessage(hDialog, IDC_TO_EDIT, WM_SETTEXT, 0, (LPARAM)L"To:");
			SendDlgItemMessage(hDialog, IDC_TO_EDIT, WM_SETFONT, (WPARAM)hBoldFont, (LPARAM)MAKELONG(TRUE, 0));
			SendDlgItemMessage(hDialog, IDC_TO_EDIT, EM_SETBKGNDCOLOR, FALSE, bkColor);

			SendDlgItemMessage(hDialog, IDC_RECIPIENTS_EDIT, EM_SETBKGNDCOLOR, FALSE, bkColor);
			SendDlgItemMessage(hDialog, IDC_MESSAGE_EDIT, EM_SETBKGNDCOLOR, FALSE, GetSysColor(COLOR_BTNFACE));
		
			bkBrush = CreateSolidBrush(bkColor); 

			GetClientRect(GetDlgItem(hDialog, IDC_DIVIDER_STATIC), &rc);
			MapWindowPoints(GetDlgItem(hDialog, IDC_DIVIDER_STATIC), hDialog, (LPPOINT)&rc, 2); 
			headerHeight = rc.bottom; 
			GetClientRect(hDialog, &rc);
			headerWidth = rc.right;
			
			SetTopMost(hDialog);
			EnableWindow(hMainWnd, FALSE);
			
			return (INT_PTR)TRUE;	
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDC_SKIPALL_BUTTON:
					NumPrivateChatEmails = 0;	
				case IDC_SKIPSESSIONWIDE_BUTTON:
					NumSessionWideChatEmails = 0;						
				case IDC_REPLYALL_BUTTON:	
				case IDC_REPLYREC_BUTTON:
				case IDC_REPLYSENDER_BUTTON:
				case IDCANCEL:
					EnableWindow(hMainWnd, TRUE);
					EndDialog(hDialog, LOWORD(wParam));
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			DeleteObject(bkBrush);
			EnableWindow(hMainWnd, TRUE);
			EndDialog(hDialog, IDCANCEL);
			break;
		case WM_PAINT:
			hdc = BeginPaint(hDialog, &ps);
			SetRect(&rc, 0, 0, headerWidth, headerHeight); 
			FillRect(hdc, &rc, bkBrush);
			EndPaint(hDialog, &ps);
			break;
		case WM_CTLCOLORSTATIC:
			if(GetWindowLong((HWND)lParam, GWL_EXSTYLE) & WS_EX_TRANSPARENT) 
			{
				SetBkColor((HDC)wParam, bkColor);
				return (INT_PTR)bkBrush;
			}
			break;
		default:
			break;
	}	
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK PlayerListDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SessionInfo *session, *tempSession;
	static int sIndex, selectedPlayer = -1;
	BOOL messageSent = FALSE;
	static BOOL *playerMask = NULL;
	NMLVDISPINFO* plvdi;
	DWORD wmId, wmEvent;
	int playerNum;
	LPNMLISTVIEW pnmv;
	LPNMLVCUSTOMDRAW customDraw;
	RECT rc, arrowRC;
	HDC arrowDC, lvDC;
	HBITMAP arrowBMP, origArrowBMP;
	BOOL found;

	switch (message)
	{
		case WM_INITDIALOG:
			SetTopMost(hDialog);
			EnableWindow(hMainWnd, FALSE);

			sIndex = (int)lParam;
			session = (SessionInfo *)LL_GetItem(&llSessions, lParam);

			swprintf(mbBuffer, MBBUFFER_SIZE, L"Player List - %s", session->sessionName);
			SetWindowText(hDialog, mbBuffer);
			
			tempSession = AllocSession(session->pGameSettings->gameID);
			
			hPlayerListLV = GetDlgItem(hDialog, IDC_PLAYERLIST_LV);
			ListView_SetExtendedListViewStyle(hPlayerListLV, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_SUBITEMIMAGES);
			SetWindowLong(hPlayerListLV, GWL_STYLE, GetWindowLong(hPlayerListLV, GWL_STYLE) | WS_CLIPCHILDREN);

			playerMask = (BOOL *)malloc(sizeof(BOOL) * session->numPlayers);
			memset(playerMask, FALSE, sizeof(BOOL) * session->numPlayers);
			
			EnableWindow(GetDlgItem(hDialog, IDC_SENDMESSAGE_ALL), FALSE);

			InitPlayerListLV(hPlayerListLV); 
			AddPlayerListToLV(hPlayerListLV, session);
				
			EnableWindow(GetDlgItem(hDialog, IDC_SENDMESSAGE_SELECTED), FALSE);
			break;
		case WM_NOTIFY:
			wmId    = ((LPNMHDR)lParam)->idFrom;
			wmEvent = ((LPNMHDR)lParam)->code;

			switch(wmEvent)
			{
				case NM_CUSTOMDRAW:
					switch( ((LPNMLVCUSTOMDRAW)lParam)->nmcd.dwDrawStage )
					{
						case CDDS_PREPAINT:
							SetWindowLong(hDialog, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
							return CDRF_NOTIFYITEMDRAW;
						case CDDS_ITEMPREPAINT:
							SetWindowLong(hDialog, DWL_MSGRESULT, CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT);
							CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
						case CDDS_ITEMPOSTPAINT:
						{
							customDraw = (LPNMLVCUSTOMDRAW)lParam;

							playerNum = customDraw->nmcd.dwItemSpec;
							if(playerNum == session->currentPlayer)
							{
								ListView_GetSubItemRect(hPlayerListLV, playerNum, 1, LVIR_BOUNDS, &rc);	
								lvDC = customDraw->nmcd.hdc;

								arrowDC = CreateCompatibleDC(lvDC);
								arrowBMP = CreateCompatibleBitmap(lvDC, rc.right - rc.left, rc.bottom - rc.top);
								origArrowBMP = (HBITMAP)SelectObject(arrowDC, arrowBMP);
								SetRect(&arrowRC, 0, 0, rc.right - rc.left, rc.bottom - rc.top);
								DrawFrameControl(arrowDC, &arrowRC, DFC_MENU, DFCS_MENUARROW);
								TransparentBlt(lvDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, arrowDC, arrowRC.left, arrowRC.top, arrowRC.right, arrowRC.bottom, RGB(255, 255, 255));	
								SelectObject(arrowDC, origArrowBMP);		
								DeleteDC(arrowDC);
								DeleteObject(arrowBMP);
							}
							break;
						}
						default:
							return CDRF_DODEFAULT;
					}
					break;
				case LVN_ITEMCHANGED: 
					// Hide checkboxes for local players.
					pnmv = (LPNMLISTVIEW) lParam; 
					if(pnmv->iSubItem == 0)
					{
						if(selectedPlayer != -1) playerMask[selectedPlayer] = FALSE;
						
						if(isYourPlayer(session, pnmv->iItem))
						{
							ListView_SetItemState(hPlayerListLV, pnmv->iItem, INDEXTOSTATEIMAGEMASK(0), LVIS_STATEIMAGEMASK);
						}
						else
						{
							playerMask[pnmv->iItem] = ListView_GetCheckState(hPlayerListLV, pnmv->iItem);
							EnableWindow(GetDlgItem(hDialog, IDC_SENDMESSAGE_ALL), TRUE);
						}
						found = FALSE;
						selectedPlayer = -1;
						for(int i = 0; i < session->numPlayers; i++)
						{
							if(playerMask[i])
							{
								found = TRUE;
								break;
							}
						}

						if(!found && (selectedPlayer = ListView_GetNextItem(hPlayerListLV, -1, LVNI_SELECTED)) != -1
								&& !isYourPlayer(session, selectedPlayer))
						{
							found = TRUE;
							playerMask[selectedPlayer] = TRUE;	
						}
						EnableWindow(GetDlgItem(hDialog, IDC_SENDMESSAGE_SELECTED), found);	
					}
					break;
				case LVN_GETDISPINFO:
					plvdi = (NMLVDISPINFO*)lParam; 
					playerNum = plvdi->item.iItem; 
					switch (plvdi->item.iSubItem)
					{	
						case 2:
							if(isYourPlayer(session, playerNum))
								plvdi->item.pszText = L"Local Player";
							else 
								plvdi->item.pszText = session->players[playerNum]->email; 
							break;
						case 3:
							plvdi->item.pszText = session->players[playerNum]->name;
							break;
						case 4:
							if(session->players[playerNum]->faction == -1)
								plvdi->item.pszText = L"Choosing  .  .  .";
							else
								plvdi->item.pszText = session->GetFactionNames()[session->players[playerNum]->faction];
							break;
						default:
							break;
					}
				default:
					break;
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDC_SENDMESSAGE_SELECTED:
					RecipientsMask = playerMask;
					playerMask = NULL;
					EnableWindow(hMainWnd, TRUE);
					EndDialog(hDialog, LOWORD(wParam));
					break;
				case IDC_SENDMESSAGE_ALL:
					RecipientsMask = NULL;
					EnableWindow(hMainWnd, TRUE);
					EndDialog(hDialog, LOWORD(wParam));
					break;
				case IDC_EDITPLAYERS_BUTTON:
					CopySession(tempSession, session);
					if(IDC_OK_BUTTON == DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EDIT_PLAYERS), hDialog, EditPlayersDialogProc, (LPARAM)tempSession))
					{
						SyncPlayerLists(session, tempSession, FALSE);
						ApplyGlobalNamesToSession(tempSession, NULL, NULL);						
									
						session->CheckGameSettingsChanged(tempSession);

						CopySession(session, tempSession);
						
						CheckPlayerListChanged(session);
					
						ListView_Update(hMainListView, sIndex);
						CheckYourTurnEnabled = TRUE;
						
						if(session->state & SESSION_MODIFY_PLAYERLIST)
						{
							ListView_DeleteAllItems(hPlayerListLV);
							free(playerMask);
							selectedPlayer = -1;
						
							playerMask = (BOOL *)malloc(sizeof(BOOL) * session->numPlayers);
							memset(playerMask, FALSE, sizeof(BOOL) * session->numPlayers);
							EnableWindow(GetDlgItem(hDialog, IDC_SENDMESSAGE_SELECTED), FALSE);
							EnableWindow(GetDlgItem(hDialog, IDC_SENDMESSAGE_ALL), FALSE);
							AddPlayerListToLV(hPlayerListLV, session);
							UpdateWindow(hPlayerListLV);
						}

						if(NewCurrentPlayer != -1)
							SendEmailYourTurn(session);
						else 
							CheckNewGame(session);
						
						//else if(session->state && (session->turnNumber != 0 || isYourTurn(session)))
						//	SendEmailStatusUpdate(session, FALSE, FALSE);
					
						SaveSessionList();
					}
					break;
				case IDCANCEL:
					EnableWindow(hMainWnd, TRUE);
					EndDialog(hDialog, LOWORD(wParam));
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			SetWindowLong(hPlayerListLV, GWL_WNDPROC, (LONG)DefaultLVProc);
			EnableWindow(hMainWnd, TRUE);
			EndDialog(hDialog, IDCANCEL);
			FreeSession(tempSession);
			if(playerMask) free(playerMask);
			break;
		default:
			return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;		
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= hLargeIcon;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PLAYMAILER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= hSmallIcon;

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	RECT rc;

	hInst = hInstance; // Store instance handle in our global variable
	
	rc.top = rc.left = 0;
	rc.right = WINDOW_CLIENT_WIDTH;
	rc.bottom = WINDOW_CLIENT_HEIGHT;

	AdjustWindowRect(&rc, WS_CAPTION|WS_BORDER|WS_MINIMIZEBOX|WS_SYSMENU, TRUE);

	hWnd = CreateWindow(szWindowClass, szTitle, (WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU),
		CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	if(nCmdShow == SW_SHOWMINIMIZED || nCmdShow == SW_SHOWMINNOACTIVE)
	{
		ShowNotifyIcon(hWnd, TRUE);
	}
	else if(nCmdShow == SW_SHOW)
		ShowWindow(hWnd, SW_SHOWNORMAL);
	else 
		ShowWindow(hWnd, nCmdShow);
	
	UpdateWindow(hWnd);

	hMainWnd = hWnd;

	return TRUE;
}

void ShowPopMenu(HWND hWnd)
{
	UINT uFlag = MF_BYPOSITION|MF_STRING;
	POINT pt;
	HMENU hPopMenu;

	GetCursorPos(&pt);
	hPopMenu = CreatePopupMenu();

	InsertMenu(hPopMenu, -1, uFlag, IDM_POPUP_OPEN, L"Open PlayMailer");
	InsertMenu(hPopMenu, -1, MF_SEPARATOR, IDM_POPUP_SEP, L"SEP");
	InsertMenu(hPopMenu, -1, uFlag, IDM_POPUP_EXIT, L"Exit");
	SetMenuDefaultItem(hPopMenu, IDM_POPUP_OPEN, FALSE);
	SetForegroundWindow(hWnd);
	TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
               pt.x, pt.y,0, hWnd, NULL);
}

void WaitHotKeyRelease(LPARAM lParam)
{
	while(1)
	{
		if( GetAsyncKeyState(HIWORD(lParam)) >= 0 &&
				( !(LOWORD(lParam) & 0x0001) || GetAsyncKeyState(VK_MENU) >= 0 ) &&
				( !(LOWORD(lParam) & 0x0002) || GetAsyncKeyState(VK_CONTROL) >= 0 ) &&
				( !(LOWORD(lParam) & 0x0004) || GetAsyncKeyState(VK_SHIFT) >= 0 ) &&
				( !(LOWORD(lParam) & 0x0008) || (GetAsyncKeyState(VK_LWIN) >= 0 && GetAsyncKeyState(VK_LWIN) >= 0) ) )
			return;

		SleepC(50);
	}
}

BOOL CheckMail() 
{
	int ret;

	if(!IsInternetConnected())
	{
		FailedFetches = 0;
		return FALSE;
	}

	ret = FetchEmails(FALSE);

	return !ret;
}

int mod(int a, int b)
{
   if(b < 0) //you can check for b == 0 separately and do what you want
     return mod(-a, -b);   
   int ret = a % b;
   if(ret < 0)
     ret+=b;
   return ret;
}

void CheckHotKeys()
{
	SessionInfo *session = (SessionInfo *)LL_GetItem(&llSessions, selectedSession);
	int ret;

	switch(HotKeyPressed)
	{
		case VK_L:
			if(session && !ExportSaveFile(session, TRUE) && (session->turnNumber == 1 || session->selectingTeams))
				_NewGame(session);
			else
				_LoadGame(session, TRUE);
			break;
		case VK_S:
			if(_SaveGame(session))
			{
				SendEmailYourTurn(session);
				session->PostSendEvent();
			}
			break;
		case VK_P:
			if(selectedSession == -1)
				MessageBoxS(NULL, L"I don't know which Player List to pull up. First select a session using PlayMailer.", L"No session selected", MB_OK | MB_ICONEXCLAMATION);
			else
				ret = DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_PLAYER_LIST), NULL, PlayerListDialogProc, selectedSession);
				if(ret == IDC_SENDMESSAGE_ALL || ret == IDC_SENDMESSAGE_SELECTED)
				{
					EditChatMessage(selectedSession, NULL, RecipientsMask, NULL, NULL);
					if(RecipientsMask) 
						free(RecipientsMask);
				}	
			break;
		case VK_M:
			EditChatMessage(selectedSession, NULL, NULL, NULL, NULL);
			break;
		case VK_O:
			if(selectedSession == -1)
				MessageBoxS(NULL, L"I don't know which Game Settings to pull up. First select a session using PlayMailer.", L"No session selected", MB_OK | MB_ICONEXCLAMATION);
			else
				DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_SESSION_SETTINGS), NULL, SessionSettingsDialogProc, selectedSession);
			break;
		case VK_H:
			ViewHotkeysHelp();
			break;
#ifdef _DEBUG
		case VK_W:
			PrintWindowNameAndClass();
			break;
#endif
		default:	
			break;
	}

#ifdef _DEBUG
	CheckMousePosKeys(HotKeyPressed);
#endif

	HotKeyPressed = 0;
}

void PrintWindowNameAndClass()
{
	TCHAR windowText[512], windowClass[512];

	HWND hFG;
	hFG = GetForegroundWindow();

	GetWindowText(hFG, windowText, 512);
	GetClassName(hFG, windowClass, 512);
	swprintf(mbBuffer, MBBUFFER_SIZE, L"Window Text: %s\nWindow Class: %s", windowText, windowClass);
	MessageBoxS(NULL, mbBuffer, L"Window Text and Class", MB_OK);
}

BOOL isYourPlayer(SessionInfo *session, int player)
{
	return !_wcsicmp(session->players[player]->email, mail->email);
}

BOOL EditPlainEmail(TCHAR *toName, TCHAR *toAddress)
{
	PlainEmail msg;
	BOOL emailSent = FALSE;

	memset(&msg, 0, sizeof(PlainEmail));
	BuildNameAddressStr(msg.to, toName, toAddress);
	BuildNameAddressStr(msg.from, mail->name, mail->email);

	if(IDC_SEND_BUTTON == DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EDIT_PLAIN_EMAIL), NULL, EditPlainEmailDialogProc, (LPARAM)&msg))
	{	
		if(SendPlainEmail(&msg))
			emailSent = TRUE;
		free(msg.message);
	}
	
	return emailSent;
}

BOOL SendPlainEmail(PlainEmail *msg)
{
	TCHAR emailPath[MAX_PATH];
	FILE *email;

	GetUniqueOutgoingEmailPath(emailPath);
	if(_wfopen_s(&email, emailPath, L"w+b"))
	{
		MessageBoxS(NULL, L"Cannot create email file.", L"File Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	
	compose_msg_w(msg->from, msg->to, msg->subject, msg->message, NULL, email);
	fclose(email);

	SetEvent( hSendEmailEvents[EVENT_SENDEMAIL_CHECK] );

	return TRUE;
}

BOOL EditChatMessage(int sessionIndex, TCHAR *fromName, BOOL *recipientsMask, TCHAR **recAddresses, TCHAR **recNames)
{
	ChatMessage chatMsg;
	SessionInfo *session;
	chatMsg.broadcast = FALSE;
	BOOL emailSent = FALSE;

	if(sessionIndex == -1)
	{
		MessageBoxS(NULL, L"I don't know which session to send the message to. First select a session using PlayMailer.", L"No session selected", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	session = (SessionInfo *)LL_GetItem(&llSessions, sessionIndex);

	// broadcast to all
	if(!recipientsMask && !recAddresses)
	{
		if(session->numPlayers == 0)
		{
			MessageBoxS(NULL, L"Nobody to send chat message to. First add some players!", L"Error sending chat message", MB_OK);
			return FALSE;
		}
	
		chatMsg.broadcast = TRUE;

		recipientsMask = (BOOL *)malloc(session->numPlayers * sizeof(BOOL));

		for(int i = 0; i < session->numPlayers; i++)
			recipientsMask[i] = TRUE;		
	}

	if(recipientsMask)
	{
		PruneRecipients(session, recipientsMask);
		BuildRecipients(session, recipientsMask, &recAddresses, &recNames);
		if(!recAddresses[0])
		{
			MessageBoxS(NULL, L"Nobody to send message to. All the players are local.", L"Nobody to send message to", MB_ICONEXCLAMATION | MB_OK);
			goto END;
		}
	}

	BuildNameAddressList(chatMsg.to, recNames, recAddresses);
	wcscpy_s(chatMsg.sessionName, MAX_SETTING, session->sessionName);
	chatMsg.sessionID = session->sessionID;
	wcscpy_s(chatMsg.from, MAX_SETTING, (fromName && fromName[0] != L'\0' ? fromName : getYourPlayerName(session)));

	if(IDC_SEND_BUTTON == DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_SEND_MESSAGE), NULL, EditMessageDialogProc, (LPARAM)&chatMsg))
	{	
		SendEmailChatMessage(sessionIndex, recAddresses, recNames, &chatMsg);
		emailSent = TRUE;
		free(chatMsg.message);
	}

END:
	if(recipientsMask)
	{
		free(recAddresses);
		free(recNames);
	}
	if(chatMsg.broadcast) free(recipientsMask);

	return emailSent;
}

void BuildRecipients(SessionInfo *session, BOOL *mask, TCHAR ***recAddresses, TCHAR ***recNames)
{
	int numRecipients = 0;

	for(int i = 0; i < TOTAL_PLAYERS(session); i++)
	{
		if(mask[i]) numRecipients++;
	}

	*recAddresses = (TCHAR **)malloc((numRecipients + 1) * sizeof(TCHAR *));	
	*recNames = (TCHAR **)malloc((numRecipients + 1) * sizeof(TCHAR *));

	int copyTo = 0;
	for(int i = 0; i < TOTAL_PLAYERS(session); i++)
	{
		if(mask[i])
		{
			(*recAddresses)[copyTo] = session->players[i]->email;
			(*recNames)[copyTo++] = session->players[i]->name;
		}
	}
	(*recAddresses)[copyTo] = NULL;
	(*recNames)[copyTo] = NULL;
}

BOOL *PruneRecipients(SessionInfo *session, BOOL *mask)
{
	for(int i = 0; i < TOTAL_PLAYERS(session); i++)
	{
		if(!mask[i] || !session->players[i]->email[0])
		{
			mask[i] = FALSE;
			continue;
		}

		// Remove your own email
		if(!_wcsicmp(session->players[i]->email, mail->email))
		{
			mask[i] = FALSE;
			continue;
		}

		// Remove duplicate recipients		
		for(int j = i + 1; j < TOTAL_PLAYERS(session); j++)
		{
			if(!_wcsicmp(session->players[i]->email, session->players[j]->email))
			mask[j] = FALSE;
		}
	}

	return mask;
}

TCHAR *getSelectedGameName()
{
	SessionInfo *session;

	if(selectedSession == -1)
		return NULL;

	session = (SessionInfo *)LL_GetItem(&llSessions, selectedSession);

	return session->sessionName;
}

BOOL isYourTurn(SessionInfo *session)
{
	if(session->numPlayers == 0 || session->currentPlayer == -1) return FALSE;

	return isYourPlayer(session, session->currentPlayer);
}

BOOL InitPlayerListLV(HWND hWndListView) 
{ 
	LVCOLUMN lvc; 
    int iCol; 
	RECT rc;
	//HWND hHeader;
	//int headerStyle;
	
	GetClientRect(hWndListView, &rc);
	
	// Initialize the LVCOLUMN structure.
    // The mask specifies that the format, width, text, and subitem members
    // of the structure are valid. 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; 
      
    // Add the columns
    for (iCol = 0; iCol < PLAYERLISTLV_NUM_COLUMNS; iCol++) 
    { 
        lvc.iSubItem = iCol;
        lvc.pszText = PlayerListLVHeaders[iCol];   
        lvc.cx = PlayerListLVColumnWidths[iCol] * rc.right / 100 - (SCROLLBAR_WIDTH / PLAYERLISTLV_NUM_COLUMNS);    
		lvc.fmt = LVCFMT_LEFT;

        if (ListView_InsertColumn(hWndListView, iCol, &lvc) == -1) 
            return FALSE; 
    } 
	
	// Disable column resizing
	//hHeader = ListView_GetHeader(hWndListView);
	//headerStyle = GetWindowLong(hHeader, GWL_STYLE);
	//SetWindowLong(hHeader, GWL_STYLE, headerStyle | HDS_NOSIZING);

	DefaultLVProc = (WNDPROC)SetWindowLong (hWndListView, GWL_WNDPROC, (LONG)PlayerListLVProc);

	return TRUE; 
} 


BOOL InitEditPlayersListView(HWND hWndListView) 
{ 
	LVCOLUMN lvc; 
    int iCol; 
	RECT rc;
	//HWND hHeader;
	//int headerStyle;
	HIMAGELIST hDummyImageList;
	HFONT hFont;
	LOGFONT lf;

	hFont = (HFONT)SendMessage(hWndListView, WM_GETFONT, 0, 0);
	GetObject(hFont, sizeof(LOGFONT), &lf);

	// Set row height (with dodgey hack)
	hDummyImageList = ImageList_Create(1, abs(lf.lfHeight) + abs(lf.lfHeight) / 2, ILC_COLOR, 1, 1); 
	ListView_SetImageList(hWndListView, hDummyImageList, LVSIL_SMALL); 

	GetClientRect(hWndListView, &rc);
	
	// Initialize the LVCOLUMN structure.
    // The mask specifies that the format, width, text, and subitem members
    // of the structure are valid. 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; 
      
    // Add the columns
    for (iCol = 0; iCol < EDITPLAYERSLV_NUM_COLUMNS; iCol++) 
    { 
        lvc.iSubItem = iCol;
        lvc.pszText = EditPlayersLVHeaders[iCol];   
        lvc.cx = EditPlayersLVColumnWidths[iCol] * rc.right / 100 - (SCROLLBAR_WIDTH / EDITPLAYERSLV_NUM_COLUMNS);    
		lvc.fmt = LVCFMT_LEFT;

        if (ListView_InsertColumn(hWndListView, iCol, &lvc) == -1) 
            return FALSE; 
    } 
	
	// Disable column resizing
	//hHeader = ListView_GetHeader(hWndListView);
	//headerStyle = GetWindowLong(hHeader, GWL_STYLE);
	//SetWindowLong(hHeader, GWL_STYLE, headerStyle | HDS_NOSIZING);

	// Hook into the listview window proc to process buttons
	DefaultLVProc = (WNDPROC)SetWindowLong (hWndListView, GWL_WNDPROC, (LONG)EditPlayersListViewProc);

	return TRUE; 
} 

BOOL InitMainListView(HWND hWndListView) 
{ 
	LVCOLUMN lvc; 
    int iCol; 
	RECT rc;
	//HWND hHeader;
	//int headerStyle;
	HIMAGELIST hDummyImageList;

	// Set row height (with dodgey hack)
	hDummyImageList = ImageList_Create(1, MAINLV_ROW_HEIGHT, ILC_COLOR, 1, 1); 
	ListView_SetImageList(hWndListView, hDummyImageList, LVSIL_SMALL); 
	
	SendMessage(hWndListView, WM_SETFONT, (WPARAM)hLargeFont, (LPARAM)MAKELONG(TRUE, 0));
	
	GetClientRect(hWndListView, &rc);
	
	// Initialize the LVCOLUMN structure.
    // The mask specifies that the format, width, text, and subitem members
    // of the structure are valid. 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT; 
      
    // Add the columns
    for (iCol = 0; iCol < MAINLV_NUM_COLUMNS; iCol++) 
    { 
        lvc.iSubItem = iCol;
        lvc.pszText = columnHeaders[iCol];   
        lvc.cx = columnWidths[iCol] * rc.right / 100 - (SCROLLBAR_WIDTH / MAINLV_NUM_COLUMNS);    
		lvc.fmt = LVCFMT_LEFT;

        if (ListView_InsertColumn(hWndListView, iCol, &lvc) == -1) 
            return FALSE; 
    } 
	
	// Disable column resizing
	//hHeader = ListView_GetHeader(hWndListView);
	//headerStyle = GetWindowLong(hHeader, GWL_STYLE);
	//SetWindowLong(hHeader, GWL_STYLE, headerStyle | HDS_NOSIZING);

	// Hook into the listview window proc to process buttons
	DefaultLVProc = (WNDPROC)SetWindowLong (hMainListView, GWL_WNDPROC, (LONG)MainListViewProc);

	return TRUE; 
} 

void SetTopMost(HWND hWnd)
{
	RECT rc;
	HWND hGameWnd;
	
	if(hCurrentDialog)
		return;

	hCurrentDialog = hWnd;

	if(hGameWnd = IsGameWindowForeground())
	{
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);

		GetWindowRect(hWnd, &rc);
		SetCursorPos(rc.right - 8, rc.bottom - 8);

		ForceSetForegroundWindow(hWnd);
	}
	else
	{
		if(settings->windowsTopMost == TOPMOST_ALWAYS && !IsFGWFullScreen())
			ForceSetForegroundWindow(hWnd);
		else
		{
			LockSetForegroundWindow(LSFW_LOCK);
			SetForegroundWindow(hWnd);
			LockSetForegroundWindow(LSFW_UNLOCK);
		}
	}

	if(!(GetWindowLong(hGameWnd, GWL_EXSTYLE) & WS_EX_TOPMOST))
		SetTimer(hMainWnd, CHECK_TOPMOST_TIMER, CHECK_TOPMOST_INTERVAL, NULL);
}

TCHAR *getCurrentPlayerFactionName(SessionInfo *session)
{
	return getFactionName(session, session->currentPlayer);
}

BOOL GetFileSelection(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle)
{
	OPENFILENAME ofn;
	szBuf[0] = L'\0';
	BOOL ret;
	TCHAR currentDir[MAX_PATH];
  
	GetCurrentDirectory(MAX_PATH, currentDir);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrTitle = szTitle;
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szBuf;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = NULL;
	ofn.lpstrDefExt = L"";
	ret = GetOpenFileName(&ofn);
	
	SetCurrentDirectory(currentDir);
	return ret;
}

BOOL GetFolderSelection(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle)
{
	LPITEMIDLIST pidl     = NULL;
	BROWSEINFO   bi       = { 0 };
	BOOL         bResult  = FALSE;

	bi.hwndOwner      = hWnd;
	bi.pszDisplayName = szBuf;
	bi.pidlRoot       = NULL;
	bi.lpszTitle      = szTitle;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

	if ((pidl = SHBrowseForFolder(&bi)) != NULL)
	{
		bResult = SHGetPathFromIDList(pidl, szBuf);
		CoTaskMemFree(pidl);
	}

	return bResult;
}

int removeFolder(TCHAR *dir) 
{
  int len = wcslen(dir) + 2; // required to set 2 nulls at end of argument to SHFileOperation.
  TCHAR *tempdir = (TCHAR*) malloc(len * sizeof(TCHAR));
  memset(tempdir,0,len);
  wcscpy_s(tempdir, len, dir);

  SHFILEOPSTRUCT file_op = {
    NULL,
    FO_DELETE,
    tempdir,
    L"",
    FOF_NOCONFIRMATION |
    FOF_NOERRORUI |
    FOF_SILENT,
    false,
    0,
    L"" 
  };
  
  int ret = SHFileOperation(&file_op);
  free(tempdir);
  return ret; // returns 0 on success, non zero on failure.
}

HFONT CreateFixedFont()
{
	return CreateFont(18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FIXED_PITCH | FF_MODERN, L"Courier New");
}

HFONT CreateWindowFont(TCHAR *name, int height, int weight)
{
	return CreateFont(height, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, name);
}

HFONT CreateDialogFont(TCHAR *name, double size, int weight)
{
	HDC hDC;
	HFONT hFont;

	hDC = GetDC(NULL);

	hFont = CreateFont(-((int)(size * GetDeviceCaps(hDC, LOGPIXELSY)) / 72), 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, name);

	ReleaseDC(NULL, hDC);

	return hFont;
}

BOOL AddPlayer(SessionInfo *session, TCHAR *email, TCHAR *name, int faction)
{
	if(session->numPlayers == MAX_PLAYERS)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Maximum of %d players allowed.", MAX_PLAYERS);
		MessageBox(hEditPlayersDialog, mbBuffer, L"Unable to add player", MB_ICONSTOP | MB_OK); 
		return FALSE;
	}

	if(!session->enableRelayTeams && session->numPlayers == session->MAX_TEAMS)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"You have exceeded the maximum number of players (%d). If you choose to continue, relay teams will be enabled.", session->MAX_TEAMS);
		if(IDOK == MessageBox(hEditPlayersDialog, mbBuffer, L"Enable relay teams?", MB_OKCANCEL | MB_ICONEXCLAMATION))
		{
			session->enableRelayTeams = TRUE;
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Maximum number of teams:  %d", session->MAX_TEAMS);
			SetDlgItemText(hEditPlayersDialog, IDC_MAX_PLAYERS_STATIC, mbBuffer);
			CheckDlgButton(hEditPlayersDialog, IDC_RELAY_TEAMS_CHECK, BST_CHECKED);
		}
		else 
			return FALSE;
	}

	CreatePlayer(session);
	session->players[session->numPlayers - 1]->state = PLAYER_ADD;

	if(email) 
		wcscpy_s(session->players[session->numPlayers - 1]->email, MAX_SETTING, email);
	if(name) 
		wcscpy_s(session->players[session->numPlayers - 1]->name, MAX_SETTING, name);

	if(faction != -1) 
		session->players[session->numPlayers - 1]->faction = faction;

	AddPlayerToLV(hEditPlayersLV, session);
	ListView_SetItemState(hEditPlayersLV, session->numPlayers - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	return TRUE;
}

Player *CreatePlayer(SessionInfo *session)
{
	Player *p;
	
	p = (Player *)malloc(sizeof(Player));

	p->email[0] = L'\0';
	p->name[0] = L'\0';
	p->faction = FACTION_RANDOM;
	p->team = -1;
	p->teamToken = FALSE;
	p->factionSortKey = -1;
	p->playerSortKey = -1;

	do
	{
		p->id = BigRand();
	}
	while(FindPlayerByID(session, p->id) != -1);

	// Un-deleted players go before deleted players in the list. Shuffle deleted players forward
	for(int i = TOTAL_PLAYERS(session); i > session->numPlayers; i--)
		session->players[i] = session->players[i-1];

	session->players[session->numPlayers] = p;
	session->numPlayers++;

	return p;
}

Player *CreateDeletedPlayer(SessionInfo *session)
{
	Player *p;
	
	p = (Player *)malloc(sizeof(Player));

	session->players[TOTAL_PLAYERS(session)] = p;
	session->numDeletedPlayers++;

	return p;
}

void DeletePlayerFromLV(SessionInfo *session, int index)
{
	if(session->numPlayers == 0 || ListView_GetNextItem(hEditPlayersLV, -1, LVNI_SELECTED) == -1)
		return;
		
	ShowWindow(hPlayerNameComboBox, SW_HIDE);
	ShowWindow(hPlayerEmailComboBox, SW_HIDE);
	ShowWindow(hFactionComboBox, SW_HIDE);
	
	ListView_DeleteItem(hEditPlayersLV, index);
		
	DeletePlayer(session, index);
}

void DeletePlayer(SessionInfo *session, int index)
{
	Player *deletedPlayer;
	int nextTeamPlayer;

	if(session->numPlayers == 0) 
		return;
		
	deletedPlayer = session->players[index];

	if(session->currentPlayer == index)
		session->currentPlayer = -1;
	else if(session->currentPlayer > index)
		session->currentPlayer--;

	if(deletedPlayer->teamToken == TRUE)
	{
		nextTeamPlayer = GetNextTeamPlayer(session, deletedPlayer->team);
		if(nextTeamPlayer != index)
			SetTeamToken(session, nextTeamPlayer);		
	}

	for(int i = index+1; i < TOTAL_PLAYERS(session); i++)
		session->players[i-1] = session->players[i];
	
	session->numPlayers--;

	if(deletedPlayer->state & PLAYER_ADD)
	{
		free(deletedPlayer);
		session->players[TOTAL_PLAYERS(session)] = NULL;
	}
	else
	{
		// Add the deleted player onto the end of the list so that other players can be notified
		session->players[TOTAL_PLAYERS(session)] = deletedPlayer;
		deletedPlayer->state = PLAYER_DELETE;
		session->numDeletedPlayers++;
	}
}

int GetWindowX(HWND hWnd)
{
	HWND hWndParent = GetParent(hWnd);
    POINT p = {0};

    MapWindowPoints(hWnd, hWndParent, &p, 1);

	return p.x;
}

int GetWindowY(HWND hWnd)
{
	HWND hWndParent = GetParent(hWnd);
    POINT p = {0};

    MapWindowPoints(hWnd, hWndParent, &p, 1);

	return p.y;
}

void ChangeYourPlayerName(SessionInfo *session, TCHAR *name)
{
	TCHAR currentPlayerName[MAX_SETTING];

	if(isYourTurn(session))
	{
		wcscpy_s(currentPlayerName, MAX_SETTING, getCurrentPlayerName(session));

		for(int i = 0; i < session->numPlayers; i++)
		{
			if(isYourPlayer(session, i) 
				&& !_wcsicmp(session->players[i]->name, currentPlayerName))
			{
				if(name[0] != L'\0')
					wcscpy_s(session->players[i]->name, MAX_SETTING, name);
				else
					wcscpy_s(session->players[i]->name, MAX_SETTING, mail->name);

				session->players[i]->state |= PLAYER_MODIFY_NAME;
			}
		}
	}
}

int FindPlayer(SessionInfo *session, TCHAR *email, TCHAR *name, int faction)
{
	for(int i = 0; i < session->numPlayers; i++)
	{
		if(email && _wcsicmp(session->players[i]->email, email))
			continue;
		if(name && _wcsicmp(session->players[i]->name, name))
			continue;
		if(faction != -1 && session->players[i]->faction != faction)
			continue;

		return i;
	}

	return -1;
}

int GetLastLocalPlayer(SessionInfo *session)
{
	int tempCurrentPlayer;
	
	if(!CheckInternalSaveFileExists(session))
		return -1;

	tempCurrentPlayer = session->currentPlayer;

	do
	{
		if(isYourTurn(session))
			return session->currentPlayer;
	} 
	while(tempCurrentPlayer != RollBackCurrentPlayer(session));

	return -1;
}

int RollBackCurrentPlayer(SessionInfo *session)
{
	session->turnNumber--;
	if(session->turnNumber < 1) 
		session->turnNumber = 1;
	
	SetTeamToken(session, GetPrevTeamPlayer(session, getCurrentPlayerTeam(session)));
	return (session->currentPlayer = GetPrevPlayerIndex(session));
}

void SyncPlayerLists(SessionInfo *out, SessionInfo *in, BOOL quiet)
{
	int currentFaction = -1, lastPlayer = -1, yourLastPlayer = -1, tempCurrentPlayer;
	BOOL found = FALSE;
	int k;

	NewCurrentPlayer = -1;

	if(out->turnNumber == 0 || in->numPlayers == 0) 
		return;

	if(in->currentPlayer != out->currentPlayer)
		in->state |= SESSION_MODIFY_CURRENTPLAYER;

	// Check if current player is still in list
	if(in->currentPlayer != -1) 
	{
		// If the current player's email has changed, send save file to new email address if possible
		if(_wcsicmp(getCurrentPlayerEmail(in), getCurrentPlayerEmail(out)))
		{
			if(isYourTurn(out))
			{
				NewCurrentPlayer = in->currentPlayer;
				return;
			}
			else
				in->currentPlayer = -1;
		}
		else
			return;
	}
		
	for(int i = 0; i < in->numPlayers; i++)
	{
		if(!_wcsicmp(in->players[i]->email, getCurrentPlayerEmail(out))
			&& in->players[i]->faction == getCurrentPlayerFaction(out))
		{
			in->currentPlayer = i;
			return;
		}
	}

	if(!quiet)
		MessageBox(hSessionSettingsDialog, L"You have deleted the current player or changed the email of the current player. PlayMailer is attempting to retrieve the save file from a previous player.", L"Current player changed.", MB_OK | MB_ICONEXCLAMATION);

	// We need to roll-back the game to a player before the deleted current player.
	
	// Find the closest player who is still in the list
	tempCurrentPlayer = out->currentPlayer;
	k = -1;

	while(tempCurrentPlayer != k)
	{
		k = RollBackCurrentPlayer(out);
		
		for(int j = 0; j < in->numPlayers; j++)
		{
			if(!_wcsicmp(out->players[k]->email, in->players[j]->email)
					&& !(out->players[k]->state & PLAYER_ADD)
					&& out->players[k]->faction != -1)
			{
				NewCurrentPlayer = j;
				lastPlayer = k;
				break;
			}
			
			if(isYourPlayer(out, k)	&& !(out->players[k]->state & PLAYER_ADD)
					&& out->players[k]->faction != -1)
				yourLastPlayer = k;
		}
	}	 
	
	if(lastPlayer == tempCurrentPlayer || lastPlayer == -1)
	{
		// None of the original players remain. roll-back the game to the faction our player
		// played when we last saved the game.
		lastPlayer = yourLastPlayer;
		
		if(lastPlayer == -1 || !CheckInternalSaveFileExists(out))
		{
			in->currentPlayer = 0;
			in->turnNumber = 0;
			in->actingCurrentPlayer = FALSE;
			return;
		}
	}

	// If there is a player with the same email address as the deleted player
	// we make them the acting current player.
	for(int i = 0; i < in->numPlayers; i++)
	{
		if(!_wcsicmp(in->players[i]->email, getCurrentPlayerEmail(out)))
		{
			NewCurrentPlayer = i;
			break;
		}
	}

	// Determine the current player.
	for(int i = 0; i < in->numPlayers; i++)
	{
		if(in->players[i]->faction > out->players[lastPlayer]->faction)
		{
			in->currentPlayer = i-1;
			if(in->currentPlayer < 0) in->currentPlayer = in->numPlayers - 1;
			found = TRUE;
			break;
		}
	}
	if(!found) in->currentPlayer = in->numPlayers - 1;

	if(!isYourPlayer(in, in->currentPlayer))
		in->actingCurrentPlayer = TRUE;
	else 
		in->actingCurrentPlayer = FALSE;
}

BOOL CheckPlayerListChanged(SessionInfo *session)
{
	BOOL factionsChanged = FALSE;

	for(int i = 0; i < TOTAL_PLAYERS(session); i++)
	{
		if(session->players[i]->state & (PLAYER_ADD | PLAYER_DELETE | PLAYER_MODIFY_NAME
				| PLAYER_MODIFY_EMAIL | PLAYER_MODIFY_FACTION))
		{
			session->state |= SESSION_MODIFY_PLAYERLIST;
			
			if(session->players[i]->state & (PLAYER_DELETE | PLAYER_MODIFY_FACTION))
				factionsChanged = TRUE;

			break;
		}
	}

	if(factionsChanged)
		PruneTeams(session);
	
	return !!(session->state & SESSION_MODIFY_PLAYERLIST);
}

void RequestPlayerNames(SessionInfo *session)
{
	int tempCurrentPlayer = session->currentPlayer;

	YourPlayerNumber = 1;

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(isYourPlayer(session, i))
		{
			session->currentPlayer = i;
			DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_NEWPLAYER_NAME), NULL, NewPlayerNameDialogProc, (LPARAM)session);
		
			YourPlayerNumber++;
		}
	}
	
	session->currentPlayer = tempCurrentPlayer;
}

Team *FindTeam(SessionInfo *session, int id)
{
	LinkedList *iter;
	Team *team;

	iter = session->teams;
	
	while(iter->next)
	{
		iter = iter->next;
		team = (Team *)iter->item;
		if(team->id == id)
			return team;
	}

	return NULL;
}

void RequestTeamSettings(SessionInfo *session)
{
	int tempCurrentPlayer = session->currentPlayer;
	BOOL keepSelectingTeams = FALSE, teamsChanged = FALSE;

	session->selectingTeams = FALSE;
	YourPlayerNumber = 1;

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->faction == FACTION_PLAYERS_CHOICE || 
			(session->ggSettings->RequestTeamSettings && !FindTeam(session, session->players[i]->team)))
		{
			session->selectingTeams = TRUE;

			if(isYourPlayer(session, i))
				break;
			
			return;
		}
	}

	if(!session->selectingTeams) 
		return;

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(isYourPlayer(session, i))
		{
			if(session->players[i]->faction == FACTION_PLAYERS_CHOICE)
			{
				session->currentPlayer = i;
				DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_NEWPLAYER_TEAM), NULL, NewPlayerFactionDialogProc, (LPARAM)session);
				teamsChanged = TRUE;
			}
			
			if(session->ggSettings->RequestTeamSettings && !FindTeam(session, session->players[i]->team))
			{
				session->currentPlayer = i;
				session->CreateTeamSettingsDialog();
				session->state |= SESSION_MODIFY_TEAMSETTINGS;
			}

			YourPlayerNumber++;
		}
		else if((session->ggSettings->RequestTeamSettings && !FindTeam(session, session->players[i]->team)) || session->players[i]->faction == FACTION_PLAYERS_CHOICE)
			keepSelectingTeams = TRUE;
	}

	session->currentPlayer = tempCurrentPlayer;
	
	if(!keepSelectingTeams) 
		session->selectingTeams = FALSE;

	if(teamsChanged)
		SortPlayerList(session);
	
	CheckPlayerListChanged(session);
	ListView_Update(hMainListView, LL_GetItemIndex(&llSessions, session));
	SaveSessionList();
}

TCHAR *getFactionName(SessionInfo *session, int i)
{
	return getFactionNameFromIndex(session, session->players[i]->faction);
}

TCHAR *getFactionNameFromIndex(SessionInfo *session, int index)
{
	if(index == FACTION_PLAYERS_CHOICE)
		return L"<Player's Choice>";
	else
		return session->GetFactionNames()[index];
}

void ReportPlayerChange(SessionInfo *session, int playerIndex, Player *newPlayer)
{
	int i = playerIndex, tempCurrentPlayer, factionSize;
	TCHAR nameAddr[MAX_EMAIL_FIELD], mbTitle[1024], factionStatusStr[1024];
	BOOL yourPlayerAdded = FALSE;

	if(newPlayer->state & PLAYER_MODIFY_FACTION)
	{
		if(isYourPlayer(session, i))
		{
			// One of your player's teams has changed
			if(session->players[i]->faction == FACTION_PLAYERS_CHOICE)
				swprintf(mbBuffer, MBBUFFER_SIZE, L"%s, your player in \"%s\" has been assigned the team: %s.", session->players[i]->name, session->sessionName, getFactionNameFromIndex(session, newPlayer->faction));						
			else
				swprintf(mbBuffer, MBBUFFER_SIZE, L"%s, your %s player in \"%s\" has been changed to %s.", session->players[i]->name, getFactionName(session, i), session->sessionName, getFactionNameFromIndex(session, newPlayer->faction));
		}
		else
		{
			// Another player's faction has changed
			if(session->players[i]->faction == FACTION_PLAYERS_CHOICE)
				swprintf(mbBuffer, MBBUFFER_SIZE, L"In \"%s\", %s has chosen the team: %s.", session->sessionName, BuildNameAddressStr(nameAddr, session->players[i]->name, session->players[i]->email), getFactionNameFromIndex(session, newPlayer->faction));
			else
				swprintf(mbBuffer, MBBUFFER_SIZE, L"In \"%s\", %s has changed teams from %s to %s.", session->sessionName, BuildNameAddressStr(nameAddr, session->players[i]->name, session->players[i]->email), getFactionName(session, i), getFactionNameFromIndex(session, newPlayer->faction));
		}
		
		wcscpy_s(mbTitle, 1024, L"Player Team Changed");
	}
	else if(newPlayer->state & PLAYER_DELETE)
	{
		if(isYourPlayer(session, i))
		{
			// One of your players was deleted
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%s, your player (%s) in \"%s\" has been deleted by someone else.", session->players[i]->name, (session->players[i]->faction == -1 ? L"<team unassigned>" : getFactionName(session, i)), session->sessionName);
		}
		else
		{
			// Another player was deleted
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%s (%s) has been removed from \"%s.\"", BuildNameAddressStr(nameAddr, session->players[i]->name, session->players[i]->email), (session->players[i]->faction == -1 ? L"<team unassigned>" : getFactionName(session, i)), session->sessionName);
		}
		wcscpy_s(mbTitle, 1024, L"Player Deleted");
	}
	else if(newPlayer->state & PLAYER_ADD)
	{
		if(isYourPlayer(session, i))
		{
			// A new player of yours has been added by someone
			swprintf(mbBuffer, MBBUFFER_SIZE, L"A new player matching your email address has been added to \"%s.\"\n\nAssigned team:  %s.", session->sessionName, getFactionName(session, i));
			yourPlayerAdded = TRUE;
		}
		else
		{
			// A new player that isn't you has been added by someone
			swprintf(mbBuffer, MBBUFFER_SIZE, L"A new player, %s, has been added to \"%s.\"\n\nAssigned team:  %s.", BuildNameAddressStr(nameAddr, session->players[i]->name, session->players[i]->email), session->sessionName, getFactionName(session, i));
		}
		wcscpy_s(mbTitle, 1024, L"New Player");
	}
	else return;

	if((factionSize = GetFactionSize(session, session->players[i]->faction)) > 1) 
	{
		if(newPlayer->state & PLAYER_DELETE)
			swprintf(factionStatusStr, 1024, L"\n\n%s team now has %d %s remaining.", getFactionName(session, i), factionSize - 1, (factionSize - 1 == 1 ? L"player" : L"players"));		
		else
			swprintf(factionStatusStr, 1024, L"\n\n%s team now has %d players.", getFactionName(session, i), factionSize);
		
		wcscat_s(mbBuffer, MBBUFFER_SIZE, factionStatusStr);
	}

	MessageBoxS(NULL, mbBuffer, mbTitle, MB_OK);

	if(yourPlayerAdded)
	{
		// Request player name
		tempCurrentPlayer = session->currentPlayer;
		session->currentPlayer = i;
		DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_NEWPLAYER_NAME), NULL, NewPlayerNameDialogProc, (LPARAM)session);
		session->currentPlayer = tempCurrentPlayer;
	}
}

void FillPlayerNameCombo(TCHAR *selectedName)
{
	LinkedList *iter;
	TCHAR *name;

	ComboBox_ResetContent(hPlayerNameComboBox);

	if(selectedName && selectedName[0] != L'\0')
	{
		SendMessage(hPlayerNameComboBox, CB_ADDSTRING, 0, (LPARAM)selectedName); 
		SendMessage(hPlayerNameComboBox, CB_SETCURSEL, 0 /* index here */, 0);
	}
	else
		SendMessage(hPlayerNameComboBox, CB_SETCURSEL, -1 /* index here */, 0);

	iter = &llNameHistory;

	while(iter->next)
	{
		iter = iter->next;

		name = (TCHAR *)iter->item;
		if(!selectedName || _wcsicmp(selectedName, name))
			 SendMessage(hPlayerNameComboBox, CB_ADDSTRING, 0, (LPARAM)name); 
	}
}

void FillPlayerEmailCombo(TCHAR *selectedEmail)
{
	LinkedList *iter;
	TCHAR *email;

	ComboBox_ResetContent(hPlayerEmailComboBox);

	if(selectedEmail && selectedEmail[0] != L'\0')
	{
		SendMessage(hPlayerEmailComboBox, CB_ADDSTRING, 0, (LPARAM)selectedEmail); 
		SendMessage(hPlayerEmailComboBox, CB_SETCURSEL, 0 /* index here */, 0);
	}
	else
		SendMessage(hPlayerEmailComboBox, CB_SETCURSEL, -1 /* index here */, 0);

	iter = &llEmailHistory;

	while(iter->next)
	{
		iter = iter->next;

		email = (TCHAR *)iter->item;
		if(!selectedEmail || _wcsicmp(selectedEmail, email))
			 SendMessage(hPlayerEmailComboBox, CB_ADDSTRING, 0, (LPARAM)email); 
	}
}

BOOL CheckGameSaved(SessionInfo *session, TCHAR *exportSave, TCHAR *importSave)
{
	HANDLE hFile;
	FILETIME lastWrite;
	ULARGE_INTEGER exportLastWrite, importLastWrite;

	if(!isYourTurn(session) && !session->actingCurrentPlayer)
		return TRUE;

	hFile = CreateFile(exportSave, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return TRUE;
	GetFileTime(hFile, NULL, NULL, &lastWrite);
	exportLastWrite.HighPart = lastWrite.dwHighDateTime;
	exportLastWrite.LowPart = lastWrite.dwLowDateTime;
	CloseHandle(hFile);

	hFile = CreateFile(importSave, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return TRUE;
	GetFileTime(hFile, NULL, NULL, &lastWrite);
	importLastWrite.HighPart = lastWrite.dwHighDateTime;
	importLastWrite.LowPart = lastWrite.dwLowDateTime;
	CloseHandle(hFile);

	if(importLastWrite.QuadPart <= exportLastWrite.QuadPart)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"You haven't saved the game \"%s\" this turn. Sending the save file will roll-back the game to your last turn. Send anyway?", session->sessionName);
		if(IDNO == MessageBoxS(NULL, mbBuffer, L"Warning game not saved", MB_ICONEXCLAMATION | MB_YESNO))
			return FALSE;
		session->turnNumber -= session->numPlayers;
		if(session->turnNumber < 1) session->turnNumber = 1;
	}

	return TRUE;
}

BOOL ImportSaveFile(SessionInfo *session)
{
	TCHAR fromPath[MAX_PATH], toPath[MAX_PATH];

	if(!(FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(session->GetSaveFolderPath())))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Invalid %s Save Folder Path - The folder \"%s\" does not exist.", session->pGameSettings->gameID, session->GetSaveFolderPath());
		MessageBoxS(NULL, mbBuffer, L"Invalid Folder Path", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	session->GetSaveFilePath(fromPath);

	if(_waccess(fromPath, 0))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error sending save file \"%s\". File not found.", fromPath);
		MessageBoxS(NULL, mbBuffer, L"File not found", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	_wmkdir(INTERNAL_SAVE_FOLDER);
		
	swprintf(toPath, MAX_PATH, L"%s\\%u.SAV", INTERNAL_SAVE_FOLDER, session->sessionID);
	
	if(!CheckGameSaved(session, toPath, fromPath))
		return FALSE;
	
	if(!CopyFile(fromPath, toPath, FALSE))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"There was an error copying the file \"%s\" to \"%s\".", fromPath, toPath);
		MessageBoxS(NULL, mbBuffer, L"File Copy Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	
	return TRUE;
}

BOOL ExportSaveFile(SessionInfo *session, BOOL quiet)
{
	TCHAR fromPath[MAX_PATH], toPath[MAX_PATH], saveName[MAX_PATH];
	int sIndex = LL_GetItemIndex(&llSessions, session);

	if(sIndex == -1) 
		return FALSE;

	session = (SessionInfo *)LL_GetItem(&llSessions, sIndex);
	
	//saveFolderPath = session->ggSettings->gameFolderPath;

	swprintf(fromPath, MAX_PATH, L"%s\\%u.SAV", INTERNAL_SAVE_FOLDER, session->sessionID);

	if(!_waccess(fromPath, 0))
	{
		if(!(FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(session->GetSaveFolderPath())))
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Invalid %s Folder Path - The folder \"%s\" does not exist.", session->pGameSettings->gameID, session->GetSaveFolderPath());
			MessageBoxS(NULL, mbBuffer, L"Invalid Folder Path", MB_OK | MB_ICONERROR);
			return FALSE;
		}

		session->GetSaveFilePath(toPath);

		if(!CopyFile(fromPath, toPath, FALSE))
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"There was an error copying the file \"%s\" to \"%s\".", fromPath, toPath);
			MessageBoxS(NULL, mbBuffer, L"File Copy Error", MB_OK | MB_ICONERROR);
			return FALSE;
		}

		if(!quiet)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"\"%s\" has been exported into your game folder as \"%s\".\n\nPress Ctrl-Shift-L from within the game window to load this save game.", session->sessionName, session->GetSaveFileName(saveName));
			MessageBoxS(NULL, mbBuffer, L"Exported save file", MB_TOPMOST | MB_SETFOREGROUND | MB_OK);
		}

		selectSession(sIndex);
		return TRUE;
	}

	selectSession(sIndex);
	return FALSE;
}

BOOL ParseDialogMailSettings(MailSettings *ms)
{
	TCHAR checkMailStr[10];
	HWND hDialog = hSettingsPages[MAIL_SETTINGS];
	TCHAR decInPassword[MAX_SETTING], decOutPassword[MAX_SETTING];

	SendDlgItemMessage(hDialog, IDC_NAME_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)ms->name);
	SendDlgItemMessage(hDialog, IDC_EMAIL_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)ms->email);
	SendDlgItemMessage(hDialog, IDC_INSERVER_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)ms->inServer);
	SendDlgItemMessage(hDialog, IDC_INLOGIN_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)ms->inLogin);
	SendDlgItemMessage(hDialog, IDC_INPASSWORD_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)decInPassword);
	SendDlgItemMessage(hDialog, IDC_OUTSERVER_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)ms->outServer);
	
	if(BST_UNCHECKED == IsDlgButtonChecked(hDialog, IDC_OUTMATCHIN_CHECK))
	{
		SendDlgItemMessage(hDialog, IDC_OUTLOGIN_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)ms->outLogin);
		SendDlgItemMessage(hDialog, IDC_OUTPASSWORD_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)decOutPassword);
	}
	else
	{
		wcscpy_s(ms->outLogin, MAX_SETTING, ms->inLogin);
		wcscpy_s(decOutPassword, MAX_SETTING, decInPassword);
	}

	SendDlgItemMessage(hDialog, IDC_CHECKMAIL_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)checkMailStr);
	ms->checkMailInterval = _wtoi(checkMailStr);

	ms->inMailProtocol = (uint8_t)SendDlgItemMessage(hDialog, IDC_INPROTOCOL_COMBO, CB_GETCURSEL, 0, 0); 
	ms->inSecurityProtocol = (uint8_t)SendDlgItemMessage(hDialog, IDC_INSECURITY_COMBO, CB_GETCURSEL, 0, 0); 
	ms->outSecurityProtocol = (uint8_t)SendDlgItemMessage(hDialog, IDC_OUTSECURITY_COMBO, CB_GETCURSEL, 0, 0); 

	EncryptString(decInPassword, ms->inPassword, &ms->inPasswordSize);
	EncryptString(decOutPassword, ms->outPassword, &ms->outPasswordSize);

	memset(decInPassword, 0, sizeof(TCHAR) * MAX_SETTING);
	memset(decOutPassword, 0, sizeof(TCHAR) * MAX_SETTING);

	if(!ValidateMailSettings(hDialog, ms, FALSE))
		return FALSE;

	return TRUE;
}

void UpdateSettings(MailSettings *ms, LinkedList *llGGSettings)
{
	TCHAR oldEmail[MAX_SETTING], oldName[MAX_SETTING];
	TCHAR oldPassword[MAX_SETTING], newPassword[MAX_SETTING];
	
	*oldEmail = *oldName = L'\0';
	IncomingSettingsChanged = FALSE;

EnterCriticalSection(&Crit);
	DecryptString(oldPassword, mail->inPassword, mail->inPasswordSize);
	DecryptString(newPassword, ms->inPassword, ms->inPasswordSize);
	if(ms->inMailProtocol != mail->inMailProtocol
			|| ms->inSecurityProtocol != mail->inSecurityProtocol
			|| _wcsicmp(ms->inServer, mail->inServer)
			|| _wcsicmp(ms->inLogin, mail->inLogin)
			|| _wcsicmp(oldPassword, newPassword))
	{
		SetEvent( hRecvEmailEvents[EVENT_RECVEMAIL_LOGOUT] );
		//WaitForSingleObject( hDoneEvent, INFINITE );
		SetEvent( hRecvEmailEvents[EVENT_RECVEMAIL_CHECK] );
		IncomingSettingsChanged = TRUE;
	}

	DecryptString(oldPassword, mail->outPassword, mail->outPasswordSize);
	DecryptString(newPassword, ms->outPassword, ms->outPasswordSize);
	if(ms->outSecurityProtocol != mail->outSecurityProtocol
			|| _wcsicmp(ms->outServer, mail->outServer)
			|| _wcsicmp(ms->outLogin, mail->outLogin)
			|| _wcsicmp(oldPassword, newPassword))
	{
		SetEvent( hSendEmailEvents[EVENT_SENDEMAIL_CHECK] );
	}

	memset(oldPassword, 0, sizeof(TCHAR) * MAX_SETTING);
	memset(newPassword, 0, sizeof(TCHAR) * MAX_SETTING);

	if(_wcsicmp(mail->email, ms->email))
		wcscpy_s(oldEmail, MAX_SETTING, mail->email);
	if(_wcsicmp(mail->name, ms->name)) 
		wcscpy_s(oldName, MAX_SETTING, mail->name);

	memcpy(mail, ms, sizeof(MailSettings));

	UpdateGGSettings(llGGSettings);
LeaveCriticalSection(&Crit);
	
	SaveGlobalGameSettings();	
	SaveMailSettings();
	
	if(oldEmail || oldName) ApplyGlobalNamesToSessionList(oldEmail, oldName);

	if(!SettingsLoaded)
	{
		SettingsLoaded = TRUE;
		SetTimer(hMainWnd, CHECK_INTERNET_TIMER, CHECK_INTERNET_INTERVAL, NULL);
	}

#ifndef _DEBUG
	if(settings->runOnStartup)
		AddProgramToStartup(settings->startMinimized);
	else
		RemoveProgramFromStartup();
#endif
}

void UpdateGGSettings(LinkedList *inGGList)
{
	LinkedList *inIter, *outIter;
	GlobalGameSettings *inGG, *outGG;

	inIter = inGGList;
	outIter = &llGlobalGameSettings;

	while(inIter->next)
	{
		inIter = inIter->next;
		outIter = outIter->next;

		inGG = (GlobalGameSettings *)inIter->item;
		outGG = (GlobalGameSettings *)outIter->item;

		memcpy(outGG, inGG, sizeof(GlobalGameSettings));
	}
}

BOOL LoadMailSettings()
{
	config_t cfg;
	config_setting_t *root;
	config_setting_t *passSetting;

	if(_waccess(MAIL_CONFIG_FILE, 0))
		return FALSE;

	config_init(&cfg);

	// Read the file. If there is an error, report it and exit. 
	if(! cfgReadFile(&cfg, MAIL_CONFIG_FILE))
	{
		if(config_error_file(&cfg))
		{
			sprintf_s((char *)mbBuffer, MBBUFFER_SIZE, "Error in config file: %s:%d - %s\n", config_error_file(&cfg),
				config_error_line(&cfg), config_error_text(&cfg));
			MessageBoxA(hMainWnd, (char *)mbBuffer, "Config File Error", MB_ICONERROR | MB_OK);
		}
		config_destroy(&cfg);
		return FALSE;
	}

	ConfigError = FALSE;

	root = config_root_setting(&cfg);
	cfgGetString(root, L"your_name", mail->name);
	cfgGetString(root, L"your_email", mail->email);
	cfgGetString(root, L"incoming_mail_server", mail->inServer);
	cfgGetString(root, L"incoming_mail_login", mail->inLogin);
	//cfgGetString(root, L"incoming_mail_password", mail->inPassword);
	passSetting = cfgGetMember(root, L"incoming_mail_password");
	if(CONFIG_TRUE == config_setting_is_array(passSetting))
	{
		mail->inPasswordSize = config_setting_length(passSetting);
		for(int i = 0; i < mail->inPasswordSize; i++)
			((int *)mail->inPassword)[i] = config_setting_get_int_elem(passSetting, i);
	}

	cfgGetString(root, L"outgoing_mail_server", mail->outServer);
	cfgGetString(root, L"outgoing_mail_login", mail->outLogin);

	//cfgGetString(root, L"outgoing_mail_password", mail->outPassword);
	passSetting = cfgGetMember(root, L"outgoing_mail_password");
	if(CONFIG_TRUE == config_setting_is_array(passSetting))
	{
		mail->outPasswordSize = config_setting_length(passSetting);
		for(int i = 0; i < mail->outPasswordSize; i++)
			((int *)mail->outPassword)[i] = config_setting_get_int_elem(passSetting, i);
	}

	mail->inMailProtocol = cfgGetInt(root, L"incoming_mail_protocol");
	mail->inSecurityProtocol = cfgGetInt(root, L"incoming_security_protocol");	
	mail->outSecurityProtocol = cfgGetInt(root, L"outgoing_security_protocol");
	mail->checkMailInterval = cfgGetInt(root, L"check_mail_interval"); 
	
	cfgGetString(root, L"dosbox_path", settings->DOSBoxPath);
	settings->enterSendsMessage = cfgGetBool(root, L"return_sends_message");
	settings->runOnStartup = cfgGetBool(root, L"run_on_startup");
	settings->startMinimized = cfgGetBool(root, L"start_minimized");

	settings->windowsTopMost = cfgGetBool(root, L"windows_topmost");
	settings->muteAllChat = cfgGetBool(root, L"mute_all_chat");
	settings->muteSessionWideChat = cfgGetBool(root, L"mute_session_wide_chat");
	settings->disableSounds = cfgGetBool(root, L"disable_sounds");
	settings->disableChatSound = cfgGetBool(root, L"disable_chat_sound");
	settings->disableYourTurnSound = cfgGetBool(root, L"disable_yourturn_sound");

	if(ConfigError == TRUE)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error in %s. Setting \"%s\" is missing or invalid.", MAIL_CONFIG_FILE, ConfigErrorString);
		MessageBox(hMainWnd, mbBuffer, L"Config File Error", MB_OK | MB_ICONERROR);
	}

	if(settings->muteSessionWideChat)
		Button_SetCheck(hMuteSWCheck, BST_CHECKED);

	if(settings->muteAllChat)
	{
		Button_SetCheck(hMuteAllCheck, BST_CHECKED);
		EnableWindow(hMuteSWCheck, FALSE);
	}

#ifndef _DEBUG
	if(settings->runOnStartup)
		AddProgramToStartup(settings->startMinimized);
	else
		RemoveProgramFromStartup();
#endif

	if(!ValidateMailSettings(hMainWnd, mail, TRUE))
		ConfigError = TRUE;

	IncomingSettingsChanged = TRUE;
	config_destroy(&cfg);

	return !ConfigError;
}

void SaveMailSettings() 
{
	config_t cfg;
	config_setting_t *root, *passSetting;

	config_init(&cfg);
	
	root = config_root_setting(&cfg);

	cfgSetString(root, L"your_name", mail->name);
	cfgSetString(root, L"your_email", mail->email);
	cfgSetInt(root, L"check_mail_interval", mail->checkMailInterval);
	cfgSetString(root, L"incoming_mail_server", mail->inServer);
	cfgSetInt(root, L"incoming_mail_protocol", mail->inMailProtocol);
	cfgSetInt(root, L"incoming_security_protocol", mail->inSecurityProtocol);
	cfgSetString(root, L"incoming_mail_login", mail->inLogin);

	//cfgSetString(root, L"incoming_mail_password", mail->inPassword);
	passSetting = cfgSettingAdd(root, L"incoming_mail_password", CONFIG_TYPE_ARRAY);
	for(int i = 0; i < mail->inPasswordSize; i++)
		config_setting_set_int_elem(passSetting, -1, ((int *)mail->inPassword)[i]);
	
	cfgSetString(root, L"outgoing_mail_server", mail->outServer);
	cfgSetInt(root, L"outgoing_security_protocol", mail->outSecurityProtocol);
	cfgSetString(root, L"outgoing_mail_login", mail->outLogin);

	//cfgSetString(root, L"outgoing_mail_password", mail->outPassword);
	passSetting = cfgSettingAdd(root, L"outgoing_mail_password", CONFIG_TYPE_ARRAY);
	for(int i = 0; i < mail->outPasswordSize; i++)
		config_setting_set_int_elem(passSetting, -1, ((int *)mail->outPassword)[i]);

	cfgSetString(root, L"dosbox_path", settings->DOSBoxPath);
	cfgSetBool(root, L"return_sends_message", settings->enterSendsMessage);
	cfgSetBool(root, L"run_on_startup", settings->runOnStartup);
	cfgSetBool(root, L"start_minimized", settings->startMinimized);

	cfgSetBool(root, L"windows_topmost", settings->windowsTopMost);
	//cfgSetBool(root, L"status_windows_topmost", settings->statusWindowsTopMost);
	cfgSetBool(root, L"mute_all_chat", settings->muteAllChat);
	cfgSetBool(root, L"mute_session_wide_chat", settings->muteSessionWideChat);
	cfgSetBool(root, L"disable_sounds", settings->disableSounds);
	cfgSetBool(root, L"disable_chat_sound", settings->disableChatSound);
	cfgSetBool(root, L"disable_yourturn_sound", settings->disableYourTurnSound);
	
	// Write out the updated configuration. 
	if(! cfgWriteFile(&cfg, MAIL_CONFIG_FILE))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error writing to file: %s", MAIL_CONFIG_FILE);
		MessageBoxS(NULL, mbBuffer, L"Config File I/O Error", MB_ICONERROR | MB_OK);
	}
	config_destroy(&cfg);
}

BOOL ValidateMailSettings(HWND hWnd, MailSettings *ms, BOOL cfgFile)
{
	int err = -1;
	BOOL isValid = TRUE;

	trimWhiteSpace(ms->name);
	trimWhiteSpace(ms->email);
	trimWhiteSpace(ms->inServer);
	trimWhiteSpace(ms->inLogin);
	//trimWhiteSpace(ms->inPassword);
	trimWhiteSpace(ms->outServer);
	trimWhiteSpace(ms->outLogin);
	//trimWhiteSpace(ms->outPassword);

	// Check for blank fields
	if(!ms->name[0]) err = 0;
	else if(!ms->email[0]) err = 1;
	else if(!ms->inServer[0]) err = 2;
	else if(!ms->inLogin[0]) err = 3;
	else if(!ms->inPasswordSize) err = 4;
	else if(!ms->outServer[0]) err = 5;

	if(err != -1)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Please fill in the \"%s\" field.", mailSettingLabels[err]);
		MessageBoxS(NULL, mbBuffer, L"Field Left Blank", MB_OK | MB_ICONSTOP);
		isValid = FALSE;
	}

	// Check email format is valid
	else if(!checkEmailFormat(ms->email))
	{
		MessageBoxS(NULL, L"\"Your Email\" address is not in the correct format.", L"Invalid Mail Setting", MB_OK | MB_ICONSTOP);
		isValid = FALSE;
	}

	else if(ms->inSecurityProtocol < 0 || ms->inSecurityProtocol > NUM_SECURITY_PROTOCOLS - 1)
	{
		MessageBoxS(NULL, L"Please select a valid Incoming Mail - Security Protocol.", L"Invalid Mail Setting", MB_OK | MB_ICONSTOP);
		isValid = FALSE;
	}

	else if(ms->outSecurityProtocol < 0 || ms->outSecurityProtocol > NUM_SECURITY_PROTOCOLS - 1)
	{
		MessageBoxS(NULL, L"Please select a valid Outgoing Mail - Security Protocol.", L"Invalid Mail Setting", MB_OK | MB_ICONSTOP);
		isValid = FALSE;
	}

	else if(ms->inMailProtocol < 0 || ms->inMailProtocol > NUM_MAIL_PROTOCOLS - 1)
	{
		MessageBoxS(NULL, L"Please select a valid Incoming Mail Protocol.", L"Invalid Mail Setting", MB_OK | MB_ICONSTOP);
		isValid = FALSE;
	}
	
	if(ms->checkMailInterval <= 0) 
		ms->checkMailInterval = 0;

	return isValid;
}

BOOL UpdateSession(SessionInfo *session)
{
	SendDlgItemMessage(hSessionSettingsDialog, IDC_GAMENAME_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)session->sessionName);

	if(!session->ParseGameSettingsDialog()) return FALSE;

	return ValidateSession(session);
}

void ApplyGlobalNamesToSessionList(TCHAR *oldEmail, TCHAR *oldName)
{
	SessionInfo *session;
	LinkedList *iter;
	
	int i = 0;
	iter = &llSessions;
	while(iter->next)
	{
		iter = iter->next;
		session = (SessionInfo *)iter->item;

		if(ApplyGlobalNamesToSession(session, oldEmail, oldName))
			SendEmailStatusUpdate(session, FALSE, TRUE);

		i++;
	}
}

BOOL ApplyGlobalNamesToSession(SessionInfo *session, TCHAR *oldEmail, TCHAR *oldName)
{
	TCHAR email[MAX_SETTING], name[MAX_SETTING];
	BOOL changes = FALSE;

	email[0] = name[0] = L'\0';

	if(oldEmail && _wcsicmp(oldEmail, mail->email))
		wcscpy_s(email, MAX_SETTING, oldEmail);
	if(oldName && _wcsicmp(oldName, mail->name))
		wcscpy_s(name, MAX_SETTING, oldName);

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(email[0] && !_wcsicmp(session->players[i]->email, email))
		{
			changes = TRUE;
			wcscpy_s(session->players[i]->email, MAX_SETTING, mail->email);
			session->players[i]->state |= PLAYER_MODIFY_EMAIL;
		}

		if(!_wcsicmp(mail->email, session->players[i]->email) 
				&& !_wcsicmp(name, session->players[i]->name))
		{
			changes = TRUE;
			wcscpy_s(session->players[i]->name, MAX_SETTING, mail->name);
			session->players[i]->state |= PLAYER_MODIFY_NAME;
		}
	}

	if(changes)
		session->state |= SESSION_MODIFY_PLAYERLIST;

	return changes;
}

GlobalGameSettings *FindGlobalGameSettings(TCHAR *id)
{
	LinkedList *iter;
	GlobalGameSettings *game;

	iter = &llGlobalGameSettings;

	while(iter->next)
	{
		iter = iter->next;
		game = (GlobalGameSettings *)iter->item;

		if(!_wcsicmp(id, game->gameID))
			return game;
	}

	return NULL;
}

BOOL ValidateCfgSession(SessionInfo *session, int sIndex) 
{
	session->recent = TRUE;

	trimWhiteSpace(session->sessionName);
	if(wcslen(session->sessionName) == 0)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error in %s in session #%d. Setting \"session_name\" cannot be blank.", SESSIONS_CONFIG_FILE, sIndex);
		MessageBoxS(NULL, mbBuffer, L"Config File Error", MB_OK | MB_ICONERROR);
	
		return FALSE;
	}

	// Check turn number is valid
	if(session->turnNumber < 0)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error in %s in session #%d. Setting \"turn_number\" must be at least 1.", SESSIONS_CONFIG_FILE, sIndex);
		MessageBoxS(NULL, mbBuffer, L"Config File Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	if(!ValidatePlayerList(session, VALIDATE_FILE)) return FALSE;

	// Check current player and player index matches player list.
	if(session->currentPlayer < 0 || session->currentPlayer > session->numPlayers - 1)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error in %s in session #%d. Setting \"current_player\" is out of range.", SESSIONS_CONFIG_FILE, sIndex);
		MessageBoxS(NULL, mbBuffer, L"Config File Error", MB_OK | MB_ICONERROR);				
		return FALSE;
	}

	return TRUE;
}

BOOL ValidateSession(SessionInfo *session) 
{
	trimWhiteSpace(session->sessionName);
	if(wcslen(session->sessionName) == 0)
	{	
		MessageBox(hSessionSettingsDialog, L"Please enter a Session Name.", L"Field Left Blank", MB_OK | MB_ICONSTOP);
		return FALSE;
	}
	
	return TRUE;
}

BOOL ValidatePlayerList(SessionInfo *session, int type)
{
	int *factionTable, factionTableSize;
	TCHAR cfgError[MBBUFFER_SIZE];
	HWND hWnd = NULL;

	cfgError[0] = L'\0';
	
	session->selectingTeams = FALSE;

	if(type == VALIDATE_FILE) 
		swprintf(cfgError, sizeof(cfgError), L"Error in %s in session #%d. ", SESSIONS_CONFIG_FILE, numSessions + 1);
	else if(type == VALIDATE_GUI)
		hWnd = hEditPlayersDialog;

	if(session->numPlayers == 0)
	{
		if(type != VALIDATE_QUIET)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sYou must add at least 1 player.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Not enough players", MB_OK | MB_ICONSTOP);
		}	
		return FALSE;
	}

	if(session->numPlayers > session->MAX_TEAMS)
	{
		if(type == VALIDATE_GUI)
		{
			if(!session->enableRelayTeams)
			{
				swprintf(mbBuffer, MBBUFFER_SIZE, L"%sYou have exceeded the maximum number of players (%d). If you choose to continue, relay teams will be enabled.", cfgError, session->MAX_TEAMS);
				if(IDOK == MessageBox(hWnd, mbBuffer, L"Enable relay teams?", MB_OKCANCEL | MB_ICONEXCLAMATION))
					session->enableRelayTeams = TRUE;
				else 
					return FALSE;
			}
		}
		else
			session->enableRelayTeams = TRUE;
	}

	factionTableSize = sizeof(int) * session->NUM_FACTIONS;
	factionTable = (int *)malloc(factionTableSize);
	memset(factionTable, -1, factionTableSize);

	for(int i = 0; i < session->numPlayers; i++)
	{
		trimWhiteSpace(session->players[i]->name);
		trimWhiteSpace(session->players[i]->email);

		// Check email format is valid
		if(!checkEmailFormat(session->players[i]->email))
		{
			if(type != VALIDATE_QUIET)
			{
				swprintf(mbBuffer, MBBUFFER_SIZE, L"%sPlease enter a valid email address for player %d.", cfgError, i+1);
				MessageBox(hWnd, mbBuffer, L"Invalid Email", MB_OK | MB_ICONSTOP);
			}
			return FALSE;
		}
	
		// Check player faction is in range
		if(type != VALIDATE_GUI)
		{
			if(session->players[i]->faction < -1 || session->players[i]->faction > session->NUM_FACTIONS - 1)
			{
				if(type != VALIDATE_QUIET)
				{
					swprintf(mbBuffer, MBBUFFER_SIZE, L"%sPlayer %d has an invalid Team value. Should be integer between 0 and %d.", cfgError, i+1, session->NUM_FACTIONS - 1);
					MessageBox(hWnd, mbBuffer, L"Invalid Team", MB_OK | MB_ICONSTOP);
				}
				return FALSE;
			}
		}

		// Check for duplicate factions
		if(!session->ggSettings->duplicateFactions && type == VALIDATE_GUI && !session->enableRelayTeams)
		{
			if(session->players[i]->faction >= 0)
			{
				if(factionTable[session->players[i]->faction] != -1)
				{	
					if(type == VALIDATE_GUI)
					{
						swprintf(mbBuffer, MBBUFFER_SIZE, L"%sPlayer %d and player %d have the same Team selected. If you choose to continue, relay teams will be enabled.", cfgError, factionTable[session->players[i]->faction]+1, i+1);
						if(IDOK == MessageBox(hWnd, mbBuffer, L"Enable relay teams?", MB_OKCANCEL | MB_ICONEXCLAMATION))
							session->enableRelayTeams = TRUE;
						else 
						{
							free(factionTable);
							return FALSE;
						}
					}
					else 
						session->enableRelayTeams = TRUE;
				}
				factionTable[session->players[i]->faction] = i;
			}
		}

		if(session->players[i]->faction == -1)
			session->selectingTeams = TRUE;

		if(session->ggSettings->RequestTeamSettings && !FindTeam(session, session->players[i]->team))
			session->selectingTeams = TRUE;
	}

	free(factionTable);

	if(type != VALIDATE_QUIET && GetNumFactions(session) > session->MAX_TEAMS)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"%sYou have exceeded the maximum number of teams (%d).", cfgError, session->MAX_TEAMS);
		MessageBox(hWnd, mbBuffer, L"Too many teams", MB_OK | MB_ICONSTOP);
		return FALSE;
	}
	
	return TRUE;
}

void AssignTeams(SessionInfo *session)
{
	for(int i = 0; i < session->numPlayers; i++)
		AssignTeam(session, i);
}

int AssignTeam(SessionInfo *session, int playerIndex)
{
	if(!session->ggSettings->duplicateFactions)
	{
		session->players[playerIndex]->team = session->players[playerIndex]->faction;
		return session->players[playerIndex]->team;
	}
	
	if(session->numPlayers > session->MAX_TEAMS)
	{
		for(int i = 0; i < session->numPlayers; i++)
		{
			if(session->players[i]->team >= 0 && session->players[i]->faction == session->players[playerIndex]->faction)
			{
				session->players[playerIndex]->team = session->players[i]->team;
				return session->players[playerIndex]->team;
			}
		}
	}

	if(session->players[playerIndex]->team < 0)
		session->players[playerIndex]->team = BigRand();
	
	return session->players[playerIndex]->team;
}

BOOL ValidateEmailSession(SessionInfo *session)
{
	trimWhiteSpace(session->sessionName);
	if(wcslen(session->sessionName) == 0)
		return FALSE;

	if(session->turnNumber < 0)
		return FALSE;

	if(!session->ValidateGameSettings(VALIDATE_EMAIL, 0))
		return FALSE;

	if(!ValidatePlayerList(session, VALIDATE_QUIET)) return FALSE;

	if(session->currentPlayer < 0 || session->currentPlayer > session->numPlayers - 1)
		return FALSE;

	return TRUE;
}

int GetPrevPlayerIndex(SessionInfo *session)
{
	// For teams
	int pIndex = session->currentPlayer;
	
	do
	{
		pIndex = IteratePlayersPrev(session, pIndex);
	} while(session->players[pIndex]->team == getCurrentPlayerTeam(session)
		&& pIndex != session->currentPlayer);

	return GetPrevTeamPlayer(session, session->players[pIndex]->team); 
}

int GetNextPlayerIndex(SessionInfo *session, int pIndex)
{
	do
	{
		pIndex = IteratePlayersNext(session, pIndex);
	} while(session->players[pIndex]->team == getCurrentPlayerTeam(session)
		&& pIndex != session->currentPlayer);
	
	return GetNextTeamPlayer(session, session->players[pIndex]->team); 
}

int GetNextPlayerIndex(SessionInfo *session)
{
	return GetNextPlayerIndex(session, session->currentPlayer); 
}

int GetNextTeamPlayer(SessionInfo *session, int team)
{
	int firstTeamPlayer = -1, nextPlayer;

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->team == team)
		{
			if(firstTeamPlayer == -1)
				firstTeamPlayer = i;

			if(session->players[i]->teamToken == TRUE)
			{
				nextPlayer = IteratePlayersNext(session, i);
				if(session->players[nextPlayer]->team == team)
					return nextPlayer;

				break;
			}
		}
	}

	return firstTeamPlayer;
}

int GetPrevTeamPlayer(SessionInfo *session, int team)
{
	int firstTeamPlayer = -1;

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->team == team)
		{
			if(firstTeamPlayer == -1)
				firstTeamPlayer = i;

			if(session->players[i]->teamToken == TRUE)
				return i;
		}
	}

	return firstTeamPlayer;
}

int IteratePlayersNext(SessionInfo *session, int index)
{
	if(index == session->numPlayers - 1)
		return 0;

	return index + 1;
}

int IteratePlayersPrev(SessionInfo *session, int index)
{
	if(index == 0)
		return session->numPlayers - 1;

	return index - 1;
}

void SetTeamToken(SessionInfo *session, int index)
{
	int team = session->players[index]->team;
	
	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->team == team)
		{
			session->players[i]->teamToken = FALSE;
			session->players[i]->state &= ~PLAYER_TEAM_TOKEN;
		}
	}

	session->players[index]->teamToken = TRUE;
	session->players[index]->state |= PLAYER_TEAM_TOKEN;
}

TCHAR *getYourPlayerName(SessionInfo *session)
{
	if(isYourTurn(session) && getCurrentPlayerName(session)[0] != L'\0')
		return getCurrentPlayerName(session);
	else 
		return mail->name;
}

int getCurrentPlayerTeam(SessionInfo *session)
{
	return session->players[session->currentPlayer]->team;
}

int getCurrentPlayerFaction(SessionInfo *session)
{
	return session->players[session->currentPlayer]->faction;
}

TCHAR *getCurrentPlayerName(SessionInfo *session)
{
	return session->players[session->currentPlayer]->name;
}

TCHAR *getCurrentPlayerEmail(SessionInfo *session)
{
	return session->players[session->currentPlayer]->email;
}

SessionInfo *CreateSession(SessionInfo *session) 
{
	SessionInfo *newSession;

	if(numSessions == MAX_SESSIONS) 
	{
		MessageBox(hSessionSettingsDialog, L"Cannot create session - maximum number of sessions exceeded.", L"Allocation Error", MB_OK | MB_ICONERROR);
		return NULL;
	}

	session->sessionID = BigRand();
	session->turnNumber = 0;
	session->recent = TRUE;
	session->currentPlayer = 0;
	
	if(!UpdateSession(session))
		return NULL;
	
	newSession = AllocSession(session->pGameSettings->gameID);

	LL_Add(&llSessions, newSession);
	AddSessionToListView(newSession);
	numSessions++;

	return newSession;
}

SessionInfo *CreateSessionFromEmail(SessionInfo *emailSession) 
{
	SessionInfo *session = NULL;

	if(numSessions == MAX_SESSIONS) 
	{
		MessageBoxS(NULL, L"Cannot create session - maximum number of sessions exceeded.", L"Allocation Error", MB_OK | MB_ICONERROR);
		return NULL;
	}

	session = AllocSession(emailSession->pGameSettings->gameID);

	session->InitGameSettings();
	UpdateSessionFromEmail(session, emailSession);
	session->sessionID = emailSession->sessionID;
	wcscpy_s(session->sessionName, MAX_SETTING, emailSession->sessionName);

	LL_Add(&llSessions, session);
	AddSessionToListView(session);
	ListView_Update(hMainListView, numSessions);
	numSessions++;

	return session;
}

void DeleteSession(int sIndex)
{
	SessionInfo *session;
	
	session = (SessionInfo *)LL_GetItem(&llSessions, sIndex);
	swprintf(mbBuffer, MBBUFFER_SIZE, L"Are you sure you want to delete \"%s\"?", session->sessionName);
	if(IDCANCEL == MessageBox(hMainWnd, mbBuffer, L"Confirm Delete", MB_OKCANCEL | MB_ICONQUESTION))
		return;
					
	// Remove listview item
	ListView_DeleteItem(hMainListView, sIndex);
	LL_RemoveIndex(&llSessions, sIndex);
	
	DeleteSaveFile(session->sessionID);
	
	numSessions--;

	// Remove listview item's buttons
	if(numSessions < MAX_BUTTON_ROWS)
	{
		for(int j = 0; j < NUM_LVBUTTONS; j++)
			DestroyWindow(lvButtons[numSessions].hButtons[j]);
	}

	if(sIndex == selectedSession)
	{
		selectedSession = -1;
		GameInProgress = FALSE;
	}
	else if(sIndex < selectedSession)
		selectedSession--;
	
	SaveSessionList();

	session->state |= SESSION_DELETED;
	DeleteYourPlayers(session);
	FreeSession(session);
}

void DeleteYourPlayers(SessionInfo *session)
{
	SessionInfo *tempSession;
	BOOL yourPlayerFound;

	tempSession = AllocSession(session->pGameSettings->gameID);
	CopySession(tempSession, session);
	
	do
	{
		yourPlayerFound = FALSE;
		for(int i = 0; i < session->numPlayers; i++)
		{
			if(isYourPlayer(session, i))
			{
				DeletePlayer(session, i);
				yourPlayerFound = TRUE;
				session->state |= SESSION_MODIFY_PLAYERLIST;
				break;
			}
		}
	}
	while(yourPlayerFound);

	SyncPlayerLists(tempSession, session, TRUE);
	CheckPlayerListChanged(session);
	
	if(NewCurrentPlayer != -1)
		SendEmailYourTurn(session);
	else if(session->state & SESSION_MODIFY_PLAYERLIST)
		SendEmailStatusUpdate(session, FALSE, FALSE);

	FreeSession(tempSession);
}

void AddPlayerListToLV(HWND hLV, SessionInfo *session)
{	
	for(int i = 0; i < session->numPlayers; i++)
		AddPlayerToLV(hLV, session);
}

void AddPlayerToLV(HWND hLV, SessionInfo *session)
{
	LVITEM lvI;
	DWORD playerNum;

	playerNum = ListView_GetItemCount(hLV);

	lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE; 
	lvI.state = 0; 
	lvI.stateMask = 0; 
	lvI.iItem = playerNum;
	lvI.iSubItem = 0;
	lvI.lParam = NULL;
	lvI.pszText = LPSTR_TEXTCALLBACK; // sends an LVN_GETDISP message.
	ListView_InsertItem(hLV, &lvI);
}

void InitPlayerEditControls(SessionInfo *session)
{
	RECT rc;
	COMBOBOXINFO comboInfo;
	
	// Create Player Email edit box
	ListView_GetSubItemRect(hEditPlayersLV, 0, 1, LVIR_LABEL, &rc);	
	hPlayerEmailComboBox = CreateWindow ( 
		L"COMBOBOX", L"", WS_TABSTOP | WS_CHILD | CBS_DROPDOWN | CBS_AUTOHSCROLL,
		0, 0, rc.right - rc.left - 10, rc.bottom - rc.top + 1, hEditPlayersLV, NULL, hInst, NULL
	);      
	SendMessage(hPlayerEmailComboBox, WM_SETFONT, (WPARAM)SendMessage(hEditPlayersLV, WM_GETFONT, 0, 0), (LPARAM)MAKELONG(TRUE, 0));

	// Create Player Name edit box
	ListView_GetSubItemRect(hEditPlayersLV, 0, 2, LVIR_LABEL, &rc);	
	hPlayerNameComboBox = CreateWindow ( 
		L"COMBOBOX", L"", WS_TABSTOP | WS_CHILD | CBS_DROPDOWN | CBS_AUTOHSCROLL,
		0, 0, rc.right - rc.left - 10, rc.bottom - rc.top + 1, hEditPlayersLV, NULL, hInst, NULL
	);      
	SendMessage(hPlayerNameComboBox, WM_SETFONT, (WPARAM)SendMessage(hEditPlayersLV, WM_GETFONT, 0, 0), (LPARAM)MAKELONG(TRUE, 0));

	CreateFactionCombo(hEditPlayersLV, session);

	// Hook the controls to detect Enter and Tab key presses
	comboInfo.cbSize = sizeof(comboInfo);
	GetComboBoxInfo(hPlayerEmailComboBox, &comboInfo);
	hPlayerEmailEdit = comboInfo.hwndItem;
	GetComboBoxInfo(hPlayerNameComboBox, &comboInfo);
	hPlayerNameEdit = comboInfo.hwndItem;
	DefaultEditProc = (WNDPROC)SetWindowLong(hPlayerEmailEdit, GWL_WNDPROC, (LONG)EditPlayersEditProc);
	DefaultEditProc = (WNDPROC)SetWindowLong(hPlayerNameEdit, GWL_WNDPROC, (LONG)EditPlayersEditProc);
}

void CreateFactionCombo(HWND parentWnd, SessionInfo *session)
{
	// Create Faction combobox

	hFactionComboBox = CreateWindow 
	( 
		L"COMBOBOX", L"", WS_TABSTOP | WS_CHILD | CBS_DROPDOWNLIST | CBS_AUTOHSCROLL,  
		0, 0, 118, 30, parentWnd, NULL, hInst, NULL 
	);  

	SendMessage(hFactionComboBox, WM_SETFONT, (WPARAM)SendMessage(hEditPlayersLV, WM_GETFONT, 0, 0), (LPARAM)MAKELONG(TRUE, 0));
	SendMessage(hFactionComboBox, CB_ADDSTRING, 0, (LPARAM)L"Random");
	SendMessage(hFactionComboBox, CB_ADDSTRING, 0, (LPARAM)L"Player's Choice");
	//SendMessage(hFactionComboBox, CB_ADDSTRING, 0, (LPARAM)L"----------------------");
	SendMessage(hFactionComboBox, CB_ADDSTRING, 0, (LPARAM)L"\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336-\x0336");
	for(int i = 0; i < session->NUM_FACTIONS; i++)											  
		SendMessage(hFactionComboBox, CB_ADDSTRING, 0, (LPARAM)session->GetFactionNames()[i]);

	DefaultComboProc = (WNDPROC)SetWindowLong(hFactionComboBox, GWL_WNDPROC, (LONG)EditPlayersEditProc);
}

void ShowPlayerEditControls(SessionInfo *session, int iItem)
{
	RECT rc;

	if(iItem == -1) 
	{
		ShowWindow(hPlayerNameComboBox, SW_HIDE);
		ShowWindow(hPlayerEmailComboBox, SW_HIDE);

		DestroyWindow(hFactionComboBox);
		CreateFactionCombo(hEditPlayersLV, session);

		SetFocus(hEditPlayersLV);
		return;
	}

	FillPlayerEmailCombo(session->players[iItem]->email); 
	ListView_GetSubItemRect(hEditPlayersLV, iItem, 1, LVIR_LABEL, &rc);	
	SetWindowPos(hPlayerEmailComboBox, NULL, rc.left, rc.top - 1, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);

	if(session->players[iItem]->state & PLAYER_ADD || isYourPlayer(session, iItem))
	{
		FillPlayerNameCombo(session->players[iItem]->name); 
		ListView_GetSubItemRect(hEditPlayersLV, iItem, 2, LVIR_LABEL, &rc);	
		SetWindowPos(hPlayerNameComboBox, NULL, rc.left, rc.top - 1, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
	}

	if(session->players[iItem]->faction != -1 || (!isYourPlayer(session, iItem) && session->players[iItem]->state & PLAYER_ADD)) 
	{
		if(isYourPlayer(session, iItem)) 
		{
			ComboBox_DeleteString(hFactionComboBox, 1);
			BaseFaction = 2;
		}
		else
			BaseFaction = 3;

		LastFactionSelected = session->players[iItem]->faction + (session->players[iItem]->faction < 0 ? 2 : BaseFaction);
		SendMessage(hFactionComboBox, CB_SETCURSEL, LastFactionSelected, 0); 
		
		ListView_GetSubItemRect(hEditPlayersLV, iItem, 3, LVIR_LABEL, &rc);	
		SetWindowPos(hFactionComboBox, NULL, rc.left + 2, rc.top - 2, rc.right - rc.left - 10, rc.bottom - rc.top + 10, SWP_NOZORDER | SWP_SHOWWINDOW);
	}
}

void ParsePlayerEditControls(SessionInfo *session, int playerNum)
{
	TCHAR tempStr[MAX_SETTING];
	int tempFaction;

	if(playerNum == -1) return;

	if(IsWindowVisible(hPlayerEmailComboBox))
	{
		ComboBox_GetText(hPlayerEmailComboBox, tempStr, MAX_SETTING);
		trimWhiteSpace(tempStr);
		if(wcscmp(tempStr, session->players[playerNum]->email))
		{
			wcscpy_s(session->players[playerNum]->email, MAX_SETTING, tempStr);
			AddHistoryString(&llEmailHistory, session->players[playerNum]->email, TRUE);
			session->players[playerNum]->state |= PLAYER_MODIFY_EMAIL;
		}
	}

	if(IsWindowVisible(hPlayerNameComboBox))
	{
		ComboBox_GetText(hPlayerNameComboBox, tempStr, MAX_SETTING);
		trimWhiteSpace(tempStr);
		if(wcscmp(tempStr, session->players[playerNum]->name))
		{
			wcscpy_s(session->players[playerNum]->name, MAX_SETTING, tempStr);
			AddHistoryString(&llNameHistory, session->players[playerNum]->name, TRUE);
			session->players[playerNum]->state |= PLAYER_MODIFY_NAME;
		}	
	}

	if(IsWindowVisible(hFactionComboBox))
	{
		tempFaction = ComboBox_GetCurSel(hFactionComboBox) - (ComboBox_GetCurSel(hFactionComboBox) < BaseFaction ? 2 : BaseFaction);
		if(tempFaction != session->players[playerNum]->faction)
		{
			session->players[playerNum]->faction = tempFaction;
			session->players[playerNum]->state |= PLAYER_MODIFY_FACTION;
		}
	}
	ListView_Update(hEditPlayersLV, playerNum);
	SavePlayerHistory();

	if(session->players[playerNum]->state & PLAYER_ADD)
		session->players[playerNum]->state &= ~(PLAYER_MODIFY_EMAIL | PLAYER_MODIFY_NAME | PLAYER_MODIFY_FACTION);
}

void AddSessionToListView(SessionInfo *session)
{
	LVITEM lvI;
	int lvButtonID;
	RECT rc;

	lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE; 
	lvI.state = 0; 
	lvI.stateMask = 0; 
	lvI.iItem = numSessions;
	lvI.iSubItem = 0;

	lvI.lParam = (LPARAM) session;
	lvI.pszText = LPSTR_TEXTCALLBACK; // sends an LVN_GETDISP message.
	ListView_InsertItem(hMainListView, &lvI);
	if(numSessions < MAX_BUTTON_ROWS)
	{
		lvButtonID = numSessions * NUM_LVBUTTONS + IDC_LVBUTTONS;
		ListView_GetSubItemRect(hMainListView, numSessions, 3, LVIR_LABEL, &rc);	
		for(int i = 0; i < NUM_LVBUTTONS; i++)
		{ 
			lvButtons[numSessions].hButtons[i] = CreateWindow ( 
				L"BUTTON", lvButtonLabels[i], WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  
				rc.left + i * LVBUTTON_XGAP + 2, rc.top + 1, LVBUTTON_WIDTH, LVBUTTON_HEIGHT, hMainListView, (HMENU)lvButtonID++, hInst, NULL	
			);     
			SendMessage(lvButtons[numSessions].hButtons[i], WM_SETFONT, (WPARAM)hSmallFont, (LPARAM)MAKELONG(TRUE, 0));
		}
	}
}

void SaveSessionList() 
{
	LinkedList *iter, *teamsIter;
	SessionInfo *session;
	config_t cfg;
	config_setting_t *root, *sessionList, *playerList, *teamList, *group, *subGroup;
	Team *team;

	config_init(&cfg);
	
	root = config_root_setting(&cfg);
	sessionList = cfgSettingAdd(root, L"sessions", CONFIG_TYPE_LIST);
	
	iter = &llSessions;
	while(iter->next)
	{
		iter = iter->next;
		session = (SessionInfo *)iter->item;
	
		group = cfgSettingAdd(sessionList, NULL, CONFIG_TYPE_GROUP);
		cfgSetInt(group, L"session_id", session->sessionID);
		cfgSetString(group, L"session_name", session->sessionName);
		cfgSetInt(group, L"turn_number", session->turnNumber);
		cfgSetInt(group, L"current_player", session->currentPlayer);
		cfgSetInt(group, L"state", session->state);
		cfgSetBool(group, L"acting_current_player", session->actingCurrentPlayer);
		cfgSetBool(group, L"enable_relay_teams", session->enableRelayTeams);

		// Add game settings
		subGroup = cfgSettingAdd(group, L"game_settings", CONFIG_TYPE_GROUP);
		cfgSetString(subGroup, L"game_id", session->pGameSettings->gameID);
		session->SaveGameSettings(subGroup);

		// Add Team Settings
		teamsIter = session->teams;
		if(teamsIter->next)
			teamList = cfgSettingAdd(group, L"team_list", CONFIG_TYPE_LIST);
		
		while(teamsIter->next)
		{
			teamsIter = teamsIter->next;
			team = (Team *)teamsIter->item;
			
			subGroup = cfgSettingAdd(teamList, NULL, CONFIG_TYPE_GROUP);
			cfgSetInt(subGroup, L"id", team->id);
			cfgSetInt(subGroup, L"state", team->state);
			cfgSetInt(subGroup, L"faction", team->faction);
			session->SaveTeamSettings(subGroup, team);
		}

		// Add player list
		playerList = cfgSettingAdd(group, L"player_list", CONFIG_TYPE_LIST);
		for(int i = 0; i < session->numPlayers; i++)
		{
			subGroup = cfgSettingAdd(playerList, NULL, CONFIG_TYPE_GROUP);
			cfgSetInt(subGroup, L"id", session->players[i]->id);
			cfgSetString(subGroup, L"name", session->players[i]->name);
			cfgSetString(subGroup, L"email", session->players[i]->email);
			cfgSetInt(subGroup, L"faction", session->players[i]->faction);
			cfgSetInt(subGroup, L"team", session->players[i]->team);
			cfgSetBool(subGroup, L"team_token", session->players[i]->teamToken);
			if(session->ggSettings->RandomFactionOrder)
				cfgSetInt(subGroup, L"team_sort_key", session->players[i]->factionSortKey);
			cfgSetInt(subGroup, L"state", session->players[i]->state);
		}
	}
	
	// Write out the updated configuration. 
	if(! cfgWriteFile(&cfg, SESSIONS_CONFIG_FILE))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error writing to file: %s", SESSIONS_CONFIG_FILE);
		MessageBoxS(NULL, mbBuffer, L"Config File I/O Error", MB_ICONERROR | MB_OK);
	}
	config_destroy(&cfg);
}

void SaveGlobalGameSettings() 
{
	LinkedList *iter;
	GlobalGameSettings *game;
	config_t cfg;
	config_setting_t *root, *gamesList, *group;

	config_init(&cfg);
	
	root = config_root_setting(&cfg);
	gamesList = cfgSettingAdd(root, L"games", CONFIG_TYPE_LIST);
	
	iter = &llGlobalGameSettings;
	while(iter->next)
	{
		iter = iter->next;
		game = (GlobalGameSettings *)iter->item;
	
		group = cfgSettingAdd(gamesList, NULL, CONFIG_TYPE_GROUP);

		// Save stuff here
		cfgSetString(group, L"game_id", game->gameID);
		cfgSetString(group, L"game_folder_path", game->gameFolderPath);
		cfgSetString(group, L"game_run_command", game->runCommand);
		cfgSetBool(group, L"use_default_run_command", game->useDefaultRunCommand);
		cfgSetBool(group, L"run_command_changed", game->runCommandChanged);
		cfgSetBool(group, L"fullscreen", game->fullScreen);
		cfgSetBool(group, L"display_game_tip", game->displayGameTip);
		
		game->SaveSettings(group);
	}
	
	// Write out the updated configuration. 
	if(! cfgWriteFile(&cfg, GAMES_CONFIG_FILE))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error writing to file: %s", GAMES_CONFIG_FILE);
		MessageBoxS(NULL, mbBuffer, L"Config File I/O Error", MB_ICONERROR | MB_OK);
	}
	config_destroy(&cfg);
}

void LoadGamePlugins()
{
	TCHAR searchStr[MAX_PATH], dllPath[MAX_PATH];
    WIN32_FIND_DATA findData;
	HANDLE hFind;
	HMODULE hMod;
	GlobalGameSettings * (*pCreatePlugin)();
	GlobalGameSettings *ggs;

	_wmkdir(PluginsPath);

	DisableWow64Redirection(TRUE);

	swprintf(searchStr, MAX_PATH, L"%s\\*.dll", PluginsPath);
 
    if((hFind = FindFirstFile(searchStr, &findData)) == INVALID_HANDLE_VALUE)
	{
		DisableWow64Redirection(FALSE);
		return;
	}

	do
	{
		swprintf(dllPath, MAX_PATH, L"%s\\%s", PluginsPath, findData.cFileName);
		if(!(hMod = LoadLibrary(dllPath)))
			continue;

		if(!(pCreatePlugin = (GlobalGameSettings * (*)())GetProcAddress(hMod, "CreatePlugin")))
		{
			FreeLibrary(hMod);
			continue;
		}

		ggs = pCreatePlugin();
		ggs->hModule = hMod;
		AssignPluginCallbacks(ggs);
		ggs->AssignStatics();

		LL_Add(&llGlobalGameSettings, ggs);
	} while(FindNextFile(hFind, &findData));

	FindClose(hFind);
	DisableWow64Redirection(FALSE);
}

void FreeGamePlugins()
{
	GlobalGameSettings *ggs;
	HMODULE hMod;
	LinkedList *iter;
	void (*pReleasePlugin)(GlobalGameSettings *);
	
	iter = &llGlobalGameSettings;

	while(iter->next)
	{
		iter = iter->next;
		ggs = (GlobalGameSettings *)iter->item;
	
		hMod = ggs->hModule;
		pReleasePlugin = (void (*)(GlobalGameSettings *))GetProcAddress(hMod, "ReleasePlugin");
		pReleasePlugin(ggs);
		
		FreeLibrary(hMod);
	}

	LL_Free(&llGlobalGameSettings);
}

BOOL LoadGlobalGameSettings() 
{
	config_t cfg;
	config_setting_t *gamesList, *group;
	GlobalGameSettings *game;
	BOOL err = FALSE;
	TCHAR gameID[MAX_SETTING];

	if(_waccess(GAMES_CONFIG_FILE, 0))
	{
		SaveGlobalGameSettings();
		return FALSE;
	}

	config_init(&cfg);

	// Read the file. If there is an error, report it and exit. 
	if(! cfgReadFile(&cfg, GAMES_CONFIG_FILE))
	{
		if(config_error_file(&cfg))
		{
			sprintf_s((char *)mbBuffer, MBBUFFER_SIZE, "Error in config file: %s:%d - %s\n", config_error_file(&cfg),
				config_error_line(&cfg), config_error_text(&cfg));
			MessageBoxA(hMainWnd, (char *)mbBuffer, "Config File Error", MB_ICONERROR | MB_OK);
		}
		config_destroy(&cfg);
		return FALSE;
	}

	gamesList = cfgLookup(&cfg, L"games");
	if(gamesList != NULL)
	{
		int count = config_setting_length(gamesList);

		for(int i = 0; i < count; i++)
		{
			ConfigError = FALSE;
			
			group = config_setting_get_elem(gamesList, i);
			cfgGetString(group, L"game_id", gameID);
			if(!(game = FindGlobalGameSettings(gameID)))
				continue;

			cfgGetString(group, L"game_folder_path", game->gameFolderPath);
			cfgGetString(group, L"game_run_command", game->runCommand);
			game->useDefaultRunCommand = cfgGetBool(group, L"use_default_run_command");
			game->runCommandChanged = cfgGetBool(group, L"run_command_changed");
			game->fullScreen = cfgGetBool(group, L"fullscreen");
			game->displayGameTip = cfgGetBool(group, L"display_game_tip");

			game->LoadSettings(group);

			if(ConfigError == TRUE)
			{
				swprintf(mbBuffer, MBBUFFER_SIZE, L"Error in %s in game #%d. Setting \"%s\" is missing or invalid.", GAMES_CONFIG_FILE, i+1, ConfigErrorString);
				MessageBox(hMainWnd, mbBuffer, L"Config File Error", MB_OK | MB_ICONERROR);
				err = TRUE;
			}
		}
	}

	config_destroy(&cfg);
	SaveGlobalGameSettings();
	if(err) return FALSE;
	
	return TRUE;
}

SessionInfo *AllocSession(TCHAR *gameID)
{
	SessionInfo *session;
	GlobalGameSettings *ggs;

	ggs = FindGlobalGameSettings(gameID);

	session = ggs->AllocSession();
	
	AssignPluginCallbacks(session);
	session->players = AllocPlayers();
	session->teams = AllocTeams();
	
	return session;
}

Player **AllocPlayers()
{
	Player **players;

	// Allocate list of pointers for player list. We allocate pointers for 2*MAX_PLAYERS 
	// because we store deleted players until a status update is emailed out.
	players = (Player **)malloc(sizeof(Player *) * MAX_PLAYERS * 2);
	memset(players, 0, sizeof(Player *) * MAX_PLAYERS * 2);

	return players;
}

void CopyPlayers(SessionInfo *out, SessionInfo *in)
{
	for(int i = 0; i < TOTAL_PLAYERS(in); i++)
	{
		out->players[i] = (Player *)malloc(sizeof(Player));
		memcpy(out->players[i], in->players[i], sizeof(Player));
	}
	out->numPlayers = in->numPlayers;
	out->numDeletedPlayers = in->numDeletedPlayers;
}

void FreePlayers(SessionInfo *session)
{
	for(int i = 0; i < TOTAL_PLAYERS(session); i++)
	{
		if(session->players[i])
		{
			free(session->players[i]);
			session->players[i] = NULL;
		}
		else 
			break;
	}
}

void FreeSession(SessionInfo *session)
{
	if(session)
	{	
		if(session->players)
		{
			FreePlayers(session);
			free(session->players);
		}

		FreeTeams(session);
		session->ggSettings->FreeSession(session);
	}
}

Team *CreateTeam(SessionInfo *session)
{
	Team *team;

	team = session->AllocTeam();
	LL_Add(session->teams, team);

	return team;
}

void DeleteTeam(SessionInfo *session, Team *team)
{
	LL_RemoveIndex(session->teams, LL_GetItemIndex(session->teams, team));
	session->FreeTeam(team);
}

LinkedList *AllocTeams()
{
	LinkedList *teams;

	teams = (LinkedList *)malloc(sizeof LinkedList);
	memset(teams, 0, sizeof(LinkedList));

	return teams;
}

void FreeTeams(SessionInfo *session)
{
	LinkedList *iter;

	if(session->teams)
	{
		iter = session->teams;
		while(iter->next)
		{
			iter = iter->next;
			session->FreeTeam((Team *)iter->item);
		}
		LL_Free(session->teams);
		session->teams = NULL;
	}
}

void CopyTeams(SessionInfo *out, SessionInfo *in)
{
	LinkedList *iter;
	Team *team;

	iter = in->teams;

	while(iter->next)
	{
		iter = iter->next;
		team = (Team *)iter->item;
		LL_Add(out->teams, team->Clone());
	}
}

void PruneTeams(SessionInfo *session)
{
	LinkedList *iter, *curr;
	Team *t;
	BOOL found;

	iter = session->teams;

	while(iter)
	{
		curr = iter;
		iter = iter->next;

		if(curr->item)
		{
			t = (Team *)curr->item;

			found = FALSE;
			for(int i = 0; i < session->numPlayers; i++)
			{
				if(t->id == session->players[i]->team)
				{
					found = TRUE;
					break;
				}
			}
			if(!found)
			{
				session->FreeTeam(t);
				LL_Remove(curr);
			}
		}
	}
}

void FreeSessionList()
{
	LinkedList *iter;
	SessionInfo *session;

	iter = &llSessions;

	while(iter->next)
	{
		iter = iter->next;

		session = (SessionInfo *)iter->item;
		FreeSession(session);
	}

	LL_Free(&llSessions);
}

void LoadSessionList() 
{
	config_t cfg;
	config_setting_t *setting, *sessionList, *sessionGroup, *playerList, *teamList;
	SessionInfo *session;
	Player *player;
	Team *team;
	GlobalGameSettings *game;
	TCHAR gameID[MAX_SETTING];
	
	numSessions = 0;
	selectedSession = -1;

	config_init(&cfg);

	// Read the file. If there is an error, report it and exit. 
	if(! cfgReadFile(&cfg, SESSIONS_CONFIG_FILE))
	{
		if(config_error_file(&cfg))
		{
			sprintf_s((char *)mbBuffer, MBBUFFER_SIZE, "Error in config file: %s:%d - %s\n", config_error_file(&cfg),
				config_error_line(&cfg), config_error_text(&cfg));
			MessageBoxA(hMainWnd, (char *)mbBuffer, "Config File Error", MB_ICONERROR | MB_OK);
		}
		config_destroy(&cfg);
		return;
	}

	sessionList = cfgLookup(&cfg, L"sessions");
	if(sessionList != NULL)
	{
		int count = config_setting_length(sessionList);

		for(int i = 0; i < count; i++)
		{
			if(numSessions == MAX_SESSIONS) 
			{
				MessageBox(hMainWnd, L"Cannot load session from config file - maximum number of sessions exceeded.", L"Allocation Error", MB_OK | MB_ICONERROR);
				break;
			}

			ConfigError = FALSE;
			session = NULL;

			// Load in settings for this session. Check for invalid settings and
			// decode UTF-8 strings
			
			sessionGroup = config_setting_get_elem(sessionList, i);

			if(setting = cfgGetMember(sessionGroup, L"game_settings"))
			{
				cfgGetString(setting, L"game_id", gameID);
			
				if(!(game = FindGlobalGameSettings(gameID)))
				{
					swprintf(mbBuffer, MBBUFFER_SIZE, L"Error in %s in session #%d. Setting \"game_id\", is an unknown Game ID.", SESSIONS_CONFIG_FILE, i);
					MessageBoxS(NULL, mbBuffer, L"Config File Error", MB_OK | MB_ICONERROR);
					continue;
				}

				session = AllocSession(gameID);
				session->LoadGameSettings(setting);
			}
			else
				ConfigError = TRUE;

			if(!ConfigError)
			{
				cfgGetString(sessionGroup, L"session_name", session->sessionName);
				session->sessionID = cfgGetInt(sessionGroup, L"session_id");
				session->turnNumber = cfgGetInt(sessionGroup, L"turn_number");
				session->currentPlayer = cfgGetInt(sessionGroup, L"current_player");
				session->state = cfgGetInt(sessionGroup, L"state");
				session->actingCurrentPlayer = cfgGetBool(sessionGroup, L"acting_current_player");
				session->enableRelayTeams = cfgGetBool(sessionGroup, L"enable_relay_teams");
			}

			if(!ConfigError && (playerList = cfgGetMember(sessionGroup, L"player_list")))
			{
				session->numPlayers = 0;

				for(int j = 0; j < config_setting_length(playerList); j++)
				{
					setting = config_setting_get_elem(playerList, j);

					player = CreatePlayer(session);

					player->id = cfgGetInt(setting, L"id");
					cfgGetString(setting, L"name", player->name);
					cfgGetString(setting, L"email", player->email);
					player->team = cfgGetInt(setting, L"team");
					player->faction = cfgGetIntD(setting, L"faction", player->team);
					player->teamToken = cfgGetBool(setting, L"team_token");
					if(session->ggSettings->RandomFactionOrder)
						player->factionSortKey = cfgGetInt(setting, L"team_sort_key");
					player->state = cfgGetInt(setting, L"state");
				}	
			}

			if(!ConfigError)
			{
				if(teamList = cfgGetMember(sessionGroup, L"team_list"))
				{
					for(int j = 0; j < config_setting_length(teamList); j++)
					{
						setting = config_setting_get_elem(teamList, j);

						team = CreateTeam(session);
						team->id = cfgGetInt(setting, L"id");
						team->state = cfgGetInt(setting, L"state");
						team->faction = cfgGetIntD(setting, L"faction", team->id);
						session->LoadTeamSettings(setting, team);
						if(!session->ValidateTeamSettings(team, VALIDATE_FILE, i, j))
							DeleteTeam(session, team);
					}
				}
				else
					ConfigError = FALSE;
			}

			if(ConfigError == TRUE)
			{
				swprintf(mbBuffer, MBBUFFER_SIZE, L"Error in %s in session #%d. Setting \"%s\" is missing or invalid.", SESSIONS_CONFIG_FILE, i+1, ConfigErrorString);
				MessageBox(hMainWnd, mbBuffer, L"Config File Error", MB_OK | MB_ICONERROR);
				FreeSession(session);
				continue;
			}

			if(!ValidateCfgSession(session, i+1) || !session->ValidateGameSettings(VALIDATE_FILE, i+1))
			{
				FreeSession(session);
				continue;
			}

			LL_Add(&llSessions, session);
			AddSessionToListView(session);
			ListView_Update(hMainListView, numSessions);
			numSessions++;
		}
	}

	config_destroy(&cfg);
	
	return;
}

void selectSession(int newIndex)
{
	int oldIndex = selectedSession;
	selectedSession = newIndex;

	if(oldIndex != -1 && oldIndex <= numSessions)
		redrawListViewItem(oldIndex);

	redrawListViewItem(newIndex);
}

void selectNextSession()
{
	LinkedList *iter;
	SessionInfo *session;
	int index;
	
	index = selectedSession;
	selectedSession = -1;

	if(index != -1 && index <= numSessions)
		redrawListViewItem(index);

	index = 0;

	iter = &llSessions;
	while(iter->next)
	{
		iter = iter->next;
		session = (SessionInfo *)iter->item;	

		if(isYourTurn(session))
		{
			selectedSession = index;
			redrawListViewItem(index);
			break;
		}
		index++;
	}
}

void redrawListViewItem(int iItem) 
{
	RECT rect;
	HWND hButton;
	
	ListView_RedrawItems(hMainListView, iItem, iItem);
	
	for(int i = 0; i < NUM_LVBUTTONS; i++)
	{
		hButton = lvButtons[iItem].hButtons[i];
		GetClientRect(hButton, &rect);
		InvalidateRect(hButton, &rect, TRUE);
	}
}

TCHAR *RemoveWhiteSpace(TCHAR *inStr, TCHAR *outStr)
{
	int inSize, copyTo = 0;

	inSize = wcslen(inStr) + 1;

	for(int i = 0; i < inSize; i++)
	{
		if(!iswspace(inStr[i]))
			outStr[copyTo++] = inStr[i];
	}
	
	return outStr;
}

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

	if(*str == 0)
	{
		wcscpy_s(inputStr, inputSize, str);
		free(str);
		return inputStr;
	}

	// Trim trailing space
	end = str + wcslen(str) - 1;
	while(end > str && iswspace(*end)) end--;

	// Write new null terminator
	*(end+1) = 0;

	wcscpy_s(inputStr, inputSize, str);

	free(temp);

	return inputStr;
}
	
unsigned int BigRand()
{
	int randNum; 
	randNum = rand(); // Low 15 bits.
	randNum = randNum | rand() << 15; // next 15

	return randNum;
}

// Simple email address validation, check address is one word and contains @.
BOOL checkEmailFormat(TCHAR *email)
{
	BOOL atFound = FALSE;

	while(iswspace(*email)) email++; // Iterate leading whitespace
	if(*email == 0 || *email == L'@')  // All spaces or leading @?
		return FALSE;

	while(*email && !iswspace(*email)) 
	{
		if(*email == L'@') atFound = TRUE;
		email++;
	}
	if(!atFound || *(email-1) == L'@') return FALSE; // No @ or trailing @.
	
	while(iswspace(*email)) email++; // Iterate trailing whitespace
	if(*email != 0) return FALSE; // Too many words?

	return TRUE;
}

TCHAR *BuildChatEmailText(ChatMessage *chatMsg)
{
	FILE *tmp;
	config_t cfg;
	config_setting_t *root;
	char *buffer_u;
	TCHAR *emailText, *tempEmailText;
	long lSize, result;

	if(WinTmpFile(&tmp))
	{
		MessageBoxS(NULL, L"Cannot create temp file.", L"File Error", MB_ICONERROR | MB_OK);
		return NULL;
	}

	config_init(&cfg);
	
	root = config_root_setting(&cfg);
	
	cfgSetString(root, L"session_name", chatMsg->sessionName);
	cfgSetInt(root, L"session_id", chatMsg->sessionID);
	cfgSetBool(root, L"broadcast", chatMsg->broadcast);
	cfgSetString(root, L"from", chatMsg->from);
	cfgSetString(root, L"message", chatMsg->message);

	// Write out the updated configuration. 
	config_write(&cfg, tmp);
	config_destroy(&cfg);

	// obtain file size:
	fseek (tmp, 0, SEEK_END);
	lSize = ftell (tmp);
	rewind (tmp);

	// allocate memory to contain the whole file:
	buffer_u = (char*) malloc (sizeof(char)*lSize + 1);

	// copy the file into the buffer:
	result = fread (buffer_u,1,lSize,tmp);
	if (result != lSize)	
	{
		MessageBoxS(NULL, L"Error reading from temp file", L"Config File I/O Error", MB_ICONERROR | MB_OK);
		fclose(tmp);
		free(buffer_u);
		return NULL;
	}
	fclose(tmp);
	buffer_u[lSize] = 0;
	
	tempEmailText = UTF8_Decode_Dyn(buffer_u);
	emailText = (TCHAR *)malloc(sizeof(TCHAR) * (wcslen(tempEmailText) + wcslen(EMAIL_COMMENT_HEADER) + 1));
	swprintf_s(emailText, wcslen(tempEmailText) + wcslen(EMAIL_COMMENT_HEADER) + 1, L"%s%s", EMAIL_COMMENT_HEADER, tempEmailText);
	free(buffer_u);
	free(tempEmailText);

	return emailText;
}

void SendEmailChatMessage(int sIndex, TCHAR **recAddresses, TCHAR **recNames, ChatMessage *chatMsg)
{
	SessionInfo *session;
	FILE *email;
	TCHAR from[MAX_EMAIL_FIELD], to[MAX_EMAIL_FIELD];
	TCHAR emailPath[MAX_PATH];
	TCHAR *emailText;

	session = (SessionInfo *)LL_GetItem(&llSessions, sIndex);

	BuildNameAddressList(to, recNames, recAddresses);
	BuildNameAddressStr(from, PLAYMAILER_FROM_NAME, mail->email);

	if(!(emailText = BuildChatEmailText(chatMsg)))
		return;

	GetUniqueOutgoingEmailPath(emailPath);
	if(_wfopen_s(&email, emailPath, L"w+b"))
	{
		MessageBoxS(NULL, L"Cannot create email file.", L"File Error", MB_ICONERROR | MB_OK);
		return;
	}

	compose_msg_w(from, to, SUBJECT_CHAT_MESSAGE, emailText, NULL, email);

	free(emailText);	
	fclose(email);

	SetEvent( hSendEmailEvents[EVENT_SENDEMAIL_CHECK] );
}

void SendEmailYourTurn(SessionInfo *session)
{
	FILE *email;
	TCHAR *recipients[2];
	TCHAR filePath[MAX_PATH], emailPath[MAX_PATH], *pFilePath = NULL, from[MAX_EMAIL_FIELD], to[MAX_EMAIL_FIELD];
	TCHAR *emailText;
	int oldCurrentPlayer, nextPlayerIndex, lastLocalPlayer;
	TCHAR *nextPlayerEmail, *nextPlayerName;
	BOOL sendingYourTurnEmail = FALSE;

	CheckYourTurnEnabled = TRUE;
	GameInProgress = FALSE;

	// if some players are still selecting teams, don't let current player send off turn request
	if(session->selectingTeams && NewCurrentPlayer == -1 && !session->actingCurrentPlayer)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Cannot send turn notice to the next player because some players are still choosing Teams. Please wait until all players have chosen a team and then re-save the game using Ctrl+Shift+S.\n");
		MessageBoxS(NULL, mbBuffer, L"Players still choosing teams!", MB_OK | MB_ICONSTOP);
		SendEmailStatusUpdate(session, TRUE, TRUE);
		return;
	}

	if(session->numPlayers == 0) 
	{
		MessageBoxS(NULL, L"Nobody to send the game to. First add some players!", L"Error sending save file", MB_OK);
		return;
	}

	session->state |= SESSION_MODIFY_CURRENTPLAYER;
	session->state |= SESSION_MODIFY_TURNNUMBER;

	if(NewCurrentPlayer != -1)
	{
		// Next player is local
		if(!_wcsicmp(session->players[NewCurrentPlayer]->email, mail->email))
		{
			session->currentPlayer = NewCurrentPlayer;
			NewCurrentPlayer = -1;

			if(!CheckInternalSaveFileExists(session))
			{
				session->currentPlayer = 0;
				session->turnNumber = 0;
				session->actingCurrentPlayer = FALSE;
				CheckYourTurnEnabled = TRUE;	
				goto END;
			}
		}
		else
			nextPlayerIndex = NewCurrentPlayer;
	}
	
	if(NewCurrentPlayer == -1)
	{
		if(session->turnNumber > 0)
		{	
			swprintf(filePath, MAX_PATH, L"%s\\%u.SAV", INTERNAL_SAVE_FOLDER, session->sessionID);
			pFilePath = filePath;
			nextPlayerIndex = GetNextPlayerIndex(session);
		}
		else nextPlayerIndex = 0;
	}

	if(session->turnNumber > 0 && NewCurrentPlayer == -1 && !isYourTurn(session) && !session->actingCurrentPlayer)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Are you sure you want to re-send turn notice for \"%s\"? Another player is currently having their turn.", session->sessionName);
		if(IDNO == MessageBoxS(NULL, mbBuffer, L"Re-send Save File?", MB_ICONEXCLAMATION | MB_YESNO))
			return;

		lastLocalPlayer = GetLastLocalPlayer(session);
		
		if(lastLocalPlayer == -1)
		{
			session->currentPlayer = 0;
			session->turnNumber = 0;
			session->actingCurrentPlayer = FALSE;
			nextPlayerIndex = 0;

			if(isYourTurn(session))
			{
				session->turnNumber = 1;
				goto END;
			}
		}
		else
			nextPlayerIndex = GetNextPlayerIndex(session);
	}

	nextPlayerEmail = session->players[nextPlayerIndex]->email;
	nextPlayerName = session->players[nextPlayerIndex]->name;

	SetTeamToken(session, session->currentPlayer);

	// Next player is local
	if(!_wcsicmp(nextPlayerEmail, mail->email))
	{
		session->currentPlayer = nextPlayerIndex;
		session->turnNumber++;
		SetTeamToken(session, session->currentPlayer);
		goto END;
	}

	GetUniqueOutgoingEmailPath(emailPath);
	if(_wfopen_s(&email, emailPath, L"w+b"))
	{
		MessageBoxS(NULL, L"Cannot create email file.", L"File Error", MB_ICONERROR | MB_OK);
		return;
	}

	recipients[0] = nextPlayerEmail;
	recipients[1] = NULL;

	BuildNameAddressStr(from, PLAYMAILER_FROM_NAME, mail->email);
	BuildNameAddressStr(to, nextPlayerName, nextPlayerEmail);
	
	oldCurrentPlayer = session->currentPlayer;
	if(NewCurrentPlayer == -1)
		session->currentPlayer = nextPlayerIndex;

	session->turnNumber++;
	SetTeamToken(session, session->currentPlayer);

	if(!(emailText = BuildEmailText(session)))
	{
		session->currentPlayer = oldCurrentPlayer;
		SetTeamToken(session, oldCurrentPlayer);
		session->turnNumber--;
		fclose(email);
		DeleteFile(emailPath);
		return;
	}
	
	if(compose_msg_w(from, to, SUBJECT_YOUR_TURN, emailText, pFilePath, email))
	{
		session->currentPlayer = oldCurrentPlayer;
		SetTeamToken(session, oldCurrentPlayer);
		session->turnNumber--;
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Cannot open save file: \"%s\"", filePath);
		MessageBoxS(NULL, mbBuffer, L"File Error", MB_ICONERROR | MB_OK);
		free(emailText);
		fclose(email);
		DeleteFile(emailPath);
		return;
	}
	
	free(emailText);
	fclose(email);

	sendingYourTurnEmail = TRUE;

END:
	session->actingCurrentPlayer = FALSE;
	session->recent = TRUE;

	if(NewCurrentPlayer != -1)
	{
		session->currentPlayer = NewCurrentPlayer;
		NewCurrentPlayer = -1;
	}

	SaveSessionList();
	ListView_Update(hMainListView, LL_GetItemIndex(&llSessions, session));

	SetEvent( hSendEmailEvents[EVENT_SENDEMAIL_CHECK] );
	SendEmailStatusUpdate(session, TRUE, TRUE);

	if(sendingYourTurnEmail)
		DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_INPROGRESS), NULL, SendEmailDialogProc, (LPARAM)to);
}

void SendEmailStatusUpdate(SessionInfo *session, BOOL excludeCurrentPlayer, BOOL quiet)
{
	FILE *email;
	TCHAR from[MAX_EMAIL_FIELD], to[MAX_EMAIL_FIELD];
	TCHAR *emailText;
	TCHAR **recAddresses, **recNames;
	BOOL *recipientsMask;
	TCHAR emailPath[MAX_PATH];
	int ret = 0;

	if(!(emailText = BuildEmailText(session)))
		return;

	recipientsMask = (BOOL *)malloc((TOTAL_PLAYERS(session)) * sizeof(BOOL));
	for(int i = 0; i < TOTAL_PLAYERS(session); i++)
	{
		if(excludeCurrentPlayer && !_wcsicmp(getCurrentPlayerEmail(session), session->players[i]->email)) 
			recipientsMask[i] = FALSE;
		else
			recipientsMask[i] = TRUE;		
	}

	PruneRecipients(session, recipientsMask);
	BuildRecipients(session, recipientsMask, &recAddresses, &recNames);
	
	free(recipientsMask);

	if(!recAddresses[0]) 
		goto END;

	GetUniqueOutgoingEmailPath(emailPath);
	if(_wfopen_s(&email, emailPath, L"w+b"))
	{
		MessageBoxS(NULL, L"Cannot create email file.", L"File Error", MB_ICONERROR | MB_OK);
		goto END;
	}

	BuildNameAddressList(to, recNames, recAddresses);
	BuildNameAddressStr(from, PLAYMAILER_FROM_NAME, mail->email);

	compose_msg_w(from, to, SUBJECT_STATUS_UPDATE, emailText, NULL, email);

	fclose(email);

	SetEvent( hSendEmailEvents[EVENT_SENDEMAIL_CHECK] );

END:
	// free all flags
	session->state = 0;
	FreeDeletedPlayers(session);
	ClearAllPlayerFlags(session);
	ClearAllTeamFlags(session);
	SaveSessionList();

	free(recAddresses);
	free(recNames);
	free(emailText);
}

BOOL CheckInternalSaveFileExists(SessionInfo *session)
{
	TCHAR filePath[MAX_PATH];
	
	swprintf(filePath, MAX_PATH, L"%s\\%u.SAV", INTERNAL_SAVE_FOLDER, session->sessionID);

	if(_waccess(filePath, 0))
		return FALSE;

	return TRUE;
}

TCHAR *BuildNameAddressStr(TCHAR *out, TCHAR *name, TCHAR *address)
{
	if(name && name[0] != L'\0')
	{
		swprintf(out, MAX_EMAIL_FIELD, L"%s <%s>", name, address);
	}
	else wcscpy_s(out, MAX_EMAIL_FIELD, address);

	return out;
}

void AddHistoryString(LinkedList *history, TCHAR *newStr, BOOL addToFront)
{
	LinkedList *iter;
	TCHAR *str;

	if(newStr[0] == L'\0') return;

	// Check if item is already in list
	iter = history;
	while(iter->next)
	{
		iter = iter->next;
		str = (TCHAR *)iter->item;

		if(!_wcsicmp(str, newStr))
		{
			free(LL_Remove(iter));	
			break;
		}
	}
	// If history is full, remove oldest item
	if(LL_Size(history) == MAX_PLAYER_HISTORY)
		free(LL_RemoveIndex(history, MAX_PLAYER_HISTORY - 1));

	// Add the new item
	if(addToFront)
		LL_Insert(history, _wcsdup(newStr), 0);
	else
		LL_Add(history, _wcsdup(newStr));
}

void SavePlayerHistory()
{
	LinkedList *iter;
	TCHAR *str;
	config_t cfg;
	config_setting_t *setting, *root;

	config_init(&cfg);
	
	root = config_root_setting(&cfg);
	
	setting = cfgSettingAdd(root, L"email_history", CONFIG_TYPE_LIST);
	iter = &llEmailHistory;
	while(iter->next)
	{
		iter = iter->next;
		str = (TCHAR *)iter->item;

		cfgAddString(setting, str);
	}

	setting = cfgSettingAdd(root, L"name_history", CONFIG_TYPE_LIST);
	iter = &llNameHistory;
	while(iter->next)
	{
		iter = iter->next;
		str = (TCHAR *)iter->item;

		cfgAddString(setting, str);
	}

	// Write out the updated configuration. 
	cfgWriteFile(&cfg, PLAYER_HISTORY_FILE);
	config_destroy(&cfg);
}

void LoadPlayerHistory()
{
	config_t cfg;
	config_setting_t *list;
	TCHAR str[MAX_SETTING];

	config_init(&cfg);

	if(! cfgReadFile(&cfg, PLAYER_HISTORY_FILE))
	{
		config_destroy(&cfg);
		return;
	}

	list = cfgLookup(&cfg, L"email_history");
	if(list != NULL)
	{
		int count = config_setting_length(list);

		for(int i = 0; i < count; i++)
		{
			cfgGetStringElem(list, i, str);

			AddHistoryString(&llEmailHistory, str, FALSE);
		}
	}

	list = cfgLookup(&cfg, L"name_history");
	if(list != NULL)
	{
		int count = config_setting_length(list);

		for(int i = 0; i < count; i++)
		{
			cfgGetStringElem(list, i, str);

			AddHistoryString(&llNameHistory, str, FALSE);
		}
	}

	config_destroy(&cfg);
}

TCHAR *BuildEmailText(SessionInfo *session)
{
	FILE *tmp;
	config_t cfg;
	config_setting_t *root, *playerList, *teamList, *group;
	char *buffer_u;
	LinkedList *teamsIter;
	Team *team;
	TCHAR *emailText, *tempEmailText;
	long lSize, result;

	if(WinTmpFile(&tmp))
	{
		MessageBoxS(NULL, L"Cannot create temp file.", L"File Error", MB_ICONERROR | MB_OK);
		return NULL;
	}

	config_init(&cfg);
	
	root = config_root_setting(&cfg);
	
	cfgSetString(root, L"session_name", session->sessionName);
	cfgSetInt(root, L"session_id", session->sessionID);
	cfgSetInt(root, L"turn_number", session->turnNumber);
	cfgSetInt(root, L"current_player", session->currentPlayer);
	cfgSetInt(root, L"state", session->state);

	group = cfgSettingAdd(root, L"game_settings", CONFIG_TYPE_GROUP);
	cfgSetString(group, L"game_id", session->pGameSettings->gameID);
	session->BuildEmailGameSettings(group);

	// Add Team Settings
	teamsIter = session->teams;
	if(teamsIter->next)
		teamList = cfgSettingAdd(root, L"team_list", CONFIG_TYPE_LIST);
		
	while(teamsIter->next)
	{
		teamsIter = teamsIter->next;
		team = (Team *)teamsIter->item;
			
		group = cfgSettingAdd(teamList, NULL, CONFIG_TYPE_GROUP);
		cfgSetInt(group, L"id", team->id);
		cfgSetInt(group, L"state", team->state);
		cfgSetInt(group, L"faction", team->faction);
		session->BuildEmailTeamSettings(group, team);
	}

	// Add player list
	playerList = cfgSettingAdd(root, L"player_list", CONFIG_TYPE_LIST);
	for(int i = 0; i < TOTAL_PLAYERS(session); i++)
	{
		group = cfgSettingAdd(playerList, NULL, CONFIG_TYPE_GROUP);
		cfgSetInt(group, L"id", session->players[i]->id);
		cfgSetString(group, L"name", session->players[i]->name);
		cfgSetString(group, L"email", session->players[i]->email);
		cfgSetInt(group, L"faction", session->players[i]->faction);
		cfgSetInt(group, L"team", session->players[i]->team);
		cfgSetBool(group, L"team_token", session->players[i]->teamToken);
		if(session->ggSettings->RandomFactionOrder)
			cfgSetInt(group, L"team_sort_key", session->players[i]->factionSortKey);
		cfgSetInt(group, L"state", session->players[i]->state);
	}

	// Write out the updated configuration. 
	config_write(&cfg, tmp);
	config_destroy(&cfg);

	// obtain file size:
	fseek (tmp, 0, SEEK_END);
	lSize = ftell (tmp);
	rewind (tmp);

	// allocate memory to contain the whole file:
	buffer_u = (char*) malloc (sizeof(char)*lSize + 1);

	// copy the file into the buffer:
	result = fread (buffer_u,1,lSize,tmp);
	if (result != lSize)	
	{
		MessageBoxS(NULL, L"Error reading from temp file", L"Config File I/O Error", MB_ICONERROR | MB_OK);
		fclose(tmp);
		free(buffer_u);
		return NULL;
	}
	fclose(tmp);
	buffer_u[lSize] = 0;
	
	tempEmailText = UTF8_Decode_Dyn(buffer_u);
	emailText = (TCHAR *)malloc(sizeof(TCHAR) * (wcslen(tempEmailText) + wcslen(EMAIL_COMMENT_HEADER) + 1));
	swprintf_s(emailText, wcslen(tempEmailText) + wcslen(EMAIL_COMMENT_HEADER) + 1, L"%s%s", EMAIL_COMMENT_HEADER, tempEmailText);
	free(buffer_u);
	free(tempEmailText);

	return emailText;
}

int SendEmail(FILE *email, TCHAR **recipients, BOOL quiet)
{
	unsigned short port = SMTP_PORT; 
	int ret, securityProtocol;
	TCHAR server[MAX_SETTING], login[MAX_SETTING], password[MAX_SETTING], fromEmail[MAX_SETTING];
	uint8_t encPassword[1024];
	int encPasswordSize;

EnterCriticalSection(&Crit);
	if(mail->outSecurityProtocol == SECURITY_SSL)
		port = SMTP_SSL_PORT;

	wcscpy_s(server, MAX_SETTING, mail->outServer);
	wcscpy_s(login, MAX_SETTING, mail->outLogin);
	memcpy(encPassword, mail->outPassword, mail->outPasswordSize);
	encPasswordSize = mail->outPasswordSize;
	wcscpy_s(fromEmail, MAX_SETTING, mail->email);
	securityProtocol = mail->outSecurityProtocol;
LeaveCriticalSection(&Crit);

	DecryptString(password, encPassword, encPasswordSize);

	ret = smtpsend_w(server, port, login, password, fromEmail, recipients, securityProtocol, FALSE, _fileno(email));

	if(quiet) return ret;
	
	if(ret) 
	{
		if(hSendEmailDialog)
			PostMessage(hSendEmailDialog, WM_CLOSE, 0, 0);
	}

	switch(ret)
	{
	case 0:
		break;
	case 25:
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Could not connect to SMTP mail server: \"%s\". Unknown adddress or connection refused.", server);
		MessageBoxS(NULL, mbBuffer, L"SMTP Mail Error", MB_ICONERROR | MB_OK);
		break;
	case 16:
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Could not connect to SMTP mail server: \"%s\". Check that this SMTP server supports your configured Outgoing Security Protocol.", server);
		MessageBoxS(NULL, mbBuffer, L"SMTP Mail Error", MB_ICONERROR | MB_OK);
		break;
	case 17:
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Could not connect to SMTP mail server: \"%s\". Incorrect login or password.", server);
		MessageBoxS(NULL, mbBuffer, L"SMTP Mail Error", MB_ICONERROR | MB_OK);
		break;
	default:
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Could not deliver email to SMTP mail server: \"%s\". Something went wrong, check your Outgoing Mail Settings.", server);
		MessageBoxS(NULL, mbBuffer, L"SMTP Mail Error", MB_ICONERROR | MB_OK);
		break;
	}

	memset(password, 0, sizeof(TCHAR) * MAX_SETTING);

	return ret;
}

int FetchEmails(BOOL quiet)
{
	int ret;
	unsigned short port; 
	TCHAR server[MAX_SETTING], login[MAX_SETTING], password[MAX_SETTING], statusText[MBBUFFER_SIZE];
	int mailProtocol, securityProtocol, encPasswordSize;
	uint8_t encPassword[1024];

EnterCriticalSection(&Crit);
	mailProtocol = mail->inMailProtocol;
	securityProtocol = mail->inSecurityProtocol;
	wcscpy_s(server, MAX_SETTING, mail->inServer);
	wcscpy_s(login, MAX_SETTING, mail->inLogin);
	memcpy(encPassword, mail->inPassword, mail->inPasswordSize);
	encPasswordSize = mail->inPasswordSize;
LeaveCriticalSection(&Crit);
	
	if(server == L'\0') 
	{
		SetWindowText(hStatusBar, L"Mail Settings not configured.");
		return 1;
	}

	SetWindowText(hStatusBar, L"Connecting to mail server . . .");

	DecryptString(password, encPassword, encPasswordSize);

	if(mail->inMailProtocol == PROTOCOL_IMAP)
	{
		if(IMAPSocket >= 0) 
			imap_idle_done();

		IMAPSocket = -1;
		
		if(securityProtocol == SECURITY_SSL)
			port = IMAP_SSL_PORT;
		else port = IMAP_PORT;

		ret = imap_fetch_w(server, port, login, password, securityProtocol, PLAYMAILER_FROM_NAME);
	
		switch(ret)
		{
			case 0:
				break;
			case 6:
				swprintf(statusText, MBBUFFER_SIZE, L"Could not connect to IMAP mail server \"%s\". Unknown adddress or connection refused.", server);
				//MessageBoxS(NULL, statusText, L"IMAP Mail Error", MB_ICONERROR | MB_OK);
				break;
			case 26:
				swprintf(statusText, MBBUFFER_SIZE, L"Could not connect to IMAP mail server \"%s\". Incorrect login or password.", server);
				if(!FailedFetches)
					MessageBoxS(NULL, statusText, L"PlayMailer Mail Error", MB_ICONERROR | MB_OK);
				break;
			default:
				swprintf(statusText, MBBUFFER_SIZE, L"Could not fetch emails from IMAP mail server \"%s\". Something went wrong, check your Incoming Mail Settings.", server);
				//MessageBoxS(NULL, statusText, L"IMAP Mail Error", MB_ICONERROR | MB_OK);
				break;
		}

		// Enter IDLE mode if available, otherwise start polling timer.
		if(!ret)
			IMAPSocket = imap_idle();
	}
	else // POP3
	{
		IMAPSocket = -1;

		if(securityProtocol == SECURITY_SSL)
			port = POP3_SSL_PORT;
		else port = POP3_PORT;

		ret = pop3_fetch_w(server, port, login, password, securityProtocol);
	
		switch(ret)
		{
			case 0:
				break;
			case 10:
				swprintf(statusText, MBBUFFER_SIZE, L"Could not connect to POP3 mail server \"%s\". Unknown adddress or connection refused.", server);
				//MessageBoxS(NULL, statusText, L"POP3 Mail Error", MB_ICONERROR | MB_OK);
				break;
			case 6:
				swprintf(statusText, MBBUFFER_SIZE, L"Could not connect to POP3 mail server \"%s\". Incorrect login or password.", server);
				if(!FailedFetches)
					MessageBoxS(NULL, statusText, L"PlayMailer Mail Error", MB_ICONERROR | MB_OK);
				break;
			default:
				swprintf(statusText, MBBUFFER_SIZE, L"Could not fetch emails from POP3 mail server \"%s\". Something went wrong, check your Incoming Mail Settings.", server);
				//MessageBoxS(NULL, statusText, L"POP3 Mail Error", MB_ICONERROR | MB_OK);
				break;
		}
		
	}

	memset(password, 0, sizeof(TCHAR) * MAX_SETTING);

	if(!ret)
		SetWindowText(hStatusBar, L"Connected.");
	else
		SetWindowText(hStatusBar, statusText);

	return ret;
}

BOOL CreateRecvEmailThread()
{
	if(!hRecvEmailThread)
	{
		for(int i = 0; i < NUM_RECVEMAIL_EVENTS; i++)
			hRecvEmailEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		hDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		if(!(hRecvEmailThread = CreateThread(NULL, 0, RecvEmailThreadProc, NULL, 0, NULL)))
		{
			MessageBoxS(NULL, L"Thread error: could not create new IMAP thread", L"IMAP Thread Error", MB_OK | MB_ICONERROR);	
			return FALSE;
		}
	}
	return TRUE;
}

void TerminateRecvEmailThread()
{
	if(hRecvEmailThread) 
	{
		SetEvent( hRecvEmailEvents[EVENT_RECVEMAIL_QUIT] );
		WaitForSingleObject( hRecvEmailThread, INFINITE );
		
		hRecvEmailThread = NULL;
		for(int i = 0; i < NUM_RECVEMAIL_EVENTS; i++)
			CloseHandle(hRecvEmailEvents[i]);

		CloseHandle(hDoneEvent);
	}
}

BOOL CreateSendEmailThread()
{
	if(!hSendEmailThread)
	{
		for(int i = 0; i < NUM_SENDEMAIL_EVENTS; i++)
			hSendEmailEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		
		if(!(hSendEmailThread = CreateThread(NULL, 0, SendEmailThreadProc, NULL, 0, NULL)))
		{
			MessageBoxS(NULL, L"Thread error: could not create send-email thread", L"Thread Error", MB_OK | MB_ICONERROR);	
			return FALSE;
		}
	}
	return TRUE;
}

void TerminateSendEmailThread()
{
	if(hSendEmailThread) 
	{
		SetEvent( hSendEmailEvents[EVENT_SENDEMAIL_QUIT] );
		WaitForSingleObject( hSendEmailThread, INFINITE );
		
		hSendEmailThread = NULL;
		for(int i = 0; i < NUM_SENDEMAIL_EVENTS; i++)
			CloseHandle(hSendEmailEvents[i]);
	}
}

int WinTmpFile(FILE **tmp)
{
	TCHAR fileName[MAX_PATH], filePath[MAX_PATH];

	GetCurrentDirectory(MAX_PATH, filePath);

	_wtmpnam_s(fileName, MAX_PATH);
	wcscat_s(filePath, MAX_PATH, fileName);

	return _wfopen_s(tmp, filePath, L"w+bD");
}

BOOL ParseEmailChatMessageText(TCHAR *text, size_t textSize, ChatMessage *chatMsg)
{
	FILE *tmp;
	config_t cfg;
	config_setting_t *root;
	long result; 
	char *text_u;

	if(WinTmpFile(&tmp))
	{
		MessageBoxS(NULL, L"Cannot create temp file.", L"File Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	text_u = UTF8_Encode_Dyn(text);

	// copy the file into the buffer:
	result = fwrite(text_u, 1, textSize, tmp);
	if (result != textSize)	
	{
		MessageBoxS(NULL, L"Error writing to temp file", L"Config File I/O Error", MB_ICONERROR | MB_OK);
		fclose(tmp);
		free(text_u);
		return FALSE;
	}

	free(text_u);
	rewind(tmp);

	config_init(&cfg);

	if(!config_read(&cfg, tmp))
	{
		fclose(tmp);
		config_destroy(&cfg);
		return FALSE;
	}
	fclose(tmp);
	
	ConfigError = FALSE;

	root = config_root_setting(&cfg);
	
	cfgGetString(root, L"session_name", chatMsg->sessionName);
	chatMsg->sessionID = cfgGetInt(root, L"session_id");
	cfgGetString(root, L"from", chatMsg->from);
	chatMsg->broadcast = cfgGetBool(root, L"broadcast");
	chatMsg->message = cfgGetStringDyn(root, L"message");

	config_destroy(&cfg);

	if(ConfigError == TRUE)
		return FALSE;

	return TRUE;
}

int ParseEmailSessionID(TCHAR *text, size_t textSize)
{
	FILE *tmp;
	config_t cfg;
	config_setting_t *root;
	long result; 
	char *text_u;
	int sessionID;

	if(WinTmpFile(&tmp))
	{
		MessageBoxS(NULL, L"Cannot create temp file.", L"File Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	text_u = UTF8_Encode_Dyn(text);

	// copy the file into the buffer:
	result = fwrite(text_u, 1, textSize, tmp);
	if (result != textSize)	
	{
		MessageBoxS(NULL, L"Error writing to temp file", L"Config File I/O Error", MB_ICONERROR | MB_OK);
		fclose(tmp);
		free(text_u);
		return FALSE;
	}

	free(text_u);
	rewind(tmp);

	config_init(&cfg);

	if(!config_read(&cfg, tmp))
	{
		fclose(tmp);
		config_destroy(&cfg);
		return FALSE;
	}
	fclose(tmp);
	
	ConfigError = FALSE;

	root = config_root_setting(&cfg);
	
	sessionID = cfgGetInt(root, L"session_id");

	config_destroy(&cfg);

	if(ConfigError == TRUE)
		return -1;

	return sessionID;
}

SessionInfo *ParseEmailSessionText(TCHAR *text, size_t textSize)
{
	FILE *tmp;
	config_t cfg;
	config_setting_t *root, *playerList, *teamList, *setting;
	long result; 
	char *text_u;
	Player *player;
	Team *team;
	GlobalGameSettings *game;
	SessionInfo *session = NULL;
	TCHAR gameID[MAX_SETTING];
	int state;

	if(WinTmpFile(&tmp))
	{
		MessageBoxS(NULL, L"Cannot create temp file.", L"File Error", MB_ICONERROR | MB_OK);
		return NULL;
	}

	text_u = UTF8_Encode_Dyn(text);

	// copy the file into the buffer:
	result = fwrite(text_u, 1, textSize, tmp);
	if (result != textSize)	
	{
		MessageBoxS(NULL, L"Error writing to temp file", L"Config File I/O Error", MB_ICONERROR | MB_OK);
		fclose(tmp);
		free(text_u);
		return NULL;
	}

	free(text_u);
	rewind(tmp);

	config_init(&cfg);

	if(!config_read(&cfg, tmp))
	{
		fclose(tmp);
		config_destroy(&cfg);
		return NULL;
	}
	fclose(tmp);
	
	ConfigError = FALSE;

	root = config_root_setting(&cfg);
	
	// Load game settings
	if(setting = cfgGetMember(root, L"game_settings"))
	{
		cfgGetString(setting, L"game_id", gameID);

		if(!(game = FindGlobalGameSettings(gameID)))
		{
			config_destroy(&cfg);
			return FALSE;
		}
		session = AllocSession(gameID);

		session->ParseEmailGameSettings(setting);
	}
	else 
		ConfigError = TRUE;

	if(!ConfigError)
	{
		cfgGetString(root, L"session_name", session->sessionName);
		session->sessionID = cfgGetInt(root, L"session_id");
		session->turnNumber = cfgGetInt(root, L"turn_number");
		session->currentPlayer = cfgGetInt(root, L"current_player");	
		session->state = cfgGetInt(root, L"state");	
	}
	// Load player list
	if(!ConfigError && (playerList = cfgGetMember(root, L"player_list")))
	{
		session->numPlayers = 0;

		for(int i = 0; i < config_setting_length(playerList); i++)
		{
			setting = config_setting_get_elem(playerList, i);

			state = cfgGetInt(setting, L"state");

			if(state & PLAYER_DELETE)
				player = CreateDeletedPlayer(session);
			else
				player = CreatePlayer(session);

			player->id = cfgGetInt(setting, L"id");
			cfgGetString(setting, L"name", player->name);
			cfgGetString(setting, L"email", player->email);
			player->team = cfgGetInt(setting, L"team");
			player->faction = cfgGetIntD(setting, L"faction", player->team);
			player->teamToken = cfgGetBool(setting, L"team_token");
			if(session->ggSettings->RandomFactionOrder)
				player->factionSortKey = cfgGetInt(setting, L"team_sort_key");
			player->state = cfgGetInt(setting, L"state");
		}	
	}

	if(!ConfigError)
	{
		if(teamList = cfgGetMember(root, L"team_list"))
		{
			for(int j = 0; j < config_setting_length(teamList); j++)
			{
				setting = config_setting_get_elem(teamList, j);

				team = CreateTeam(session);
				team->id = cfgGetInt(setting, L"id");
				team->state = cfgGetInt(setting, L"state");
				team->faction = cfgGetIntD(setting, L"faction", team->id);
				session->LoadTeamSettings(setting, team);
				if(!session->ValidateTeamSettings(team, VALIDATE_QUIET, 0, 0))
					DeleteTeam(session, team);
			}
		}
		else
			ConfigError = FALSE;
	}	

	config_destroy(&cfg);

	if(ConfigError == TRUE)
	{
		FreeSession(session);
		return NULL;
	}

	if(!ValidateEmailSession(session))
	{
		FreeSession(session);
		return NULL;
	}

	return session;
}

TCHAR *BuildNameAddressList(TCHAR *outStr, TCHAR **names, TCHAR **addresses)
{
	TCHAR nameAddr[MAX_EMAIL_FIELD];

	outStr[0] = L'\0';

	for(int i = 0; addresses[i] && addresses[i][0] != L'\0'; i++)
	{
		wcscat_s(outStr, MAX_EMAIL_FIELD, BuildNameAddressStr(nameAddr, names[i], addresses[i]));
		wcscat_s(outStr, MAX_EMAIL_FIELD, L", ");
	}
	outStr[wcslen(outStr) - 2] = L'\0';

	return outStr;
}

void ParseEmailChatMessage(MailMessageW *msg)
{
	ChatMessage chatMsg;
	int ret;
	TCHAR *replyFromName = NULL, replyToName[MAX_SETTING];
	TCHAR *tempName, *tempAddress;
	int tempIndex = -1;
	TCHAR *recAddresses[2], *recNames[2];
	SessionInfo *session;
	int sessionIndex;
		
	memset(&chatMsg, 0, sizeof(ChatMessage));

	if(!ParseEmailChatMessageText(msg->text, msg->textsize, &chatMsg))
		return;

	wcscpy_s(replyToName, MAX_SETTING, chatMsg.from);
	BuildNameAddressList(chatMsg.to, msg->to_names, msg->to_addresses);
	BuildNameAddressStr(chatMsg.from, replyToName, msg->from_address);

	if(session = FindSessionByID(chatMsg.sessionID))
	{
		if( (chatMsg.broadcast && !NumSessionWideChatEmails)
				|| (!chatMsg.broadcast && !NumPrivateChatEmails) )
			goto END;

		sessionIndex = LL_GetItemIndex(&llSessions, session);
		
		if(ChatSoundEnabled && !settings->disableSounds && !settings->disableChatSound && (IsGameWindowForeground() || !IsFGWFullScreen()))
		{
			PlaySound(SOUND_MESSAGE, hInst, SND_RESOURCE | SND_ASYNC);
			ChatSoundEnabled = FALSE;
		}
		
		ret = DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_RECEIVE_MESSAGE), NULL, ReceiveMessageDialogProc, (LPARAM)&chatMsg);
	
		for(int i = 0; msg->to_addresses[i]; i++)
		{
			if(!_wcsicmp(mail->email, msg->to_addresses[i]))
			{
				replyFromName = msg->to_names[i];
				break;
			}
		}

		switch(ret)
		{
			case IDC_REPLYALL_BUTTON:
				EditChatMessage(sessionIndex, replyFromName, NULL, NULL, NULL);
				break;
			case IDC_REPLYREC_BUTTON:
				for(int i = 0; msg->to_addresses[i]; i++)
				{
					if(!_wcsicmp(mail->email, msg->to_addresses[i]))
					{
						// Preserve the old strings so libetpan_helper can free() them later.
						tempAddress = msg->to_addresses[i];
						tempName = msg->to_names[i];
						tempIndex = i;

						msg->to_addresses[i] = msg->from_address;
						msg->to_names[i] = replyToName;

						break;
					}
				}
				EditChatMessage(sessionIndex, replyFromName, NULL, msg->to_addresses, msg->to_names);
				// Restore the old strings so libetpan_helper can free() them.
				if(tempIndex != -1)
				{
					msg->to_addresses[tempIndex] = tempAddress;
					msg->to_names[tempIndex] = tempName;
				}
				break;
			case IDC_REPLYSENDER_BUTTON:
				recNames[0] = replyToName;
				recNames[1] = NULL;
				recAddresses[0] = msg->from_address;
				recAddresses[1] = NULL;
				EditChatMessage(sessionIndex, replyFromName, NULL, recAddresses, recNames);
				break;
			default:
				break;
		}
	}
END:
	free(chatMsg.message);
}

TCHAR *getYourFactionList(SessionInfo *session)
{
	int *factionTable, factionTableSize, yourFactionListSize;
	TCHAR factionCount[128], *yourFactionList;

	factionTableSize = sizeof(int) * (session->NUM_FACTIONS + 1);
	factionTable = (int *)malloc(factionTableSize);
	memset(factionTable, 0, factionTableSize);
	
	yourFactionListSize = MAX_SETTING * (session->NUM_FACTIONS + 1);
	yourFactionList = (TCHAR *)malloc(yourFactionListSize);
	yourFactionList[0] = L'\0';

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(isYourPlayer(session, i))
			factionTable[session->players[i]->faction + 1]++;
	}
	
	for(int i = 0; i < session->NUM_FACTIONS + 1; i++)
	{
		if(factionTable[i])
		{
			wcscat_s(yourFactionList, yourFactionListSize, getFactionNameFromIndex(session, i - 1));
			if(factionTable[i] > 1)
			{
				swprintf(factionCount, sizeof(factionCount), L" (%d players)", factionTable[i]); 
				wcscat_s(yourFactionList, yourFactionListSize, factionCount);
			}	
			wcscat_s(yourFactionList, yourFactionListSize, L", ");
		}
	}
	yourFactionList[wcslen(yourFactionList) - 2] = L'\0';
	
	free(factionTable);

	return yourFactionList;
}

void ParseEmailStatusUpdate(MailMessageW *msg, BOOL yourTurn)
{
	SessionInfo *session, *emailSession;
	TCHAR *yourFactionList;
	
	IsNewSession = FALSE;

	if(!(emailSession = ParseEmailSessionText(msg->text, msg->textsize)))
		return;

	session = FindSessionByID(emailSession->sessionID);	
	if(session)
	{
		if(yourTurn)
		{		
			if(msg->filedata == NULL)
				emailSession->actingCurrentPlayer = TRUE;

			if(!emailSession->actingCurrentPlayer)
			{
				ApplyGlobalNamesToSession(session, getCurrentPlayerEmail(emailSession), NULL);
				ApplyGlobalNamesToSession(emailSession, getCurrentPlayerEmail(emailSession), NULL);
			}

			if(emailSession->turnNumber <= session->turnNumber)
			{
				if(AlertSoundEnabled && !settings->disableSounds && !settings->disableYourTurnSound && (IsGameWindowForeground() || !IsFGWFullScreen())) 
				{
					PlaySound(SOUND_ALERT, hInst, SND_RESOURCE | SND_ASYNC);
					AlertSoundEnabled = FALSE;
				}
			
				swprintf(mbBuffer, MBBUFFER_SIZE, L"A player has rolled-back the game \"%s\" to an earlier turn. This could be due to the player list being altered.\n\n(Note: If you are currently having your turn, then you will need to re-load the save file and start over)", session->sessionName);
				MessageBoxS(NULL, mbBuffer, L"Game rolled-back", MB_OK);
				GameInProgress = FALSE;
			}
		}
		
		UpdateSessionFromEmail(session, emailSession);
	}
	else
	{
		if(emailSession->state & SESSION_DELETED) 
			goto END;

		IsNewSession = TRUE;

		if(yourTurn)
			ApplyGlobalNamesToSession(emailSession, getCurrentPlayerEmail(emailSession), NULL);
		else if(-1 == FindPlayer(emailSession, mail->email, NULL, -1))
			goto END;

		if(AlertSoundEnabled && !settings->disableSounds && !settings->disableYourTurnSound && (IsGameWindowForeground() || !IsFGWFullScreen())) 
		{
			PlaySound(SOUND_ALERT, hInst, SND_RESOURCE | SND_ASYNC);
			AlertSoundEnabled = FALSE;
		}

		yourFactionList = getYourFactionList(emailSession);
		swprintf(mbBuffer, MBBUFFER_SIZE, L"You have received an invitation from %s to join a game of %s named \"%s\", starting from turn %d.\n\nYour assigned Teams are:  %s.\n\nWould you like to accept this invitation?", msg->from_address, emailSession->ggSettings->gameID, emailSession->sessionName, getRoundNumber(emailSession), yourFactionList);
		free(yourFactionList);
		
		emailSession->state = 0;
		ClearAllPlayerFlags(emailSession);
		ClearAllTeamFlags(emailSession);
		
		if(IDNO == MessageBoxS(NULL, mbBuffer, L"New Game Request", MB_YESNO | MB_ICONQUESTION))
		{
			DeleteYourPlayers(emailSession);
			goto END;
		}

		if(!(session = CreateSessionFromEmail(emailSession)))
			goto END;
	}

	if(IsNewSession)
	{
		SaveSessionList();
		RequestPlayerNames(session);
		DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_SESSION_SETTINGS), NULL, SessionSettingsDialogProc, numSessions - 1);
	}

	if(yourTurn)
	{
		if(session->actingCurrentPlayer)
		{
			if(!CheckInternalSaveFileExists(session))
			{
				session->currentPlayer = 0;
				session->turnNumber = 0;
				session->actingCurrentPlayer = FALSE;
				CheckYourTurnEnabled = TRUE;	
			}
			else if(!session->selectingTeams)
				SendEmailYourTurn(session);
		}
		else
		{
			if(session->turnNumber > 1)
				WriteSaveFile(msg->filedata, msg->filesize, session->sessionID);
			
			CheckYourTurnEnabled = TRUE;	
			session->recent = TRUE;
		}
	}

	if(session->selectingTeams)
	{
		SaveSessionList();
		RequestTeamSettings(session);
	}
	
	CheckNewGame(session); 

	SaveSessionList();
	ListView_Update(hMainListView, LL_GetItemIndex(&llSessions, session));

END:
	IsNewSession = FALSE;
	FreeSession(emailSession);
}

BOOL CheckNewGame(SessionInfo *session)
{
	BOOL ret = FALSE;

	if(session->turnNumber == 0 && !session->selectingTeams)
	{
		ret = TRUE;
		
		if(isYourTurn(session)) 
		{
			session->turnNumber++;	
			session->state |= SESSION_MODIFY_TURNNUMBER;
			session->state |= SESSION_MODIFY_CURRENTPLAYER;
			session->recent = TRUE;
			CheckYourTurnEnabled = TRUE;
		}
	}

	if(session->state)
		SendEmailStatusUpdate(session, FALSE, FALSE);
	
	return ret;
}

void CheckYourTurn()
{
	LinkedList *iter;
	SessionInfo *session;

	if(!CheckYourTurnEnabled)
		return;

	iter = &llSessions;

	while(iter->next)
	{
		iter = iter->next;

		session = (SessionInfo *)iter->item;

		if(session->selectingTeams)
		{
			RequestTeamSettings(session);
			CheckNewGame(session);
		}
		
		if(GameInProgress)
			continue;
		
		if(session->recent == FALSE || session->selectingTeams || (!isYourTurn(session) && !session->actingCurrentPlayer))
			continue;

		if(AlertSoundEnabled && !settings->disableSounds && !settings->disableYourTurnSound && (IsGameWindowForeground() || !IsFGWFullScreen()))
		{
			PlaySound(SOUND_ALERT, hInst, SND_RESOURCE | SND_ASYNC);
			AlertSoundEnabled = FALSE;
		}
	
		if(LoadGameRequest(session, TRUE)) break;
	}

	CheckYourTurnEnabled = FALSE;
	AlertSoundEnabled = TRUE;
}

BOOL RunGameRequest(SessionInfo *session, BOOL yourTurn)
{
	session->recent = FALSE;
	LoadGameYourTurn = yourTurn;
	int ret;
	TCHAR *gameRunCommand = session->ggSettings->runCommand;

	if((!isYourTurn(session) && !session->actingCurrentPlayer) || session->selectingTeams)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Unable to load new game \"%s.\" because it is not your turn.", session->sessionName);
		MessageBoxS(NULL, mbBuffer, L"Cannot load game", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	selectSession(LL_GetItemIndex(&llSessions, session));

	if((!session->ggSettings->KillBeforeRunGame && session->ggSettings->FindGameWindow()) || gameRunCommand[0] != L'\0')
	{
		ret = DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_RUN_GAME), NULL, RunGameDialogProc, (LPARAM)session);
		if(ret == IDC_RUNNOW_BUTTON) 
			return _NewGame(session);
		else if(ret == IDCANCEL)
			return FALSE;
	}
	else
		return _NewGame(session);
	
	SetTimer(hMainWnd, GAME_IN_PROGRESS_TIMER, GAME_IN_PROGRESS_INTERVAL, NULL);

	return TRUE;
}

BOOL LoadGameRequest(SessionInfo *session, BOOL yourTurn)
{
	int ret;
	TCHAR *gameRunCommand = session->ggSettings->runCommand;

	if((session->turnNumber == 1 || session->selectingTeams) && !CheckInternalSaveFileExists(session))  
		return RunGameRequest(session, yourTurn);

	session->recent = FALSE;

	LoadGameYourTurn = yourTurn;

	if((!session->ggSettings->KillBeforeLoadGame && session->ggSettings->FindGameWindow()) || gameRunCommand[0] != L'\0')
	{	
		ret = DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_LOAD_GAME), NULL, LoadGameDialogProc, (LPARAM)session);
			
		if(ret == IDC_LOADNOW_BUTTON)
		{
			selectSession(LL_GetItemIndex(&llSessions, session));
			_LoadGame(session, TRUE);			
		}
		else if(ret == IDC_EXPORT_BUTTON)
		{
			selectSession(LL_GetItemIndex(&llSessions, session));
			if(session->ggSettings->KillBeforeLoadGame) 
				session->ggSettings->KillGame();
			ExportSaveFile(session, FALSE);
		}
		else return FALSE;
	}
	else 
	{	
		if(yourTurn)
		{
			ret = DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_EXPORT_GAME), NULL, LoadGameDialogProc, (LPARAM)session);
			
			if(ret == IDCANCEL) return FALSE;
		}

		selectSession(LL_GetItemIndex(&llSessions, session));
		if(session->ggSettings->KillBeforeLoadGame) 
			session->ggSettings->KillGame();
		ExportSaveFile(session, FALSE);
		
		if(isYourTurn(session))
			GameInProgress = TRUE;
	}

	SetTimer(hMainWnd, GAME_IN_PROGRESS_TIMER, GAME_IN_PROGRESS_INTERVAL, NULL);
	return TRUE;
}

void ParseEmail(MailMessageW *msg)
{
	// Check this email was sent by PlayMailer.
	if(wcscmp(msg->from_name, PLAYMAILER_FROM_NAME)) 
		return;

	if(!wcscmp(msg->subject, SUBJECT_YOUR_TURN))
		ParseEmailStatusUpdate(msg, TRUE);
	else if(!wcscmp(msg->subject, SUBJECT_STATUS_UPDATE))
		ParseEmailStatusUpdate(msg, FALSE);
	else if(!wcscmp(msg->subject, SUBJECT_CHAT_MESSAGE))
		ParseEmailChatMessage(msg);
}

int MessageBoxS(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	int ret;
		
	HotKeysEnabled = FALSE;
	SuspendResumeGame(TRUE);
	SetWindowsHookEx(WH_CBT, DisableMainWndProc, NULL, GetCurrentThreadId());
	ret = MessageBox(hWnd, lpText, lpCaption, uType | MB_SETFOREGROUND | MB_TOPMOST);
	UnhookWindowsHookEx(hMsgBoxHook);
	SuspendResumeGame(FALSE);
	HotKeysEnabled = TRUE;

	return ret;
}

LRESULT CALLBACK DisableMainWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd;
 
    if(nCode < 0)
        return CallNextHookEx(hMsgBoxHook, nCode, wParam, lParam);
 
    switch(nCode)
    {
    case HCBT_CREATEWND:
        hwnd = (HWND)wParam;
		EnableWindow(hMainWnd, FALSE);

		return 0;
	case HCBT_DESTROYWND:
		EnableWindow(hMainWnd, TRUE);
		
		return 0;
    }
    
    return CallNextHookEx(hMsgBoxHook, nCode, wParam, lParam);
}

INT_PTR DialogBoxParamS(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	INT_PTR ret;
	HotKeysEnabled = FALSE;
	
	SuspendResumeGame(TRUE);

	ret = DialogBoxParam(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
	
	hCurrentDialog = NULL;
	KillTimer(hMainWnd, CHECK_TOPMOST_TIMER);
	
	SuspendResumeGame(FALSE);
	HotKeysEnabled = TRUE;

	return ret;
}

BOOL WriteSaveFile(const char *fileData, uint32_t fileSize, uint32_t fileID)
{
	TCHAR filePath[MAX_PATH];
	FILE *out;

	_wmkdir(INTERNAL_SAVE_FOLDER);

	swprintf(filePath, MAX_PATH, L"%s\\%u.SAV", INTERNAL_SAVE_FOLDER, fileID);
	if(_wfopen_s(&out, filePath, L"wb"))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Unable to open %s for writing", filePath);
		MessageBoxS(NULL, mbBuffer, L"File I/O Error", MB_OK);
		return FALSE;
	}

	if(fileSize != fwrite(fileData, 1, fileSize, out))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error writing to file: %s", filePath);
		MessageBoxS(NULL, mbBuffer, L"File I/O Error", MB_OK);
		fclose(out);
		return FALSE;
	}

	fclose(out);
	return TRUE;
}

void DeleteSaveFile(uint32_t fileID)
{
	TCHAR filePath[MAX_PATH];

	swprintf(filePath, MAX_PATH, L"%s\\%u.SAV", INTERNAL_SAVE_FOLDER, fileID);
	DeleteFile(filePath);
}

int getRoundNumber(SessionInfo *session)
{
	int numPlayers = getNumPlayers(session);

	return (session->turnNumber + numPlayers) / numPlayers; 
}

int getNumPlayers(SessionInfo *session)
{
	return session->numPlayers;
}

void UpdateSessionFromEmail(SessionInfo *session, SessionInfo *emailSession)
{
	if(emailSession->actingCurrentPlayer) 
		session->actingCurrentPlayer = TRUE;
	
	session->selectingTeams = emailSession->selectingTeams;
	
	session->pGameSettings->gameID = emailSession->pGameSettings->gameID;
	session->pGameSettings->ggSettings = session->ggSettings = emailSession->ggSettings;
	
	if(IsNewSession || emailSession->state & SESSION_MODIFY_TURNNUMBER)
		session->turnNumber = emailSession->turnNumber;

	if(IsNewSession || emailSession->state & SESSION_MODIFY_GAMESETTINGS)
		session->UpdateEmailGameSettings(emailSession);
		
	if(IsNewSession || emailSession->state & (SESSION_MODIFY_PLAYERLIST | SESSION_MODIFY_CURRENTPLAYER))
		UpdatePlayerList(session, emailSession);

	if(IsNewSession || emailSession->state & SESSION_MODIFY_TEAMSETTINGS)
		UpdateTeamSettings(session, emailSession);

	if(session->turnNumber != 1)
		session->SetGameSettingsMask();
}

void UpdateTeamSettings(SessionInfo *out, SessionInfo *in)
{
	LinkedList *iter;
	Team *inTeam, *outTeam;

	if(IsNewSession)
	{
		CopyTeams(out, in);
		return;
	}

	iter = in->teams;

	while(iter->next)
	{
		iter = iter->next;
		inTeam = (Team *)iter->item;

		if(!(outTeam = FindTeam(out, inTeam->id)))
		{
			outTeam = CreateTeam(out);
			outTeam->id = inTeam->id;
			outTeam->state = 0;
		}

		outTeam->faction = inTeam->faction;
		out->UpdateEmailTeamSettings(outTeam, inTeam);
	}
}

void UpdatePlayerList(SessionInfo *out, SessionInfo *in)
{
	int outPlayerIndex;
	Player *inPlayer, *outPlayer;
	BOOL teamsChanged = FALSE;

	if(IsNewSession)
	{
		CopyPlayers(out, in);
		out->currentPlayer = in->currentPlayer;
		return;
	}

	YourPlayerNumber = CountYourPlayers(in) + 1;

	for(int i = TOTAL_PLAYERS(in) - 1; i >= 0; i--)
	{
		inPlayer = in->players[i];

		if(isYourPlayer(in, i))
			YourPlayerNumber--;

		outPlayerIndex = FindPlayerByID(out, inPlayer->id);

		if(outPlayerIndex == -1)
		{
			if(inPlayer->state & PLAYER_ADD)
			{
				outPlayerIndex = out->numPlayers;

				outPlayer = CreatePlayer(out);
				memcpy(outPlayer, inPlayer, sizeof(Player));
				outPlayer->state = 0;

				ReportPlayerChange(out, outPlayerIndex, inPlayer);
			}
		}

		if(outPlayerIndex != -1)
		{
			outPlayer = out->players[outPlayerIndex];

			if(inPlayer->state & PLAYER_DELETE)
			{
				ReportPlayerChange(out, outPlayerIndex, inPlayer);
				// Setting ADD flag, causes player to be removed straight away rather than having delete flag set.
				out->players[outPlayerIndex]->state |= PLAYER_ADD;
				DeletePlayer(out, outPlayerIndex);
				teamsChanged = TRUE;
				continue;
			}

			if(inPlayer->state & PLAYER_MODIFY_EMAIL)
			{
				wcscpy_s(outPlayer->email, MAX_SETTING, inPlayer->email);
			}
			if(inPlayer->state & PLAYER_MODIFY_NAME)
			{
				wcscpy_s(outPlayer->name, MAX_SETTING, inPlayer->name);
			}
			if(inPlayer->state & PLAYER_MODIFY_FACTION)
			{
				ReportPlayerChange(out, outPlayerIndex, inPlayer);
				outPlayer->faction = inPlayer->faction;
				outPlayer->factionSortKey = inPlayer->factionSortKey;
				teamsChanged = TRUE;
			}

			if(in->state & SESSION_MODIFY_CURRENTPLAYER && i == in->currentPlayer)
				out->currentPlayer = outPlayerIndex;

			if(inPlayer->state & PLAYER_TEAM_TOKEN)
				SetTeamToken(out, outPlayerIndex);
		}
	}

	SortPlayerList(out);
	if(teamsChanged)
		PruneTeams(out);
	AssignTeams(out);
}

int CountYourPlayers(SessionInfo *session)
{
	int numPlayers = 0;

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(isYourPlayer(session, i))
			numPlayers++;
	}

	return numPlayers;
}

void ClearPlayerFlags(SessionInfo *session, int flags)
{
	for(int i = 0; i < session->numPlayers; i++)
		session->players[i]->state &= ~flags;
}

void ClearAllPlayerFlags(SessionInfo *session)
{
	for(int i = 0; i < session->numPlayers; i++)
		session->players[i]->state = 0;
}

void ClearAllTeamFlags(SessionInfo *session)
{
	LinkedList *iter;
	Team *team;

	iter = session->teams;

	while(iter->next)
	{
		iter = iter->next;
		team = (Team *)iter->item;

		team->state = 0;
	}
}

void FreeDeletedPlayers(SessionInfo *session)
{
	for(int i = session->numPlayers; i < TOTAL_PLAYERS(session); i++)
	{
		free(session->players[i]);
		session->players[i] = NULL;
	}

	session->numDeletedPlayers = 0;
}

int FindSessionIndexByID(uint32_t id)
{
	LinkedList *iter;
	SessionInfo *currSession;
	int index = 0;

	iter = &llSessions;

	while(iter->next)
	{
		iter = iter->next;

		currSession = (SessionInfo *)iter->item;
		if(currSession->sessionID == id)
			return index;

		index++;
	}

	return -1;
}

SessionInfo *FindSessionByID(uint32_t id)
{
	LinkedList *iter;
	SessionInfo *currSession;

	iter = &llSessions;

	while(iter->next)
	{
		iter = iter->next;

		currSession = (SessionInfo *)iter->item;
		if(currSession->sessionID == id)
			return currSession;
	}

	return NULL;
}

SessionInfo *FindSessionByGameID(TCHAR *gameID)
{
	LinkedList *iter;
	SessionInfo *currSession;

	iter = &llSessions;

	while(iter->next)
	{
		iter = iter->next;

		currSession = (SessionInfo *)iter->item;
		if(!_wcsicmp(currSession->ggSettings->gameID, gameID))
			return currSession;
	}

	return NULL;
}

TCHAR *GetUniqueOutgoingEmailPath(TCHAR *filePath)
{
	HANDLE hFile;
	WIN32_FIND_DATA fileData;
	uint32_t highestFileNumber = 0, fileNumber;
	int len;

	_wmkdir(OUTGOING_EMAIL_FOLDER);

	DisableWow64Redirection(TRUE);

	swprintf(filePath, MAX_PATH, L"%s\\*.eml", OUTGOING_EMAIL_FOLDER);
	if((hFile = FindFirstFile(filePath, &fileData)) != INVALID_HANDLE_VALUE)
	{
		do 
		{
			len = wcscspn(fileData.cFileName, L".");
			wcsncpy_s(filePath, MAX_PATH, fileData.cFileName, len);
			filePath[len] = L'\0';
			fileNumber = _wtoi(filePath);
			if(fileNumber > highestFileNumber)
				highestFileNumber = fileNumber;
		}
		while(FindNextFile(hFile, &fileData));
	}  
	FindClose(hFile);
	DisableWow64Redirection(FALSE);

	swprintf(filePath, MAX_PATH, L"%s\\%u.eml", OUTGOING_EMAIL_FOLDER, highestFileNumber+1);

	return filePath;
}

void CleanEmailFolder()
{
	HANDLE hFile;
	WIN32_FIND_DATA fileData;
	TCHAR filePath[MAX_PATH];

	DisableWow64Redirection(TRUE);

	swprintf(filePath, MAX_PATH, L"%s\\*.eml", INCOMING_EMAIL_FOLDER);
	if((hFile = FindFirstFile(filePath, &fileData)) == INVALID_HANDLE_VALUE)
	{
		DisableWow64Redirection(FALSE);
		RemoveDirectory(INCOMING_EMAIL_FOLDER);	
		return;
	}

	do 
	{
		swprintf(filePath, MAX_PATH, L"%s\\%s", INCOMING_EMAIL_FOLDER, fileData.cFileName);
		DeleteFile(filePath);
	}
	while(FindNextFile(hFile, &fileData));
    
	FindClose(hFile);
	DisableWow64Redirection(FALSE);
	RemoveDirectory(INCOMING_EMAIL_FOLDER);
}

int *GetEmailNumbers(TCHAR *filePath)
{
	HANDLE hFile;
	WIN32_FIND_DATA fileData;
	int *emailNumbers, numEmails = 0, i = 0;

	DisableWow64Redirection(TRUE);

	if((hFile = FindFirstFile(filePath, &fileData)) == INVALID_HANDLE_VALUE)
	{
		DisableWow64Redirection(FALSE);
		return NULL;
	}
		
	do 
	{	
		numEmails++;
	}
	while(FindNextFile(hFile, &fileData));
	FindClose(hFile);

	emailNumbers = (int *)malloc(sizeof(int) * (numEmails + 1));

	if((hFile = FindFirstFile(filePath, &fileData)) == INVALID_HANDLE_VALUE)
	{
		DisableWow64Redirection(FALSE);
		return NULL;
	}
	
	do 
	{	
		fileData.cFileName[wcscspn(fileData.cFileName, L".")] = L'\0';
		emailNumbers[i++] = _wtoi(fileData.cFileName);
	}
	while(i < numEmails && FindNextFile(hFile, &fileData));
	FindClose(hFile);
	DisableWow64Redirection(FALSE);

	emailNumbers[numEmails] = -1;
	
	QuickSortInt(emailNumbers, numEmails);
	return emailNumbers;
}

void ParseEmails()
{
	MailMessageW msg;
	TCHAR filePath[MAX_PATH];
	int *emailNumbers;
	ChatMessage chatMsg;

	if(!ParseMailEnabled)
		return;
	ParseMailEnabled = FALSE;

	if(!MailCheckedFirstTime)
	{
		SetForegroundWindow(hMainWnd);
		DestroyWindow(hFetchMailDialog);
	}

	ChatSoundEnabled = TRUE;

	NumSessionWideChatEmails = 0;
	NumPrivateChatEmails = 0;

	swprintf(filePath, MAX_PATH, L"%s\\*.eml", INCOMING_EMAIL_FOLDER);
	if(!(emailNumbers = GetEmailNumbers(filePath)))
	{
		//RemoveDirectory(INCOMING_EMAIL_FOLDER);	
		goto END;
	}

	// parse status emails and count chat emails
	for(int i = 0; emailNumbers[i] != -1; i++)
	{
		swprintf(filePath, MAX_PATH, L"%s\\%u.eml", INCOMING_EMAIL_FOLDER, emailNumbers[i]);
		if(!load_msg_w(filePath, &msg))
		{		
			if(wcscmp(msg.subject, SUBJECT_CHAT_MESSAGE))
			{
				ParseEmail(&msg);		
				DeleteFile(filePath);
				emailNumbers[i] = -2;
			}
			else if(!settings->muteAllChat)
			{
				if(!wcscmp(msg.from_name, PLAYMAILER_FROM_NAME)) 
				{
					if(ParseEmailChatMessageText(msg.text, msg.textsize, &chatMsg))
					{
						if(FindSessionByID(chatMsg.sessionID))
						{
							if(chatMsg.broadcast)
								NumSessionWideChatEmails++;
							else
								NumPrivateChatEmails++;
						}
						free(chatMsg.message);
					}			
				}
			}
			free_msg_w(&msg);
		}
		else
		{
			//DeleteFile(filePath);
			emailNumbers[i] = -2;
		}
	}
	
	if(settings->muteSessionWideChat)
		NumSessionWideChatEmails = 0;
	
	// parse chat emails
	for(int i = 0; emailNumbers[i] != -1; i++)
	{
		if(emailNumbers[i] == -2) 
			continue;
		
		swprintf(filePath, MAX_PATH, L"%s\\%u.eml", INCOMING_EMAIL_FOLDER, emailNumbers[i]);
		if(!load_msg_w(filePath, &msg))
		{	
			if(!wcscmp(msg.subject, SUBJECT_CHAT_MESSAGE))
				ParseEmail(&msg);		
			free_msg_w(&msg);
			DeleteFile(filePath);
		}
	}

	free(emailNumbers);

END:
	if(!MailCheckedFirstTime)
	{
		MailCheckedFirstTime = TRUE;

		CheckAllPlayerTeams();
		MailCheckedFirstTime = TRUE;		
		HotKeysEnabled = TRUE;
		CheckYourTurnEnabled = TRUE;
	}
}

void SortPlayerList(SessionInfo *session)
{
	int *factionTable, factionTableSize;
	int skip, factionsRemaining;

	factionTableSize = sizeof(int) * session->NUM_FACTIONS;
	factionTable = (int *)malloc(factionTableSize);
	memset(factionTable, -1, factionTableSize);
	
	if(session->turnNumber > 0 && session->currentPlayer != -1)	
		session->players[session->currentPlayer]->state |= PLAYER_CURRENT;

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->faction == FACTION_RANDOM)
			session->players[i]->faction = AssignRandomFaction(session);
	}

	// clear teams for deleted players
	for(int i = session->numPlayers; i < TOTAL_PLAYERS(session); i++)
		session->players[i]->factionSortKey = session->players[i]->faction = INT_MAX;
		
	// Sort player list by factions
	if(session->ggSettings->RandomFactionOrder)
	{
		factionsRemaining = session->NUM_FACTIONS;
		
		for(int i = 0; i < session->numPlayers; i++)
		{
			if(session->players[i]->factionSortKey != -1 && session->players[i]->faction >= 0)
			{
				if(factionTable[session->players[i]->faction] == -1)
					factionsRemaining--;

				factionTable[session->players[i]->faction] = session->players[i]->factionSortKey;
			}
		}

		for(int i = 0; factionsRemaining; i++)
		{
			skip = rand() % factionsRemaining;
			for(int j = 0; skip > -1; j++)
			{
				if(factionTable[j] == -1)
					skip--;

				if(skip == -1)
					factionTable[j] = i;				
			}

			factionsRemaining--;
		}
	}

	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->ggSettings->RandomFactionOrder && session->players[i]->faction >= 0)
			session->players[i]->factionSortKey = factionTable[session->players[i]->faction];
		else
			session->players[i]->factionSortKey = session->players[i]->faction;
	}	

	for(int i = 0; i < TOTAL_PLAYERS(session); i++)
		session->players[i]->playerSortKey = i;

	qsort(session->players, TOTAL_PLAYERS(session), sizeof(Player *), ComparePlayerFactions);

	// Set current player again
	if(session->turnNumber > 0 && session->currentPlayer != -1)
	{
		for(int i = 0; i < session->numPlayers; i++)
		{
			if(session->players[i]->state & PLAYER_CURRENT)
			{
				session->currentPlayer = i;
				session->players[i]->state &= ~PLAYER_CURRENT;
				break;
			}
		}
	}

	free(factionTable);

	for(int i = 0; i < session->numPlayers; i++)
		AssignTeam(session, i);
}

int AssignRandomFaction(SessionInfo *session)
{
	int smallestNumPlayers, numSmallestFactions = 0, randFaction;
	int *factionTable, factionTableSize;
	int ret = 0;

	factionTableSize = sizeof(int) * session->NUM_FACTIONS;
	factionTable = (int *)malloc(factionTableSize);
	
	memset(factionTable, 0, factionTableSize);
	
	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->faction >= 0)
			factionTable[session->players[i]->faction]++;
	}

	// Maximum teams has been reached. Make the remaining teams off limits.
	if(session->MAX_TEAMS == GetNumFactions(session))
	{
		for(int i = 0; i < session->NUM_FACTIONS; i++)
		{
			if(factionTable[i] == 0)
				factionTable[i] = INT_MAX;
		}
	}

	// Count the number of equally smallest teams
	smallestNumPlayers = INT_MAX;
	for(int j = 0; j < session->NUM_FACTIONS; j++)
	{
		if(factionTable[j] < smallestNumPlayers)
		{
			smallestNumPlayers = factionTable[j];
			numSmallestFactions = 1;
		}
		else if(factionTable[j] == smallestNumPlayers)
			numSmallestFactions++;
	}	
			
	randFaction = rand() % numSmallestFactions + 1;
	for(int j = 0; j < session->NUM_FACTIONS; j++)
	{
		if(factionTable[j] == smallestNumPlayers)
			randFaction--;
		if(!randFaction) 
		{
			ret = j;
			break;
		}
	}

	free(factionTable);
	return ret;
}

int ComparePlayerFactions(const void *x, const void *y)
{
	int xElement, yElement;
	Player *xP, *yP;

	xP = *(Player **)x;
	yP = *(Player **)y;

	if(xP->factionSortKey < 0) 
		xElement = INT_MAX + xP->factionSortKey;
	else 
		xElement = xP->factionSortKey;

	if(yP->faction < 0) 
		yElement = INT_MAX + yP->factionSortKey;
	else 
		yElement = yP->factionSortKey;
	
	if(xElement == yElement)
		return xP->playerSortKey - yP->playerSortKey;

	return xElement - yElement;
}

int FindPlayerByID(SessionInfo *session, int id)
{
	for(int i = 0; i < session->numPlayers; i++)
	{
		if(session->players[i]->id == id)
			return i;
	}

	return -1;
}

DWORD WINAPI RecvEmailThreadProc(LPVOID lpParam)
{
	DWORD result, timeout = INFINITE;

	while(1)
	{
		result = WaitForMultipleObjects(NUM_RECVEMAIL_EVENTS, hRecvEmailEvents, FALSE, timeout);

		// create timer event

		switch(result)
		{
		case WAIT_OBJECT_0 + EVENT_RECVEMAIL_IMAPSOCKET:	
		case WAIT_OBJECT_0 + EVENT_RECVEMAIL_CHECK:
		case WAIT_TIMEOUT:	
			if(IMAPSocket >= 0)
				WSAEventSelect(IMAPSocket, hRecvEmailEvents[EVENT_RECVEMAIL_IMAPSOCKET], 0);

			do
			{
				if(CheckMail())
				{
					FailedFetches = 0;

					if(mail->inMailProtocol == PROTOCOL_IMAP && IMAPSocket >= 0)
						WSAEventSelect(IMAPSocket, hRecvEmailEvents[EVENT_RECVEMAIL_IMAPSOCKET], FD_READ | FD_CLOSE);	
				}	
				else
				{
					FailedFetches++;
					break;
				}
			} while(IMAPSocket == -2);

			PostMessage(hMainWnd, WM_PARSE_MAIL, 0, 0);	
			timeout = GetIncomingTimerInterval();
			
			break;
		case WAIT_OBJECT_0 + EVENT_RECVEMAIL_LOGOUT:
		case WAIT_OBJECT_0 + EVENT_RECVEMAIL_QUIT:
			imap_idle_done();
			imap_logout();
			IMAPSocket = -1;
			FailedFetches = 0;
			
			if(result == WAIT_OBJECT_0 + EVENT_RECVEMAIL_QUIT)
				return 0;
			
			break;
		default:
			return GetLastError();
		}
	}
}

int GetIncomingTimerInterval()
{
	if(FailedFetches >= 5)
		return LONG_POLLING_INTERVAL;
	else if(IMAPSocket != -1)
		return IMAP_IDLE_INTERVAL;
	else if(mail->checkMailInterval) 
		return mail->checkMailInterval * SECONDS;
	else 
		return (mail->inMailProtocol == PROTOCOL_IMAP ? IMAP_POLLING_INTERVAL : POP3_POLLING_INTERVAL);
}

DWORD WINAPI SendEmailThreadProc(LPVOID lpParam)
{
	DWORD result, timeout = INFINITE, failedSends, quiet = FALSE;

	while(1)
	{
		result = WaitForMultipleObjects(NUM_SENDEMAIL_EVENTS, hSendEmailEvents, FALSE, timeout);

		switch(result)
		{
		case WAIT_OBJECT_0 + EVENT_SENDEMAIL_CHECK:
			failedSends = 0;	
			quiet = FALSE;
		case WAIT_TIMEOUT:		
			if(!IsInternetConnected())
			{
				timeout = OUTGOING_TIMER_INTERVAL;
				failedSends = 0;
				break;
			}

			if(CheckOutgoingMail(quiet))
			{
				failedSends++;

				if(failedSends >= MAX_FAILED_SENDS)
					timeout = LONG_OUTGOING_TIMER_INTERVAL;
				else 
					timeout = OUTGOING_TIMER_INTERVAL;
			}
			else
			{
				failedSends = 0;
				timeout = INFINITE;
			}
			quiet = TRUE;
			break;
		case WAIT_OBJECT_0 + EVENT_SENDEMAIL_QUIT:
			return 0;
		default:
			return GetLastError();
		}
	}
}

int CheckOutgoingMail(BOOL quiet)
{
	TCHAR filePath[MAX_PATH];
	FILE *email = NULL;
	MailMessageW msg;
	int ret = 0, *emailNumbers;

	swprintf(filePath, MAX_PATH, L"%s\\*.eml", OUTGOING_EMAIL_FOLDER);
	if(!(emailNumbers = GetEmailNumbers(filePath)))
	{
		//RemoveDirectory(OUTGOING_EMAIL_FOLDER);	
		return 0;
	}

	for(int i = 0; emailNumbers[i] != -1; i++)
	{
		swprintf(filePath, MAX_PATH, L"%s\\%u.eml", OUTGOING_EMAIL_FOLDER, emailNumbers[i]);

		if(load_msg_w(filePath, &msg))
			continue;

		if(_wfopen_s(&email, filePath, L"r+b"))
		{
			free_msg_w(&msg);
			continue;
		}

		ret = SendEmail(email, msg.to_addresses, quiet);
		
		free_msg_w(&msg);
		fclose(email);
		
		if(!ret) 
			DeleteFile(filePath);
		else
			goto END;
	}

END:
	free(emailNumbers);
	if(!ret) 
		RemoveDirectory(OUTGOING_EMAIL_FOLDER);
	
	return ret;
}

void ViewHotkeysHelp()
{
	MessageBoxS(NULL, L"\
Use these hotkeys from within the game window, e.g. in DOSBox.\n\n\
Ctrl+Shift+ S - Save the game and send it to the next player.\n\
       REMEMBER THIS HOTKEY. When you have finished your turn,\n\
       use this hotkey INSTEAD of selecting 'End Turn'. The game\n\
       will be sent to the next player.\n\
Ctrl+Shift+ L - Load the currently selected session.\n\
Ctrl+Shift+ M - Send a chat message to all players.\n\
Ctrl+Shift+ P - Open the Player List (Send chat to individual players.)\n\
Ctrl+Shift+ O - Open the Settings page for this session.\n\
Ctrl+Shift+ H - Show this help window.\n\
	", L"PlayMailer In-game Hotkeys", MB_OK);
}

void RunSchTask()
{ 
	TCHAR queryStr[]=TEXT("cmd /c schtasks /QUERY /TN \"PlayMailer\" >temp.txt");
	TCHAR lineBuffer[100];
	FILE *fileOut;
	OSVERSIONINFO versionInfo;
	BOOL createSchTask = FALSE, fileOpened = FALSE;
	
	versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);	
	GetVersionEx(&versionInfo);
	
	if(versionInfo.dwMajorVersion < 6) // Only support mouse disabling in Vista and above
		return;

	ExecuteCmd(queryStr);
	for(int i = 0; i < 100; i++)
	{
		if(!_wfopen_s(&fileOut, L"temp.txt", L"r"))
		{
			fileOpened = TRUE;
			break;
		}
		Sleep(50);
	}
	if(!fileOpened)
	{
		MessageBox(NULL, L"Error creating scheduled tasks", L"File I/O Error", MB_OK | MB_ICONERROR);
		return;
	}

	if(!fgetws(lineBuffer, 100, fileOut))
		createSchTask = TRUE;
		
	fclose(fileOut);
	DeleteFile(L"temp.txt");

	if(createSchTask)
	{
		ShellExecute(NULL, L"runas", AppPath, L"/CREATE_SCHTASK", 0, SW_HIDE);
		exit(0);
	}
	
	ExecuteCmdEx(L"schtasks /RUN /TN \"PlayMailer\"", NULL, FALSE);
} 

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
BOOL IsWow64()
{
	LPFN_ISWOW64PROCESS fnIsWow64Process;
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
    }
    return bIsWow64;
}

BOOL AddProgramToStartup(BOOL minimized)
{
	TCHAR args[512];

	if(MajorVersion < 6)
		return AddProgramToStartupRegistry(minimized);

	swprintf(args, 512, L"/START %s", (minimized ? L"/MINIMIZED" : L""));
	CreateSchTask(L"PlayMailer Startup", AppPath, args, SCHTASK_ONLOGON);

	return TRUE;
}

BOOL RemoveProgramFromStartup()
{
	if(MajorVersion < 6)
		return RemoveProgramFromStartupRegistry();

	return ExecuteCmd(L"schtasks /Delete /TN \"PlayMailer Startup\"");
}

BOOL AddProgramToStartupRegistry(BOOL minimized)
{
	TCHAR appPath[MAX_PATH], runCommand[MAX_PATH];
	HKEY runKey;

	if(!GetModuleFileName(NULL, appPath, MAX_PATH))
		return FALSE;

	swprintf(runCommand, MAX_PATH, L"\"%s\"", appPath);

	if(minimized)
		wcscat_s(runCommand, MAX_PATH, L" /MINIMIZED");

	if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &runKey) != ERROR_SUCCESS)
 		return FALSE;
 
	DWORD pathLenInBytes = wcslen(runCommand) * sizeof(*runCommand);

	if (RegSetValueEx(runKey, APP_TITLE, 0, REG_SZ, (LPBYTE)runCommand, pathLenInBytes) != ERROR_SUCCESS)
	{
		RegCloseKey(runKey);
		return FALSE;
	}

	RegCloseKey(runKey);
	return TRUE;
}

BOOL RemoveProgramFromStartupRegistry()
{
	TCHAR szPath[MAX_PATH];
	HKEY runKey;

	swprintf(szPath, MAX_PATH, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run\\%s", APP_TITLE);

	if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &runKey) != ERROR_SUCCESS)
 		return FALSE;
 
	if (RegDeleteValue(runKey, APP_TITLE) != ERROR_SUCCESS)
	{
		RegCloseKey(runKey);
		return FALSE;
	}

	RegCloseKey(runKey);
	return TRUE;
}

BOOL ExecuteCmd(TCHAR *cmd)
{
	return ExecuteCmdEx(cmd, NULL, TRUE);
}

BOOL ExecuteCmdEx(TCHAR *cmd, TCHAR *dir, BOOL bWait)
{
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	TCHAR runCommand[3 * MAX_PATH];
	BOOL bSuccess;
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	
	wcscpy_s(runCommand, MAX_PATH, cmd);

	if (bSuccess = CreateProcessW(NULL, runCommand, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, dir, &si, &pi))
	{
		if(bWait)
			WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	return bSuccess;
}

void SetAppPaths()
{
#ifndef _DEBUG
	TCHAR szPath[MAX_PATH];
	HRESULT err;

	// Set current directory to AppData folder.
	if(S_OK != (err = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath)))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error getting CSIDL_APPDATA path: %d", err);
		MessageBox(NULL, mbBuffer, L"Error getting folder path", MB_OK | MB_ICONERROR);
	}

	//SHGetSpecialFolderPath( NULL, szPath, CSIDL_APPDATA, TRUE );
	wcscat_s(szPath, MAX_PATH, L"\\");
	wcscat_s(szPath, MAX_PATH, APP_TITLE);

	if(-1 == _wmkdir(szPath) && errno != EEXIST)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error creating AppData folder: %d : %s", errno, szPath);
		MessageBox(NULL, mbBuffer, L"Error creating folder", MB_OK | MB_ICONERROR);
	}

	// Enable this for release
	if(!SetCurrentDirectory(szPath))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Error setting Current Directory: %d : %s", GetLastError(), szPath);
		MessageBox(NULL, mbBuffer, L"Error setting Current Directory", MB_OK | MB_ICONERROR);
	}
#endif

	// Find install folder 
	GetModuleFileName(NULL, AppPath, MAX_PATH);
	wcscpy_s(AppFolderPath, MAX_PATH, AppPath);
	*wcsrchr(AppFolderPath, L'\\') = L'\0';

	// Enable this for release
	// Set help file path
#ifndef _DEBUG
	swprintf(HelpPath, MAX_PATH, L"%s\\%s Help.chm", AppFolderPath, APP_TITLE);
	if(_waccess(HelpPath, 0))
		swprintf(HelpPath, MAX_PATH, L"..\\help\\%s Help.chm", APP_TITLE); 
#else
	swprintf(HelpPath, MAX_PATH, L"..\\help\\%s Help.chm", APP_TITLE); 
#endif

#ifndef _DEBUG
	swprintf(PluginsPath, MAX_PATH, L"%s\\plugins", AppFolderPath);
#else
	swprintf(PluginsPath, MAX_PATH, L"plugins");
#endif
}

BOOL AddLinesToFile(TCHAR *srcPath, TCHAR *destPath, SearchReplace *strings, int numSearches)
{
	FILE *srcFile, *destFile;
	TCHAR buffer[2048], noWSBuff[2048];

	if(_wfopen_s(&srcFile, srcPath, L"r"))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Cannot open file for reading: %s", srcPath);
		MessageBoxS(NULL, mbBuffer, L"File Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	if(_wfopen_s(&destFile, destPath, L"w"))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Cannot open file for writing: %s", destPath);
		MessageBoxS(NULL, mbBuffer, L"File Error", MB_ICONERROR | MB_OK);
		fclose(srcFile);
		return FALSE;
	}

	while(fgetws(buffer, 2048, srcFile))
	{
		fputws(buffer, destFile);

		RemoveWhiteSpace(buffer, noWSBuff);
		for(int i = 0; i < numSearches; i++)
		{
			if(!_wcsnicmp(noWSBuff, strings[i].search, wcslen(strings[i].search)))
			{
				fputws(strings[i].replace, destFile);
				fputwc(L'\n', destFile);
			}
		}
	}

	fclose(srcFile);
	fclose(destFile);

	return TRUE;
}

BOOL ReplaceLinesInFile(TCHAR *srcPath, TCHAR *destPath, SearchReplace *strings, int numSearches)
{
	FILE *srcFile, *destFile;
	TCHAR buffer[2048], noWSBuff[2048];
	BOOL found;

	if(_wfopen_s(&srcFile, srcPath, L"r"))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Cannot open file for reading: %s", srcPath);
		MessageBoxS(NULL, mbBuffer, L"File Error", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	if(_wfopen_s(&destFile, destPath, L"w"))
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Cannot open file for writing: %s", destPath);
		MessageBoxS(NULL, mbBuffer, L"File Error", MB_ICONERROR | MB_OK);
		fclose(srcFile);
		return FALSE;
	}

	while(fgetws(buffer, 2048, srcFile))
	{
		found = FALSE;

		RemoveWhiteSpace(buffer, noWSBuff);
		for(int i = 0; i < numSearches; i++)
		{
			if(!_wcsnicmp(noWSBuff, strings[i].search, wcslen(strings[i].search)))
			{
				fputws(strings[i].replace, destFile);
				fputwc(L'\n', destFile);
				found = TRUE;
				break;
			}
		}

		if(!found)
			fputws(buffer, destFile);
	}

	fclose(srcFile);
	fclose(destFile);

	return TRUE;
}

void ShowTaskBar(BOOL show)
{
	HWND taskbar = FindWindow(_T("Shell_TrayWnd"), NULL);
	HWND start = FindWindow(_T("Button"), NULL);

	if (taskbar != NULL) {
		ShowWindow(taskbar, show ? SW_SHOW : SW_HIDE);
		UpdateWindow(taskbar);
	}
	if (start != NULL) { 
		// Vista
		ShowWindow(start, show ? SW_SHOW : SW_HIDE);
		UpdateWindow(start);
	}       
}
