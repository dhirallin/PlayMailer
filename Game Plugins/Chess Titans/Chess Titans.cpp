#include "stdafx.h"
#include "resource.h"
#include "Chess Titans.h"

TCHAR *GChessTitans::FactionNames[] = {L"White", L"Black"};
const int GChessTitans::NUM_DIFFICULTIES	= 10;

PLUGINS_API GlobalGameSettings *CreatePlugin()
{
	return new GGChessTitansSettings();
}

PLUGINS_API void ReleasePlugin(GlobalGameSettings *ggs)
{
	free(ggs);
	ggs = 0;
}

void GGChessTitansSettings::InitDialogGG()
{
	HWND hDialog = *PTR_hGGChildDialog;

	if(this->useDefaultRunCommand)
	{
		CheckDlgButton(hDialog, IDC_DEFAULT_RUN_CHECK, BST_CHECKED);
		Edit_Enable(GetDlgItem(hDialog, IDC_RUNCOMMAND_EDIT), FALSE);
		Button_Enable(GetDlgItem(hDialog, IDC_RUNCOMMAND_BUTTON), FALSE);
		CreateRunCommand();
	}
	SendDlgItemMessage(hDialog, IDC_RUNCOMMAND_EDIT, WM_SETTEXT, 0, (LPARAM)this->runCommand);

	CheckDlgButton(hDialog, IDC_FULLSCREEN_CHECK, this->fullScreen);
}

void GGChessTitansSettings::ParseDialogGG()
{
	HWND hDialog = *PTR_hGGChildDialog;

	if(BST_CHECKED == IsDlgButtonChecked(hDialog, IDC_DEFAULT_RUN_CHECK))
	{
		CreateRunCommand();
		this->useDefaultRunCommand = TRUE;
	}
	else
	{
		this->useDefaultRunCommand = FALSE;
		SendDlgItemMessage(hDialog, IDC_RUNCOMMAND_EDIT, WM_GETTEXT, MAX_PATH, (LPARAM)this->runCommand);
	}

	this->fullScreen = IsDlgButtonChecked(hDialog, IDC_FULLSCREEN_CHECK);
}

BOOL GGChessTitansSettings::ValidateGlobalGameSettings(int type, int gIndex)
{
	if(this->useDefaultRunCommand)
		CreateRunCommand();

	return TRUE;
}

INT_PTR CALLBACK GGChessTitansSettings::GGDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
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

void GGChessTitansSettings::CreateRunCommand()
{
	ExpandEnvironmentStrings(DEFAULT_RUN_COMMAND, runCommand, MAX_PATH);
}
  
SessionInfo *GGChessTitansSettings::AllocSession()
{
	return (SessionInfo *)new GChessTitans(this);
}

void GChessTitans::GetGameTip(TCHAR *buffer)
{
	swprintf(buffer, MBBUFFER_SIZE, L"After you have made your move, send the game to the next player by pressing CTRL + SHIFT + S.\n\n(To view all PlayMailer in-game hotkeys, press CTRL + SHIFT + H.)");
}

void GChessTitans::MakeSaveFolder()
{
	TCHAR buffer[MAX_PATH];
	TCHAR *lastSlash, *secondLastSlash;

	wcscpy_s(buffer, MAX_PATH, GetSaveFolderPath());
	lastSlash = wcsrchr(buffer, L'\\');
	*lastSlash = 0;
	
	secondLastSlash = wcsrchr(buffer, L'\\');
	*secondLastSlash = 0;
	_wmkdir(buffer);

	*secondLastSlash = L'\\';
	*lastSlash = L'\\';
	_wmkdir(buffer);
}

BOOL GChessTitans::NewGame()
{
	GChessTitansSettings *gameSettings = &this->gameSettings;
	HWND hSaveWindow;

	for(int i = 0; i < 100; i++)
	{
		if(hSaveWindow = FindWindow(NULL, L"Saved Game Found"))
		{
			SetForegroundWindow(hSaveWindow);
			PressKey(VK_ESCAPE);
			break;
		}
		SleepC(40);
	}
	
	CheckFullScreen(this);

	if(numPlayers == 1 || GetNumFactions(this) == 1)
	{
		PressKey(VK_F5);

		if(this->players[0]->faction == 0)
			PressKey(VK_W);
		else
			PressKey(VK_B);

		for(int i = 0; i < 11; i++)
			PressKey(VK_TAB);
		for(int i = 0; i < 9; i++)
			PressKey(VK_LEFT);
		for(int i = 0; i < gameSettings->difficulty - 1; i++)
			PressKey(VK_RIGHT);

		PressKey(VK_RETURN);
		PressKey(VK_N);
	}
	else
	{
		PressKey(VK_F3);
		PressKey(VK_N);
	}

	return TRUE;
}

BOOL GChessTitans::LoadGame()
{
	HWND hSaveWindow;

	for(int i = 0; i < 100; i++)
	{
		if(hSaveWindow = FindWindow(NULL, L"Saved Game Found"))
		{
			SetForegroundWindow(hSaveWindow);
			PressKey(VK_RETURN);
			break;
		}
		SleepC(50);
	}
	
	CheckFullScreen(this);

	return TRUE;
}

void GGChessTitansSettings::CloseGameWindow(HWND hWnd)
{
	HWND hSaveWindow;
	
	PostMessage(hWnd, WM_CLOSE, 0, 0);
	for(int i = 0; i < 30; i++)
	{
		if(hSaveWindow = FindWindow(NULL, L"Exit Game"))
		{
			SetForegroundWindow(hSaveWindow);
			PressKey(VK_D);
			break;
		}
		SleepC(50);
	}
}

BOOL GChessTitans::SaveGame()
{
	HWND hSaveWindow;
	
	StartSaveFileThread(this);
	
	PressHotKey(VK_MENU, VK_F4);
	for(int i = 0; i < 30; i++)
	{
		if(hSaveWindow = FindWindow(NULL, L"Exit Game"))
		{
			SetForegroundWindow(hSaveWindow);
			PressKey(VK_RETURN);
			break;
		}
		SleepC(50);
	}
	
	if(!WaitForWriteFileThread())
		return FALSE;

	SetForegroundWindow(*PTR_hMainWnd);
	
	return TRUE;
}

void GChessTitans::PostSendEvent()
{
	if(!gameSettings.exitAfterTurn && !isYourTurn(this))
		_LoadGame(this, FALSE);
}

BOOL GGChessTitansSettings::IsGameWindow(HWND hwnd)
{
	TCHAR wndBuffer[1024], classBuffer[1024];

	if(!hwnd) return FALSE;

	GetWindowText(hwnd, wndBuffer, 1024);
	GetClassName(hwnd, classBuffer, 1024);

	if(!wcscmp(L"Saved Game Found", wndBuffer) && !wcscmp(L"#32770", classBuffer))
		return TRUE; 

	if(!wcscmp(L"Chess Titans", wndBuffer) && !wcscmp(L"ChessWindowClass", classBuffer))
		return TRUE;
	
	return FALSE;
}

BOOL GChessTitans::EnterFullScreen()
{
	HWND hFG;

	hFG = GetForegroundWindow();	
	if(!ggSettings->IsGameWindow(hFG) || IsZoomed(hFG))
		return FALSE;

	PressHotKey(VK_MENU, VK_SPACE);
	PressKey(VK_X);

	return TRUE;
}

BOOL GChessTitans::LeaveFullScreen()
{
	HWND hFG;

	hFG = GetForegroundWindow();	
	if(!ggSettings->IsGameWindow(hFG) || !IsZoomed(hFG))
		return FALSE;

	PressHotKey(VK_MENU, VK_SPACE);
	PressKey(VK_R);

	return TRUE;
}

void GChessTitans::InitGameSettingsDialog()
{
	GChessTitansSettings *game;
	HWND hDlg = *PTR_hSessionSettingsDialog, hGameDlg;
	
	hGameDlg = *PTR_hGameSettingsDialog = CreateDialogParam(this->ggSettings->hModule, MAKEINTRESOURCE(IDD_GAME), *PTR_hSessionSettingsDialog, NULL, NULL);
	SetWindowPos(*PTR_hGameSettingsDialog, GetDlgItem(*PTR_hSessionSettingsDialog, EDITPLAYERS_BUTTON), DLUToPixelsX(hDlg, GAME_SETTINGS_PANE_X), DLUToPixelsY(hDlg, GAME_SETTINGS_PANE_Y), 0, 0, SWP_NOSIZE);

	SendDlgItemMessage(hGameDlg, IDC_DIFFICULTY_SLIDER, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(1, 10));

	game = &this->gameSettings;
				
	SendDlgItemMessage(hGameDlg, IDC_DIFFICULTY_SLIDER, TBM_SETPOS, TRUE, game->difficulty);
	if(this->turnNumber > 1)
		EnableWindow(GetDlgItem(*PTR_hGameSettingsDialog, IDC_DIFFICULTY_SLIDER), FALSE);
		
	CheckDlgButton(hGameDlg, IDC_EXIT_CHECK, game->exitAfterTurn);
}

BOOL GChessTitans::ParseGameSettingsDialog()
{
	GChessTitansSettings *game = &this->gameSettings;
	HWND hDlg = *PTR_hGameSettingsDialog;

	game->difficulty = (uint8_t)SendDlgItemMessage(hDlg, IDC_DIFFICULTY_SLIDER, TBM_GETPOS, 0, 0); 
	game->exitAfterTurn = IsDlgButtonChecked(hDlg, IDC_EXIT_CHECK);

	return ValidateGameSettings(VALIDATE_GUI, 0);
}

BOOL GChessTitans::ValidateGameSettings(BOOL type, int sIndex)
{
	TCHAR cfgError[MBBUFFER_SIZE];
	GChessTitansSettings *game = &this->gameSettings;
	HWND hWnd = NULL;

	cfgError[0] = L'\0';
	
	if(type == VALIDATE_FILE) 
		swprintf(cfgError, sizeof(cfgError), L"Error in session \"%s\". ", this->sessionName);
	else if(type == VALIDATE_GUI)
		hWnd = *PTR_hSessionSettingsDialog;

	if(game->difficulty < 1 || game->difficulty > NUM_DIFFICULTIES)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sSetting \"computer_difficulty\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	return TRUE;
}

void GChessTitans::InitGameSettings()
{
	memset(&gameSettings, 0, sizeof(GChessTitansSettings));

	gameSettings.difficulty = 5;
	gameSettings.exitAfterTurn = FALSE;
}

void GChessTitans::SaveGameSettings(config_setting_t *group)
{
	cfgSetInt(group, L"computer_difficulty", this->gameSettings.difficulty);
	cfgSetBool(group, L"exit_after_turn", this->gameSettings.exitAfterTurn);
}

void GChessTitans::LoadGameSettings(config_setting_t *group)
{
	this->gameSettings.difficulty = cfgGetInt(group, L"computer_difficulty");
	this->gameSettings.exitAfterTurn = cfgGetBool(group, L"exit_after_turn");
}

void GChessTitans::BuildEmailGameSettings(config_setting_t *group)
{
	cfgSetInt(group, L"computer_difficulty", this->gameSettings.difficulty);
}

void GChessTitans::ParseEmailGameSettings(config_setting_t *group)
{
	this->gameSettings.difficulty = cfgGetInt(group, L"computer_difficulty");
}

void GChessTitans::UpdateEmailGameSettings(SessionInfo *in)
{
	GChessTitans *wIn = (GChessTitans *)in;

	this->gameSettings.difficulty = wIn->gameSettings.difficulty;
}

TCHAR *GChessTitans::GetSaveFileName(TCHAR *name)
{
	wcscpy_s(name, MAX_PATH, L"ChessTitans.ChessTitansSave-ms");

	return name;
}

TCHAR *GChessTitans::GetSaveFolderPath()
{
	ExpandEnvironmentStrings(L"%HOMEDRIVE%%HOMEPATH%\\Saved Games\\Microsoft Games\\Chess Titans\\", ((GGChessTitansSettings *)ggSettings)->SAVE_FOLDER_PATH, MAX_PATH);
	
	return ((GGChessTitansSettings *)ggSettings)->SAVE_FOLDER_PATH;
}





