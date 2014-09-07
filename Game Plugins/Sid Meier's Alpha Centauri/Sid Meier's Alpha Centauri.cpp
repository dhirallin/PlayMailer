#include "stdafx.h"
#include "resource.h"
#include "Sid Meier's Alpha Centauri.h"

TCHAR *GAlphaCentauri::FactionNames[] = {L"The Green", L"The Despot", L"The Scientist", L"The Mogul", L"The Survivalist", L"The Fundamentalist", L"The Humanitarian"};

const int GAlphaCentauri::NUM_MAP_TYPES = 4;
TCHAR *GAlphaCentauri::MapTypeNames[] = {L"Make Random Map", L"Customize Random Map", L"The Map of Planet", L"Huge Map of Planet"};

const int GAlphaCentauri::NUM_DIFFICULTIES = 6;
TCHAR *GAlphaCentauri::DifficultyNames[] = {L"Citizen", L"Specialist", L"Talent", L"Librarian", L"Thinker", L"Transcend"};

const int GAlphaCentauri::NUM_PLANET_SIZES = 6;
TCHAR *GAlphaCentauri::PlanetSizeNames[] = {L"Tiny Planet", L"Small Planet", L"Standard Planet", L"Large Planet", L"Huge Planet", L"Custom Size"};

const int GAlphaCentauri::NUM_GAME_RULES_MENU_ITEMS = 3;
TCHAR *GAlphaCentauri::GameRulesMenuNames[] = {L"Play with Standard Rules", L"Play with Current Rules", L"Customize Rules"};

const int GAlphaCentauri::NUM_OCEAN_COVERAGES = 3;
TCHAR *GAlphaCentauri::OceanCoverageNames[] = {L"30-50% of Surface", L"50-70% of Surface", L"70-90% of Surface"};

const int GAlphaCentauri::NUM_EROSIVE_FORCES = 3;
TCHAR *GAlphaCentauri::ErosiveForcesNames[] = {L"Strong", L"Average", L"Weak"};

const int GAlphaCentauri::NUM_NATIVE_LIFE_FORMS = 3;
TCHAR *GAlphaCentauri::NativeLifeFormNames[] = {L"Rare", L"Average", L"Abundant"};

const int GAlphaCentauri::NUM_CLOUD_COVERS = 3;
TCHAR *GAlphaCentauri::CloudCoverNames[] = {L"Sparse", L"Average", L"Dense"};

const int GAlphaCentauri::NUM_CUSTOM_RULES = 18;
TCHAR *GAlphaCentauri::CustomRuleNames[] = {
	L"Higher Goal: Allow Victory by Transcendence.",
	L"Total War: Allow Victory by Conquest.",
	L"Peace In Our Time: Allow Diplomatic Victory.",
	L"Mine, All Mine: Allow Economic Victory.",
	L"One For All: Allow Cooperative Victory.",
	L"Do or Die: Don't restart eliminated players.",
	L"Look First: Flexible starting locations.",
	L"Tech Stagnation: Slower rate of research discoveries.",
	L"Spoils of War: Steal tech when conquer base.",
	L"Blind Research: Cannot set precise research goals.",
	L"Intense Rivalry: Opponents more aggressive.",
	L"No Unity Survey: World Map not visible.",
	L"No Unity Scattering: Supply Pods only at landing sites.",
	L"Bell Curve: No Random Events.",
	L"Time Warp: Accelerated Start.",
	L"Iron Man: Save/Restore restricted to exit.",
	L"Randomize faction leader personalities.",
	L"Ranomize faction leader social agendas."
};
BOOL GAlphaCentauri::CustomRuleDefaults[] = {TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};

GAlphaCentauriTeam GAlphaCentauri::FactionDefaults[] = {
	GAlphaCentauriTeam(L"Deirdre", L"Gaia's Stepdaughters", L"Gaians", L"Gaian", L"Lady", L"beautiful", L"Female"),
	GAlphaCentauriTeam(L"Yang", L"Human Hive", L"Hive", L"Hive", L"Chairman", L"ruthless", L"Male"),
	GAlphaCentauriTeam(L"Zakharov", L"University of Planet", L"University", L"University", L"Provost", L"brilliant", L"Male"),
	GAlphaCentauriTeam(L"Morgan", L"Morgan Industries", L"Morganites", L"Morganic", L"CEO", L"shrewd", L"Male"),
	GAlphaCentauriTeam(L"Santiago", L"Spartan Federation", L"Spartans", L"Spartan", L"Colonel", L"vigilant", L"Female"),
	GAlphaCentauriTeam(L"Miriam", L"The Lord's Believers", L"Believers", L"Believing", L"Sister", L"pious", L"Female"),
	GAlphaCentauriTeam(L"Lal", L"Peacekeeping Forces", L"Peacekeepers", L"Peacekeeper", L"Brother", L"humane", L"Male")
};

PLUGINS_API GlobalGameSettings *CreatePlugin()
{
	return new GGAlphaCentauriSettings();
}

PLUGINS_API void ReleasePlugin(GlobalGameSettings *ggs)
{
	free(ggs);
	ggs = 0;
}

SessionInfo *GGAlphaCentauriSettings::AllocSession()
{
	return (SessionInfo *)new GAlphaCentauri(this);
}

void GGAlphaCentauriSettings::CloseGameWindow(HWND hWnd)
{
	SetForegroundWindow(hWnd);

	PressHotKey(VK_CONTROL, VK_Q);
	PressKey(VK_DOWN);
	PressKey(VK_RETURN);
	SleepC(750);
	PressKey(VK_ESCAPE);
}

GAlphaCentauriTeam *GAlphaCentauri::AllocTeam()
{
	struct GAlphaCentauriTeam *team;

	team = new GAlphaCentauriTeam();

	return team;
}

INT_PTR CALLBACK GAlphaCentauri::TeamSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static GAlphaCentauri *session;
	static GAlphaCentauriTeam *team;
	int factionNum;
	TCHAR playerList[1024];

	switch(message)
	{
	case WM_INITDIALOG:
		session = (GAlphaCentauri *)lParam;
		factionNum = session->players[session->currentPlayer]->faction;
		
		SendDlgItemMessage(hDialog, IDC_NAME_EDIT, WM_SETTEXT, 0, (LPARAM)FactionDefaults[factionNum].name);
		SendDlgItemMessage(hDialog, IDC_FORMAL_EDIT, WM_SETTEXT, 0, (LPARAM)FactionDefaults[factionNum].formal);
		SendDlgItemMessage(hDialog, IDC_NOUN_EDIT, WM_SETTEXT, 0, (LPARAM)FactionDefaults[factionNum].noun);
		SendDlgItemMessage(hDialog, IDC_ADJECTIVE_EDIT, WM_SETTEXT, 0, (LPARAM)FactionDefaults[factionNum].adjective);
		SendDlgItemMessage(hDialog, IDC_TITLE_EDIT, WM_SETTEXT, 0, (LPARAM)FactionDefaults[factionNum].title);
		SendDlgItemMessage(hDialog, IDC_DESCRIPTION_EDIT, WM_SETTEXT, 0, (LPARAM)FactionDefaults[factionNum].description);
		SendDlgItemMessage(hDialog, IDC_GENDER_EDIT, WM_SETTEXT, 0, (LPARAM)FactionDefaults[factionNum].gender);

		SendDlgItemMessage(hDialog, IDC_NAME_EDIT, EM_LIMITTEXT, 23, 0);
		SendDlgItemMessage(hDialog, IDC_FORMAL_EDIT, EM_LIMITTEXT, 39, 0);
		SendDlgItemMessage(hDialog, IDC_NOUN_EDIT, EM_LIMITTEXT, 23, 0);
		SendDlgItemMessage(hDialog, IDC_ADJECTIVE_EDIT, EM_LIMITTEXT, 23, 0);
		SendDlgItemMessage(hDialog, IDC_TITLE_EDIT, EM_LIMITTEXT, 23, 0);
		SendDlgItemMessage(hDialog, IDC_DESCRIPTION_EDIT, EM_LIMITTEXT, 23, 0);
		SendDlgItemMessage(hDialog, IDC_GENDER_EDIT, EM_LIMITTEXT, 23, 0);

		session->BuildTeamList(session, session->players[session->currentPlayer]->team, playerList);
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Select settings for %s (%s)", session->getCurrentPlayerFactionName(session), playerList);
		SendDlgItemMessage(hDialog, IDC_SELECT_SETTINGS_STATIC, WM_SETTEXT, 0, (LPARAM)mbBuffer);

		team = new GAlphaCentauriTeam();
		team->id = session->getCurrentPlayerTeam(session);
		team->faction = session->getCurrentPlayerFaction(session);
		team->state = TEAM_MODIFY;

		//session->SetTopMost(hDialog);
		//EnableWindow(*session->PTR_hMainWnd, FALSE);
		return TRUE;
	case WM_COMMAND:
		if(HIWORD(wParam) == EN_SETFOCUS)
			SendDlgItemMessage(hDialog, LOWORD(wParam), EM_SETSEL, 0, -1);

		switch(LOWORD(wParam)) 
		{	
		case IDOK:
			SendDlgItemMessage(hDialog, IDC_NAME_EDIT, WM_GETTEXT, 24, (LPARAM)team->name);
			SendDlgItemMessage(hDialog, IDC_FORMAL_EDIT, WM_GETTEXT, 40, (LPARAM)team->formal);
			SendDlgItemMessage(hDialog, IDC_NOUN_EDIT, WM_GETTEXT, 24, (LPARAM)team->noun);
			SendDlgItemMessage(hDialog, IDC_ADJECTIVE_EDIT, WM_GETTEXT, 24, (LPARAM)team->adjective);
			SendDlgItemMessage(hDialog, IDC_TITLE_EDIT, WM_GETTEXT, 24, (LPARAM)team->title);
			SendDlgItemMessage(hDialog, IDC_DESCRIPTION_EDIT, WM_GETTEXT, 24, (LPARAM)team->description);
			SendDlgItemMessage(hDialog, IDC_GENDER_EDIT, WM_GETTEXT, 24, (LPARAM)team->gender);

			session->LL_Add(session->teams, team);
			
			//EnableWindow(*session->PTR_hMainWnd, TRUE);
			EndDialog(hDialog, LOWORD(wParam));
				
			break;
		}
		break;
	}

	return FALSE;
}

BOOL GAlphaCentauri::ValidateTeamSettings(Team *team, int type, int sIndex, int tIndex)
{
	GAlphaCentauriTeam *pTeam = (GAlphaCentauriTeam *)team;
	TCHAR cfgError[MBBUFFER_SIZE];
	HWND hWnd = NULL;

	cfgError[0] = L'\0';

	if(type == VALIDATE_FILE) 
		swprintf(cfgError, sizeof(cfgError), L"Error in session \"%s\", team #%d. ", this->sessionName, tIndex);

	trimWhiteSpace(pTeam->name);
	trimWhiteSpace(pTeam->formal);
	trimWhiteSpace(pTeam->noun);
	trimWhiteSpace(pTeam->adjective);
	trimWhiteSpace(pTeam->title);
	trimWhiteSpace(pTeam->description);
	trimWhiteSpace(pTeam->gender);

	if(pTeam->name[0] == L'\0')
		wcscpy_s(pTeam->name, MAX_SETTING, FactionDefaults[pTeam->faction].name);
	if(pTeam->formal[0] == L'\0')
		wcscpy_s(pTeam->formal, MAX_SETTING, FactionDefaults[pTeam->faction].formal);
	if(pTeam->noun[0] == L'\0')
		wcscpy_s(pTeam->noun, MAX_SETTING, FactionDefaults[pTeam->faction].noun);
	if(pTeam->adjective[0] == L'\0')
		wcscpy_s(pTeam->adjective, MAX_SETTING, FactionDefaults[pTeam->faction].adjective);
	if(pTeam->title[0] == L'\0')
		wcscpy_s(pTeam->title, MAX_SETTING, FactionDefaults[pTeam->faction].title);
	if(pTeam->description[0] == L'\0')
		wcscpy_s(pTeam->description, MAX_SETTING, FactionDefaults[pTeam->faction].description);
	if(pTeam->gender[0] == L'\0')
		wcscpy_s(pTeam->gender, MAX_SETTING, FactionDefaults[pTeam->faction].gender);
	
	return TRUE;
}

DLGPROC GAlphaCentauri::GetTeamSettingsDialogProc()
{
	return TeamSettingsDialogProc;
}

BOOL GAlphaCentauri::NewGame()
{
	int yPos;
	TCHAR customMapSize[9];
	GAlphaCentauriSettings *gSettings = (GAlphaCentauriSettings *)&(this->gameSettings); 
	GAlphaCentauriTeam *team;
	LinkedList *iter;
	GGAlphaCentauriSettings *gg = (GGAlphaCentauriSettings *)ggSettings;

	/*if(gg->firstRun)
	{
		SleepC(7000);
		gg->firstRun = FALSE;
	}*/

	for(int i = 0; i < 10; i++)
	{
		PressLButton();
		SleepC(1000);
	}
	SleepC(5000);
	ResetMouse();
	MoveMouseMod(670, 470);
	PressLButton();
	ResetMouse();
	MoveMouseMod(400, 338);
	PressLButton();
	PressKey(VK_RETURN);

	// Game type
	MoveMouseMod(250, 130);
	PressLButton();
	SleepC(250);

	// Map type

	// Load map file
	if(gSettings->mapFile[0] != L'\0')
	{
		MoveMouseMod(0, 120);
		PressLButton();
		PressStringKeys(gSettings->mapFile, MAX_PATH);
		PressKey(VK_RETURN);
	}
	else
	{
		yPos = -120;
		yPos += gSettings->mapType * 60;
		MoveMouseMod(0, yPos);
		PressLButton();
		SleepC(250);

		if(gSettings->mapType == 0 || gSettings->mapType == 1)
		{
			// Planet Size			
			ResetMouseVert();
			yPos = 290;
			yPos += gSettings->planetSize * 60;
			MoveMouseMod(0, yPos);
			PressLButton();
			SleepC(250);

			if(gSettings->planetSize == 5)
			{
				// Custom Planet Size
				_itow_s(gSettings->horizontalMapSize, customMapSize, 9, 10);
				PressStringKeys(customMapSize, 8);
				PressKey(VK_TAB);
				_itow_s(gSettings->verticalMapSize, customMapSize, 9, 10);
				PressStringKeys(customMapSize, 8);
				PressKey(VK_RETURN);
			}

			if(gSettings->mapType == 1)
			{
				// Customize Map
				ResetMouseVert();
				MoveMouseMod(0, 470);
				
				yPos = gSettings->oceanCoverage * 60;
				MoveMouseMod(0, yPos);
				PressLButton();
				SleepC(250);
				MoveMouseMod(0, -yPos);
				yPos = gSettings->erosiveForces * 60;
				MoveMouseMod(0, yPos);
				PressLButton();
				SleepC(250);
				MoveMouseMod(0, -yPos);
				yPos = gSettings->nativeLifeForms * 60;
				MoveMouseMod(0, yPos);
				PressLButton();
				SleepC(250);
				MoveMouseMod(0, -yPos);
				yPos = gSettings->cloudCover * 60;
				MoveMouseMod(0, yPos);
				PressLButton();
				SleepC(250);
			}
		}
	}

	// Difficulty
	SleepC(250);
	ResetMouseVert();
	yPos = 290;
	yPos += gSettings->difficulty * 60;
	MoveMouseMod(0, yPos);
	PressLButton();

	// Game Rules Menu
	ResetMouseVert();
	yPos = 470;
	yPos += gSettings->gameRules * 60;
	MoveMouseMod(0, yPos);
	PressLButton();

	if(gSettings->gameRules == 2)
	{
		// Customize Rules
		MoveMouseMod(-300, 0);
		PressLButton();
		ResetMouseVert();
		MoveMouseMod(0, 263);
		*PTR_MouseModifier = 1.0;
		for(int i = 0; i < NUM_CUSTOM_RULES; i++)
		{
			if(((long long)CustomRuleDefaults[i] << (NUM_CUSTOM_RULES - 1 - i)) ^ (gameSettings.customRules & ((long long)1 << (NUM_CUSTOM_RULES - 1 - i))))
				PressLButton();

			MoveMouseMod(0, 35);
		}
		*PTR_MouseModifier = 2.0;
		PressKey(VK_RETURN);
	}

	// Setup Teams
	iter = this->teams;

	while(iter->next)
	{
		iter = iter->next;
		team = (GAlphaCentauriTeam *)iter->item;

		// Select faction
		ResetMouse();
		MoveMouseMod(460, 473);
		yPos = 15 * team->faction;
		MoveMouseMod(0, yPos);
		PressLButton();
		// Press 'Other'
		MoveMouseMod(0, 15 * (6 - team->faction) + 28);
		PressLButton();

		// Press 'Restore Defaults'
		MoveMouseMod(-100, 0);
		PressLButton();
		// Enter faction details
		if(wcscmp(team->name, FactionDefaults[team->faction].name))
			PressStringKeys(team->name, 23);
		PressKey(VK_TAB);
		if(wcscmp(team->formal, FactionDefaults[team->faction].formal))
			PressStringKeys(team->formal, 39);
		PressKey(VK_TAB);
		if(wcscmp(team->noun, FactionDefaults[team->faction].noun))
			PressStringKeys(team->noun, 23);
		PressKey(VK_TAB);
		if(wcscmp(team->adjective, FactionDefaults[team->faction].adjective))
			PressStringKeys(team->adjective, 23);
		PressKey(VK_TAB);
		if(wcscmp(team->title, FactionDefaults[team->faction].title))
			PressStringKeys(team->title, 23);
		PressKey(VK_TAB);
		if(wcscmp(team->description, FactionDefaults[team->faction].description))
			PressStringKeys(team->description, 23);
		PressKey(VK_TAB);
		if(wcscmp(team->gender, FactionDefaults[team->faction].gender))
			PressStringKeys(team->gender, 23);
		PressKey(VK_RETURN);
		
		// Add more players?
		if(iter->next)
			PressKey(VK_DOWN);
		PressKey(VK_RETURN);
	}

	return TRUE;
}

BOOL GAlphaCentauri::LoadGame()
{
	TCHAR saveFileName[MAX_PATH];
	GGAlphaCentauriSettings *gg = (GGAlphaCentauriSettings *)ggSettings;

	/*if(gg->firstRun)
	{
		SleepC(7000);
		gg->firstRun = FALSE;
	}*/

	for(int i = 0; i < 10; i++)
	{
		PressLButton();
		SleepC(1000);
	}
	SleepC(5000);
	

	//SleepC(8000);
	//PressLButton();
	//SleepC(8000);
	ResetMouse();
	MoveMouseMod(670, 410);
	PressLButton();
	SleepC(500);
	GetSaveFileName(saveFileName);
	PressStringKeys(saveFileName, MAX_PATH);
	PressKey(VK_RETURN);
	SleepC(500);
	ResetMouse();
	MoveMouseMod(400, 336);
	PressLButton();
	SleepC(500);
	PressKey(VK_RETURN);
	SleepC(3000);
	PressKey(VK_RETURN);

	return TRUE;
}

BOOL GAlphaCentauri::SaveGame()
{
	TCHAR saveFileName[MAX_PATH];

	ResetMouse();
	MoveMouseMod(725, 468);
	PressLButton();
	ResetMouse();
	MoveMouseMod(400, 298);
	PressLButton();
	MoveMouseMod(0, 25);
	PressLButton();
	SleepC(500);
	MoveMouseMod(-130, 65);
	PressLButton();
	SleepC(750);
	GetSaveFileName(saveFileName);
	PressStringKeys(saveFileName, MAX_PATH);
	StartSaveFileThread(this);
	PressKey(VK_RETURN);
	SleepC(500);
	PressKey(VK_RETURN);
	return WaitForWriteFileThread();
}

INT_PTR CALLBACK GAlphaCentauri::GameSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR pathStr[MAX_PATH], *pMapFile;
	static SessionInfo *session;

	switch(message)
	{
		case WM_INITDIALOG:
			session = (SessionInfo *)lParam;
			return TRUE;
		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_BROWSE_MAP_FILE_BUTTON)
			{
				pathStr[0] = 0;
				if(pGetFileSelection(GetParent(hDialog), pathStr, L"Please select a map file.", NULL, NULL))
				{
					if(pMapFile = wcsrchr(pathStr, L'\\'))
						pMapFile++;
					else
						pMapFile = pathStr;
					SendDlgItemMessage(hDialog, IDC_MAP_FILE_EDIT, WM_SETTEXT, 0, (LPARAM)pMapFile);
				}
			}
			else if(LOWORD(wParam) == IDC_EDIT_CUSTOM_RULES_BUTTON)
				DialogBoxParam(session->ggSettings->hModule, MAKEINTRESOURCE(IDD_CUSTOMIZE_RULES), hDialog, GAlphaCentauri::CustomizeRulesDialogProc, (LPARAM)session);
			else if(HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == BN_CLICKED) 
				GAlphaCentauri::UpdateGameSettingsDialog(hDialog);
			break;
		default:
			break;
	}

	return FALSE;
}

INT_PTR CALLBACK GAlphaCentauri::CustomizeRulesDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static GAlphaCentauri *session;
	BOOL attrChecked;

	switch(message)
	{
		case WM_INITDIALOG:
			session = (GAlphaCentauri *)lParam;
			for(int i = 0; i < NUM_CUSTOM_RULES; i++)
				SendDlgItemMessage(hDialog, IDC_RULE0_CHECK + i, WM_SETTEXT, 0, (LPARAM)CustomRuleNames[i]);

			for(int i = NUM_CUSTOM_RULES - 1; i >= 0; i--)
			{
				if(session->gameSettings.customRules & ((long long)1 << i))
					SendDlgItemMessage(hDialog, IDC_RULE0_CHECK + (NUM_CUSTOM_RULES - 1 - i), BM_SETCHECK, BST_CHECKED, 0);	
			}
			
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_STANDARD_RULES_BUTTON:
				for(int i = 0; i < NUM_CUSTOM_RULES; i++)
					SendDlgItemMessage(hDialog, IDC_RULE0_CHECK + i, BM_SETCHECK, CustomRuleDefaults[i], 0);
				
				break;
			case IDOK:
				session->gameSettings.customRules = 0;
				for(int i = 0; i < NUM_CUSTOM_RULES; i++)
				{
					attrChecked = IsDlgButtonChecked(hDialog, IDC_RULE0_CHECK + NUM_CUSTOM_RULES - 1 - i);
					session->gameSettings.customRules |= ((long long)attrChecked << i);
				}

				EndDialog(hDialog, LOWORD(wParam));
				break;
			case IDCANCEL:
				EndDialog(hDialog, LOWORD(wParam));
				break;
			}
			break;
		default:
			break;
	}

	return FALSE;
}

void GAlphaCentauri::InitGameSettingsDialog()
{
	GAlphaCentauriSettings *game;
	HWND hDlg = *PTR_hSessionSettingsDialog, hGameDlg;
	
	hGameDlg = *PTR_hGameSettingsDialog = CreateDialogParam(this->ggSettings->hModule, MAKEINTRESOURCE(IDD_GAME), *PTR_hSessionSettingsDialog, GameSettingsDialogProc, (LPARAM)this);
	SetWindowPos(*PTR_hGameSettingsDialog, GetDlgItem(*PTR_hSessionSettingsDialog, EDITPLAYERS_BUTTON), DLUToPixelsX(hDlg, GAME_SETTINGS_PANE_X), DLUToPixelsY(hDlg, GAME_SETTINGS_PANE_Y), 0, 0, SWP_NOSIZE);
	
	game = &this->gameSettings;

	for(int i = 0; i < NUM_MAP_TYPES; i++)
		SendDlgItemMessage(hGameDlg, IDC_MAPTYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)MapTypeNames[i]); 
	
	for(int i = 0; i < NUM_DIFFICULTIES; i++)
		SendDlgItemMessage(hGameDlg, IDC_DIFFICULTY_COMBO, CB_ADDSTRING, 0, (LPARAM)DifficultyNames[i]); 

	for(int i = 0; i < NUM_PLANET_SIZES; i++)
		SendDlgItemMessage(hGameDlg, IDC_PLANET_SIZE_COMBO, CB_ADDSTRING, 0, (LPARAM)PlanetSizeNames[i]); 

	for(int i = 0; i < NUM_GAME_RULES_MENU_ITEMS; i++)
		SendDlgItemMessage(hGameDlg, IDC_GAMERULES_COMBO, CB_ADDSTRING, 0, (LPARAM)GameRulesMenuNames[i]); 

	for(int i = 0; i < NUM_OCEAN_COVERAGES; i++)
		SendDlgItemMessage(hGameDlg, IDC_OCEAN_COVERAGE_COMBO, CB_ADDSTRING, 0, (LPARAM)OceanCoverageNames[i]); 

	for(int i = 0; i < NUM_EROSIVE_FORCES; i++)
		SendDlgItemMessage(hGameDlg, IDC_EROSIVE_FORCES_COMBO, CB_ADDSTRING, 0, (LPARAM)ErosiveForcesNames[i]); 

	for(int i = 0; i < NUM_NATIVE_LIFE_FORMS; i++)
		SendDlgItemMessage(hGameDlg, IDC_NATIVE_LIFE_FORMS_COMBO, CB_ADDSTRING, 0, (LPARAM)NativeLifeFormNames[i]); 

	for(int i = 0; i < NUM_CLOUD_COVERS; i++)
		SendDlgItemMessage(hGameDlg, IDC_CLOUD_COVER_COMBO, CB_ADDSTRING, 0, (LPARAM)CloudCoverNames[i]); 

	SendDlgItemMessage(hGameDlg, IDC_MAPTYPE_COMBO, CB_SETCURSEL, game->mapType, 0);
	SendDlgItemMessage(hGameDlg, IDC_DIFFICULTY_COMBO, CB_SETCURSEL, game->difficulty, 0);
	SendDlgItemMessage(hGameDlg, IDC_PLANET_SIZE_COMBO, CB_SETCURSEL, game->planetSize, 0);
	SendDlgItemMessage(hGameDlg, IDC_GAMERULES_COMBO, CB_SETCURSEL, game->gameRules, 0);
	SendDlgItemMessage(hGameDlg, IDC_OCEAN_COVERAGE_COMBO, CB_SETCURSEL, game->oceanCoverage, 0);
	SendDlgItemMessage(hGameDlg, IDC_EROSIVE_FORCES_COMBO, CB_SETCURSEL, game->erosiveForces, 0);
	SendDlgItemMessage(hGameDlg, IDC_NATIVE_LIFE_FORMS_COMBO, CB_SETCURSEL, game->nativeLifeForms, 0);
	SendDlgItemMessage(hGameDlg, IDC_CLOUD_COVER_COMBO, CB_SETCURSEL, game->cloudCover, 0);
	CheckDlgButton(hGameDlg, IDC_LOAD_MAP_FILE_CHECK, (game->mapFile[0] != NULL));
	SendDlgItemMessage(hGameDlg, IDC_MAP_FILE_EDIT, WM_SETTEXT, 0, (LPARAM)game->mapFile);
	_itow_s(game->horizontalMapSize, mbBuffer, MBBUFFER_SIZE, 10);
	SendDlgItemMessage(hGameDlg, IDC_HORIZONTAL_MAPSIZE_EDIT, WM_SETTEXT, 0, (LPARAM)mbBuffer);
	_itow_s(game->verticalMapSize, mbBuffer, MBBUFFER_SIZE, 10);
	SendDlgItemMessage(hGameDlg, IDC_VERTICAL_MAPSIZE_EDIT, WM_SETTEXT, 0, (LPARAM)mbBuffer);

	UpdateGameSettingsDialog(hGameDlg);

	if(this->turnNumber > 1)
	{
		EnableWindow(GetDlgItem(hGameDlg, IDC_EDIT_CUSTOM_RULES_BUTTON), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_MAPTYPE_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_MAP_FILE_EDIT), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_BROWSE_MAP_FILE_BUTTON), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_DIFFICULTY_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_GAMERULES_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_OCEAN_COVERAGE_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_EROSIVE_FORCES_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_NATIVE_LIFE_FORMS_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_CLOUD_COVER_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_PLANET_SIZE_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_HORIZONTAL_MAPSIZE_EDIT), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_VERTICAL_MAPSIZE_EDIT), FALSE);
	}
}

void GAlphaCentauri::UpdateGameSettingsDialog(HWND hGameDlg)
{
	BOOL bEnable;
	int show;

	show = ((SendDlgItemMessage(hGameDlg, IDC_MAPTYPE_COMBO, CB_GETCURSEL, 0, 0) != 1 
		|| IsDlgButtonChecked(hGameDlg, IDC_LOAD_MAP_FILE_CHECK)) ? SW_HIDE : SW_SHOW);
	ShowWindow(GetDlgItem(hGameDlg, IDC_OCEAN_COVERAGE_COMBO), show);
	ShowWindow(GetDlgItem(hGameDlg, IDC_EROSIVE_FORCES_COMBO), show);
	ShowWindow(GetDlgItem(hGameDlg, IDC_NATIVE_LIFE_FORMS_COMBO), show);
	ShowWindow(GetDlgItem(hGameDlg, IDC_CLOUD_COVER_COMBO), show);
	ShowWindow(GetDlgItem(hGameDlg, IDC_OCEAN_COVERAGE_STATIC), show);
	ShowWindow(GetDlgItem(hGameDlg, IDC_EROSIVE_FORCES_STATIC), show);
	ShowWindow(GetDlgItem(hGameDlg, IDC_NATIVE_LIFE_FORMS_STATIC), show);
	ShowWindow(GetDlgItem(hGameDlg, IDC_CLOUD_COVER_STATIC), show);

	show = ((SendDlgItemMessage(hGameDlg, IDC_MAPTYPE_COMBO, CB_GETCURSEL, 0, 0) == 2 
		|| SendDlgItemMessage(hGameDlg, IDC_MAPTYPE_COMBO, CB_GETCURSEL, 0, 0) == 3 
		|| IsDlgButtonChecked(hGameDlg, IDC_LOAD_MAP_FILE_CHECK)) ? SW_HIDE : SW_SHOW);
	ShowWindow(GetDlgItem(hGameDlg, IDC_PLANET_SIZE_COMBO), show);
	ShowWindow(GetDlgItem(hGameDlg, IDC_PLANET_SIZE_STATIC), show);

	show = ((SendDlgItemMessage(hGameDlg, IDC_PLANET_SIZE_COMBO, CB_GETCURSEL, 0, 0) != 5 
		|| SendDlgItemMessage(hGameDlg, IDC_MAPTYPE_COMBO, CB_GETCURSEL, 0, 0) == 2 
		|| SendDlgItemMessage(hGameDlg, IDC_MAPTYPE_COMBO, CB_GETCURSEL, 0, 0) == 3 
		|| IsDlgButtonChecked(hGameDlg, IDC_LOAD_MAP_FILE_CHECK)) ? SW_HIDE : SW_SHOW);
	ShowWindow(GetDlgItem(hGameDlg, IDC_HORIZONTAL_MAPSIZE_EDIT), show);
	ShowWindow(GetDlgItem(hGameDlg, IDC_VERTICAL_MAPSIZE_EDIT), show);
	ShowWindow(GetDlgItem(hGameDlg, IDC_HORIZONTAL_MAPSIZE_STATIC), show);
	ShowWindow(GetDlgItem(hGameDlg, IDC_VERTICAL_MAPSIZE_STATIC), show);

	bEnable = (IsDlgButtonChecked(hGameDlg, IDC_LOAD_MAP_FILE_CHECK) ? FALSE : TRUE);
	EnableWindow(GetDlgItem(hGameDlg, IDC_MAPTYPE_COMBO), bEnable);
	EnableWindow(GetDlgItem(hGameDlg, IDC_MAP_FILE_EDIT), !bEnable);
	EnableWindow(GetDlgItem(hGameDlg, IDC_BROWSE_MAP_FILE_BUTTON), !bEnable);

	bEnable = (SendDlgItemMessage(hGameDlg, IDC_GAMERULES_COMBO, CB_GETCURSEL, 0, 0) == 2 ? TRUE : FALSE);
	EnableWindow(GetDlgItem(hGameDlg, IDC_EDIT_CUSTOM_RULES_BUTTON), bEnable);
}

BOOL GAlphaCentauri::ParseGameSettingsDialog()
{
	GAlphaCentauriSettings *game = &this->gameSettings;
	HWND hGameDlg = *PTR_hGameSettingsDialog;
	TCHAR *pMapFile, pathStr[MAX_PATH];

	game->mapType = (int)SendDlgItemMessage(hGameDlg, IDC_MAPTYPE_COMBO, CB_GETCURSEL, 0, 0);
	game->difficulty = (int)SendDlgItemMessage(hGameDlg, IDC_DIFFICULTY_COMBO, CB_GETCURSEL, 0, 0);
	game->gameRules = (int)SendDlgItemMessage(hGameDlg, IDC_GAMERULES_COMBO, CB_GETCURSEL, 0, 0);
	game->planetSize = (int)SendDlgItemMessage(hGameDlg, IDC_PLANET_SIZE_COMBO, CB_GETCURSEL, 0, 0);
	game->oceanCoverage = (int)SendDlgItemMessage(hGameDlg, IDC_OCEAN_COVERAGE_COMBO, CB_GETCURSEL, 0, 0);
	game->erosiveForces = (int)SendDlgItemMessage(hGameDlg, IDC_EROSIVE_FORCES_COMBO, CB_GETCURSEL, 0, 0);
	game->nativeLifeForms = (int)SendDlgItemMessage(hGameDlg, IDC_NATIVE_LIFE_FORMS_COMBO, CB_GETCURSEL, 0, 0);
	game->cloudCover = (int)SendDlgItemMessage(hGameDlg, IDC_CLOUD_COVER_COMBO, CB_GETCURSEL, 0, 0);
	
	SendDlgItemMessage(hGameDlg, IDC_HORIZONTAL_MAPSIZE_EDIT, WM_GETTEXT, MAX_PATH, (LPARAM)mbBuffer);
	game->horizontalMapSize = _wtoi(mbBuffer);
	SendDlgItemMessage(hGameDlg, IDC_VERTICAL_MAPSIZE_EDIT, WM_GETTEXT, MAX_PATH, (LPARAM)mbBuffer);
	game->verticalMapSize = _wtoi(mbBuffer);
	
	if(IsDlgButtonChecked(hGameDlg, IDC_LOAD_MAP_FILE_CHECK))
	{
		SendDlgItemMessage(hGameDlg, IDC_MAP_FILE_EDIT, WM_GETTEXT, MAX_PATH, (LPARAM)pathStr);
		if(pMapFile = wcsrchr(pathStr, L'\\'))
			wcscpy_s(game->mapFile, MAX_PATH, pMapFile+1);
		else
			wcscpy_s(game->mapFile, MAX_PATH, pathStr);
	}
	else
		game->mapFile[0] = L'\0';

	return ValidateGameSettings(VALIDATE_GUI, 0);
}

BOOL GAlphaCentauri::ValidateGameSettings(BOOL type, int sIndex)
{
	TCHAR cfgError[MBBUFFER_SIZE];
	GAlphaCentauriSettings *game = &this->gameSettings;
	HWND hWnd = NULL;

	cfgError[0] = L'\0';

	if(type == VALIDATE_FILE) 
		swprintf(cfgError, sizeof(cfgError), L"Error in session \"%s\". ", this->sessionName);
	else if(type == VALIDATE_GUI)
		hWnd = *PTR_hSessionSettingsDialog;

	/*if(type != VALIDATE_EMAIL)
	{
		if(game->mapFile[0] != L'\0' && _waccess(game->mapFile, 0))
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Cannot find map file: %s", game->mapFile);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
	}*/

	if(game->mapType < 0 || game->mapType >= NUM_MAP_TYPES)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sSetting \"map_type\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->difficulty < 0 || game->difficulty >= NUM_DIFFICULTIES)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sSetting \"difficulty\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->planetSize < 0 || game->planetSize >= NUM_PLANET_SIZES)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sSetting \"planet_size\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->gameRules < 0 || game->gameRules >= NUM_GAME_RULES_MENU_ITEMS)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sSetting \"game_rules\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->oceanCoverage < 0 || game->oceanCoverage >= NUM_OCEAN_COVERAGES)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sSetting \"ocean_coverage\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->erosiveForces < 0 || game->erosiveForces >= NUM_EROSIVE_FORCES)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sSetting \"erosive_forces\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->nativeLifeForms < 0 || game->nativeLifeForms >= NUM_NATIVE_LIFE_FORMS)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sSetting \"native_life_forms\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->cloudCover < 0 || game->cloudCover >= NUM_CLOUD_COVERS)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sSetting \"cloud_cover\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->horizontalMapSize < 0 || game->horizontalMapSize > 99999999)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sSetting \"horizontal_map_size\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->verticalMapSize < 0 || game->verticalMapSize > 99999999)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%sSetting \"vertical_map_size\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	return TRUE;
}

void GAlphaCentauri::InitGameSettings()
{
	memset(&gameSettings, 0, sizeof(GAlphaCentauriSettings));
	gameSettings.horizontalMapSize = 40;
	gameSettings.verticalMapSize = 80;

	for(int i = 0; i < NUM_CUSTOM_RULES; i++)
		gameSettings.customRules |= ((long long)CustomRuleDefaults[i] << (NUM_CUSTOM_RULES - 1 - i));
}

void GAlphaCentauri::SaveGameSettings(config_setting_t *group)
{
	cfgSetInt(group, L"map_type", this->gameSettings.mapType);
	cfgSetInt(group, L"difficulty", this->gameSettings.difficulty);
	cfgSetInt(group, L"planet_size", this->gameSettings.planetSize);
	cfgSetInt(group, L"game_rules", this->gameSettings.gameRules);
	cfgSetInt(group, L"ocean_coverage", this->gameSettings.oceanCoverage);
	cfgSetInt(group, L"erosive_forces", this->gameSettings.erosiveForces);
	cfgSetInt(group, L"native_life_forms", this->gameSettings.nativeLifeForms);
	cfgSetInt(group, L"cloud_cover", this->gameSettings.cloudCover);
	cfgSetInt(group, L"horizontal_map_size", this->gameSettings.horizontalMapSize);
	cfgSetInt(group, L"vertical_map_size", this->gameSettings.verticalMapSize);
	cfgSetInt64(group, L"custom_rules", this->gameSettings.customRules); 
	cfgSetString(group, L"map_file", this->gameSettings.mapFile);
}

void GAlphaCentauri::LoadGameSettings(config_setting_t *group)
{
	this->gameSettings.mapType = cfgGetInt(group, L"map_type");
	this->gameSettings.difficulty = cfgGetInt(group, L"difficulty");
	this->gameSettings.planetSize = cfgGetInt(group, L"planet_size");
	this->gameSettings.gameRules = cfgGetInt(group, L"game_rules");
	this->gameSettings.oceanCoverage = cfgGetInt(group, L"ocean_coverage");
	this->gameSettings.erosiveForces = cfgGetInt(group, L"erosive_forces");
	this->gameSettings.nativeLifeForms = cfgGetInt(group, L"native_life_forms");
	this->gameSettings.cloudCover = cfgGetInt(group, L"cloud_cover");
	this->gameSettings.horizontalMapSize = cfgGetInt(group, L"horizontal_map_size");
	this->gameSettings.verticalMapSize = cfgGetInt(group, L"vertical_map_size");
	this->gameSettings.customRules = cfgGetInt64(group, L"custom_rules");
	cfgGetString(group, L"map_file", this->gameSettings.mapFile);
}

void GAlphaCentauri::BuildEmailGameSettings(config_setting_t *group)
{
	cfgSetInt(group, L"map_type", this->gameSettings.mapType);
	cfgSetInt(group, L"difficulty", this->gameSettings.difficulty);
	cfgSetInt(group, L"planet_size", this->gameSettings.planetSize);
	cfgSetInt(group, L"game_rules", this->gameSettings.gameRules);
	cfgSetInt(group, L"ocean_coverage", this->gameSettings.oceanCoverage);
	cfgSetInt(group, L"erosive_forces", this->gameSettings.erosiveForces);
	cfgSetInt(group, L"native_life_forms", this->gameSettings.nativeLifeForms);
	cfgSetInt(group, L"cloud_cover", this->gameSettings.cloudCover);
	cfgSetInt(group, L"horizontal_map_size", this->gameSettings.horizontalMapSize);
	cfgSetInt(group, L"vertical_map_size", this->gameSettings.verticalMapSize);
	cfgSetInt64(group, L"custom_rules", this->gameSettings.customRules); 
	cfgSetString(group, L"map_file", this->gameSettings.mapFile);
}

void GAlphaCentauri::ParseEmailGameSettings(config_setting_t *group)
{
	this->gameSettings.mapType = cfgGetInt(group, L"map_type");
	this->gameSettings.difficulty = cfgGetInt(group, L"difficulty");
	this->gameSettings.planetSize = cfgGetInt(group, L"planet_size");
	this->gameSettings.gameRules = cfgGetInt(group, L"game_rules");
	this->gameSettings.oceanCoverage = cfgGetInt(group, L"ocean_coverage");
	this->gameSettings.erosiveForces = cfgGetInt(group, L"erosive_forces");
	this->gameSettings.nativeLifeForms = cfgGetInt(group, L"native_life_forms");
	this->gameSettings.cloudCover = cfgGetInt(group, L"cloud_cover");
	this->gameSettings.horizontalMapSize = cfgGetInt(group, L"horizontal_map_size");
	this->gameSettings.verticalMapSize = cfgGetInt(group, L"vertical_map_size");
	this->gameSettings.customRules = cfgGetInt64(group, L"custom_rules");
	cfgGetString(group, L"map_file", this->gameSettings.mapFile);
}

void GAlphaCentauri::UpdateEmailGameSettings(SessionInfo *in)
{
	GAlphaCentauri *pIn = (GAlphaCentauri *)in;
	memcpy(&(this->gameSettings), &(pIn->gameSettings), sizeof(GAlphaCentauriSettings));
}

void GAlphaCentauri::SaveTeamSettings(config_setting_t *group, Team *team)
{
	GAlphaCentauriTeam *pTeam = (GAlphaCentauriTeam *)team; 

	cfgSetString(group, L"name", pTeam->name);
	cfgSetString(group, L"formal", pTeam->formal);
	cfgSetString(group, L"noun", pTeam->noun);
	cfgSetString(group, L"adjective", pTeam->adjective);
	cfgSetString(group, L"title", pTeam->title);
	cfgSetString(group, L"description", pTeam->description);
	cfgSetString(group, L"gender", pTeam->gender);
}

void GAlphaCentauri::LoadTeamSettings(config_setting_t *group, Team *team)
{
	GAlphaCentauriTeam *pTeam = (GAlphaCentauriTeam *)team; 

	cfgGetString(group, L"name", pTeam->name);
	cfgGetString(group, L"formal", pTeam->formal);
	cfgGetString(group, L"noun", pTeam->noun);
	cfgGetString(group, L"adjective", pTeam->adjective);
	cfgGetString(group, L"title", pTeam->title);
	cfgGetString(group, L"description", pTeam->description);
	cfgGetString(group, L"gender", pTeam->gender);
}

void GAlphaCentauri::BuildEmailTeamSettings(config_setting_t *group, Team *team)
{
	GAlphaCentauriTeam *pTeam = (GAlphaCentauriTeam *)team; 

	cfgSetString(group, L"name", pTeam->name);
	cfgSetString(group, L"formal", pTeam->formal);
	cfgSetString(group, L"noun", pTeam->noun);
	cfgSetString(group, L"adjective", pTeam->adjective);
	cfgSetString(group, L"title", pTeam->title);
	cfgSetString(group, L"description", pTeam->description);
	cfgSetString(group, L"gender", pTeam->gender);
}

void GAlphaCentauri::ParseEmailTeamSettings(config_setting_t *group, Team *team)
{
	GAlphaCentauriTeam *pTeam = (GAlphaCentauriTeam *)team; 

	cfgGetString(group, L"name", pTeam->name);
	cfgGetString(group, L"formal", pTeam->formal);
	cfgGetString(group, L"noun", pTeam->noun);
	cfgGetString(group, L"adjective", pTeam->adjective);
	cfgGetString(group, L"title", pTeam->title);
	cfgGetString(group, L"description", pTeam->description);
	cfgGetString(group, L"gender", pTeam->gender);
}

void GAlphaCentauri::UpdateEmailTeamSettings(Team *out, Team *in)
{
	GAlphaCentauriTeam *pOut = (GAlphaCentauriTeam *)out; 
	GAlphaCentauriTeam *pIn = (GAlphaCentauriTeam *)in;

	wcscpy_s(pOut->name, 24, pIn->name);
	wcscpy_s(pOut->formal, 40, pIn->formal);
	wcscpy_s(pOut->noun, 24, pIn->noun);
	wcscpy_s(pOut->adjective, 24, pIn->adjective);
	wcscpy_s(pOut->title, 24, pIn->title);
	wcscpy_s(pOut->description, 24, pIn->description);
	wcscpy_s(pOut->gender, 24, pIn->gender);
}

TCHAR *GAlphaCentauri::GetSaveFileName(TCHAR *name)
{
	swprintf(name, MAX_PATH, L"%s %d.SAV", this->sessionName, this->sessionID);

	return name;
}

TCHAR *GAlphaCentauri::GetSaveFolderPath()
{
	swprintf(ggSettings->saveFolderPath, MAX_PATH, L"%ssaves\\", ggSettings->gameFolderPath);
	
	return ggSettings->saveFolderPath;
}











