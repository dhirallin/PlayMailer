#ifndef CALLBACKS_H
#define CALLBACKS_H

struct GeneralSettings;
struct Player;
class SessionInfo;
struct Team;

struct PluginCallbacks
{
	HWND *PTR_hSessionSettingsDialog;
	HWND *PTR_hGameSettingsDialog;
	HWND *PTR_hGGChildDialog;
	BOOL *PTR_NewGameInstance;
	HWND *PTR_hMainWnd;
	GeneralSettings **PTR_settings;
	int EDITPLAYERS_BUTTON;
	double *PTR_MouseModifier;
	int *PTR_MouseSpeed;

	void (*SaveGlobalGameSettings)();
	void (*SaveSessionList)();
	config_setting_t * (*cfgSetInt)(config_setting_t *parent, const TCHAR *name, int value);
	int (*cfgGetInt)(const config_setting_t *setting, const TCHAR *name);
	int (*cfgGetBool)(const config_setting_t *setting, const TCHAR *name);
	config_setting_t * (*cfgSetBool)(config_setting_t *parent, const TCHAR *name, int value);
	TCHAR * (*cfgGetString)(const config_setting_t *setting, const TCHAR *name, TCHAR *outStr);
	config_setting_t * (*cfgSetString)(config_setting_t *parent, const TCHAR *name, const TCHAR *value);
	config_setting_t * (*cfgSetInt64)(config_setting_t *parent, const TCHAR *name, long long value);
	long long (*cfgGetInt64)(const config_setting_t *setting, const TCHAR *name);
	int (*DLUToPixelsX)(HWND hDialog, int dluX);
	int (*DLUToPixelsY)(HWND hDialog, int dluY);
	void (*PressStringKeys)(TCHAR *name, int length);
	BOOL (*WaitForSaveFile)(SessionInfo *session);
	BOOL (*StartSaveFileThread)(SessionInfo *session);
	BOOL (*WaitForSaveFileThread)();
	BOOL (*ExportSaveFile)(SessionInfo *session, BOOL quiet);
	BOOL (*ImportSaveFile)(SessionInfo *session);
	void (*PressKey)(WORD vKey);
	void (*PressHotKey)(WORD vModKey, WORD vKey);
	void (*MoveMouseMod)(int horPos, int verPos);
	void (*MoveMouse)(int horPos, int verPos);
	void (*SetMouseSpeed)(int speed);
	void (*ResetMouse)();
	void (*ResetMouseHoriz)();
	void (*ResetMouseVert)();
	void (*PressLButton)();
	void (*PressLButtonDown)();
	void (*PressLButtonUp)();
	void (*CopyPlayers)(SessionInfo *out, SessionInfo *in);
	void (*FreePlayers)(SessionInfo *session);
	int (*GetNumFactions)(SessionInfo *session);
	TCHAR * (*GetDOSBoxPath)(TCHAR *gameID);
	BOOL (*GetDOSBoxConfPath)(TCHAR *outPath);
	BOOL (*ReplaceLinesInFile)(TCHAR *srcPath, TCHAR *destPath, SearchReplace *strings, int numSearches);
	Player ** (*AllocPlayers)();
	BOOL (*GetFolderSelection)(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle);
	BOOL (*GetFileSelection)(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle);
	TCHAR * (*trimWhiteSpace)(TCHAR *inputStr);
	int (*MessageBoxS)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
	INT_PTR (*DialogBoxParamS)(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
	BOOL (*CheckFullScreen)(SessionInfo *session);
	BOOL (*_LoadGame)(SessionInfo *session, BOOL exportSave);
	BOOL (*isYourTurn)(SessionInfo *session);
	int (*GetNextPlayerIndex)(SessionInfo *session, int pIndex);
	int (*mod)(int a, int b);
	void (*LL_FreeAll)(LinkedList_PTR list);
	LinkedList_PTR (*LL_Add)(LinkedList_PTR list, void *item);
	size_t (*LL_Size)(LinkedList_PTR list);
	int (*LL_GetItemIndex)(LinkedList_PTR list, void *item);
	TCHAR * (*getYourPlayerName)(SessionInfo *session);
	int (*getCurrentPlayerTeam)(SessionInfo *session);
	int (*getCurrentPlayerFaction)(SessionInfo *session);
	TCHAR * (*getCurrentPlayerFactionName)(SessionInfo *session);
	LinkedList * (*AllocTeams)();
	void (*FreeTeams)(SessionInfo *session);
	void (*CopyTeams)(SessionInfo *out, SessionInfo *in);
	TCHAR * (*BuildTeamList)(SessionInfo *session, int team, TCHAR *playerNames);
	void (*SetTopMost)(HWND hWnd);
	HFONT (*CreateDialogFont)(TCHAR *name, double size, int weight);
	Team * (*FindTeam)(SessionInfo *session, int id);
	void (*SleepC)(DWORD benchMilliSeconds);
	void (*CreateSchTask)(TCHAR *taskName, TCHAR *taskPath, TCHAR *taskArgs, int trigger);
	void (*PressKeyDown)(WORD vKey, BOOL delay);
	void (*PressKeyUp)(WORD vKey, BOOL delay);
	void (*SpinUpCDDrive)();
};

#endif



