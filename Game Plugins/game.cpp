#include "stdafx.h"
#include "PlayMailer.h"
#include "game.h"
#include "resource.h"
#include "data_structures.h"

BOOL (*pGetFolderSelection)(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle);
BOOL (*pGetFileSelection)(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle, TCHAR *initialDir, TCHAR *filter);
HFONT (*pCreateDialogFont)(TCHAR *name, double size, int weight);
TCHAR mbBuffer[MBBUFFER_SIZE];

GlobalGameSettings *CurrentGG;

TCHAR *GlobalGameSettings::GAME_EXE_NAME_LIST[];
int GlobalGameSettings::NUM_GAME_EXE_NAMES = 1;

SearchReplace GlobalGameSettings::ConfigReplaceStrings[] = { 
	{L"fullscreen=", L"fullscreen=false"}
};
int GlobalGameSettings::NUM_CONFIG_REPLACE_STRINGS = 1;

#ifndef IDC_FOLDERPATH_EDIT
#define IDC_FOLDERPATH_EDIT 1
#endif

#ifndef IDC_FOLDERPATH_BUTTON
#define IDC_FOLDERPATH_BUTTON 1
#endif

#ifndef IDD_TEAM
#define IDD_TEAM 1
#endif

TCHAR *GlobalGameSettings::GetGameExeName()
{
	TCHAR exePath[MAX_PATH];

	if(this->GetNumGameExeNames() == 1)
		return GAME_EXE_NAME;

	if(gameFolderPath[0] != L'\0')
	{
		for(int i = 0; i < this->GetNumGameExeNames(); i++)
		{
			swprintf(exePath, MAX_PATH, L"%s%s", gameFolderPath, this->GetGameExeNameList()[i]);	
			if(!_waccess(exePath, 0))
				return this->GetGameExeNameList()[i];
		}
	}

	return this->GetGameExeNameList()[0];
}

void GlobalGameSettings::InitDialogGG()
{
	HWND hDialog = *PTR_hGGChildDialog;

	if(this->useDefaultRunCommand)
	{
		CheckDlgButton(hDialog, IDC_DEFAULT_RUN_CHECK, BST_CHECKED);
		Edit_Enable(GetDlgItem(hDialog, IDC_RUNCOMMAND_EDIT), FALSE);
		Button_Enable(GetDlgItem(hDialog, IDC_RUNCOMMAND_BUTTON), FALSE);
		
		//CreateRunCommand();
	}
	SendDlgItemMessage(hDialog, IDC_RUNCOMMAND_EDIT, WM_SETTEXT, 0, (LPARAM)this->runCommand);

	SendDlgItemMessage(hDialog, IDC_FOLDERPATH_EDIT, WM_SETTEXT, 0, (LPARAM)this->gameFolderPath);

	CheckDlgButton(hDialog, IDC_FULLSCREEN_CHECK, this->fullScreen);

	if(!this->fullScreenToggle)
		ShowWindow(GetDlgItem(hDialog, IDC_FULLSCREEN_CHECK), SW_HIDE);
}


void GlobalGameSettings::ParseDialogGG()
{
	HWND hDialog = *PTR_hGGChildDialog;

	SendDlgItemMessage(hDialog, IDC_FOLDERPATH_EDIT, WM_GETTEXT, MAX_PATH, (LPARAM)this->gameFolderPath);

	if(BST_CHECKED == IsDlgButtonChecked(hDialog, IDC_DEFAULT_RUN_CHECK))
	{
		this->useDefaultRunCommand = TRUE;
	}
	else
	{
		this->useDefaultRunCommand = FALSE;
		SendDlgItemMessage(hDialog, IDC_RUNCOMMAND_EDIT, WM_GETTEXT, MAX_PATH, (LPARAM)this->runCommand);
	}

	this->fullScreen = IsDlgButtonChecked(hDialog, IDC_FULLSCREEN_CHECK);
}

HWND GlobalGameSettings::CreateGGDialog(HWND hParent) 
{
	return CreateDialogParam(this->hModule, MAKEINTRESOURCE(IDD_GLOBAL), hParent, this->GetGGDialogProc(), (LPARAM)this);
}

DLGPROC GlobalGameSettings::GetGGDialogProc()
{
	return GGDialogProc;
}

BOOL GlobalGameSettings::ValidateGlobalGameSettings(int type, int gIndex)
{
	trimWhiteSpace(this->gameFolderPath);

	if(!this->gameFolderPath[0])
	{
		if(type != VALIDATE_QUIET)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Game Folder Path for \"%s\" cannot be left blank.", this->gameID);
			MessageBoxS(NULL, mbBuffer, L"Invalid Folder Path", MB_OK | MB_ICONSTOP);
		}
		return FALSE;
	}

	// Make sure folder path has trailing '\'
	if(this->gameFolderPath[wcslen(this->gameFolderPath) - 1] != L'\\')
		wcscat_s(this->gameFolderPath, MAX_PATH, L"\\");

	if(!(FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(this->gameFolderPath)))
	{
		if(type != VALIDATE_QUIET)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Invalid Game Folder Path for \"%s\" - the folder does not exist.", this->gameID);
			MessageBoxS(NULL, mbBuffer, L"Invalid Folder Path", MB_OK | MB_ICONSTOP);
		}
		return FALSE;
	}

	if(this->useDefaultRunCommand)
		CreateRunCommand();

	if(this->runCommandChanged)
	{
		this->Install();
		this->runCommandChanged = FALSE;
	}

	return TRUE;
}

INT_PTR CALLBACK GlobalGameSettings::GGDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR tempPathStr[MAX_PATH], quotedTempPathStr[MAX_PATH];
	static GlobalGameSettings *ggSettings;

	switch(message)
	{
	case WM_INITDIALOG:
		ggSettings = (GlobalGameSettings *)lParam;

		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{	
			case IDC_FOLDERPATH_BUTTON:
				swprintf(mbBuffer, MBBUFFER_SIZE, L"Please select your %s folder.", ggSettings->gameID);
				if(pGetFolderSelection(GetParent(hDialog), tempPathStr, mbBuffer))
					SendDlgItemMessage(hDialog, IDC_FOLDERPATH_EDIT, WM_SETTEXT, 0, (LPARAM)tempPathStr);
				break;
			case IDC_RUNCOMMAND_BUTTON:
				swprintf(mbBuffer, MBBUFFER_SIZE, L"Please select your %s executable file or launcher application.", ggSettings->gameID);
				if(pGetFileSelection(GetParent(hDialog), tempPathStr, mbBuffer, NULL, NULL))
				{
					swprintf(quotedTempPathStr, MAX_PATH, L"\"%s\"", tempPathStr);
					SendDlgItemMessage(hDialog, IDC_RUNCOMMAND_EDIT, WM_SETTEXT, 0, (LPARAM)quotedTempPathStr);
				}
				break;
			case IDC_DEFAULT_RUN_CHECK:
				Edit_Enable(GetDlgItem(hDialog, IDC_RUNCOMMAND_EDIT), !IsDlgButtonChecked(hDialog, IDC_DEFAULT_RUN_CHECK));
				Button_Enable(GetDlgItem(hDialog, IDC_RUNCOMMAND_BUTTON), !IsDlgButtonChecked(hDialog, IDC_DEFAULT_RUN_CHECK));
				break;
		}
		break;
	default:
		break;
	}

	return FALSE;
}

BOOL GlobalGameSettings::IsGameWindow(HWND hwnd)
{
	TCHAR buffer[1024];

	if(!hwnd) return FALSE;

	if(this->runInDOSBox)
	{
		GetWindowText(hwnd, buffer, 1024);
		if( (wcsncmp(buffer, L"SDL_app", 7) && wcsncmp(buffer, L"DOSBox", 6) )
				|| wcsstr(buffer, L"Status"))
			return FALSE;
	
		GetClassName(hwnd, buffer, 8);
		if(wcsncmp(buffer, L"SDL_app", 7)) 
			return FALSE;
	}
	else
	{
		GetWindowText(hwnd, buffer, 1024);
		if(wcscmp(this->windowText, buffer))
			return FALSE;
	
		GetClassName(hwnd, buffer, 1024);
		if(wcscmp(this->windowClass, buffer))
			return FALSE;
	}

	return TRUE;
}

void GlobalGameSettings::CloseGameWindow(HWND hWnd)
{
	PostMessage(hWnd, WM_CLOSE, 0, 0);
}

void GlobalGameSettings::CreateRunCommand()
{
	TCHAR srcPath[MAX_PATH], destPath[MAX_PATH], appDataPath[MAX_PATH];
	TCHAR oldRunCommand[MAX_PATH];
	SearchReplace *srStrings, *srChildStrings;
	int totalSRStrings;
	
	wcscpy_s(oldRunCommand, MAX_PATH, this->runCommand);

	if(this->runInDOSBox)
	{
		if(!GetDOSBoxPath(this->gameID))
		{
			this->runCommand[0] = L'\0';
			if(wcscmp(this->runCommand, oldRunCommand))
				this->runCommandChanged = TRUE;
			return;
		}
	
		swprintf(this->runCommand, MAX_PATH, L"\"%s\" \"%s%s\"", (*PTR_settings)->DOSBoxPath, this->gameFolderPath, this->GetGameExeName());
	
		if(!GetDOSBoxConfPath(srcPath))
		{
			if(wcscmp(this->runCommand, oldRunCommand))
				this->runCommandChanged = TRUE;
			return;
		}
	
		GetCurrentDirectory(MAX_PATH, appDataPath);
		swprintf(destPath, MAX_PATH, L"%s\\%s\\%s-%s", appDataPath, DOSBOX_CONFIGS_FOLDER, this->gameID, wcsrchr(srcPath, L'\\') + 1);
		_wmkdir(DOSBOX_CONFIGS_FOLDER);	

		totalSRStrings = GetNumConfigReplaceStrings() + NUM_CONFIG_REPLACE_STRINGS;
		srStrings = (SearchReplace *)malloc(sizeof(SearchReplace) * totalSRStrings);
		srChildStrings = GetConfigReplaceStrings();

		memcpy_s(srStrings, totalSRStrings * sizeof(SearchReplace), ConfigReplaceStrings, NUM_CONFIG_REPLACE_STRINGS * sizeof(SearchReplace));
		memcpy_s(srStrings + NUM_CONFIG_REPLACE_STRINGS, totalSRStrings * sizeof(SearchReplace), srChildStrings, GetNumConfigReplaceStrings() * sizeof(SearchReplace));

		if(!_waccess(destPath, 0) || ReplaceLinesInFile(srcPath, destPath, srStrings, totalSRStrings))
		{
			wcscat_s(this->runCommand, MAX_PATH, L" -conf ");
			wcscat_s(this->runCommand, MAX_PATH, L"\"");
			wcscat_s(this->runCommand, MAX_PATH, destPath);
			wcscat_s(this->runCommand, MAX_PATH, L"\"");
		}

		free(srStrings);
	}
	else
	{
		swprintf(this->runCommand, MAX_PATH, L"\"%s%s\"", this->gameFolderPath, this->GetGameExeName());
	}

	if(wcscmp(this->runCommand, oldRunCommand))
		this->runCommandChanged = TRUE;
}

void GlobalGameSettings::FreeSession(SessionInfo *session)
{
	free(session);
}

void GlobalGameSettings::LoadSettings(config_setting_t *group)
{

}

void GlobalGameSettings::SaveSettings(config_setting_t *group)
{

}

HWND GlobalGameSettings::IsGameWindowForeground()
{
	HWND hFG;

	hFG = GetForegroundWindow();
	if(!hFG) 
		return NULL;
	
	if(IsGameWindow(hFG))
		return hFG;

	return NULL;
}

HWND GlobalGameSettings::IsGameWindowTopMost()
{
	HWND hFG;

	if((hFG = IsGameWindowForeground()) && GetWindowLong(hFG, GWL_EXSTYLE) & WS_EX_TOPMOST)
		return hFG;

	return NULL;
}

void GlobalGameSettings::AssignStatics()
{
	pGetFolderSelection = GetFolderSelection;
	pGetFileSelection = GetFileSelection;
	pCreateDialogFont = CreateDialogFont;
}

HWND GlobalGameSettings::FindGameWindow()
{
	HWND hWnd;
	
	if(hWnd = IsGameWindowForeground()) 
		return hWnd;

	CurrentGG = this;

	if(!EnumWindows(EnumIsGameWindow, (LPARAM)&hWnd))
		return hWnd;

	return NULL;
}

BOOL CALLBACK GlobalGameSettings::EnumIsGameWindow(HWND hwnd, LPARAM lParam)
{
	if(CurrentGG->IsGameWindow(hwnd))
	{
		*(HWND *)lParam = hwnd;
		return FALSE;
	}
	else
		*(HWND *)lParam = NULL;

	return TRUE;
}

void GlobalGameSettings::KillGame()
{
	int ret;
	DWORD pid;
	HANDLE hProc;
	HWND hWnd;
	
	while(hWnd = FindGameWindow())
	{
		GetWindowThreadProcessId(hWnd, &pid);
		if(!(hProc = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid)))
			break;

		CurrentGG = this;
		EnumWindows(EnumCloseGameWindow, LPARAM(pid));

		ret = WaitForSingleObject(hProc, 1000);
				
		if(ret == WAIT_TIMEOUT)
		{
			if(!TerminateProcess(hProc, 0))
			{
				CloseHandle(hProc);
				break;
			}
		}
		else if(ret != WAIT_OBJECT_0)
		{
			CloseHandle(hProc);
			break;	
		}
		CloseHandle(hProc);
	}
}

BOOL CALLBACK GlobalGameSettings::EnumCloseGameWindow(HWND hWnd, LPARAM lParam)
{
	DWORD pid;

	GetWindowThreadProcessId(hWnd, &pid);
	if(pid == lParam)
		CurrentGG->CloseGameWindow(hWnd);
	
	return TRUE;
}

void GlobalGameSettings::InitInput(HWND hWnd)
{
	POINT pt;

	if(!IsGameWindowTopMost())
	{	
		pt.x = pt.y = 0;
		ClientToScreen(hWnd, &pt);
		SetCursorPos(pt.x, pt.y); 
		PressLButton();
	}

	SetMouseModifier(hWnd);
}

void GlobalGameSettings::SetMouseModifier(HWND hWnd)
{
	if(!(GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST))
		*PTR_MouseModifier = MOUSE_MODIFIER;	
	else
		*PTR_MouseModifier = FS_MOUSE_MODIFIER;
}

void Game::GetGameTip(TCHAR *buffer)
{
	swprintf(buffer, MBBUFFER_SIZE, L"Once you have finished your turn, DO NOT select End Turn from within %s. Instead you must use the PlayMailer Send Game hotkey: CTRL + SHIFT + S, to send the game to the next player.\n\n(To view all in-game hotkeys, press CTRL + SHIFT + H.)", this->ggSettings->gameID);
}

void Game::CreateTeamSettingsDialog()
{
	DialogBoxParamS(this->ggSettings->hModule, MAKEINTRESOURCE(IDD_TEAM), *PTR_hMainWnd, this->GetTeamSettingsDialogProc(), (LPARAM)this);
}

void Game::FreeTeam(Team *team)
{
	free(team);
}

BOOL Game::EnterFullScreen()
{
	HWND hFG;

	if(!ggSettings->fullScreenToggle)
		return FALSE;

	hFG = GetForegroundWindow();	
	if(!ggSettings->IsGameWindow(hFG) || (GetWindowLong(hFG, GWL_EXSTYLE) & WS_EX_TOPMOST))
		return FALSE;

	ToggleFullScreen();
	MoveMouse(10, 10);
	//DebugWindow(hFG);

	return TRUE;
}

BOOL Game::PreSaveGameEvent()
{
	HWND hWnd = GetForegroundWindow();
	ggSettings->SetMouseModifier(hWnd);

	return TRUE;
}

BOOL Game::PreNewGameEvent()
{
	if(ggSettings->WarnBeforeNewGame)
	{
		swprintf(mbBuffer, MBBUFFER_SIZE, L"After you press OK, PlayMailer will begin setting up a new game within %s. Please do not use your mouse or keyboard until setup is complete.", this->ggSettings->gameID);
		MessageBoxS(NULL, mbBuffer, L"Prepare for Setup", MB_OK);
	}

	return TRUE;
}

void Game::PostNewGameEvent()
{
	if(ggSettings->WarnBeforeNewGame)
		MessageBoxS(NULL, L"Setup is complete. You may now begin your turn.", L"Setup Complete", MB_OK);
}

BOOL Game::LeaveFullScreen()
{
	HWND hFG;

	if(!ggSettings->fullScreenToggle)
		return FALSE;

	hFG = GetForegroundWindow();	
	if(!ggSettings->IsGameWindow(hFG) || !(GetWindowLong(hFG, GWL_EXSTYLE) & WS_EX_TOPMOST))
		return FALSE;

	ToggleFullScreen();
	return TRUE;
}

TCHAR *Game::GetSaveFilePath(TCHAR *path)
{
	TCHAR buffer[MAX_PATH];

	swprintf(path, MAX_PATH, L"%s%s", GetSaveFolderPath(), GetSaveFileName(buffer));

	return path;
}

TCHAR *Game::GetSaveFolderPath()
{
	return this->ggSettings->gameFolderPath;
}

void Game::ToggleFullScreen()
{
	PressHotKey(VK_LMENU, VK_RETURN);
	//Sleep(3000);
}

BOOL Game::ApplyGameSettings()
{
	return TRUE;
}

uint32_t Game::GetSettingsChanging()
{
	return NULL;
}

void Game::SetGameSettingsMask()
{

}

BOOL Game::CheckGameSettingsChanged(SessionInfo *newSession)
{
	return FALSE;
}

