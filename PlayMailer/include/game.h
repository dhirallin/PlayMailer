#ifndef GAME_H
#define GAME_H

#include "const.h"
#include "callbacks.h"

class SessionInfo;

extern BOOL (*pGetFolderSelection)(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle);
extern BOOL (*pGetFileSelection)(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle, TCHAR *initialDir, TCHAR *filter);
extern HFONT (*pCreateDialogFont)(TCHAR *name, double size, int weight);
extern HWND hGameSettingsDialog, hGGChildDialog;

class GlobalGameSettings : public PluginCallbacks
{
public:
	TCHAR *gameID;
	TCHAR *gameDetails;
	TCHAR gameFolderPath[MAX_PATH];
	TCHAR saveFolderPath[MAX_PATH];
	TCHAR runCommand[3 * MAX_PATH];
	int MAX_TEAMS;
	int NUM_FACTIONS;
	double FS_MOUSE_MODIFIER;
	double MOUSE_MODIFIER;
	BOOL fullScreen;
	BOOL useDefaultRunCommand;
	BOOL displayGameTip;
	TCHAR *GAME_EXE_NAME;
	static TCHAR *GAME_EXE_NAME_LIST[];
	static int NUM_GAME_EXE_NAMES;
	int RUN_DELAY;
	HMODULE hModule;
	BOOL RandomFactionOrder;	
	BOOL KillBeforeLoadGame;
	BOOL KillBeforeRunGame;
	BOOL runCommandChanged;
	BOOL WarnBeforeNewGame;
	BOOL RequestTeamSettings;
	BOOL fullScreenToggle;
	BOOL runInDOSBox;
	BOOL restoreColorDepth;
	BOOL duplicateFactions;
	//BOOL runAsAdministrator;
	TCHAR *windowText;
	TCHAR *windowClass;
	static SearchReplace ConfigReplaceStrings[];
	static int NUM_CONFIG_REPLACE_STRINGS;

	virtual TCHAR *GetGameExeName();
	virtual	TCHAR **GetGameExeNameList() { return NULL; };
	virtual int GetNumGameExeNames() { return 1; };
	virtual SessionInfo *AllocSession() = 0;
	virtual void FreeSession(SessionInfo *session);
	virtual BOOL IsGameWindow(HWND hwnd);
	static BOOL CALLBACK EnumIsGameWindow(HWND hwnd, LPARAM lParam);
	virtual void KillGame();
	virtual HWND IsGameWindowForeground();
	virtual HWND IsGameWindowTopMost();
	virtual void InitInput(HWND hWnd);
	static BOOL CALLBACK EnumCloseGameWindow(HWND hWnd, LPARAM lParam);	
	virtual HWND FindGameWindow();
	virtual void CloseGameWindow(HWND hWnd);
	virtual void InitDialogGG();
	virtual void ParseDialogGG();
	virtual DLGPROC GetGGDialogProc();
	virtual HWND CreateGGDialog(HWND hParent);
	virtual BOOL ValidateGlobalGameSettings(int type, int gIndex);
	static INT_PTR CALLBACK GGDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
	virtual void CreateRunCommand();
	virtual void LoadSettings(config_setting_t *group);
	virtual void SaveSettings(config_setting_t *group);
	virtual void AssignStatics();
	virtual SearchReplace *GetConfigReplaceStrings() { return NULL; }
	virtual int GetNumConfigReplaceStrings() { return 0; }
	virtual void Install() {}
	virtual void SetMouseModifier(HWND hWnd);

	virtual GlobalGameSettings *Clone() = 0;
	virtual GlobalGameSettings *Clone(GlobalGameSettings *out) = 0;

	GlobalGameSettings()  
	{
		gameFolderPath[0] = L'\0';
		runCommand[0] = L'\0';
		GAME_EXE_NAME = L"";
		runCommandChanged = FALSE;
		FS_MOUSE_MODIFIER = 1;
		MOUSE_MODIFIER = 1;
		WarnBeforeNewGame = FALSE;
		RequestTeamSettings = FALSE;
		fullScreen = TRUE;
		fullScreenToggle = TRUE;
		useDefaultRunCommand = TRUE;
		displayGameTip = TRUE;
		runInDOSBox = TRUE;
		restoreColorDepth = FALSE;
		duplicateFactions = FALSE;
	}
};

struct GameSettings
{
	TCHAR *gameID;
	GlobalGameSettings *ggSettings;
	uint32_t settingsMask;
};

struct Team
{
	uint32_t id;
	uint32_t state;
	uint32_t faction;

	virtual Team *Clone() = 0;
};

class Game : public PluginCallbacks
{
public:
	int NUM_FACTIONS;
	int MAX_TEAMS;

	GameSettings *pGameSettings;
	GlobalGameSettings *ggSettings;
	TCHAR sessionRunCommand[MAX_PATH];
	
	virtual DLGPROC GetTeamSettingsDialogProc() { return NULL; }
	virtual GameSettings *GetGameSettings() { return NULL; };	
	virtual void CreateTeamSettingsDialog();
	virtual BOOL LoadGame() = 0;
	virtual BOOL NewGame() = 0;
	virtual BOOL SaveGame() = 0;
	virtual void GetGameTip(TCHAR *buffer);
	virtual BOOL EnterFullScreen();
	virtual BOOL LeaveFullScreen();
	virtual void InitGameSettingsDialog() = 0;
	virtual BOOL ParseGameSettingsDialog() = 0;
	virtual BOOL ValidateGameSettings(BOOL type, int sIndex) = 0;
	virtual void InitGameSettings() {}
	virtual void SaveGameSettings(config_setting_t *group) = 0;
	virtual void LoadGameSettings(config_setting_t *group) = 0;
	virtual void BuildEmailGameSettings(config_setting_t *group) = 0;
	virtual void ParseEmailGameSettings(config_setting_t *group) = 0;
	virtual void UpdateEmailGameSettings(SessionInfo *in) = 0;
	virtual void SaveTeamSettings(config_setting_t *group, Team *team) {}
	virtual void LoadTeamSettings(config_setting_t *group, Team *team) {}
	virtual void BuildEmailTeamSettings(config_setting_t *group, Team *team) {}
	virtual void ParseEmailTeamSettings(config_setting_t *group, Team *team) {}
	virtual void UpdateEmailTeamSettings(Team *out, Team *in) {}
	virtual BOOL ApplyGameSettings();
	virtual uint32_t GetSettingsChanging();
	virtual void SetGameSettingsMask();
	virtual BOOL CheckGameSettingsChanged(SessionInfo *newSession);
	virtual TCHAR *GetSaveFilePath(TCHAR *path);
	virtual TCHAR *GetSaveFileName(TCHAR *name) = 0;
	virtual TCHAR *GetSaveFolderPath();
	virtual TCHAR **GetFactionNames() = 0;
	virtual void PostSendEvent() {}
	virtual BOOL PreSaveGameEvent();
	virtual BOOL PreNewGameEvent();
	virtual void PostNewGameEvent();
	virtual BOOL PreLoadGameEvent() { return TRUE; };
	virtual void PostLoadGameEvent() {};
	virtual void ToggleFullScreen();
	virtual Team *AllocTeam() { return NULL; }
	virtual void FreeTeam(Team *team);
	virtual BOOL ValidateTeamSettings(Team *team, int type, int sIndex, int tIndex) { return TRUE; };

	Game() {}

	void InitGame(GlobalGameSettings *ggs) 
	{
		ggSettings = ggs;
		pGameSettings = GetGameSettings();
		
		this->MAX_TEAMS = ggSettings->MAX_TEAMS;
		this->NUM_FACTIONS = ggSettings->NUM_FACTIONS;
		this->sessionRunCommand[0] = L'\0';

		InitGameSettings();
		
		pGameSettings->ggSettings = ggs;
		pGameSettings->gameID = ggSettings->gameID;
	}
};

#endif