#include "stdafx.h"
#include "resource.h"
#include "Master of Orion 2.h"

TCHAR *GMasterOfOrion2::FactionNames[] = {L"Alkari", L"Bulrathi", L"Darlocks", L"Elerians", L"Gnolams", L"Humans", L"Klackons", L"Meklars", L"Mrrshan", L"Psilons", L"Sakkra", L"Silicoids", L"Trilarians"};

const int GMasterOfOrion2::NUM_DIFFICULTIES	= 5;
TCHAR *GMasterOfOrion2::DifficultyNames[] = {L"Tutor", L"Easy", L"Average", L"Hard", L"Impossible"};

const int GMasterOfOrion2::NUM_TOTAL_PLAYERS = 7;
TCHAR *GMasterOfOrion2::TotalPlayersNames[] = {L"2 Players", L"3 Players", L"4 Players", L"5 Players", L"6 Players", L"7 Players", L"8 Players"};

const int GMasterOfOrion2::NUM_TECH_LEVELS = 3;
TCHAR *GMasterOfOrion2::TechLevelNames[] = {L"Pre Warp", L"Average", L"Advanced"};

const int GMasterOfOrion2::NUM_GALAXY_SIZES = 4;
TCHAR *GMasterOfOrion2::GalaxySizeNames[] = {L"Small", L"Medium", L"Large", L"Huge"};

const int GMasterOfOrion2::NUM_GALAXY_AGES = 3;
TCHAR *GMasterOfOrion2::GalaxyAgeNames[] = {L"Mineral Rich", L"Average", L"Organic Rich"};

const int GMasterOfOrion2::NUM_BANNER_COLORS = 8;
TCHAR *GMasterOfOrion2::BannerColorNames[] = {L"Red", L"Yellow", L"Green", L"Grey", L"Blue", L"Brown", L"Purple", L"Orange"};

SearchReplace GGMasterOfOrion2Settings::ConfigReplaceStrings[] = { 
	{L"blocksize=", L"blocksize=2048"}, 
	{L"prebuffer=", L"prebuffer=240"} 
};
int GGMasterOfOrion2Settings::NUM_CONFIG_REPLACE_STRINGS = 2;

int NUM_CUSTOM_ATTRIBUTES = 53;
TCHAR *CustomAttributeNames[] = {
	L"-50% Growth",			// 0
	L"+50% Growth",			// 1
	L"+100% Growth",		// 2
	L"-1/2 Food",			// 3
	L"+1 Food",				// 4
	L"+2 Food",				// 5
	L"-1 Production",		// 6
	L"+1 Production",		// 7
	L"+2 Production",		// 8
	L"-1 Research",			// 9
	L"+1 Research",			// 10
	L"+2 Research",			// 11
	L"-0.5 BC",				// 12
	L"+0.5 BC",				// 13
	L"+1.0 BC",				// 14
	L"-20",					// 15
	L"+25",					// 16
	L"+50",					// 17
	L"-20",					// 18
	L"+20",					// 19
	L"+50",					// 20
	L"-10",					// 21
	L"+10",					// 22
	L"+20",					// 23
	L"-10",					// 24
	L"+10",					// 25
	L"+20",					// 26
	L"Feudal",				// 27
	L"Dictatorship",		// 28
	L"Democracy",			// 29
	L"Unification",			// 30
	L"Low-G World",			// 31
	L"High-G World",		// 32
	L"Aquatic",				// 33
	L"Subterranean",		// 34
	L"Large Home World",	// 35
	L"Rich Home World",		// 36
	L"Poor Home World",		// 37
	L"Artifacts World",		// 38
	L"Cybernetic",			// 39
	L"Lithovore",			// 40
	L"Repulsive",			// 41
	L"Charismatic",			// 42
	L"Uncreative",			// 43
	L"Creative",			// 44
	L"Tolerant",			// 45
	L"Fantastic Traders",	// 46
	L"Telepathic",			// 47
	L"Lucky",				// 48
	L"Omniscient",			// 49
	L"Stealthy Ships",		// 50
	L"Trans Dimensional",	// 51
	L"Warlord"				// 52
};

int CustomAttributePicks[] = {
	-4, 3, 6, -3, 4, 7, -3, 3, 6, -3, 3, 6, -4, 5, 8, 
	-2, 3, 7, -2, 2, 4, -2, 2, 4, -3, 3, 6, -4, 0, 7, 6,
	-5, 6, 5, 6, 1, 2, -1, 3, 4, 10, -6, 3, -4, 8, 10, 4, 6, 3, 3, 4, 5, 4
};

int NUM_CUSTOM_GROUPS = 15;
int CustomAttributeGroups[][2] = {
	{0, 3}, {3, 3}, {6, 3}, {9, 3}, {12, 3}, 
	{15, 3}, {18, 3}, {21, 3}, {24, 3}, {27, 4},
	{31, 2}, {36, 2}, {39, 2}, {41, 2}, {43, 2}
};

long long DefaultRaceCustomPicks[] = {
	0x00000801004000LL,
	0x00000241100000LL,
	0x00000005000004LL,
	0x00001202000028LL,
	0x00004001200050LL,
	0x00000000800400LL,
	0x01200000420200LL,
	0x00100001002000LL,
	0x00000101010001LL,
	0x00020001220100LL,
	0x05000012060000LL,
	0x10000001001880LL,
	0x00000001080002LL
};

PLUGINS_API GlobalGameSettings *CreatePlugin()
{
	return new GGMasterOfOrion2Settings();
}

PLUGINS_API void ReleasePlugin(GlobalGameSettings *ggs)
{
	free(ggs);
	ggs = 0;
}

SessionInfo *GGMasterOfOrion2Settings::AllocSession()
{
	return (SessionInfo *)new GMasterOfOrion2(this);
}

SearchReplace *GGMasterOfOrion2Settings::GetConfigReplaceStrings()
{
	return ConfigReplaceStrings;
}

int GGMasterOfOrion2Settings::GetNumConfigReplaceStrings()
{
	return NUM_CONFIG_REPLACE_STRINGS;
}

void GGMasterOfOrion2Settings::Install()
{
	KillGame();
	BringSetSoundToFront();

	PressKey(VK_RETURN);
	SleepC(500);
	PressKey(VK_RETURN);
	SleepC(500);
	PressKey(VK_RETURN);
	SleepC(500);
	PressKey(VK_RETURN);
	SleepC(1000);
	PressKey(VK_DOWN);
	SleepC(500);
	PressKey(VK_RETURN);
	SleepC(500);
	PressKey(VK_RETURN);
	SleepC(500);
	PressKey(VK_RETURN);
	SleepC(500);
	PressKey(VK_RETURN);
	SleepC(1000);
	PressKey(VK_DOWN);
	SleepC(500);
	PressKey(VK_DOWN);
	SleepC(500);
	PressKey(VK_DOWN);
	SleepC(500);
	PressKey(VK_RETURN);

	Sleep(500);
	KillGame();
}

BOOL GGMasterOfOrion2Settings::BringSetSoundToFront()
{
	HWND hWnd;

	if(!(hWnd = FindGameWindow()) && !(hWnd = RunSetSound()))
		return FALSE;
	
	if(IsIconic(hWnd))
		ShowWindow(hWnd, SW_SHOWNORMAL);
	
	SetForegroundWindow(hWnd);
	for(int i = 0; i < 100; i++)
	{
		if(GetForegroundWindow() == hWnd) 
		{
			SleepC(500); // Wait some more for good measure
			return TRUE;
			break;
		}
		SleepC(50);
	}	

	return FALSE;
}

HWND GGMasterOfOrion2Settings::RunSetSound()
{
	HWND hWnd;
	TCHAR setSoundPath[MAX_PATH], *DOSBox;

	if(!(DOSBox = GetDOSBoxPath(gameID)))
		return NULL;

	swprintf(setSoundPath, MAX_PATH, L"\"%s\" \"%ssetsound.exe\"", DOSBox, this->gameFolderPath);

	STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	
	if(!CreateProcess(NULL, setSoundPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		return NULL;

	CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

	SleepC(2 * SECONDS);

	for(int i = 0; i < 100; i++)
	{
		if(hWnd = FindGameWindow()) 
		{
			return hWnd;
		}
		SleepC(100);
	}

	return NULL;
}

void GMasterOfOrion2::PreNewGameEvent()
{
	TCHAR srcPath[MAX_PATH];

	swprintf(srcPath, MAX_PATH, L"%sMOX.SET", ggSettings->gameFolderPath);
	DeleteFile(srcPath);

	Game::PreNewGameEvent();
}

GMasterOfOrion2Team *GMasterOfOrion2::AllocTeam()
{
	struct GMasterOfOrion2Team *team;

	team = new GMasterOfOrion2Team();

	return team;
}

INT_PTR CALLBACK GMasterOfOrion2::CustomRaceDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static GMasterOfOrion2Team *team;
	TCHAR nameStr[64], picksStr[4];
	int nameLen, picksLen, strLen;
	int button, group, attribute, groupAttr;
	BOOL buttonState, attrChecked;
	static int picks = 10, score = 200;
	HFONT boldFont = NULL, normalFont = NULL, largeFont = NULL;

	switch(message)
	{
	case WM_INITDIALOG:
		team = (GMasterOfOrion2Team *)lParam;

		for(int i = 0; i < NUM_CUSTOM_ATTRIBUTES; i++)
		{
			wcscpy_s(nameStr, 64, CustomAttributeNames[i]);
			swprintf(picksStr, 4, L"%+d", CustomAttributePicks[i]);
			picksLen = wcslen(picksStr);
			nameLen = wcslen(nameStr);
			if(i < 31)
				strLen = 21;
			else
				strLen = 26;
			for(int j = 0; j < strLen - nameLen - picksLen - 1; j++)
				wcscat_s(nameStr, 64, L".");
				
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%s %s", nameStr, picksStr);
			SendDlgItemMessage(hDialog, IDC_CHECK1 + i, WM_SETTEXT, 0, (LPARAM)mbBuffer);
		}

		boldFont = pCreateDialogFont(L"Courier New",  8 /*-MulDiv(8, GetDeviceCaps(GetDC(hDialog), LOGPIXELSY), 72)*/, FW_BOLD);
		for(int i = IDC_HEADING_STATIC1; i <= IDC_HEADING_STATIC11; i++)
			SendDlgItemMessage(hDialog, i, WM_SETFONT, (WPARAM)boldFont, (LPARAM)MAKELONG(TRUE, 0));

		normalFont = pCreateDialogFont(FONT_NORMAL, FONT_SIZE_NORMAL, FW_NORMAL);
		SendDlgItemMessage(hDialog, IDOK, WM_SETFONT, (WPARAM)normalFont, (LPARAM)MAKELONG(TRUE, 0));
		SendDlgItemMessage(hDialog, IDC_CLEAR_BUTTON, WM_SETFONT, (WPARAM)normalFont, (LPARAM)MAKELONG(TRUE, 0));

		largeFont = pCreateDialogFont(FONT_NORMAL, FONT_SIZE_LARGE, FW_BOLD);
		SendDlgItemMessage(hDialog, IDC_RACE_STATIC, WM_SETFONT, (WPARAM)largeFont, (LPARAM)MAKELONG(TRUE, 0));
		SendDlgItemMessage(hDialog, IDC_RACE_STATIC, WM_SETTEXT, 0, (LPARAM)FactionNames[team->faction]);

		_itow_s(picks, mbBuffer, MBBUFFER_SIZE, 10);
		SendDlgItemMessage(hDialog, IDC_PICKS_VALUE_STATIC, WM_SETTEXT, 0, (LPARAM)mbBuffer);
		swprintf(mbBuffer, MBBUFFER_SIZE, L"%d%%", score);
		SendDlgItemMessage(hDialog, IDC_SCORE_VALUE_STATIC, WM_SETTEXT, 0, (LPARAM)mbBuffer);
		
		picks = SetDefaultRaceCustomPicks(hDialog, team->faction);
		score = 100 + picks * 10;
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{	
		case IDOK:
			if(picks < 0) 
			{
				MessageBox(hDialog, L"Picks remaining must be greater than or equal to zero.", L"Negative Picks Value", MB_OK | MB_ICONSTOP);
				break;
			}

			for(int i = 0; i < NUM_CUSTOM_ATTRIBUTES; i++)
			{
				attrChecked = IsDlgButtonChecked(hDialog, IDC_CHECK1 + NUM_CUSTOM_ATTRIBUTES - 1 - i);
				team->custom |= ((long long)attrChecked << i);
			}

			EndDialog(hDialog, LOWORD(wParam));
			break;
		case IDCANCEL:
			EndDialog(hDialog, LOWORD(wParam));
			break;
		case IDC_CLEAR_BUTTON:
			for(int i = 0; i < NUM_CUSTOM_ATTRIBUTES; i++)
			{
				SendDlgItemMessage(hDialog, IDC_CHECK1 + i, BM_SETCHECK, BST_UNCHECKED, 0);
				Button_Enable(GetDlgItem(hDialog, IDC_CHECK1 + i), TRUE);
			}
			SendDlgItemMessage(hDialog, IDC_CHECK1 + 28, BM_SETCHECK, BST_CHECKED, 0);

			picks = 10;
			score = 200;
			_itow_s(picks, mbBuffer, MBBUFFER_SIZE, 10);
			SendDlgItemMessage(hDialog, IDC_PICKS_VALUE_STATIC, WM_SETTEXT, 0, (LPARAM)mbBuffer);
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%d%%", score);
			SendDlgItemMessage(hDialog, IDC_SCORE_VALUE_STATIC, WM_SETTEXT, 0, (LPARAM)mbBuffer);
			break;
		}
		button = LOWORD(wParam);
		if(button >= IDC_CHECK1 && button < IDC_CHECK1 + NUM_CUSTOM_ATTRIBUTES)
		{
			attribute = button - IDC_CHECK1;
			buttonState = IsDlgButtonChecked(hDialog, button);

			if(!buttonState && attribute >= 27 && attribute <= 30)
			{
				SendDlgItemMessage(hDialog, IDC_CHECK1 + attribute, BM_SETCHECK, BST_CHECKED, 0);
				break;
			}
						
			picks -= (buttonState ? 1 : -1) * CustomAttributePicks[attribute];
			score -= (buttonState ? 1 : -1) * CustomAttributePicks[attribute] * 10;
			
			if(buttonState && -1 != (group = FindCustomGroup(attribute)))
			{
				for(int i = 0; i < CustomAttributeGroups[group][1]; i++)
				{
					groupAttr = CustomAttributeGroups[group][0] + i;
					if(groupAttr != attribute)
					{
						if(IsDlgButtonChecked(hDialog, groupAttr + IDC_CHECK1))
						{
							picks += CustomAttributePicks[groupAttr];
							score += CustomAttributePicks[groupAttr] * 10;
							SendDlgItemMessage(hDialog, groupAttr + IDC_CHECK1, BM_SETCHECK, BST_UNCHECKED, 0);
							break;
						}
					}
				}
			}
			
			_itow_s(picks, mbBuffer, MBBUFFER_SIZE, 10);
			SendDlgItemMessage(hDialog, IDC_PICKS_VALUE_STATIC, WM_SETTEXT, 0, (LPARAM)mbBuffer);
			swprintf(mbBuffer, MBBUFFER_SIZE, L"%d%%", score);
			SendDlgItemMessage(hDialog, IDC_SCORE_VALUE_STATIC, WM_SETTEXT, 0, (LPARAM)mbBuffer);
		
			DisableCustomAttributes(hDialog, picks);
		}
		break;
	case WM_DESTROY:
		DeleteObject(largeFont);
		DeleteObject(normalFont);
		DeleteObject(boldFont);
		break;
	}

	return FALSE;
}

int GMasterOfOrion2::FindCustomGroup(int attribute)
{
	for(int i = 0; i < NUM_CUSTOM_GROUPS; i++)
	{
		if(attribute >= CustomAttributeGroups[i][0] && attribute < CustomAttributeGroups[i][0] + CustomAttributeGroups[i][1])
			return i;
	}

	return -1; 
}

void GMasterOfOrion2::DisableCustomAttributes(HWND hDialog, int picks)
{
	int negPicks = 0;
	int itemChecked = -1;
	int attr;

	for(int i = 0; i < NUM_CUSTOM_ATTRIBUTES; i++)
	{
		if(IsDlgButtonChecked(hDialog, IDC_CHECK1 + i) && CustomAttributePicks[i] < 0)
			negPicks -= CustomAttributePicks[i];
	}

	for(int i = 0; i < NUM_CUSTOM_ATTRIBUTES; i++)
	{
		if(!IsDlgButtonChecked(hDialog, IDC_CHECK1 + i) && (picks - CustomAttributePicks[i] < 0
				|| (CustomAttributePicks[i] < 0 && negPicks - CustomAttributePicks[i] > 10)))
			Button_Enable(GetDlgItem(hDialog, IDC_CHECK1 + i), FALSE);
		//else if(IsDlgButtonChecked(hDialog, IDC_CHECK1 + i) && picks + CustomAttributePicks[i] < 0)
		//	Button_Enable(GetDlgItem(hDialog, IDC_CHECK1 + i), FALSE);
		else
			Button_Enable(GetDlgItem(hDialog, IDC_CHECK1 + i), TRUE);
	}

	for(int i = 0; i < NUM_CUSTOM_GROUPS; i++)
	{
		itemChecked = -1;

		for(int j = 0; j < CustomAttributeGroups[i][1]; j++)
		{
			if(IsDlgButtonChecked(hDialog, IDC_CHECK1 + CustomAttributeGroups[i][0] + j))
			{	
				itemChecked = CustomAttributeGroups[i][0] + j;
				break;
			}
		}		
		
		if(itemChecked != -1)
		{
			for(int j = 0; j < CustomAttributeGroups[i][1]; j++)
			{
				attr = CustomAttributeGroups[i][0] + j;

				if(!IsDlgButtonChecked(hDialog, IDC_CHECK1 + attr))
				{
					if(picks - CustomAttributePicks[attr] + CustomAttributePicks[itemChecked] < 0
							|| (CustomAttributePicks[attr] < 0 && CustomAttributePicks[itemChecked] < 0 && negPicks - CustomAttributePicks[attr] + CustomAttributePicks[itemChecked] > 10))
						Button_Enable(GetDlgItem(hDialog, IDC_CHECK1 + attr), FALSE);
					else
						Button_Enable(GetDlgItem(hDialog, IDC_CHECK1 + attr), TRUE);
				}
			}
		}
	}
}

int GMasterOfOrion2::SetDefaultRaceCustomPicks(HWND hDialog, int faction)
{
	long long attrs = DefaultRaceCustomPicks[faction];
	int picks = 10;

	for(int i = NUM_CUSTOM_ATTRIBUTES - 1; i >= 0; i--)
	{
		if(attrs & ((long long)1 << i))
		{
			SendDlgItemMessage(hDialog, IDC_CHECK1 + (NUM_CUSTOM_ATTRIBUTES - 1 - i), BM_SETCHECK, BST_CHECKED, 0);	
			picks -= CustomAttributePicks[NUM_CUSTOM_ATTRIBUTES - 1 - i];	
		}
	}
	DisableCustomAttributes(hDialog, picks);

	return picks;
}

INT_PTR CALLBACK GMasterOfOrion2::TeamSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	static GMasterOfOrion2 *session;
	static GMasterOfOrion2Team *team;
	int comboIndex;
	static BOOL colorTable[NUM_BANNER_COLORS];
	LinkedList *iter;
	TCHAR playerList[1024];

	switch(message)
	{
	case WM_INITDIALOG:
		session = (GMasterOfOrion2 *)lParam;
		
		memset(&colorTable, FALSE, NUM_BANNER_COLORS * sizeof(BOOL));
		iter = session->teams;
		while(iter->next)
		{
			iter = iter->next;
			colorTable[((GMasterOfOrion2Team *)iter->item)->bannerColor] = TRUE;
		}

		for(int i = 0; i < NUM_BANNER_COLORS; i++)
		{
			if(colorTable[i] == FALSE)
				SendDlgItemMessage(hDialog, IDC_BANNER_COLOR_COMBO, CB_ADDSTRING, 0, (LPARAM)BannerColorNames[i]); 
		}
		SendDlgItemMessage(hDialog, IDC_BANNER_COLOR_COMBO, CB_SETCURSEL, 0, 0);
		SendDlgItemMessage(hDialog, IDC_RULER_EDIT, WM_SETTEXT, 0, (LPARAM)session->getYourPlayerName(session));
		
		session->BuildTeamList(session, session->players[session->currentPlayer]->team, playerList);
		swprintf(mbBuffer, MBBUFFER_SIZE, L"Select settings for %s (%s)", session->getCurrentPlayerFactionName(session), playerList);
		SendDlgItemMessage(hDialog, IDC_SELECT_SETTINGS_STATIC, WM_SETTEXT, 0, (LPARAM)mbBuffer);

		team = new GMasterOfOrion2Team();
		team->id = session->getCurrentPlayerTeam(session);
		team->faction = session->getCurrentPlayerFaction(session);
		team->state = TEAM_MODIFY;

		//session->SetTopMost(hDialog);
		//EnableWindow(*session->PTR_hMainWnd, FALSE);
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) 
		{	
		case IDOK:
			SendDlgItemMessage(hDialog, IDC_RULER_EDIT, WM_GETTEXT, MAX_SETTING, (LPARAM)team->ruler);
			comboIndex = SendDlgItemMessage(hDialog, IDC_BANNER_COLOR_COMBO, CB_GETCURSEL, 0, 0); 
			for(int i = 0; i < NUM_BANNER_COLORS; i++)
			{
				if(colorTable[i] == FALSE)
					comboIndex--;

				if(comboIndex == -1)
				{
					team->bannerColor = i;
					break;
				}
			}

			if(team->ruler[0] == L'\0')
				wcscpy_s(team->ruler, MAX_SETTING, session->getYourPlayerName(session));
			
			session->LL_Add(session->teams, team);
			
			//EnableWindow(*session->PTR_hMainWnd, TRUE);
			EndDialog(hDialog, LOWORD(wParam));
				
			break;
		case IDC_CUSTOM_RACE_BUTTON:
			if(session->gameSettings.difficulty < 2)
			{
				MessageBox(*session->PTR_hMainWnd, L"Customizing races is not allowed on Tutor and Easy levels.", L"Difficulty too low", MB_OK | MB_ICONEXCLAMATION);
				break;
			}
			DialogBoxParam(session->ggSettings->hModule, MAKEINTRESOURCE(IDD_CUSTOM_RACE), hDialog, session->CustomRaceDialogProc, (LPARAM)team);
			break;
		}
		break;
	case WM_DESTROY:
		//EnableWindow(*session->PTR_hMainWnd, TRUE);
		//EndDialog(hDialog, IDCANCEL);
		break;
	}

	return FALSE;
}

BOOL GMasterOfOrion2::ValidateTeamSettings(Team *team, int type, int sIndex, int tIndex)
{
	GMasterOfOrion2Team *pTeam = (GMasterOfOrion2Team *)team;
	TCHAR cfgError[MBBUFFER_SIZE];
	HWND hWnd = NULL;

	cfgError[0] = L'\0';

	if(type == VALIDATE_FILE) 
		swprintf(cfgError, sizeof(cfgError), L"Error in %s in session #%d, team #%d", SESSIONS_CONFIG_FILE, sIndex, tIndex);

	trimWhiteSpace(pTeam->ruler);
	if(pTeam->ruler[0] == L'\0')
	{
		if(type == VALIDATE_FILE)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Setting \"ruler_name\" cannot be left blank.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(pTeam->bannerColor < 0 || pTeam->bannerColor >= NUM_BANNER_COLORS)
	{
		if(type == VALIDATE_FILE)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Setting \"banner_color\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	return TRUE;
}

DLGPROC GMasterOfOrion2::GetTeamSettingsDialogProc()
{
	return TeamSettingsDialogProc;
}

INT_PTR CALLBACK GMasterOfOrion2::GameSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_TACTICAL_COMBAT_CHECK)
			{
				if(IsDlgButtonChecked(hDialog, IDC_TACTICAL_COMBAT_CHECK))
					MessageBox(GetParent(hDialog), L"Please note: PlayMailer does not support Tactical Combat, i.e. it will not send the save game between players during combat. Therefore you are advised to turn this option off.", L"Important", MB_ICONEXCLAMATION | MB_OK);
			}
			break;
		default:
			break;
	}

	return FALSE;
}

BOOL GMasterOfOrion2::NewGame()
{
	int numTeams;
	int xPos = 0, yPos = 0;
	GMasterOfOrion2Team *team;
	LinkedList *iter;

	// Main menus
	PressLButton();
	SleepC(4000);
	PressKey(VK_M);
	SleepC(1000);
	PressKey(VK_H);
	PressKey(VK_S);
	SleepC(500);

	// Game settings
	ResetMouse();
	MoveMouseMod(175, 160);
	for(int i = 0; i < gameSettings.difficulty; i++)
		PressLButton();
	MoveMouseMod(0, 146);
	for(int i = 0; i < mod(gameSettings.totalPlayers - 5, 7); i++)
		PressLButton();
	MoveMouseMod(160, 0);
	for(int i = 0; i < mod(gameSettings.techLevel - 1, 3); i++)
		PressLButton();
	MoveMouseMod(0, -146);
	for(int i = 0; i < mod(gameSettings.galaxySize - 1, 4); i++)
		PressLButton();
	MoveMouseMod(160, 0);
	for(int i = 0; i < mod(gameSettings.galaxyAge - 1, 3); i++)
		PressLButton();
	MoveMouseMod(0, 133);
	
	if(gameSettings.tacticalCombat)
		PressLButton();
	MoveMouseMod(0, 38);
	if(!gameSettings.randomEvents)
		PressLButton();
	MoveMouseMod(0, 38);
	if(!gameSettings.antaransAttack)
	PressLButton();
	MoveMouseMod(0, 57);
	PressLButton();
	SleepC(500);

	// Create players
	numTeams = LL_Size(this->teams);
	iter = this->teams;

	while(iter->next)
	{
		iter = iter->next;
		team = (GMasterOfOrion2Team *)iter->item;

		// If custom set, choose custom for race.
		if(team->custom)
		{
			ResetMouse();
			MoveMouseMod(562, 417);
			PressLButton();
		}

		// Choose race
		ResetMouse();
		xPos = 430; 
		yPos = 117;
		if(team->faction > 6) 
			xPos += 132;
		yPos += team->faction % 7 * 50;
		MoveMouseMod(xPos, yPos);
		PressLButton();
		SleepC(500);

		// Choose custom race attributes
		if(team->custom)
			InputCustomAttributes(team->custom);

		// Enter name
		for(int j = 0; j < 13; j++)
			PressKey(VK_BACK);
		PressStringKeys(team->ruler, 13);
		PressKey(VK_RETURN);
		SleepC(500);

		// Choose player colour
		ResetMouse();
		xPos = 140;
		yPos = 200;
		if(team->bannerColor > 3)
			yPos += 140;
		xPos += team->bannerColor % 4 * 130;
		MoveMouseMod(xPos, yPos);
		PressLButton();
		SleepC(1500);

		// Check if all players created
		ResetMouse();
		if(iter->next)
			MoveMouseMod(330, 240);
		else
			MoveMouseMod(330, 278);
		PressLButton();
		PressKey(VK_Y);
		SleepC(500);
	}

	// Select first player
	if(numTeams > 1)
	{	
		yPos = 234;
		yPos -= 20 * (numTeams - 2);
		SleepC(1500);
		ResetMouse();
		MoveMouseMod(300, yPos);
		PressLButton();
		SleepC(500);
	}

	SleepC(2000);
	if(CheckFullScreen(this))
		SleepC(5000);
	return TRUE;
}

void GMasterOfOrion2::InputCustomAttributes(long long custom)
{
	long long attr = NUM_CUSTOM_ATTRIBUTES - 1;

	ResetMouse();

	// Press Clear button
	MoveMouseMod(120, 481);
	PressLButton();
	MoveMouseMod(0, -600);

	// Set column 1
	MoveMouseMod(0, 80);
	for(int i = 0; i < 5; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			if(custom & ((long long)1 << attr))
				PressLButton();
			attr--;

			if(j != 2)
				MoveMouseMod(0, 14);
		}

		if(i != 4)
			MoveMouseMod(0, 51);
	}

	// Set column 2
	MoveMouseMod(200, -600);
	MoveMouseMod(0, 80);
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			if(custom & ((long long)1 << attr))
				PressLButton();
			attr--;

			if(j != 2)
				MoveMouseMod(0, 14);
		}
		MoveMouseMod(0, 51);
	}
	for(int j = 0; j < 4; j++)
	{
		if(custom & ((long long)1 << attr))
			PressLButton();
		attr--;

		if(j != 3)
			MoveMouseMod(0, 14);
	}

	// Set column 3
	MoveMouseMod(243, -600);
	MoveMouseMod(0, 80);
	for(int j = 0; j < 22; j++)
	{
		if(custom & ((long long)1 << attr))
			PressLButton();
		attr--;

		if(j != 21)
			MoveMouseMod(0, 17);
	}

	// Press Accept button
	MoveMouseMod(0, 47);
	PressLButton();
	SleepC(500);
}

BOOL GMasterOfOrion2::LoadGame()
{
	int xPos, yPos; 
	int numTeams;

	numTeams = LL_Size(this->teams);
	
	if(*PTR_NewGameInstance) 
	{	
		PressLButton();
		SleepC(1500);
		xPos = 310, yPos = 88;
	}
	else
	{
		PressKey(VK_ESCAPE);
		PressKey(VK_G);
		xPos = 280, yPos = 63;
	}

	PressKey(VK_L);
	ResetMouse();
	yPos += (gameSettings.saveSlot - 1) * 33;
	MoveMouseMod(xPos, yPos);
	PressKey(VK_RETURN);
	
	// End Turn
	PressKey(VK_T);
	SleepC(500);

	// Choose next player
	if(numTeams > 1)
	{
		yPos = 234;
		yPos -= 20 * (numTeams - 2);
		yPos += 38 * LL_GetItemIndex(this->teams, FindTeam(this, getCurrentPlayerTeam(this)));
		ResetMouse();
		MoveMouseMod(300, yPos);
		PressLButton();
	}

	CheckFullScreen(this);

	return TRUE;
}

BOOL GMasterOfOrion2::SaveGame()
{
	int xPos = 280, yPos = 63;
	PressKey(VK_ESCAPE);
	PressKey(VK_G);
	PressKey(VK_S);
	
	yPos += (gameSettings.saveSlot - 1) * 32; 
	ResetMouse();
	MoveMouseMod(xPos, yPos);

	PressKey(VK_RETURN);
	for(int i = 0; i < 30; i++)
		PressKey(VK_BACK);
	PressStringKeys(this->sessionName, 30);
	
	StartSaveFileThread(this);
	PressKey(VK_RETURN);
	return WaitForSaveFileThread();

	return TRUE;
}

void GMasterOfOrion2::InitGameSettingsDialog()
{
	GMasterOfOrion2Settings *game;
	HWND hDlg = *PTR_hSessionSettingsDialog, hGameDlg;
	
	hGameDlg = *PTR_hGameSettingsDialog = CreateDialogParam(this->ggSettings->hModule, MAKEINTRESOURCE(IDD_GAME), *PTR_hSessionSettingsDialog, GameSettingsDialogProc, NULL);
	SetWindowPos(*PTR_hGameSettingsDialog, GetDlgItem(*PTR_hSessionSettingsDialog, EDITPLAYERS_BUTTON), DLUToPixelsX(hDlg, GAME_SETTINGS_PANE_X), DLUToPixelsY(hDlg, GAME_SETTINGS_PANE_Y), 0, 0, SWP_NOSIZE);
	
	game = &this->gameSettings;

	for(int i = 0; i < NUM_DIFFICULTIES; i++)
		SendDlgItemMessage(hGameDlg, IDC_DIFFICULTY_COMBO, CB_ADDSTRING, 0, (LPARAM)DifficultyNames[i]); 
	
	for(int i = 0; i < NUM_TOTAL_PLAYERS; i++)
		SendDlgItemMessage(hGameDlg, IDC_PLAYERS_COMBO, CB_ADDSTRING, 0, (LPARAM)TotalPlayersNames[i]); 

	for(int i = 0; i < NUM_GALAXY_SIZES; i++)
		SendDlgItemMessage(hGameDlg, IDC_GALAXYSIZE_COMBO, CB_ADDSTRING, 0, (LPARAM)GalaxySizeNames[i]); 

	for(int i = 0; i < NUM_GALAXY_AGES; i++)
		SendDlgItemMessage(hGameDlg, IDC_GALAXYAGE_COMBO, CB_ADDSTRING, 0, (LPARAM)GalaxyAgeNames[i]); 

	for(int i = 0; i < NUM_TECH_LEVELS; i++)
		SendDlgItemMessage(hGameDlg, IDC_TECHLEVEL_COMBO, CB_ADDSTRING, 0, (LPARAM)TechLevelNames[i]); 

	SendDlgItemMessage(hGameDlg, IDC_SAVESLOT_RADIO1 + game->saveSlot - 1, BM_SETCHECK, BST_CHECKED, 0);
	
	SendDlgItemMessage(hGameDlg, IDC_DIFFICULTY_COMBO, CB_SETCURSEL, game->difficulty, 0);
	SendDlgItemMessage(hGameDlg, IDC_PLAYERS_COMBO, CB_SETCURSEL, game->totalPlayers - 2, 0);
	SendDlgItemMessage(hGameDlg, IDC_GALAXYSIZE_COMBO, CB_SETCURSEL, game->galaxySize, 0);
	SendDlgItemMessage(hGameDlg, IDC_GALAXYAGE_COMBO, CB_SETCURSEL, game->galaxyAge, 0);
	SendDlgItemMessage(hGameDlg, IDC_TECHLEVEL_COMBO, CB_SETCURSEL, game->techLevel, 0);

	CheckDlgButton(hGameDlg, IDC_TACTICAL_COMBAT_CHECK, game->tacticalCombat);
	CheckDlgButton(hGameDlg, IDC_RANDOM_EVENTS_CHECK, game->randomEvents);
	CheckDlgButton(hGameDlg, IDC_ANTARANS_ATTACK_CHECK, game->antaransAttack);
	
	if(this->turnNumber > 1)
	{
		EnableWindow(GetDlgItem(hGameDlg, IDC_DIFFICULTY_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_PLAYERS_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_GALAXYSIZE_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_GALAXYAGE_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_TECHLEVEL_COMBO), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_TACTICAL_COMBAT_CHECK), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_RANDOM_EVENTS_CHECK), FALSE);
		EnableWindow(GetDlgItem(hGameDlg, IDC_ANTARANS_ATTACK_CHECK), FALSE);
	}
}

BOOL GMasterOfOrion2::ParseGameSettingsDialog()
{
	GMasterOfOrion2Settings *game = &this->gameSettings;
	HWND hDlg = *PTR_hGameSettingsDialog;

	for(int i = IDC_SAVESLOT_RADIO1; i <= IDC_SAVESLOT_RADIO9; i++)
	{
		if(IsDlgButtonChecked(hDlg, i)) 
			game->saveSlot = i - IDC_SAVESLOT_RADIO1 + 1;
	}

	game->tacticalCombat = IsDlgButtonChecked(hDlg, IDC_TACTICAL_COMBAT_CHECK);
	game->randomEvents = IsDlgButtonChecked(hDlg, IDC_RANDOM_EVENTS_CHECK);
	game->antaransAttack = IsDlgButtonChecked(hDlg, IDC_ANTARANS_ATTACK_CHECK);

	game->difficulty = SendDlgItemMessage(hDlg, IDC_DIFFICULTY_COMBO, CB_GETCURSEL, 0, 0); 
	game->totalPlayers = SendDlgItemMessage(hDlg, IDC_PLAYERS_COMBO, CB_GETCURSEL, 0, 0) + 2; 
	game->galaxySize = SendDlgItemMessage(hDlg, IDC_GALAXYSIZE_COMBO, CB_GETCURSEL, 0, 0); 
	game->galaxyAge = SendDlgItemMessage(hDlg, IDC_GALAXYAGE_COMBO, CB_GETCURSEL, 0, 0); 
	game->techLevel = SendDlgItemMessage(hDlg, IDC_TECHLEVEL_COMBO, CB_GETCURSEL, 0, 0); 

	return ValidateGameSettings(VALIDATE_GUI, 0);
}

BOOL GMasterOfOrion2::ValidateGameSettings(BOOL type, int sIndex)
{
	TCHAR cfgError[MBBUFFER_SIZE];
	GMasterOfOrion2Settings *game = &this->gameSettings;
	HWND hWnd = NULL;

	cfgError[0] = L'\0';

	if(type == VALIDATE_FILE) 
		swprintf(cfgError, sizeof(cfgError), L"Error in %s in session #%d. ", SESSIONS_CONFIG_FILE, sIndex);
	else if(type == VALIDATE_GUI)
		hWnd = *PTR_hSessionSettingsDialog;

	if(type != VALIDATE_EMAIL)
	{
		// Check save slot is valid
		if(game->saveSlot < 1 || game->saveSlot > 9)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Setting \"save_slot\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}

	if(game->difficulty < 0 && game->difficulty >= NUM_DIFFICULTIES)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Setting \"computer_difficulty\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->totalPlayers < 0 && game->totalPlayers >= NUM_TOTAL_PLAYERS + 2)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Setting \"total_players\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->galaxySize < 0 && game->galaxySize >= NUM_GALAXY_SIZES)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Setting \"galaxy_size\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->galaxyAge < 0 && game->galaxyAge >= NUM_GALAXY_AGES)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Setting \"galaxy_age\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	if(game->techLevel < 0 && game->techLevel >= NUM_TECH_LEVELS)
	{
		if(type != VALIDATE_EMAIL)
		{
			swprintf(mbBuffer, MBBUFFER_SIZE, L"Setting \"tech_level\" is out of range.", cfgError);
			MessageBox(hWnd, mbBuffer, L"Invalid setting", MB_OK | MB_ICONERROR);
		}
		return FALSE;
	}

	return TRUE;
}

void GMasterOfOrion2::InitGameSettings()
{
	memset(&gameSettings, 0, sizeof(GMasterOfOrion2Settings));

	this->gameSettings.saveSlot = 1;
	this->gameSettings.difficulty = 2;
	this->gameSettings.galaxySize = 1;
	this->gameSettings.galaxyAge = 1;
	this->gameSettings.techLevel = 1;
	this->gameSettings.totalPlayers = 5;
	this->gameSettings.tacticalCombat = FALSE;
	this->gameSettings.randomEvents = TRUE;
	this->gameSettings.antaransAttack = TRUE;
}

void GMasterOfOrion2::SaveGameSettings(config_setting_t *group)
{
	cfgSetInt(group, L"save_slot", this->gameSettings.saveSlot);
	cfgSetInt(group, L"difficulty", this->gameSettings.difficulty);
	cfgSetInt(group, L"galaxy_size", this->gameSettings.galaxySize);
	cfgSetInt(group, L"galaxy_age", this->gameSettings.galaxyAge);
	cfgSetInt(group, L"tech_level", this->gameSettings.techLevel);
	cfgSetInt(group, L"total_players", this->gameSettings.totalPlayers);
	cfgSetBool(group, L"tactical_combat", this->gameSettings.tacticalCombat);
	cfgSetBool(group, L"random_events", this->gameSettings.randomEvents);
	cfgSetBool(group, L"antarans_attack", this->gameSettings.antaransAttack);
}

void GMasterOfOrion2::LoadGameSettings(config_setting_t *group)
{
	this->gameSettings.saveSlot = cfgGetInt(group, L"save_slot");
	this->gameSettings.difficulty = cfgGetInt(group, L"difficulty");
	this->gameSettings.galaxySize = cfgGetInt(group, L"galaxy_size");
	this->gameSettings.galaxyAge = cfgGetInt(group, L"galaxy_age");
	this->gameSettings.techLevel = cfgGetInt(group, L"tech_level");
	this->gameSettings.totalPlayers = cfgGetInt(group, L"total_players");
	this->gameSettings.tacticalCombat = cfgGetBool(group, L"tactical_combat");
	this->gameSettings.randomEvents = cfgGetBool(group, L"random_events");
	this->gameSettings.antaransAttack = cfgGetBool(group, L"antarans_attack");
}

void GMasterOfOrion2::BuildEmailGameSettings(config_setting_t *group)
{
	cfgSetInt(group, L"difficulty", this->gameSettings.difficulty);
	cfgSetInt(group, L"galaxy_size", this->gameSettings.galaxySize);
	cfgSetInt(group, L"galaxy_age", this->gameSettings.galaxyAge);
	cfgSetInt(group, L"tech_level", this->gameSettings.techLevel);
	cfgSetInt(group, L"total_players", this->gameSettings.totalPlayers);
	cfgSetBool(group, L"tactical_combat", this->gameSettings.tacticalCombat);
	cfgSetBool(group, L"random_events", this->gameSettings.randomEvents);
	cfgSetBool(group, L"antarans_attack", this->gameSettings.antaransAttack);
}

void GMasterOfOrion2::ParseEmailGameSettings(config_setting_t *group)
{
	this->gameSettings.difficulty = cfgGetInt(group, L"difficulty");
	this->gameSettings.galaxySize = cfgGetInt(group, L"galaxy_size");
	this->gameSettings.galaxyAge = cfgGetInt(group, L"galaxy_age");
	this->gameSettings.techLevel = cfgGetInt(group, L"tech_level");
	this->gameSettings.totalPlayers = cfgGetInt(group, L"total_players");
	this->gameSettings.tacticalCombat = cfgGetBool(group, L"tactical_combat");
	this->gameSettings.randomEvents = cfgGetBool(group, L"random_events");
	this->gameSettings.antaransAttack = cfgGetBool(group, L"antarans_attack");
}

void GMasterOfOrion2::UpdateEmailGameSettings(SessionInfo *in)
{
	GMasterOfOrion2 *pIn = (GMasterOfOrion2 *)in;

	this->gameSettings.difficulty = pIn->gameSettings.difficulty;
	this->gameSettings.galaxySize = pIn->gameSettings.galaxySize;
	this->gameSettings.galaxyAge = pIn->gameSettings.galaxyAge;
	this->gameSettings.techLevel = pIn->gameSettings.techLevel;
	this->gameSettings.totalPlayers = pIn->gameSettings.totalPlayers;
	this->gameSettings.tacticalCombat = pIn->gameSettings.tacticalCombat;
	this->gameSettings.randomEvents = pIn->gameSettings.randomEvents;
	this->gameSettings.antaransAttack = pIn->gameSettings.antaransAttack;
}

void GMasterOfOrion2::SaveTeamSettings(config_setting_t *group, Team *team)
{
	GMasterOfOrion2Team *pTeam = (GMasterOfOrion2Team *)team; 

	cfgSetInt(group, L"banner_color", pTeam->bannerColor);
	cfgSetString(group, L"ruler_name", pTeam->ruler);
	cfgSetInt64(group, L"custom_attributes", pTeam->custom); 
}

void GMasterOfOrion2::LoadTeamSettings(config_setting_t *group, Team *team)
{
	GMasterOfOrion2Team *pTeam = (GMasterOfOrion2Team *)team; 

	pTeam->bannerColor = cfgGetInt(group, L"banner_color");
	pTeam->custom = cfgGetInt64(group, L"custom_attributes");
	cfgGetString(group, L"ruler_name", pTeam->ruler);
}

void GMasterOfOrion2::BuildEmailTeamSettings(config_setting_t *group, Team *team)
{
	GMasterOfOrion2Team *pTeam = (GMasterOfOrion2Team *)team; 

	cfgSetInt(group, L"banner_color", pTeam->bannerColor);
	cfgSetString(group, L"ruler_name", pTeam->ruler);
	cfgSetInt64(group, L"custom_attributes", pTeam->custom);
}

void GMasterOfOrion2::ParseEmailTeamSettings(config_setting_t *group, Team *team)
{
	GMasterOfOrion2Team *pTeam = (GMasterOfOrion2Team *)team; 

	pTeam->bannerColor = cfgGetInt(group, L"banner_color");
	pTeam->custom = cfgGetInt64(group, L"custom_attributes");
	cfgGetString(group, L"ruler_name", pTeam->ruler);
}

void GMasterOfOrion2::UpdateEmailTeamSettings(Team *out, Team *in)
{
	GMasterOfOrion2Team *pOut = (GMasterOfOrion2Team *)out; 
	GMasterOfOrion2Team *pIn = (GMasterOfOrion2Team *)in;

	wcscpy_s(pOut->ruler, MAX_SETTING, pIn->ruler);
	pOut->bannerColor = pIn->bannerColor;
	pOut->custom = pIn->custom;
}

TCHAR *GMasterOfOrion2::GetSaveFileName(TCHAR *name)
{
	swprintf(name, MAX_PATH, L"SAVE%d.GAM", this->gameSettings.saveSlot);

	return name;
}









