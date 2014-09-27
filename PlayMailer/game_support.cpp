#include "stdafx.h"
#include "game_support.h"

//HHOOK hMouseHook, hKeyboardHook;
HWND hGameSettingsDialog, hGGChildDialog;

BOOL NewGameInstance;
double MouseModifier = 1;

HANDLE hWriteFileThread = NULL;
TCHAR WriteFilePath[MAX_PATH];

BOOL (*IsProgramWindowCallBack)(GlobalGameSettings *ggs, HWND hWnd);
GlobalGameSettings *GlobalGameSettings_PTR;

int MouseSpeed = 2;

BOOL _NewGame(SessionInfo *session)
{
	GlobalGameSettings *ggSettings = session->ggSettings;
	TCHAR *pRunCommand;
	
	if(!session)
	{
		MessageBoxS(NULL, L"I don't know which game to load! First select a session in PlayMailer.", L"Unable to Load Game", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if((!isYourTurn(session) && !session->actingCurrentPlayer) || session->selectingTeams)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Unable to load new game \"%s.\" because it is not your turn.", session->sessionName);
		MessageBoxS(NULL, mbBuffer, L"Cannot load game", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(session->ggSettings->KillBeforeRunGame)
		session->ggSettings->KillGame();

	if((!session->ggSettings->KillBeforeRunGame && session->ggSettings->FindGameWindow()) || ggSettings->runCommand[0] == L'\0')
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"%s (%s), it is your turn in the new game \"%s\".\n\nYou have the very first turn, so you need to start a fresh copy of %s and setup the game manually.", getCurrentPlayerName(session), getCurrentPlayerFactionName(session), session->sessionName, session->ggSettings->gameID);
		MessageBoxS(NULL, mbBuffer, L"Your turn in new game!", MB_OK);

		GameInProgress = TRUE;
		return TRUE;
	}

	if(!session->PreNewGameEvent())
		return FALSE;

	if(!BringGameToFront())
	{
		if(session->sessionRunCommand[0] != L'\0')
			pRunCommand = session->sessionRunCommand;
		else
			pRunCommand = session->ggSettings->runCommand;

		swprintf(mbBuffer, MBBUFFER_SIZE, L"Unable to run command \'%s\'.", pRunCommand);
		MessageBoxS(NULL, mbBuffer, L"Error running game", MB_OK | MB_ICONERROR);
		
		return FALSE;
	}

	DisableInput(TRUE);

	if(!session->NewGame())
	{
		DisableInput(FALSE);
		return FALSE;
	}
	DisableInput(FALSE);

	if(session->ggSettings->restoreColorDepth)
		RestoreColorDepth();

	session->PostNewGameEvent();
	DisplayGameTip(session);

	if(isYourTurn(session))
		GameInProgress = TRUE;

	return TRUE;
}

void DisplayGameTip(SessionInfo *session)
{
	if(session->ggSettings->displayGameTip && session->turnNumber <= session->numPlayers
			&& (session->currentPlayer == 0 || _wcsicmp(session->players[GetPrevPlayerIndex(session)]->email, getCurrentPlayerEmail(session))))
	{
		session->GetGameTip(mbBuffer);
		DialogBoxParamS(hInst, MAKEINTRESOURCE(IDD_TIP_OF_THE_DAY), NULL, GameTipDialogProc, (LPARAM)session);
	}
}

void AddKeyPress(INPUT *keys, WORD vKey, int *numKeys)
{
	AddKeyDown(keys, vKey, numKeys);
	AddKeyUp(keys, vKey, numKeys);
}

void AddHotKeyPress(INPUT *keys, WORD vModKey, WORD vKey, int *numKeys)
{
	AddKeyDown(keys, vModKey, numKeys);
	AddKeyDown(keys, vKey, numKeys);
	AddKeyUp(keys, vModKey, numKeys);
	AddKeyUp(keys, vKey, numKeys);
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
		SleepC(50);
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
		SleepC(50);
}

void AddKeyDown(INPUT *keys, WORD vKey, int *numKeys)
{
	memset(&keys[*numKeys], 0, sizeof(INPUT));
	keys[*numKeys].type = INPUT_KEYBOARD;
	keys[*numKeys].ki.wVk = vKey;
	(*numKeys)++;
}

void AddKeyUp(INPUT *keys, WORD vKey, int *numKeys)
{
	memset(&keys[*numKeys], 0, sizeof(INPUT));
	keys[*numKeys].type = INPUT_KEYBOARD;
	keys[*numKeys].ki.dwFlags = KEYEVENTF_KEYUP;
	keys[*numKeys].ki.wVk = vKey;
	(*numKeys)++;
}

void PressKeyFast(WORD vKey)
{
	PressKeyDown(vKey, FALSE);
	PressKeyUp(vKey, FALSE);
}

void PressKey(WORD vKey)
{
	PressKeyDown(vKey, TRUE);
	PressKeyUp(vKey, TRUE);
}

void PressHotKey(WORD vModKey, WORD vKey)
{
	PressKeyDown(vModKey, TRUE);
	PressKeyDown(vKey, TRUE);
	PressKeyUp(vModKey, TRUE);
	PressKeyUp(vKey, TRUE);
}

BOOL CheckFullScreen(SessionInfo *session)
{
	if(session->ggSettings->fullScreen) 
		return session->EnterFullScreen();
	else 
		return session->LeaveFullScreen();
}

BOOL _SaveGame(SessionInfo *session)
{
	if(!IsGameWindowForeground()) 
		return FALSE;
	
	if(!session)
	{
		MessageBoxS(NULL, L"I don't know which game session to save! First load a session using the Mailer.", L"Unable to Save Game", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!session->PreSaveGameEvent())
		return FALSE;

	DisableInput(TRUE);

	if(!session->SaveGame())
	{
		DisableInput(FALSE);
		MessageBoxS(NULL, L"The game did not save correctly, please try again.", L"Problem saving game", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	DisableInput(FALSE);
	return ImportSaveFile(session);
}

void PressStringKeys(TCHAR *name, int length)
{
	short code;

	for(int j = 0; (length == 0 || j < length) && name[j] != L'\0'; j++)
	{
		code = VkKeyScan(name[j]);

		if(HIBYTE(code) != 1 && HIBYTE(code) != 0) continue;

		if(HIBYTE(code) == 1)
			PressHotKey(VK_LSHIFT, LOBYTE(code));
		else
			PressKey(LOBYTE(code));
	}
}

BOOL _IsGameWindow(GlobalGameSettings *ggs, HWND hWnd)
{
	return ggs->IsGameWindow(hWnd);
}

void _InitInput(GlobalGameSettings *ggs, HWND hWnd)
{
	ggs->InitInput(hWnd);
}

HWND IsGameWindowTopMost()
{
	HWND hFG;

	if((hFG = IsGameWindowForeground()) && GetWindowLong(hFG, GWL_EXSTYLE) & WS_EX_TOPMOST)
		return hFG;

	return NULL;
}

BOOL SuspendResumeGame(BOOL suspend)
{
	HWND hFG;
	static HANDLE hDBThread = NULL;
	static int suspendCount = 0;
	static HHOOK hKeyboardHook, hMouseHook;

	if(suspend) 
	{
		if(!suspendCount)
		{
			hFG = GetForegroundWindow();

			if(!IsGameWindowForeground())
				return FALSE;

			if(!(GetWindowLong(hFG, GWL_EXSTYLE) & WS_EX_TOPMOST))
				return FALSE;

			hDBThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, GetWindowThreadProcessId(hFG, 0));
			SuspendThread(hDBThread);

			hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, InputProc, NULL, 0); 
			hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, InputProc, NULL, 0); 
		
			ShowTaskBar(FALSE);
		}

		suspendCount++;
	}
	else if(!suspend && suspendCount)
	{	
		suspendCount--;

		if(!suspendCount)
		{
			UnhookWindowsHookEx(hMouseHook); 
			UnhookWindowsHookEx(hKeyboardHook); 
			ResumeThread(hDBThread);
			CloseHandle(hDBThread);
			hDBThread = NULL;

			ShowTaskBar(TRUE);
		}
	}
	else
		return FALSE;

	return TRUE;
}

BOOL IsFGWFullScreen()
{
	HWND hFG;
	MONITORINFO mi = { sizeof(mi) };
	RECT rc;

	hFG = GetForegroundWindow();
	
	if(GetWindowLong(hFG, GWL_EXSTYLE) & WS_EX_TOPMOST)
		return TRUE;
	
	GetWindowRect(hFG, &rc);
	GetMonitorInfo(MonitorFromWindow(hFG, MONITOR_DEFAULTTOPRIMARY), &mi);
    if(mi.rcMonitor.right - mi.rcMonitor.left <= rc.right - rc.left && 
			mi.rcMonitor.bottom - mi.rcMonitor.top <= rc.bottom - rc.top)
	{
		return TRUE;
	}

	return FALSE;
}

void SpinUpCDDrive()
{
	TCHAR drivesList[512];
	TCHAR *p = drivesList;
	TCHAR drive[5];
	
	wcscpy_s(drive, 5, L" :\\");

	if(GetLogicalDriveStrings(511, drivesList)) 
	{
		do
		{
			drive[0] = *p;

			if(GetDriveType(drive) == DRIVE_CDROM)
			{
				GetFileAttributes(drive);
			}

			while(*p++);
		} while(*p);
	}
}

HWND IsGameWindowForeground()
{
	SessionInfo *session;

	if(selectedSession != -1)
	{
		session = (SessionInfo *)LL_GetItem(&llSessions, selectedSession);
		return session->ggSettings->IsGameWindowForeground();
	}

	return NULL;
}

int BringProgramToFront(TCHAR *runCommand, TCHAR *startInFolder, GlobalGameSettings *ggs, BOOL (*isProgramWindow)(GlobalGameSettings *ggs, HWND hWnd), void (*initProgramInput)(GlobalGameSettings *ggs, HWND hWnd), int runDelay)
{
	HWND hWnd, hFG;
	int ret = 1;

	if(isProgramWindow == NULL) 
		isProgramWindow = _IsGameWindow;
	if(initProgramInput == NULL)
		initProgramInput = _InitInput;

	ProcSpeed = GetProcSpeed();

	if(!(hWnd = FindProgramWindow(ggs, isProgramWindow)))
	{
		if(!(hWnd = RunProgram(runCommand, startInFolder, ggs, isProgramWindow, runDelay)))
			return 0;
		else
			ret = 2;
	}

	DisableInput(TRUE);

	if(IsIconic(hWnd))
		ShowWindow(hWnd, SW_SHOWNORMAL);
	SetForegroundWindow(hWnd);
	
	for(int i = 0; i < 100; i++)
	{
		if((hFG = GetForegroundWindow()) != NULL)
		{
			if(isProgramWindow(ggs, hFG)) 
			{
				SleepC(500); // Wait some more for good measure
				initProgramInput(ggs, hWnd);
				DisableInput(FALSE);

				return ret;	
			}

			SetForegroundWindow(hWnd);
		}

		SleepC(50);
	}	

	DisableInput(FALSE);
	return 0;
}

HWND RunProgram(TCHAR *runCommand, TCHAR *startInFolder, GlobalGameSettings *ggs, BOOL (*isProgramWindow)(GlobalGameSettings *ggs, HWND hWnd), int runDelay)
{
	HWND hWnd;
		
	if(runCommand[0] == L'\0')
		return NULL;

	if(!ExecuteCmdEx(runCommand, startInFolder, FALSE))
		return NULL;

	DisableInput(TRUE);
	for(int i = 0; i < 10; i++)
	{
		SleepC(MAX(1, runDelay / 10));
		CheckMessageQueue();
	}
	DisableInput(FALSE);

	for(int i = 0; i < 100; i++)
	{
		if(hWnd = FindProgramWindow(ggs, isProgramWindow)) 
			return hWnd;

		SleepC(100);
	}

	return NULL;
}

HWND FindProgramWindow(GlobalGameSettings *ggs, BOOL (*isProgramWindow)(GlobalGameSettings *ggs, HWND hWnd))
{
	HWND hWnd;
	
	if(hWnd = IsProgramWindowForeground(ggs, isProgramWindow)) 
		return hWnd;

	IsProgramWindowCallBack = isProgramWindow;
	GlobalGameSettings_PTR = ggs;

	if(!EnumWindows(EnumIsProgramWindow, (LPARAM)&hWnd))
		return hWnd;

	return NULL;
}

BOOL CALLBACK EnumIsProgramWindow(HWND hWnd, LPARAM lParam)
{
	if(IsProgramWindowCallBack(GlobalGameSettings_PTR, hWnd))
	{
		*(HWND *)lParam = hWnd;
		return FALSE;
	}
	else
		*(HWND *)lParam = NULL;

	return TRUE;
}

HWND IsProgramWindowForeground(GlobalGameSettings *ggs, BOOL (*isProgramWindow)(GlobalGameSettings *ggs, HWND hWnd))
{
	HWND hFG;

	hFG = GetForegroundWindow();
	if(!hFG) 
		return NULL;
	
	if(isProgramWindow(ggs, hFG))
		return hFG;

	return NULL;	
}

BOOL BringGameToFront()
{
	int ret;
	SessionInfo *session;
	TCHAR *pRunCommand, *startInFolder;

	if(selectedSession == -1)
		return FALSE;
	session = (SessionInfo *)LL_GetItem(&llSessions, selectedSession);

	NewGameInstance = FALSE;

	if(session->ggSettings->gameFolderPath[0])
		startInFolder = session->ggSettings->gameFolderPath;
	
	if(session->sessionRunCommand[0] != L'\0')
		pRunCommand = session->sessionRunCommand;
	else
		pRunCommand = session->ggSettings->runCommand;

	ret = BringProgramToFront(pRunCommand, startInFolder, session->ggSettings, _IsGameWindow, _InitInput, session->ggSettings->RUN_DELAY);

	if(ret == PROGRAM_EXECUTED)
		NewGameInstance = TRUE;

	return !!ret;
}

/*
BOOL BringGameToFront()
{
	HWND hWnd, hFG;
	SessionInfo *session;

	ProcSpeed = GetProcSpeed();

	if(selectedSession == -1)
		return FALSE;
	session = (SessionInfo *)LL_GetItem(&llSessions, selectedSession);
	
	NewGameInstance = FALSE;

	if(!(hWnd = session->ggSettings->FindGameWindow()) && !(hWnd = RunGame(session)))
		return FALSE;

	if(IsIconic(hWnd))
		ShowWindow(hWnd, SW_SHOWNORMAL);
	SetForegroundWindow(hWnd);
	
	for(int i = 0; i < 100; i++)
	{
		if((hFG = GetForegroundWindow()) != NULL)
		{
			if(session->ggSettings->IsGameWindow(hFG)) 
			{
				SleepC(500); // Wait some more for good measure
				session->ggSettings->InitInput(hWnd);
	
				return TRUE;	
			}

			SetForegroundWindow(hWnd);
		}

		SleepC(50);
	}	

	return FALSE;
}*/

void RestoreColorDepth()
{
	DEVMODE dev;
	int colorDepth, horiz, vert;

	dev.dmSize = sizeof(DEVMODE);
	dev.dmDriverExtra = 0;
	EnumDisplaySettingsEx(NULL, ENUM_REGISTRY_SETTINGS, &dev, NULL);
	colorDepth = dev.dmBitsPerPel;
	horiz = dev.dmPelsWidth;
	vert = dev.dmPelsHeight;
	EnumDisplaySettingsEx(NULL, ENUM_CURRENT_SETTINGS, &dev, NULL);
	dev.dmBitsPerPel = colorDepth;
	ChangeDisplaySettings(NULL, 0);
	SleepC(1500);
	ChangeDisplaySettings(&dev, CDS_FULLSCREEN);
}



void DebugWindow(HWND hWnd)
{
	TCHAR winText[1024], winClass[1024], buffer[4096];
	DWORD styles, exStyles;
	HWND activeWin, focusWin, FGWin;
	FILE *logFile;
	WINDOWPLACEMENT wp;
	GUITHREADINFO gti;

	wp.length = sizeof(WINDOWPLACEMENT);
	gti.cbSize = sizeof(GUITHREADINFO);

	if(_wfopen_s(&logFile, L"debug_window.log", L"w"))
	{
		MessageBox(NULL, L"Error opening debug_window.log", L"File error", MB_OK);
		return;
	}

	for(int i = 0; i < 1000; i++)
	{
		GetGUIThreadInfo(GetWindowThreadProcessId(hWnd, NULL), &gti);
		FGWin = GetForegroundWindow();
		activeWin = GetActiveWindow();
		focusWin = GetFocus();
		styles = GetWindowLong(hWnd, GWL_STYLE);
		exStyles = GetWindowLong(hWnd, GWL_EXSTYLE);
		GetWindowText(hWnd, winText, 1024);
		GetClassName(hWnd, winClass, 1024);
		GetWindowPlacement(hWnd, &wp);

		swprintf(buffer, 4096, L"Styles: %d, Ex-Styles: %d, Title: %s, Class: %s, ThFlags: %d, ThActive: %d, ThFocus: %d, ThCapture: %d, WinFG: %d, WinActive: %d, WinFocus: %d, WPFlags: %d, WPShow: %d\n", styles, exStyles, winText, winClass, gti.flags, gti.hwndActive, gti.hwndFocus, gti.hwndCapture, FGWin, activeWin, focusWin, wp.flags, wp.showCmd);
		fputws(buffer, logFile);
		SleepC(10);
	}

	fclose(logFile);
}

void TestMouseSequence()
{

}

void PressLButtonDown()
{
	INPUT mouse;
		
	memset(&mouse, 0, sizeof(INPUT));
	mouse.type = INPUT_MOUSE;
	mouse.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	mouse.mi.dwExtraInfo = PLAYMAILER_TAG;
	SendInput(1, &mouse, sizeof(INPUT));

	SleepC(150);
}

void PressRButtonDown()
{
	INPUT mouse;
		
	memset(&mouse, 0, sizeof(INPUT));
	mouse.type = INPUT_MOUSE;
	mouse.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	mouse.mi.dwExtraInfo = PLAYMAILER_TAG;
	SendInput(1, &mouse, sizeof(INPUT));

	SleepC(150);
}

void PressLButtonUp()
{
	INPUT mouse;
		
	memset(&mouse, 0, sizeof(INPUT));
	mouse.type = INPUT_MOUSE;
	mouse.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	mouse.mi.dwExtraInfo = PLAYMAILER_TAG;
	SendInput(1, &mouse, sizeof(INPUT));

	SleepC(150);
}

void PressRButtonUp()
{
	INPUT mouse;
		
	memset(&mouse, 0, sizeof(INPUT));
	mouse.type = INPUT_MOUSE;
	mouse.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	mouse.mi.dwExtraInfo = PLAYMAILER_TAG;
	SendInput(1, &mouse, sizeof(INPUT));

	SleepC(150);
}

void PressLButton()
{
	PressLButtonDown();
	PressLButtonUp();
}

void PressRButton()
{
	PressRButtonDown();
	PressRButtonUp();
}

void SetFixedMouseSpeed(BOOL enable)
{
	int mouseParams[3], newSpeed = 7;
	static int oldSpeed;

	SystemParametersInfo(SPI_GETMOUSE, 0, mouseParams, 0);
	mouseParams[2] = !enable;
	SystemParametersInfo(SPI_SETMOUSE, 0, mouseParams, SPIF_SENDCHANGE);

	if(enable)
		SystemParametersInfo(SPI_GETMOUSESPEED, 0, &oldSpeed, 0);
	else
		newSpeed = oldSpeed;
	
	SystemParametersInfo(SPI_SETMOUSESPEED, 0, (PVOID)newSpeed, SPIF_SENDCHANGE);
}

void CheckMousePosKeys(int keyPressed)
{
	static int horPos = 0, verPos = 0, keyCounter = 0;
	static BOOL negative = FALSE;
	static double oldMouseModifier = 0;
	SessionInfo *session;
	static BOOL mouseDisabled = FALSE;

	if(selectedSession == -1)
		return;
	session = (SessionInfo *)LL_GetItem(&llSessions, selectedSession);

	switch(HotKeyPressed)
	{
	case VK_T:
		//TestMouseSequence();
		break;
	case VK_R:
		ResetMouse();
		keyCounter = 0;
		horPos = verPos = 0;
		negative = FALSE;
		break;
	case VK_D:
		if(!oldMouseModifier)
		{
			session->ggSettings->SetMouseModifier(GetForegroundWindow());
			break;
		}
		
		if(MouseModifier == 1.0)
			MouseModifier = oldMouseModifier;
		else
		{
			oldMouseModifier = MouseModifier;
			MouseModifier = 1.0; 
		}
		break;
	case VK_V:
		ResetMouseVert();
		keyCounter = 0;
		horPos = verPos = 0;
		negative = FALSE;
		break;
	case VK_Z:
		ResetMouseHoriz();
		keyCounter = 0;
		horPos = verPos = 0;
		negative = FALSE;
		break;
	case VK_U:
		if(keyCounter < 3)
		{
			MoveMouseMod(0, -verPos);
			keyCounter = 3;
		}
		else
		{
			MoveMouseMod(-horPos, 0);
			keyCounter = 0;
		}
		horPos = verPos = 0;
		break;
	case VK_MINUS:
		negative = TRUE;
		break;
	}

	if(HotKeyPressed >= VK_0 && HotKeyPressed <= VK_9)
	{
		if(keyCounter < 3)
		{
			horPos += (HotKeyPressed - 0x30) * (int)pow(10.0, 2 - keyCounter);
			if(keyCounter == 2)
			{
				if(negative) 
				{
					horPos = -horPos;
					negative = FALSE;
				}
				MoveMouseMod(horPos, 0);
				verPos = 0;
			}
		}
		else
		{
			verPos += (HotKeyPressed - 0x30) * (int)pow(10.0, 5 - keyCounter);
			if(keyCounter == 5)
			{
				if(negative)
				{
					verPos = -verPos;
					negative = FALSE;
				}
				MoveMouseMod(0, verPos);
			}
		}
		
		keyCounter++;
		if(keyCounter == 6) 
		{
			keyCounter = 0;
			horPos = 0;
		}
	}
}

void SetMousePos(int horPos, int verPos)
{
	ResetMouse();
	MoveMouse(horPos, verPos);
}

void MoveMouseMod(int horPos, int verPos)
{
	horPos = (int)(horPos * MouseModifier);
	verPos = (int)(verPos * MouseModifier);

	MoveMouse(horPos, verPos);
}

void MoveMouse(int horPos, int verPos)
{
	INPUT mouse;

	SetFixedMouseSpeed(TRUE);
	
	while(1)
	{
		memset(&mouse, 0, sizeof(INPUT));
		mouse.type = INPUT_MOUSE;
		mouse.mi.dwFlags = MOUSEEVENTF_MOVE;
		mouse.mi.dx = ((int)abs(horPos) > 99 ? (horPos > 0 ? 99 : -99) : horPos);
		mouse.mi.dy = ((int)abs(verPos) > 99 ? (verPos > 0 ? 99 : -99) : verPos);
		mouse.mi.dwExtraInfo = PLAYMAILER_TAG;
		SendInput(1, &mouse, sizeof(INPUT));
		
		SleepC(20);
		if(!horPos && !verPos)
			break;

		if(horPos > 0)
		{
			horPos -= 99; 
			if(horPos < 0) horPos = 0;
		}
		else
		{
			horPos += 99;
			if(horPos > 0) horPos = 0;
		}
		if(verPos > 0)
		{
			verPos -= 99; 
			if(verPos < 0) verPos = 0;
		}
		else
		{
			verPos += 99;
			if(verPos > 0) verPos = 0;
		}
	}

	SetFixedMouseSpeed(FALSE);
}

void ResetMouse()
{
	HWND hWnd;
	RECT rc;
	
	hWnd = GetForegroundWindow();
	GetClientRect(hWnd, &rc);
	MoveMouseMod(-rc.right * 3, -rc.bottom * 3);
}

void ResetMouseVert()
{
	HWND hWnd;
	RECT rc;
	
	hWnd = GetForegroundWindow();
	GetClientRect(hWnd, &rc);
	MoveMouseMod(0, -rc.bottom * 3);
}

void ResetMouseHoriz()
{
	HWND hWnd;
	RECT rc;
	
	hWnd = GetForegroundWindow();
	GetClientRect(hWnd, &rc);
	MoveMouseMod(-rc.right * 3, 0);
}

void SetMouseSpeed(int speed)
{
	MouseSpeed = speed;
}

BOOL _LoadGame(SessionInfo *session, BOOL exportSave)
{
	TCHAR *pRunCommand;

	if(!session)
	{
		MessageBoxS(NULL, L"I don't know which game to load! First select a session in PlayMailer.", L"Unable to Load Game", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(session->ggSettings->KillBeforeLoadGame) 
		session->ggSettings->KillGame();
	if(exportSave)
		ExportSaveFile(session, TRUE);
	
	if(!session->PreLoadGameEvent())
		return FALSE;

	if(!BringGameToFront())
	{
		if(session->sessionRunCommand[0] != L'\0')
			pRunCommand = session->sessionRunCommand;
		else
			pRunCommand = session->ggSettings->runCommand;

		swprintf(mbBuffer, MBBUFFER_SIZE, L"Unable to run command \'%s\'.\nYou can run the game manually and then press Ctrl-Shift-L to load the save file.", pRunCommand);
		MessageBoxS(NULL, mbBuffer, L"Error running game", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	DisableInput(TRUE);

	if(!session->LoadGame())
	{
		DisableInput(FALSE);
		return FALSE;
	}

	DisableInput(FALSE);

	if(session->ggSettings->restoreColorDepth)
		RestoreColorDepth();
	
	session->PostLoadGameEvent();
	DisplayGameTip(session);

	if(isYourTurn(session))
		GameInProgress = TRUE;

	return TRUE;
}

void DisableInput(BOOL disable)
{
	static HHOOK hMouseHook = NULL, hKeyboardHook = NULL;

	/*TCHAR runCommand[MAX_PATH];

	swprintf(runCommand, MAX_PATH, L"\"%s\\bin\\%s\" %s", AppFolderPath, (IsWow64() ? L"PlayMailerDI64.exe" : L"PlayMailerDI32.exe"), (disable ? L"/DISABLE" : L"/ENABLE"));
	ExecuteCmd(runCommand);*/

	if(disable)
	{
		hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, DisableMouseProc, NULL, 0); 
		hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, DisableKeyboardProc, NULL, 0); 
	}
	else
	{
		if(hMouseHook)
			UnhookWindowsHookEx(hMouseHook);
		if(hKeyboardHook)
			UnhookWindowsHookEx(hKeyboardHook);

		hMouseHook = hKeyboardHook = NULL;
	}
}

HWND RunGame(SessionInfo *session)
{
	HWND hWnd;
	TCHAR *startInFolder = NULL;
	TCHAR *pRunCommand = session->ggSettings->runCommand;

	if(session->ggSettings->runCommand[0] == L'\0')
		return NULL;

	if(session->ggSettings->gameFolderPath[0])
		startInFolder = session->ggSettings->gameFolderPath;
	
	if(session->sessionRunCommand[0] != L'\0')
		pRunCommand = session->sessionRunCommand;

	if(!ExecuteCmdEx(pRunCommand, startInFolder, FALSE))
		return NULL;

	SleepC(session->ggSettings->RUN_DELAY);

	for(int i = 0; i < 100; i++)
	{
		if(hWnd = session->ggSettings->FindGameWindow()) 
		{
			NewGameInstance = TRUE;
			return hWnd;
		}
		SleepC(100);
	}

	return NULL;
}

LRESULT CALLBACK InputProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
	if(nCode < 0) 
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	else
		return 0;
}

LRESULT CALLBACK DisableMouseProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
	MSLLHOOKSTRUCT *mouseHookStruct;
	
	if(nCode < 0) 
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	else
	{
		mouseHookStruct = (MSLLHOOKSTRUCT *)lParam;
		if(mouseHookStruct->dwExtraInfo == PLAYMAILER_TAG)
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		else
			return TRUE;
	}
}

LRESULT CALLBACK DisableKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
	KBDLLHOOKSTRUCT *kbHookStruct;

	if(nCode < 0) 
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	else
	{
		kbHookStruct = (KBDLLHOOKSTRUCT *)lParam;
		if(kbHookStruct->dwExtraInfo == PLAYMAILER_TAG)
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		else
			return TRUE;
	}
}

TCHAR *GetDOSBoxPath(TCHAR *gameID)
{
	if(!FindDOSBox() && (!settings->DOSBoxPath[0] || GetFileAttributes(settings->DOSBoxPath) == INVALID_FILE_ATTRIBUTES))
	{	
		swprintf(mbBuffer, MBBUFFER_SIZE, L"DOSBox 0.74 or higher is required to run %s.\nPlease download and install DOSBox before continuing.", gameID);
		MessageBox(NULL, mbBuffer, L"DOSBox Required", MB_CANCELTRYCONTINUE | MB_ICONEXCLAMATION);
		if(!FindDOSBox())
		{
			if(GetFolderSelection(NULL, settings->DOSBoxPath, L"DOSBox 0.74 or higher cannot be found. Please select your DOSBox install folder."))
			{
				wcscat_s(settings->DOSBoxPath, MAX_PATH, L"\\DOSBox.exe");
				if(GetFileAttributes(settings->DOSBoxPath) == INVALID_FILE_ATTRIBUTES)
				{	
					MessageBox(NULL, L"DOSBox.exe not found in the selected folder.", L"DOSBox not found", MB_OK | MB_ICONERROR);
					return NULL;
				}
			}
			else return NULL;
		}
	}

	return settings->DOSBoxPath;
}

TCHAR *FindDOSBox()
{
	TCHAR programFilesPath[MAX_PATH], path[MAX_PATH];
	HANDLE hFile;
	WIN32_FIND_DATA fileData;
	double version = 0, highestVersion = -1;
	BOOL DOSBoxFound = FALSE;
	
	if(!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, programFilesPath)))
		return NULL;

	swprintf(path, MAX_PATH, L"%s\\DOSBox*", programFilesPath); 
	if((hFile = FindFirstFile(path, &fileData)) == INVALID_HANDLE_VALUE)
		return NULL;

	do
	{
		swprintf(path, MAX_PATH, L"%s\\%s\\DOSBox.exe", programFilesPath, fileData.cFileName); 
		if(GetFileAttributes(path) == INVALID_FILE_ATTRIBUTES)
			continue;
		
		DOSBoxFound = TRUE;

		version = _wtof(wcschr(fileData.cFileName, L'-') + 1);
		if(version >= highestVersion)
		{
			highestVersion = version;
			wcscpy_s(settings->DOSBoxPath, MAX_PATH, path);
		}
	}
	while(FindNextFile(hFile, &fileData));
    
	FindClose(hFile);

	if(DOSBoxFound)
		return settings->DOSBoxPath;

	return NULL;
}

BOOL StartWriteFileThread(TCHAR *filePath)
{
	wcscpy_s(WriteFilePath, MAX_PATH, filePath);

	if(!(hWriteFileThread = CreateThread(NULL, 0, WriteFileProc, WriteFilePath, 0, NULL)))
	{
		MessageBoxS(NULL, L"Thread error: could not create new thread", L"Thread Error", MB_OK | MB_ICONERROR);	
		return FALSE;
	}

	return TRUE;
 }

 BOOL WaitForWriteFileThread()
 {
	 DWORD exitCode;

	 WaitForSingleObject( hWriteFileThread, INFINITE );
	 GetExitCodeThread( hWriteFileThread, &exitCode );
	 CloseHandle( hWriteFileThread );

	 return exitCode;
 }

DWORD WINAPI WriteFileProc(LPVOID lpParam)
{
	return WaitForWriteFile((TCHAR *)lpParam);
}	

BOOL StartSaveFileThread(SessionInfo *session)
{
	TCHAR saveFilePath[MAX_PATH];

	session->GetSaveFilePath(saveFilePath);
	return StartWriteFileThread(saveFilePath);
}

BOOL WaitForWriteFile(TCHAR *filePath)
{
	HANDLE hDir, hEvents[2], hFile;
	BYTE buffer[8192];
	DWORD offset = 0, bytesReturned, ret;
	PFILE_NOTIFY_INFORMATION pNotify;
	TCHAR *fileName, folderPath[MAX_PATH], currFileName[MAX_PATH];
	OVERLAPPED overlapped;
	LARGE_INTEGER liDueTime;
	
	wcscpy_s(folderPath, MAX_PATH, filePath);
	PathRemoveFileSpec(folderPath);
	fileName = PathFindFileName(filePath);

	if(INVALID_HANDLE_VALUE == (hDir = CreateFile( folderPath,          
			FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL)))
		return FALSE;
	
	hEvents[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	overlapped.hEvent = hEvents[0];

	hEvents[1] = CreateWaitableTimer(NULL, TRUE, NULL);
	liDueTime.QuadPart = -100000000LL;
	SetWaitableTimer(hEvents[1], &liDueTime, 0, NULL, NULL, FALSE);

	while(1)
	{
		if(!ReadDirectoryChangesW(hDir, buffer, sizeof(buffer), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE,
				&bytesReturned, &overlapped, NULL))
		{
			SleepC(50);
			continue;
		}

		ret = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		switch(ret) 
		{
			case WAIT_OBJECT_0:
				do
				{
					pNotify = (PFILE_NOTIFY_INFORMATION)&buffer[offset];
					offset += pNotify->NextEntryOffset;
 
					lstrcpynW(currFileName, pNotify->FileName, min(MAX_PATH, pNotify->FileNameLength / sizeof(WCHAR) + 1));
			
					if(!_wcsicmp(fileName, currFileName))
					{
						SleepC(500); // Make sure all writes are finished.
						hFile = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0,NULL);
						if (hFile != INVALID_HANDLE_VALUE) 
						{
							CloseHandle(hFile);
							CloseHandle(hEvents[0]);
							CloseHandle(hEvents[1]);
							CloseHandle(hDir);
							return TRUE;
						}
					}
				} while (pNotify->NextEntryOffset != 0);
				break;
			case WAIT_OBJECT_0 + 1:
			default:
				CloseHandle(hEvents[0]);
				CloseHandle(hEvents[1]);
				CloseHandle(hDir);
				return FALSE;
		}
	}

	return FALSE;
}

/*
 BOOL WaitForSaveFileThread()
 {
	 DWORD exitCode;

	 WaitForSingleObject( hSaveFileThread, INFINITE );
	 GetExitCodeThread( hSaveFileThread, &exitCode );
	 CloseHandle( hSaveFileThread );

	 return exitCode;
 }

DWORD WINAPI SaveFileProc(LPVOID lpParam)
{
	return WaitForSaveFile((SessionInfo *)lpParam);
}

BOOL WaitForSaveFile(SessionInfo *session)
{
	HANDLE hDir, hEvents[2], hFile;
	BYTE buffer[8192];
	DWORD offset = 0, bytesReturned, ret;
	PFILE_NOTIFY_INFORMATION pNotify;
	TCHAR fileName[MAX_PATH], saveFile[MAX_PATH];
	OVERLAPPED overlapped;
	LARGE_INTEGER liDueTime;
	TCHAR *saveFolderPath = session->GetSaveFolderPath();
	
	session->GetSaveFilePath(saveFile);
	
	if(INVALID_HANDLE_VALUE == (hDir = CreateFile( saveFolderPath,          
			FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL)))
		return FALSE;
	
	hEvents[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	overlapped.hEvent = hEvents[0];

	hEvents[1] = CreateWaitableTimer(NULL, TRUE, NULL);
	liDueTime.QuadPart = -100000000LL;
	SetWaitableTimer(hEvents[1], &liDueTime, 0, NULL, NULL, FALSE);

	while(1)
	{
		if(!ReadDirectoryChangesW(hDir, buffer, sizeof(buffer), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE,
				&bytesReturned, &overlapped, NULL))
		{
			SleepC(50);
			continue;
		}

		ret = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		switch(ret) 
		{
			case WAIT_OBJECT_0:
				do
				{
					pNotify = (PFILE_NOTIFY_INFORMATION)&buffer[offset];
					offset += pNotify->NextEntryOffset;
 
					lstrcpynW(fileName, pNotify->FileName, min(MAX_PATH, pNotify->FileNameLength / sizeof(WCHAR) + 1));
			
					if(!_wcsicmp(session->GetSaveFileName(mbBuffer), fileName))
					{
						SleepC(500); // Make sure all writes are finished.
						hFile = CreateFile(saveFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0,NULL);
						if (hFile != INVALID_HANDLE_VALUE) 
						{
							CloseHandle(hFile);
							CloseHandle(hEvents[0]);
							CloseHandle(hEvents[1]);
							CloseHandle(hDir);
							return TRUE;
						}
					}
				} while (pNotify->NextEntryOffset != 0);
				break;
			case WAIT_OBJECT_0 + 1:
			default:
				CloseHandle(hEvents[0]);
				CloseHandle(hEvents[1]);
				CloseHandle(hDir);
				return FALSE;
		}
	}

	return FALSE;
}
*/

BOOL GetDOSBoxConfPath(TCHAR *outPath)
{
	FILE *stdFile;
	TCHAR path[MAX_PATH];
	
	// Cause DOSBox to create the default config file and output path to stdout.txt
	swprintf(path, MAX_PATH, L"\"%s\" -printconf -noconsole", settings->DOSBoxPath);
	ExecuteCmd(path);

	if(_wfopen_s(&stdFile, L"stdout.txt", L"r"))
		return FALSE;

	fgetws(outPath, MAX_PATH, stdFile);
	fclose(stdFile);

	DeleteFile(L"stdout.txt");
	DeleteFile(L"stderr.txt");

	outPath[wcslen(outPath) - 1] = L'\0';

	if(_waccess(outPath, 0))
		return FALSE;

	return TRUE;
}