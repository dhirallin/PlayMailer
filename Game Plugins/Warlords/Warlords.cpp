#include "stdafx.h"
#include "resource.h"
#include "warlords.h"

// Bit flags
const int GWarlords::COMPUTER_ENHANCED	= 8;
const int GWarlords::INTENSE_COMBAT		= 31;

const int GWarlords::SCENARIO_NONE		= 0;
const int GWarlords::SCENARIO_WLED		= 1;
const int GWarlords::SCENARIO_WLEDIT	= 2;

TCHAR *GWarlords::FactionNames[] = {L"The Sirians", L"Storm Giants", L"Grey Dwarves", L"Orcs of Kor", L"Elvallie", L"The Selentines", L"Horse Lords", L"Lord Bane"}; 

const int GWarlords::NUM_DIFFICULTIES	= 4;
TCHAR *GWarlords::DifficultyNames[] = {L"Knight (Easiest)", L"Baron", L"Lord", L"Warlord (Hardest)"};

TCHAR *GGWarlordsSettings::GAME_EXE_NAME_LIST[] = {L"warlords.exe", L"wl210.exe"};
int GGWarlordsSettings::NUM_GAME_EXE_NAMES = 2;

SearchReplace GGWarlordsSettings::ConfigReplaceStrings[] = {L"aspect=", L"aspect=true"};
int GGWarlordsSettings::NUM_CONFIG_REPLACE_STRINGS = 1;

TCHAR **GGWarlordsSettings::GetGameExeNameList()
{
	return GAME_EXE_NAME_LIST; 
}

int GGWarlordsSettings::GetNumGameExeNames()
{
	return NUM_GAME_EXE_NAMES;
}

PLUGINS_API GlobalGameSettings *CreatePlugin()
{
	return new GGWarlordsSettings();
}

PLUGINS_API void ReleasePlugin(GlobalGameSettings *ggs)
{
	free(ggs);
	ggs = 0;
}

SessionInfo *GGWarlordsSettings::AllocSession()
{
	return (SessionInfo *)new GWarlords(this);
}

void GGWarlordsSettings::LoadSettings(config_setting_t *group)
{
	lastObserveState = cfgGetBool(group, L"last_observe_state");
	lastSoundState = cfgGetBool(group, L"last_sound_state");
}

void GGWarlordsSettings::SaveSettings(config_setting_t *group)
{
	cfgSetBool(group, L"last_observe_state", lastObserveState);
	cfgSetBool(group, L"last_sound_state", lastSoundState);
}

SearchReplace *GGWarlordsSettings::GetConfigReplaceStrings()
{
	return ConfigReplaceStrings;
}

int GGWarlordsSettings::GetNumConfigReplaceStrings()
{
	return NUM_CONFIG_REPLACE_STRINGS;
}

INT_PTR CALLBACK GWarlords::GameSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SessionInfo *session;

	switch(message)
	{
		case WM_INITDIALOG:
			session = (SessionInfo *)lParam;
			return TRUE;
		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED) 
				GWarlords::UpdateGameSettingsDialog(hDialog);
			break;
		default:
			break;
	}

	return FALSE;
}

void GWarlords::UpdateGameSettingsDialog(HWND hGameDlg)
{
	BOOL show;

	show = (IsDlgButtonChecked(hGameDlg, IDC_SCENARIO_NONE_RADIO) ? false : true);
	
	EnableWindow(GetDlgItem(hGameDlg, IDC_SCENARIOPATH_EDIT), show);
	EnableWindow(GetDlgItem(hGameDlg, IDC_SCENARIOPATH_BUTTON), show);
}

BOOL GWarlords::NewGame()
{
	GWarlordsSettings *gameSettings = &this->gameSettings;

	gameSettings->settingsMask = 0;
	for(int i = 0; i < this->numPlayers; i++)
		SET_BIT(gameSettings->settingsMask, this->players[i]->faction);  
	
	for(int i = 0; i < NUM_FACTIONS; i++)
	{
		if(!CHECK_BIT(gameSettings->settingsMask, i))
		{
			for(int j = 0; j < gameSettings->difficulty + 1; j++)
				PressKey(VK_F1 + i);
		}
	}

	PressKey(VK_S);

	CheckFullScreen(this);

	return TRUE;
}

void GWarlords::GetGameTip(TCHAR *buffer)
{
	swprintf(buffer, MBBUFFER_SIZE, L"Once you have finished your turn, DO NOT select End Turn from within %s. Instead you must use the PlayMailer Send Game hotkey:\nCTRL + SHIFT + S, to send the game to the next player.\n\nMake sure that when you use this hotkey you are in the main %s window, e.g. not in the City Production menu.\n\n(To view all in-game hotkeys, press CTRL + SHIFT + H.)", this->ggSettings->gameID, this->ggSettings->gameID);
}

BOOL GWarlords::LoadGame()
{
	int saveSlot;

	saveSlot = this->gameSettings.saveSlot;

	// Bring up load window
	if(*PTR_NewGameInstance) 
	{	
		PressKey(VK_L);
		SleepC(4500); // Wait for Load window to appear
	}
	else
	{
		ResetMouse();
		MoveMouseMod(0, 345);
		MoveMouseMod(980, 0);
		PressKey(VK_RETURN);
		
		PressHotKey(VK_LMENU, VK_L);
		SleepC(750);
	}

	// Activate cursor
	PressKey(VK_DOWN);
	PressKey(VK_UP);
	
	// Move cursor to top left button
	for(int j = 0; j < 4; j++)
		PressKey(VK_UP);
	PressKey(VK_LEFT);

	// Select save slot
	for(int j = 0; j < (saveSlot - 1) % 4 + 1; j++)
		PressKey(VK_DOWN);
	if(saveSlot >= 5) 
		PressKey(VK_RIGHT);
	PressKey(VK_RETURN);

	// Press Load button
	if(saveSlot >= 5) 
		PressKey(VK_LEFT);
	for(int j = 0; j < (saveSlot - 1) % 4 + 1; j++)
		PressKey(VK_UP);
	PressKey(VK_RETURN);

	SleepC(3000); // Wait for game to load
	
	if(ApplyGameSettings())
		SaveGame();

	// End Turn
	PressHotKey(VK_LMENU, VK_E);

	CheckFullScreen(this);

	return TRUE;
}

BOOL GWarlords::SaveGame()
{
	int saveSlot;

	saveSlot = this->gameSettings.saveSlot;

	ApplyGameSettings();

	// Exit any menus currently displaying
	ResetMouse();
	MoveMouseMod(0, 345);
	MoveMouseMod(980, 0);
	PressKey(VK_RETURN);

	// Bring up save window
	PressHotKey(VK_LMENU, VK_S);

	// Activate cursor
	PressKey(VK_DOWN);
	PressKey(VK_UP);

	// Move cursor to top left button
	for(int j = 0; j < 4; j++)
		PressKey(VK_UP);
	PressKey(VK_LEFT);

	// Select save slot
	for(int j = 0; j < (saveSlot - 1) % 4 + 1; j++)
		PressKey(VK_DOWN);
	if(saveSlot >= 5) 
		PressKey(VK_RIGHT);
	PressKey(VK_RETURN);

	SleepC(200);

	PressStringKeys(this->sessionName, 15);

	PressKey(VK_RETURN);

	// Press Save button
	if(saveSlot >= 5) 
		PressKey(VK_LEFT);
	for(int j = 0; j < (saveSlot - 1) % 4 + 1; j++)
		PressKey(VK_UP);
	
	StartSaveFileThread(this);
	PressKey(VK_RETURN);
	return WaitForSaveFileThread();
}

void GWarlords::InitGameSettingsDialog()
{
	GWarlordsSettings *game;
	HWND hDlg = *PTR_hSessionSettingsDialog;
	
	*PTR_hGameSettingsDialog = CreateDialogParam(this->ggSettings->hModule, MAKEINTRESOURCE(IDD_GAME), *PTR_hSessionSettingsDialog, GameSettingsDialogProc, (LPARAM)this);
	SetWindowPos(*PTR_hGameSettingsDialog, GetDlgItem(*PTR_hSessionSettingsDialog, EDITPLAYERS_BUTTON), DLUToPixelsX(hDlg, GAME_SETTINGS_PANE_X), DLUToPixelsY(hDlg, GAME_SETTINGS_PANE_Y), 0, 0, SWP_NOSIZE);

	for(int i = 0; i < NUM_DIFFICULTIES; i++)
		SendDlgItemMessage(*PTR_hGameSettingsDialog, IDC_DIFFICULTY_COMBO, CB_ADDSTRING, 0, (LPARAM)DifficultyNames[i]); 
	
	game = &this->gameSettings;

	SendDlgItemMessage(*PTR_hGameSettingsDialog, IDC_SAVESLOT_RADIO1 + game->saveSlot - 1, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage(*PTR_hGameSettingsDialog, IDC_DIFFICULTY_COMBO, CB_SETCURSEL, game->difficulty, 0);

	CheckDlgButton(*PTR_hGameSettingsDialog, IDC_OBSERVEOFF_CHECK, game->observeOff);
	CheckDlgButton(*PTR_hGameSettingsDialog, IDC_SOUNDOFF_CHECK, game->soundOff);
	CheckDlgButton(*PTR_hGameSettingsDialog, IDC_ENHANCE_CHECK, game->enhanced);
	CheckDlgButton(*PTR_hGameSettingsDialog, IDC_INTENSE_COMBAT_CHECK, game->intenseCombat);
	
	if(this->turnNumber > 1)
		EnableWindow(GetDlgItem(*PTR_hGameSettingsDialog, IDC_DIFFICULTY_COMBO), FALSE);

	SendDlgItemMessage(*PTR_hGameSettingsDialog, IDC_SCENARIO_NONE_RADIO + game->scenarioType, BM_SETCHECK, BST_CHECKED, 0);

	UpdateGameSettingsDialog(*PTR_hGameSettingsDialog);
}

BOOL GWarlords::ParseGameSettingsDialog()
{
	GWarlordsSettings *game = &this->gameSettings;
	
	game->observeOff = FALSE;
	game->soundOff = FALSE;
	game->enhanced = FALSE;
	game->intenseCombat = FALSE;
	
	// Find checked radio button
	for(int i = IDC_SAVESLOT_RADIO1; i <= IDC_SAVESLOT_RADIO8; i++)
	{
		if(IsDlgButtonChecked(*PTR_hGameSettingsDialog, i)) game->saveSlot = i - IDC_SAVESLOT_RADIO1 + 1;
	}

	if(BST_CHECKED == IsDlgButtonChecked(*PTR_hGameSettingsDialog, IDC_OBSERVEOFF_CHECK))
		game->observeOff = TRUE;
	if(BST_CHECKED == IsDlgButtonChecked(*PTR_hGameSettingsDialog, IDC_SOUNDOFF_CHECK))
		game->soundOff = TRUE;
	if(BST_CHECKED == IsDlgButtonChecked(*PTR_hGameSettingsDialog, IDC_ENHANCE_CHECK))
		game->enhanced = TRUE;
	if(BST_CHECKED == IsDlgButtonChecked(*PTR_hGameSettingsDialog, IDC_INTENSE_COMBAT_CHECK))
		game->intenseCombat = TRUE;

	game->difficulty = (uint8_t)SendDlgItemMessage(*PTR_hGameSettingsDialog, IDC_DIFFICULTY_COMBO, CB_GETCURSEL, 0, 0); 

	return ValidateGameSettings(VALIDATE_GUI, 0);
}

BOOL GWarlords::ValidateGameSettings(BOOL type, int sIndex)
{
	TCHAR cfgError[MBBUFFER_SIZE];
	GWarlordsSettings *game = &this->gameSettings;
	HWND hWnd = NULL;

	cfgError[0] = L'\0';
	
	if(type == VALIDATE_FILE) 
		swprintf(cfgError, sizeof(cfgError), L"Error in %s in session #%d. ", SESSIONS_CONFIG_FILE, sIndex);
	else if(type == VALIDATE_GUI)
		hWnd = *PTR_hSessionSettingsDialog;

	if(type != VALIDATE_EMAIL)
	{
		// Check save slot is valid
		if(game->saveSlot < 1 || game->saveSlot > 8)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Setting \"save_slot\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}

	if(game->difficulty >= NUM_DIFFICULTIES)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Setting \"computer_difficulty\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	return TRUE;
}

void GWarlords::InitGameSettings()
{
	memset(&gameSettings, 0, sizeof(GWarlordsSettings));

	this->gameSettings.saveSlot = 1;
	this->gameSettings.observeOff = TRUE;
	this->gameSettings.difficulty = 3;
	this->gameSettings.scenarioType = SCENARIO_NONE;
	this->gameSettings.scenarioPath[0] = L'\0';
}

void GWarlords::SaveGameSettings(config_setting_t *group)
{
	cfgSetInt(group, L"save_slot", this->gameSettings.saveSlot);
	cfgSetBool(group, L"observe_off", this->gameSettings.observeOff);
	cfgSetBool(group, L"sound_off", this->gameSettings.soundOff);
	cfgSetInt(group, L"computer_difficulty", this->gameSettings.difficulty);
	cfgSetBool(group, L"computer_enhanced", this->gameSettings.enhanced);
	cfgSetBool(group, L"intense_combat", this->gameSettings.intenseCombat);
	cfgSetInt(group, L"settings_mask", this->gameSettings.settingsMask);
}

void GWarlords::LoadGameSettings(config_setting_t *group)
{
	this->gameSettings.saveSlot = cfgGetInt(group, L"save_slot");
	this->gameSettings.observeOff = cfgGetBool(group, L"observe_off");
	this->gameSettings.soundOff = cfgGetBool(group, L"sound_off");
	this->gameSettings.difficulty = cfgGetInt(group, L"computer_difficulty");
	this->gameSettings.enhanced = cfgGetBool(group, L"computer_enhanced");
	this->gameSettings.intenseCombat = cfgGetBool(group, L"intense_combat");
	this->gameSettings.settingsMask = cfgGetInt(group, L"settings_mask");
}

void GWarlords::BuildEmailGameSettings(config_setting_t *group)
{
	cfgSetInt(group, L"computer_difficulty", this->gameSettings.difficulty);
	cfgSetBool(group, L"computer_enhanced", this->gameSettings.enhanced);
	cfgSetBool(group, L"intense_combat", this->gameSettings.intenseCombat);
}

void GWarlords::ParseEmailGameSettings(config_setting_t *group)
{
	this->gameSettings.difficulty = cfgGetInt(group, L"computer_difficulty");
	this->gameSettings.enhanced = cfgGetBool(group, L"computer_enhanced");
	this->gameSettings.intenseCombat = cfgGetBool(group, L"intense_combat");
}

void GWarlords::UpdateEmailGameSettings(SessionInfo *in)
{
	GWarlords *wIn = (GWarlords *)in;

	this->gameSettings.enhanced = wIn->gameSettings.enhanced;
	this->gameSettings.intenseCombat = wIn->gameSettings.intenseCombat;
	this->gameSettings.difficulty = wIn->gameSettings.difficulty;
}

BOOL GWarlords::ApplyGameSettings()
{
	uint32_t settingsChanging;
	BOOL settingsChanged = FALSE;
	GGWarlordsSettings *ggSettings = (GGWarlordsSettings *)this->ggSettings;
	GWarlordsSettings *gameSettings = &this->gameSettings;
	BOOL observeOff = gameSettings->observeOff;
	BOOL soundOff = gameSettings->soundOff;
	
	// Toggle Observe Off
	if((*PTR_NewGameInstance == TRUE && observeOff) ||
		(*PTR_NewGameInstance == FALSE && observeOff != ggSettings->lastObserveState))
	{
		ToggleObserveOff();
	}
	ggSettings->lastObserveState = observeOff;

	// Toggle Sound Off
	if((*PTR_NewGameInstance == TRUE && soundOff) ||
		(*PTR_NewGameInstance == FALSE && soundOff != ggSettings->lastSoundState))
	{
		ToggleSoundOff();
	}
	ggSettings->lastSoundState = soundOff;

	// Update Control Settings (human/non-human, enhanced, intense combat)
	if(settingsChanging = GetSettingsChanging())
	{
		PressKey(VK_O);
		SleepC(1000);

		// Activate cursor
		PressKey(VK_DOWN);
		PressKey(VK_UP);

		// Move cursor to first player's human/nonhuman flag.
		for(int j = 0; j < 9; j++)
			PressKey(VK_UP);
		for(int j = 0; j < 2; j++)
			PressKey(VK_LEFT);
		PressKey(VK_RIGHT);
	
		for(int faction = 0; faction < NUM_FACTIONS; faction++)
		{
			if(CHECK_BIT(settingsChanging, faction))
				PressKey(VK_RETURN);

			// Set enhanced
			if(CHECK_BIT(settingsChanging, COMPUTER_ENHANCED + faction))
			{
				PressKey(VK_RIGHT);
				PressKey(VK_RETURN);
				PressKey(VK_LEFT);
			}

			// Go down to next player
			PressKey(VK_DOWN);
		}
				
		// Set intense combat
		if(CHECK_BIT(settingsChanging, INTENSE_COMBAT))
			PressKey(VK_RETURN);

		// Go to OK button and press it
		PressKey(VK_DOWN);
		PressKey(VK_RETURN);

		SleepC(500);

		settingsChanged = TRUE;
	}
	
	*PTR_NewGameInstance = FALSE;
	SaveGlobalGameSettings();
	SaveSessionList();

	return settingsChanged;
}

uint32_t GWarlords::GetSettingsChanging()
{
	GWarlordsSettings *game = &this->gameSettings;
	uint32_t settingsChanging, factionMask = 0;

	for(int i = 0; i < this->numPlayers; i++)
		SET_BIT(factionMask, this->players[i]->faction);

	settingsChanging = game->settingsMask ^ factionMask;

	// Set enhanced flags that are changing
	settingsChanging ^= (uint8_t) (~(game->enhanced - 1) 
		& (settingsChanging ^ ~game->settingsMask)) << COMPUTER_ENHANCED;
	
	settingsChanging ^= game->intenseCombat << INTENSE_COMBAT;
	
	SetGameSettingsMask();

	return settingsChanging;
}

void GWarlords::SetGameSettingsMask()
{
	GWarlordsSettings *game = &this->gameSettings;
	
	game->settingsMask = 0;

	for(int i = 0; i < this->numPlayers; i++)
		SET_BIT(game->settingsMask, this->players[i]->faction);

	if(game->enhanced)
		game->settingsMask |= ((uint8_t)(~game->settingsMask) << COMPUTER_ENHANCED);
	if(game->intenseCombat) 
		SET_BIT(game->settingsMask, INTENSE_COMBAT);
}

BOOL GWarlords::CheckGameSettingsChanged(SessionInfo *newSession)
{
	GWarlordsSettings *oldSettings = &this->gameSettings;
	GWarlordsSettings *newSettings = &((GWarlords *)newSession)->gameSettings;

	if( oldSettings->difficulty != newSettings->difficulty
			|| oldSettings->enhanced != newSettings->enhanced
			|| oldSettings->intenseCombat != newSettings->intenseCombat )
	{
		newSession->state |= SESSION_MODIFY_GAMESETTINGS;
		return TRUE;
	}

	return FALSE;
}

TCHAR *GWarlords::GetSaveFileName(TCHAR *name)
{
	swprintf(name, MAX_PATH, L"WAR%d.SAV", this->gameSettings.saveSlot);

	return name;
}

void GWarlords::ToggleObserveOff()
{
	PressHotKey(VK_LMENU, VK_O);
}

void GWarlords::ToggleSoundOff()
{
	PressHotKey(VK_LMENU, VK_M);
}




