#ifndef _PLAYMAILER_H
#define _PLAYMAILER_H

#include "const.h"
#include "resource.h"
#include "game.h"
#include "game_support.h"
#include "callbacks.h"

struct GeneralSettings
{
	TCHAR DOSBoxPath[MAX_PATH];
	BOOL enterSendsMessage;
	BOOL runOnStartup;
	BOOL startMinimized;
	int  windowsTopMost;
	BOOL muteAllChat;
	BOOL muteSessionWideChat;
	BOOL disableSounds;
	BOOL disableChatSound;
	BOOL disableYourTurnSound;
};

#define MAX_PLAYERS				32768
struct Player
{
	uint32_t id;
	TCHAR email[MAX_SETTING];
	TCHAR name[MAX_SETTING];
	int faction;
	int team;
	BOOL teamToken;
	int state;
	int factionSortKey;
	int playerSortKey;
};

#define MAX_SESSIONS			1024
class SessionInfo : public Game
{
public:
	uint32_t sessionID;
	TCHAR sessionName[MAX_SETTING];
	int turnNumber;
	int currentPlayer;
	int numPlayers;
	int numDeletedPlayers;
	int state;
	Player **players;
	LinkedList *teams;
	BOOL recent;
	BOOL actingCurrentPlayer;
	BOOL selectingTeams;
	BOOL enableRelayTeams;

	SessionInfo() 
	{
		sessionID = turnNumber = currentPlayer = numPlayers = numDeletedPlayers 
			= state = recent = actingCurrentPlayer = selectingTeams = enableRelayTeams = 0;

		sessionName[0] = 0;
	}

	virtual SessionInfo *Clone(SessionInfo *out) = 0;
};

typedef struct PlainEmail_t
{
	TCHAR from[MAX_EMAIL_FIELD];
	TCHAR to[MAX_EMAIL_FIELD];
	TCHAR subject[MAX_EMAIL_FIELD];
	TCHAR *message;
} PlainEmail;

typedef struct ChatMessage_t
{
	uint32_t sessionID;
	TCHAR sessionName[MAX_SETTING];
	TCHAR from[MAX_EMAIL_FIELD];
	TCHAR to[MAX_EMAIL_FIELD];
	BOOL broadcast;
	TCHAR *message;
} ChatMessage;

enum SettingsPages 
{
	GENERAL_SETTINGS,
	MAIL_SETTINGS,
	GAME_SETTINGS,
	NOTIFY_SETTINGS
};

typedef struct MailSettings_t
{
	TCHAR name[MAX_SETTING];
	TCHAR email[MAX_SETTING];

	TCHAR inServer[MAX_SETTING];
	uint8_t inMailProtocol;
	uint8_t inSecurityProtocol;
	TCHAR inLogin[MAX_SETTING];
	uint8_t inPassword[1024];
	int inPasswordSize;

	TCHAR outServer[MAX_SETTING];
	uint8_t outSecurityProtocol;
	TCHAR outLogin[MAX_SETTING];
	uint8_t outPassword[1024];
	int outPasswordSize;

	uint32_t checkMailInterval;
} MailSettings;

enum TopMost 
{
	TOPMOST_ALWAYS,
	TOPMOST_GAME_ACTIVE,
};

// Main window control IDs
#define IDC_SETTINGS_BUTTON			100	
#define IDC_NEWGAME_BUTTON			101
#define IDC_HOTKEYS_BUTTON			102
#define IDC_HELP_BUTTON				103
#define IDC_CONTACT_AUTHOR_BUTTON	110
#define	IDC_MUTEALL_CHECK			106
#define	IDC_MUTESW_CHECK			107
#define IDC_MAIN_LV					108
#define IDC_STATUSBAR				109

#define NUM_LVBUTTONS			5
#define MAX_BUTTON_ROWS			10

#define IDC_LVBUTTONS			200
enum LVButtonIDs 
{
	LVBUTTON_LOAD = IDC_LVBUTTONS,
	LVBUTTON_SEND,			
	LVBUTTON_PLAYERS,		
	LVBUTTON_SETTINGS,		
	LVBUTTON_DELETE			
};

typedef struct LVSessionButtons_t
{
	union 
	{
		struct 
		{
			HWND hLoadButton;
			HWND hSendButton;
			HWND hPlayers;
			HWND hSettingsButton;
			HWND hDeleteButton;
		};
		HWND hButtons[5];
	};
} LVSessionButtons;

#define MBBUFFER_SIZE	2048

typedef int WINAPI Wow64DisableWow64FsRedirectionFn(PVOID *OldValue);
typedef int WINAPI Wow64RevertWow64FsRedirectionFn(PVOID OldValue);
void DisableWow64Redirection(BOOL disable);

extern HINSTANCE hInst;
extern DWORD ProcSpeed;
extern GeneralSettings *settings;
extern TCHAR mbBuffer[MBBUFFER_SIZE];
extern TCHAR gameRunCommand[]; 
extern BOOL GameInProgress;
extern TCHAR saveFolderPath[MAX_PATH];
extern HWND hSessionSettingsDialog;
extern LinkedList llSessions;
extern int selectedSession;
extern int HotKeyPressed;
extern TCHAR AppPath[], AppFolderPath[];
extern HOOKPROC hkprcDisableHLInputProc;
extern HINSTANCE hinstDisableHLInputDLL; 

// Session functions
INT_PTR CALLBACK	SessionSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	LoadGameDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	RunGameDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	GameTipDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
SessionInfo *CreateSession(SessionInfo *session);
SessionInfo *CreateSessionFromEmail(SessionInfo *emailSession);
BOOL UpdateSession(SessionInfo *session);
void DeleteSession(int sIndex);
BOOL ValidateSession(SessionInfo *session);
BOOL ValidateCfgSession(SessionInfo *session, int sIndex);
BOOL ValidateEmailSession(SessionInfo *session);
void AddSessionToListView(SessionInfo *session);
SessionInfo *FindSessionByID(uint32_t id);
int FindSessionIndexByID(uint32_t id);
SessionInfo *FindSessionByGameID(TCHAR *gameID);
void SaveSessionList();
void LoadSessionList();
void selectNextSession();
void selectSession(int newIndex);
void UpdateSessionFromEmail(SessionInfo *session, SessionInfo *emailSession);
SessionInfo *AllocSession(TCHAR *gameID);
void FreeSession(SessionInfo *session);
void FreeSessionList();
void CopySession(SessionInfo *out, SessionInfo *in);
BOOL ApplyGlobalNamesToSession(SessionInfo *session, TCHAR *oldEmail, TCHAR *oldName);
void ApplyGlobalNamesToSessionList(TCHAR *oldEmail, TCHAR *oldName);
SessionInfo *ParseEmailSessionText(TCHAR *text, size_t textSize);
int ParseEmailSessionID(TCHAR *text, size_t textSize);
void CheckYourTurn();
BOOL CheckNewGame(SessionInfo *session);
int getRoundNumber(SessionInfo *session);
int getSelectedSaveSlot();
TCHAR *getSelectedGameName();
BOOL isYourTurn(SessionInfo *session);
BOOL ExportSaveFile(SessionInfo *session, BOOL quiet);
BOOL ImportSaveFile(SessionInfo *session);
BOOL WriteSaveFile(const char *fileData, uint32_t fileSize, uint32_t fileID);
void DeleteSaveFile(uint32_t fileID);
BOOL CheckGameSaved(SessionInfo *session, TCHAR *exportSave, TCHAR *exxportSave);
BOOL CheckInternalSaveFileExists(SessionInfo *session);
BOOL LoadGameRequest(SessionInfo *session, BOOL yourTurn);
BOOL RunGameRequest(SessionInfo *session, BOOL yourTurn);
void DeleteSessionFileStore(SessionInfo *session);
TCHAR *GetSessionFileStorePath(SessionInfo *session, TCHAR *path);
TCHAR *GetGGFileStorePath(GlobalGameSettings *ggs, TCHAR *path);

// Player and Team functions
INT_PTR CALLBACK	EditPlayersDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	EditPlayersListViewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	EditPlayersEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	NewPlayerNameDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	PlayerListDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	PlayerListLVProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int FindPlayerByID(SessionInfo *session, int id);
void UpdatePlayerList(SessionInfo *out, SessionInfo *in);
void UpdateTeamSettings(SessionInfo *out, SessionInfo *in);
Player *CreatePlayer(SessionInfo *session);
Player *CreateDeletedPlayer(SessionInfo *session);
void DeleteYourPlayers(SessionInfo *session);
void FreeDeletedPlayers(SessionInfo *session);
Player **AllocPlayers();
void CopyPlayers(SessionInfo *out, SessionInfo *in);
void FreePlayers(SessionInfo *session);
void ClearPlayerFlags(SessionInfo *session, int flags);
void ClearAllPlayerFlags(SessionInfo *session);
void ClearAllTeamFlags(SessionInfo *session);
void SavePlayerHistory();
void LoadPlayerHistory();
BOOL InitPlayerListLV(HWND hWndListView);
void SetCurrentPlayerIcon(HWND hLV, SessionInfo *session);
void FillPlayerEmailCombo(TCHAR *selectedEmail);
void FillPlayerNameCombo(TCHAR *selectedName);
BOOL InitEditPlayersListView(HWND hWndListView);
void AddPlayerToLV(HWND hLV, SessionInfo *session);
void AddPlayerListToLV(HWND hLV, SessionInfo *session);
void ShowPlayerEditControls(SessionInfo *session, int iItem);
void InitPlayerEditControls(SessionInfo *session);
void ParsePlayerEditControls(SessionInfo *session, int playerNum);
BOOL AddPlayer(SessionInfo *session, TCHAR *email, TCHAR *name, int faction);
void DeletePlayer(SessionInfo *session, int index);
void DeletePlayerFromLV(SessionInfo *Session, int index);
void SortPlayerList(SessionInfo *session);
BOOL ValidatePlayerList(SessionInfo *session, int type);
void ChangeYourPlayerName(SessionInfo *session, TCHAR *name);
void RequestPlayerNames(SessionInfo *session);
void SyncPlayerLists(SessionInfo *out, SessionInfo *in, BOOL quiet);
BOOL CheckPlayerListChanged(SessionInfo *session);
void ReportPlayerChange(SessionInfo *session, int playerIndex, Player *newPlayer);
int FindPlayer(SessionInfo *session, TCHAR *email, TCHAR *name, int faction);
int GetLastLocalPlayer(SessionInfo *session);
TCHAR *getYourPlayerName(SessionInfo *session);
int GetNextPlayerIndex(SessionInfo *session);
int GetNextPlayerIndex(SessionInfo *session, int pIndex);
int GetPrevPlayerIndex(SessionInfo *session);
TCHAR *getCurrentPlayerEmail(SessionInfo *session);
TCHAR *getCurrentPlayerName(SessionInfo *session);
int getNumPlayers(SessionInfo *session);
BOOL isYourPlayer(SessionInfo *session, int player);
int CountYourPlayers(SessionInfo *session);
void CreateFactionCombo(HWND parentWnd, SessionInfo *session);
int GetNextTeamPlayer(SessionInfo *session, int team);
int GetPrevTeamPlayer(SessionInfo *session, int team);
int IteratePlayersNext(SessionInfo *session, int index);
int IteratePlayersPrev(SessionInfo *session, int index);
int RollBackCurrentPlayer(SessionInfo *session);
void SetTeamToken(SessionInfo *session, int index);
int AssignRandomFaction(SessionInfo *session);
int AssignTeam(SessionInfo *session, int playerIndex);
void AssignTeams(SessionInfo *session);
int ComparePlayerFactions(const void *x, const void *y);
TCHAR *BuildFactionPlayerList(SessionInfo *session, int faction, TCHAR *playerNames);
TCHAR *BuildTeamList(SessionInfo *session, int team, TCHAR *playerNames);
int GetSmallestFactionSize(SessionInfo *session);
int GetLargestFactionSize(SessionInfo *session);
int GetFactionSize(SessionInfo *session, int faction);
int GetNumFactions(SessionInfo *session);
int getCurrentPlayerFaction(SessionInfo *session);
int getCurrentPlayerTeam(SessionInfo *session);
TCHAR *getCurrentPlayerFactionName(SessionInfo *session);
TCHAR *getFactionName(SessionInfo *session, int i);
TCHAR *getFactionNameFromIndex(SessionInfo *session, int index);
TCHAR *getYourFactionList(SessionInfo *session);
LinkedList *AllocTeams();
void FreeTeams(SessionInfo *session);
void CopyTeams(SessionInfo *out, SessionInfo *in);
void PruneTeams(SessionInfo *session);
void RequestTeamSettings(SessionInfo *session);
void CheckAllPlayerTeams();
Team *FindTeam(SessionInfo *session, int id);
Team *CreateTeam(SessionInfo *session);
void DeleteTeam(SessionInfo *session, Team *team);

// Mail Functions
INT_PTR CALLBACK	SendEmailDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	FetchMailDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	SendStatusUpdateDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	EditMessageDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	ReceiveMessageDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK EditPlainEmailDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
BOOL SendPlainEmail(PlainEmail *msg);
BOOL EditPlainEmail(TCHAR *toName, TCHAR *toAddress);
BOOL checkEmailFormat(TCHAR *email);
void CleanEmailFolder();
int *GetEmailNumbers(TCHAR *filePath);
TCHAR *GetUniqueOutgoingEmailPath(TCHAR *filePath);
BOOL EditChatMessage(int sessionIndex, TCHAR *fromName, BOOL *recipientsMask, TCHAR **recAddresses, TCHAR **recNames);
BOOL *PruneRecipients(SessionInfo *session, BOOL *mask);
void BuildRecipients(SessionInfo *session, BOOL *mask, TCHAR ***recAddresses, TCHAR ***recNames);
void SendEmailYourTurn(SessionInfo *session);
void SendEmailStatusUpdate(SessionInfo *session, BOOL excludeCurrentPlayer, BOOL quiet);
void SendEmailChatMessage(int sIndex, TCHAR **recAddresses, TCHAR **recNames, ChatMessage *chatMsg);
int SendEmail(FILE *email, TCHAR **recipients, BOOL quiet);
TCHAR *BuildNameAddressStr(TCHAR *out, TCHAR *name, TCHAR *address);
TCHAR *BuildNameAddressList(TCHAR *outStr, TCHAR **names, TCHAR **addresses);
TCHAR *BuildEmailText(SessionInfo *session);
TCHAR *BuildChatEmailText(ChatMessage *chatMsg, BOOL broadcast);
int FetchEmails(BOOL quiet);
void ParseEmails();
void ParseEmail(MailMessageW *msg);
void ParseEmailChatMessage(MailMessageW *msg);
BOOL ParseEmailChatMessageText(TCHAR *text, size_t textSize, ChatMessage *chatMsg);
void ParseEmailStatusUpdate(MailMessageW *msg, BOOL yourTurn);
BOOL CreateRecvEmailThread();
void TerminateRecvEmailThread();
BOOL CreateSendEmailThread();
void TerminateSendEmailThread();
DWORD WINAPI RecvEmailThreadProc(LPVOID lpParam);
DWORD WINAPI SendEmailThreadProc(LPVOID lpParam);
void StartOutgoingMailTimer();
BOOL CheckMail();
int CheckOutgoingMail(BOOL quiet);
TCHAR *GetFirstOutgoingEmail(TCHAR *filePath);
int GetIncomingTimerInterval();

// Settings Functions
INT_PTR CALLBACK	SettingsDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	MailSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	GeneralSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	NotifySettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	GGSettingsDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	GGTopLevelDialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);
HWND InitDialogGeneralSettings();
HWND InitDialogMailSettings(MailSettings *ms);
HWND InitDialogGGSettings(LinkedList *llGGSettings);
HWND InitDialogNotifySettings();
BOOL ParseDialogGeneralSettings();
BOOL ParseDialogMailSettings(MailSettings *ms);
BOOL ParseDialogGGSettings(LinkedList *llGGSettings);
BOOL ParseDialogNotifySettings();
BOOL ValidateGlobalGameSettings(GlobalGameSettings *game, int type, int gIndex);
BOOL ValidateGGSettingsList(LinkedList *llGGSettings);
BOOL ValidateGGSettingsListTopLevel();
BOOL ValidateMailSettings(HWND hDialog, MailSettings *ms, BOOL cfgFile);
void UpdateSettings(MailSettings *ms, LinkedList *llGGSettings);
void UpdateGGSettings(LinkedList *inGGList);
BOOL LoadMailSettings();
BOOL LoadGlobalGameSettings();
void SaveMailSettings();
void SaveGlobalGameSettings();
void CopyGGSettingsList(LinkedList *out, LinkedList *in);
HWND CreateGGChildDialog(int gameIndex, HWND hParent);
int ChangeSettingsPage(int settingsPage);
GlobalGameSettings *FindGlobalGameSettings(TCHAR *id);
int FindGGIndexByID(LinkedList *llGGSettings, TCHAR *gameID);

// Other functions
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	MainListViewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK	StatusBarProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	QuitDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
uint32_t GetIPFromName(char *name);
uint8_t GetCPULoad();
uint64_t GetLoopSpeed();
int mod(int a, int b);
void CreateSchTask(TCHAR *taskName, TCHAR *taskPath, TCHAR *taskArgs, int trigger);
//void CreateSchTaskElevated(TCHAR *taskName, TCHAR *taskPath);
void CheckSchTasks();
void RunSchTask();
BOOL IsWow64();
//DWORD GetProcSpeed();
DWORD GetBaseProcSpeed();
DWORD GetProcSpeed();
void SleepC(DWORD benchMilliSeconds);
void LoadGamePlugins();
void FreeGamePlugins();
int WinTmpFile(FILE **tmp);
BOOL AddProgramToStartupRegistry(BOOL minimized);
BOOL RemoveProgramFromStartupRegistry();
BOOL AddProgramToStartup(BOOL minimized);
BOOL RemoveProgramFromStartup();
void SetAppPaths();
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int nCmdShow);
int MessageBoxS(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
INT_PTR DialogBoxParamS(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
void ViewHotkeysHelp();
void ShowPopMenu(HWND hWnd);
int GetWindowX(HWND hWnd);
int GetWindowY(HWND hWnd);
unsigned int BigRand(void);
void redrawListViewItem(int iItem);
void ForceSetForegroundWindow(HWND hWnd);
void SetTopMost(HWND hWnd);
TCHAR *trimWhiteSpace(TCHAR *inputStr);
TCHAR *RemoveWhiteSpace(TCHAR *inStr, TCHAR *outStr);
HFONT CreateFixedFont();
void AssignPluginCallbacks(PluginCallbacks *callbacks);
BOOL GetFileSelection(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle, TCHAR *initialDir, TCHAR *filter);
BOOL GetFolderSelection(HWND hWnd, LPTSTR szBuf, LPCTSTR szTitle);
int removeFolder(TCHAR *dir);
void AddHistoryString(LinkedList *history, TCHAR *newStr, BOOL addToFront);
BOOL InitMainListView(HWND hWndListView);
void ActivateMailer();
void WaitHotKeyRelease(LPARAM lParam);
void CheckHotKeys();
BOOL IsInternetConnected();
int DLUToPixelsX(HWND hDialog, int dluX);
int DLUToPixelsY(HWND hDialog, int dluY);
void MoveDlgItemRelative(HWND hDlg, int nIDDlgItem, int xOffset, int yOffset);
void SetWindowPosDialogRelative(HWND hDialog, HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
void MoveWindowRelative(HWND hWnd, int xOffset, int yOffset);
HFONT CreateDialogFont(TCHAR *name, double size, int weight);
HFONT CreateWindowFont(TCHAR *name, int height, int weight);
BOOL ReplaceLinesInFile(TCHAR *srcPath, TCHAR *destPath, SearchReplace *strings, int numSearches);
BOOL AddLinesToFile(TCHAR *srcPath, TCHAR *destPath, SearchReplace *strings, int numSearches);
int ReplaceSubStrings(TCHAR *dest, size_t destSize, TCHAR *src, TCHAR *searchStr, TCHAR *replaceStr);
BOOL ExecuteCmd(TCHAR *cmd);
BOOL ExecuteCmdEx(TCHAR *cmd, TCHAR *dir, BOOL bWait);
void CentreWindow(HWND hWnd);
void WaitUntilWindowDrawn();
void ShowTaskBar(BOOL show);
void PrintWindowNameAndClass();
LRESULT CALLBACK DisableMainWndProc(int nCode, WPARAM wParam, LPARAM lParam);
BOOL EncryptString(TCHAR *decPass, uint8_t *encPass, int *encPassSize);
BOOL DecryptString(TCHAR *decPass, uint8_t *encPass, int encPassSize);
DWORD GetFileCRC(TCHAR *filePath);

#endif