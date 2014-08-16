#ifndef ChessTitans_H
#define ChessTitans_H

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

class GGChessTitansSettings : public GlobalGameSettings
{
public:
	TCHAR DEFAULT_RUN_COMMAND[MAX_PATH];
	TCHAR SAVE_FOLDER_PATH[MAX_PATH];
	
	SessionInfo *AllocSession();
	void CloseGameWindow(HWND hWnd);
	BOOL IsGameWindow(HWND hwnd);
	void InitDialogGG();
	void ParseDialogGG();
	void CreateRunCommand();
	BOOL ValidateGlobalGameSettings(int type, int gIndex);
	
	GGChessTitansSettings() 
	{
		TCHAR ProgramFilesPath[MAX_PATH];

		this->gameID = L"Chess Titans"; 
		this->gameDetails = L"Windows Vista/Windows 7 Chess Program";
		this->MAX_TEAMS = 2;
		this->NUM_FACTIONS = 2;
		this->RUN_DELAY = 0;
		this->windowText = L"Chess Titans";
		this->windowClass = L"ChessWindowClass";

		this->runInDOSBox = FALSE;
		this->KillBeforeRunGame = TRUE;
		this->KillBeforeLoadGame = TRUE;
		this->RandomFactionOrder = FALSE;

		if(!GetEnvironmentVariable(L"ProgramW6432", ProgramFilesPath, MAX_PATH))
			GetEnvironmentVariable(L"ProgramFiles", ProgramFilesPath, MAX_PATH);
		swprintf(this->DEFAULT_RUN_COMMAND, MAX_PATH, L"\"%s\\Microsoft Games\\Chess\\Chess.exe\"", ProgramFilesPath);
	}

	static INT_PTR CALLBACK GGDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
	DLGPROC GetGGDialogProc()
	{
		return GGDialogProc;
	}

	GGChessTitansSettings *Clone()
	{
		return new GGChessTitansSettings(*this);
	}

	GGChessTitansSettings *Clone(GlobalGameSettings *out)
	{
		GGChessTitansSettings *ret = (GGChessTitansSettings *)out;

		*ret = *this;
		return ret;
	}
};

class GChessTitans : public SessionInfo
{
public:
	static const int NUM_DIFFICULTIES;

	static TCHAR *FactionNames[];
	
	struct GChessTitansSettings : GameSettings
	{
		int difficulty;
		BOOL exitAfterTurn;
	} gameSettings;

	// Overridden functions
	BOOL LoadGame();
	BOOL NewGame();
	BOOL SaveGame();
	void GetGameTip(TCHAR *buffer);
	BOOL EnterFullScreen();
	BOOL LeaveFullScreen();
	void InitGameSettingsDialog();
	BOOL ParseGameSettingsDialog();
	BOOL ValidateGameSettings(BOOL type, int sIndex);
	void InitGameSettings();
	void SaveGameSettings(config_setting_t *group);
	void LoadGameSettings(config_setting_t *group);
	void BuildEmailGameSettings(config_setting_t *group);
	void ParseEmailGameSettings(config_setting_t *group);
	void UpdateEmailGameSettings(SessionInfo *in);
	TCHAR *GetSaveFileName(TCHAR *name);
	TCHAR *GetSaveFolderPath();
	void PostSendEvent();
	void MakeSaveFolder();

	TCHAR **GetFactionNames()
	{
		return FactionNames;
	}

	GameSettings *GetGameSettings()
	{
		return &gameSettings;
	}

	GChessTitans(GlobalGameSettings *ggs)
	{
		InitGame(ggs); // Do not remove this
		MakeSaveFolder();
	}

	GChessTitans *Clone(SessionInfo *out)
	{
		GChessTitans *ret = (GChessTitans *)out;

		*ret = *this;
		return ret;
	}
};

#endif