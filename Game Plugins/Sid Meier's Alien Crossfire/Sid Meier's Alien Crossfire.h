#ifndef ALIEN_CROSSFIRE_H
#define ALIEN_CROSSFIRE_H

#ifdef PLUGIN_EXPORTS
#define PLUGINS_API __declspec(dllexport)
#else
#define PLUGINS_API __declspec(dllimport)
#endif

#include "game.h"
#include "data_structures.h"
#include "PlayMailer.h"
#include "callbacks.h"

extern "C" PLUGINS_API GlobalGameSettings *CreatePlugin();
extern "C" PLUGINS_API void ReleasePlugin(GlobalGameSettings *ggs);

class GGAlienCrossfireSettings : public GlobalGameSettings
{
public:
	TCHAR DEFAULT_RUN_COMMAND[MAX_PATH];

	SessionInfo *AllocSession();
	void CloseGameWindow(HWND hWnd);

	BOOL firstRun;
	
	GGAlienCrossfireSettings() 
	{
		this->gameID = L"Sid Meier's Alien Crossfire"; 
		this->gameDetails = L"Firaxis Games 2000";
		this->MAX_TEAMS = 7;
		this->NUM_FACTIONS = 14;
		this->RUN_DELAY = 5 * SECONDS;
		this->FS_MOUSE_MODIFIER = 2.0;
		this->MOUSE_MODIFIER = 2.0;
		this->GAME_EXE_NAME = L"terranx.exe";
		this->WarnBeforeNewGame = TRUE;
		this->RandomFactionOrder = TRUE;
		this->KillBeforeRunGame = TRUE;
		this->KillBeforeLoadGame = TRUE;
		this->RequestTeamSettings = TRUE;
		this->fullScreenToggle = FALSE;
		this->runInDOSBox = FALSE;
		this->restoreColorDepth = TRUE;
		this->duplicateFactions = TRUE;
		this->windowText = L"Sid Meier's Alpha Centauri";
		this->windowClass = L"JackalClass";
		this->firstRun = TRUE;
	}

	static INT_PTR CALLBACK GGDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);

	GGAlienCrossfireSettings *Clone()
	{
		return new GGAlienCrossfireSettings(*this);
	}

	GGAlienCrossfireSettings *Clone(GlobalGameSettings *out)
	{
		GGAlienCrossfireSettings *ret = (GGAlienCrossfireSettings *)out;

		*ret = *this;
		return ret;
	}
};

class GAlienCrossfireTeam : public Team
{
public:
	TCHAR name[24];
	TCHAR formal[40];
	TCHAR noun[24];
	TCHAR adjective[24];
	TCHAR title[24];
	TCHAR description[24];
	TCHAR gender[24];

	GAlienCrossfireTeam(TCHAR *name, TCHAR *formal, TCHAR *noun, TCHAR *adjective, TCHAR *title, TCHAR *description, TCHAR *gender)
	{
		wcscpy_s(this->name, 24, name);
		wcscpy_s(this->formal, 40, formal);
		wcscpy_s(this->noun, 24, noun);
		wcscpy_s(this->adjective, 24, adjective);
		wcscpy_s(this->title, 24, title);
		wcscpy_s(this->description, 24, description);
		wcscpy_s(this->gender, 24, gender);
	}

	GAlienCrossfireTeam()
	{
		name[0] = L'\0';
		formal[0] = L'\0';
		noun[0] = L'\0';
		adjective[0] = L'\0';
		title[0] = L'\0';
		description[0] = L'\0';
		gender[0] = L'\0';
	}

	GAlienCrossfireTeam *Clone() 
	{ 
		return new GAlienCrossfireTeam(*this);
	}
};

class GAlienCrossfire : public SessionInfo
{
public:
	static TCHAR *FactionNames[];
	static TCHAR *MapTypeNames[];
	static const int NUM_MAP_TYPES;
	static TCHAR *DifficultyNames[];
	static const int NUM_DIFFICULTIES;
	static TCHAR *PlanetSizeNames[];
	static const int NUM_PLANET_SIZES;
	static TCHAR *GameRulesMenuNames[];
	static const int NUM_GAME_RULES_MENU_ITEMS;
	static TCHAR *OceanCoverageNames[];
	static const int NUM_OCEAN_COVERAGES;
	static TCHAR *ErosiveForcesNames[];
	static const int NUM_EROSIVE_FORCES;
	static TCHAR *NativeLifeFormNames[];
	static const int NUM_NATIVE_LIFE_FORMS;
	static TCHAR *CloudCoverNames[];
	static const int NUM_CLOUD_COVERS;
	static TCHAR *CustomRuleNames[];
	static const int NUM_CUSTOM_RULES;
	static BOOL CustomRuleDefaults[];
	static GAlienCrossfireTeam FactionDefaults[];
	
	struct GAlienCrossfireSettings : GameSettings
	{
		int mapType;
		int difficulty;
		int planetSize;
		int gameRules;
		int oceanCoverage;
		int erosiveForces;
		int nativeLifeForms;
		int cloudCover;
		TCHAR mapFile[MAX_PATH];
		int horizontalMapSize;
		int verticalMapSize;
		long long customRules;
	} gameSettings;

	// Overridden functions
	static INT_PTR CALLBACK TeamSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
	DLGPROC GetTeamSettingsDialogProc();
	BOOL LoadGame();
	BOOL NewGame();
	BOOL SaveGame();
	void InitGameSettingsDialog();
	BOOL ParseGameSettingsDialog();
	BOOL ValidateGameSettings(BOOL type, int sIndex);
	void InitGameSettings();
	void SaveGameSettings(config_setting_t *group);
	void LoadGameSettings(config_setting_t *group);
	void BuildEmailGameSettings(config_setting_t *group);
	void ParseEmailGameSettings(config_setting_t *group);
	void UpdateEmailGameSettings(SessionInfo *in);
	void SaveTeamSettings(config_setting_t *group, Team *team);
	void LoadTeamSettings(config_setting_t *group, Team *team);
	void BuildEmailTeamSettings(config_setting_t *group, Team *team);
	void ParseEmailTeamSettings(config_setting_t *group, Team *team);
	void UpdateEmailTeamSettings(Team *out, Team *in);
	TCHAR *GetSaveFileName(TCHAR *name);
	TCHAR *GetSaveFolderPath();
	GAlienCrossfireTeam *AllocTeam();
	BOOL ValidateTeamSettings(Team *team, int type, int sIndex, int tIndex);
	static void UpdateGameSettingsDialog(HWND hGameDlg);
	static INT_PTR CALLBACK GameSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK CustomizeRulesDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);

	TCHAR **GetFactionNames()
	{
		return FactionNames;
	}

	GameSettings *GetGameSettings()
	{
		return &gameSettings;
	}

	GAlienCrossfire(GlobalGameSettings *ggs)
	{
		InitGame(ggs); // Do not remove this
	}

	GAlienCrossfire *Clone(SessionInfo *out)
	{
		GAlienCrossfire *ret = (GAlienCrossfire *)out;
		*ret = *this;	

		return ret;
	}
};

#endif