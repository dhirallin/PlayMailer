#ifndef WARLORDS_H
#define WARLORDS_H

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

class GGWarlordsSettings : public GlobalGameSettings
{
public:
	BOOL lastObserveState;
	BOOL lastSoundState;
	static SearchReplace ConfigReplaceStrings[];
	static int NUM_CONFIG_REPLACE_STRINGS;
	static TCHAR *GAME_EXE_NAME_LIST[];
	static int NUM_GAME_EXE_NAMES;

	SessionInfo *AllocSession();
	SearchReplace *GetConfigReplaceStrings();
	int GetNumConfigReplaceStrings();
	TCHAR **GetGameExeNameList();
	int GetNumGameExeNames();

	GGWarlordsSettings() : lastObserveState(FALSE), lastSoundState(FALSE)
	{
		this->gameID = L"Warlords"; 
		this->gameDetails = L"SSG 1990";
		this->MAX_TEAMS = 8;
		this->NUM_FACTIONS = 8;
		this->RUN_DELAY = 6 * SECONDS;
		this->FS_MOUSE_MODIFIER = 2.0;

		this->KillBeforeRunGame = TRUE;
		this->KillBeforeLoadGame = FALSE;
		this->RandomFactionOrder = FALSE;
	}

	static INT_PTR CALLBACK GGDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);

	void LoadSettings(config_setting_t *group);
	void SaveSettings(config_setting_t *group);

	GGWarlordsSettings *Clone()
	{
		return new GGWarlordsSettings(*this);
	}

	GGWarlordsSettings *Clone(GlobalGameSettings *out)
	{
		GGWarlordsSettings *ret = (GGWarlordsSettings *)out;

		*ret = *this;
		return ret;
	}
};

class GWarlords : public SessionInfo
{
public:
	static TCHAR *FactionNames[];

	static const int NUM_DIFFICULTIES;
	static TCHAR *DifficultyNames[];
	// Bit flags
	static const int COMPUTER_ENHANCED;
	static const int INTENSE_COMBAT;
		
	struct GWarlordsSettings : GameSettings
	{
		int saveSlot;
		BOOL observeOff;
		BOOL soundOff;
		int difficulty;
		BOOL enhanced;
		BOOL intenseCombat;
	} gameSettings;

	// Overridden functions
	BOOL LoadGame();
	BOOL NewGame();
	BOOL SaveGame();
	void GetGameTip(TCHAR *buffer);
	void InitGameSettingsDialog();
	BOOL ParseGameSettingsDialog();
	BOOL ValidateGameSettings(BOOL type, int sIndex);
	void InitGameSettings();
	void SaveGameSettings(config_setting_t *group);
	void LoadGameSettings(config_setting_t *group);
	void BuildEmailGameSettings(config_setting_t *group);
	void ParseEmailGameSettings(config_setting_t *group);
	void UpdateEmailGameSettings(SessionInfo *in);
	BOOL ApplyGameSettings();
	uint32_t GetSettingsChanging();
	void SetGameSettingsMask();
	BOOL CheckGameSettingsChanged(SessionInfo *newSession);
	TCHAR *GetSaveFileName(TCHAR *name);

	TCHAR **GetFactionNames()
	{
		return FactionNames;
	}

	GameSettings *GetGameSettings()
	{
		return &gameSettings;
	}

	// Warlords functions
	void ToggleObserveOff();
	void ToggleSoundOff();

	GWarlords(GlobalGameSettings *ggs)
	{
		InitGame(ggs); // Do not remove this
	}

	GWarlords *Clone(SessionInfo *out)
	{
		GWarlords *ret = (GWarlords *)out;
		*ret = *this;
		
		return ret;
	}
};

#endif