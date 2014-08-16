#ifndef MasterOfOrion2_H
#define MasterOfOrion2_H

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

class GGMasterOfOrion2Settings : public GlobalGameSettings
{
public:
	TCHAR DEFAULT_RUN_COMMAND[MAX_PATH];
	TCHAR SAVE_FOLDER_PATH[MAX_PATH];
	static SearchReplace ConfigReplaceStrings[];
	static int NUM_CONFIG_REPLACE_STRINGS;

	BOOL BringSetSoundToFront();
	HWND RunSetSound();
	SessionInfo *AllocSession();
	SearchReplace *GetConfigReplaceStrings();
	int GetNumConfigReplaceStrings();
	void Install();
	
	GGMasterOfOrion2Settings() 
	{
		this->gameID = L"Master of Orion II"; 
		this->gameDetails = L"MicroProse 1996";
		this->MAX_TEAMS = 8;
		this->NUM_FACTIONS = 13;
		this->RUN_DELAY = 5 * SECONDS;
		this->FS_MOUSE_MODIFIER = 2.0;
		this->GAME_EXE_NAME = L"orion2.exe";
		this->WarnBeforeNewGame = TRUE;
		this->RandomFactionOrder = TRUE;
		this->KillBeforeRunGame = TRUE;
		this->KillBeforeLoadGame = FALSE;
		this->RequestTeamSettings = TRUE;
	}

	static INT_PTR CALLBACK GGDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);

	GGMasterOfOrion2Settings *Clone()
	{
		return new GGMasterOfOrion2Settings(*this);
	}

	GGMasterOfOrion2Settings *Clone(GlobalGameSettings *out)
	{
		GGMasterOfOrion2Settings *ret = (GGMasterOfOrion2Settings *)out;

		*ret = *this;
		return ret;
	}
};

struct GMasterOfOrion2Team : Team
{
	TCHAR ruler[MAX_SETTING];
	int bannerColor;
	long long custom;

	GMasterOfOrion2Team()
	{
		ruler[0] = L'\0';
		bannerColor = -1;
		custom = 0;
	}
	GMasterOfOrion2Team *Clone() 
	{ 
		return new GMasterOfOrion2Team(*this);
	}
};

class GMasterOfOrion2 : public SessionInfo
{
public:
	static TCHAR *FactionNames[];

	static const int NUM_DIFFICULTIES;
	static TCHAR *DifficultyNames[];

	static const int NUM_TOTAL_PLAYERS;
	static TCHAR *TotalPlayersNames[];
	
	static const int NUM_TECH_LEVELS;
	static TCHAR *TechLevelNames[];

	static const int NUM_GALAXY_SIZES;
	static TCHAR *GalaxySizeNames[];

	static const int NUM_GALAXY_AGES;
	static TCHAR *GalaxyAgeNames[];

	static const int NUM_BANNER_COLORS;
	static TCHAR *BannerColorNames[];
	
	struct GMasterOfOrion2Settings : GameSettings
	{
		int saveSlot;
		int difficulty;
		int galaxySize;
		int galaxyAge;
		int techLevel;
		int totalPlayers;
		BOOL tacticalCombat;
		BOOL randomEvents;
		BOOL antaransAttack;
	} gameSettings;

	// Overridden functions
	static INT_PTR CALLBACK TeamSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK GameSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK CustomRaceDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
	static void DisableCustomAttributes(HWND hDialog, int picks);
	static int FindCustomGroup(int attribute);
	static int SetDefaultRaceCustomPicks(HWND hDialog, int faction);
	void InputCustomAttributes(long long custom);
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
	void PreNewGameEvent();
	struct GMasterOfOrion2Team *AllocTeam();
	BOOL ValidateTeamSettings(Team *team, int type, int sIndex, int tIndex);

	TCHAR **GetFactionNames()
	{
		return FactionNames;
	}

	GameSettings *GetGameSettings()
	{
		return &gameSettings;
	}

	GMasterOfOrion2(GlobalGameSettings *ggs)
	{
		InitGame(ggs); // Do not remove this
	}

	GMasterOfOrion2 *Clone(SessionInfo *out)
	{
		GMasterOfOrion2 *ret = (GMasterOfOrion2 *)out;
		*ret = *this;	

		return ret;
	}
};

#endif