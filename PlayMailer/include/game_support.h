#ifndef GAME_SUPPORT_H
#define GAME_SUPPORT_H

#include "PlayMailer.h"

extern BOOL NewGameInstance;
extern HWND hGameSettingsDialog;
extern HWND hGGChildDialog;
extern double MouseModifier;
extern int MouseSpeed;

#define PROGRAM_FAILED				0
#define PROGRAM_ALREADY_RUNNING		1
#define PROGRAM_EXECUTED			2

BOOL _LoadGame(SessionInfo *session, BOOL exportSave);
BOOL _NewGame(SessionInfo *session);
BOOL _SaveGame(SessionInfo *session);
BOOL SuspendResumeGame(BOOL suspend);
void DisplayGameTip(SessionInfo *session);
LRESULT CALLBACK InputProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DisableMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DisableKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void PressStringKeys(TCHAR *name, int length);
int BringProgramToFront(TCHAR *runCommand, TCHAR *startInFolder, GlobalGameSettings *ggs, BOOL (*isProgramWindow)(GlobalGameSettings *ggs, HWND hWnd), void (*initProgramInput)(GlobalGameSettings *ggs, HWND hWnd), int runDelay);
HWND RunProgram(TCHAR *runCommand, TCHAR *startInFolder, GlobalGameSettings *ggs, BOOL (*isProgramWindow)(GlobalGameSettings *ggs, HWND hWnd), int runDelay);
HWND FindProgramWindow(GlobalGameSettings *ggs, BOOL (*isProgramWindow)(GlobalGameSettings *ggs, HWND hWnd));
HWND IsProgramWindowForeground(GlobalGameSettings *ggs, BOOL (*isProgramWindow)(GlobalGameSettings *ggs, HWND hWnd));
BOOL CALLBACK EnumIsProgramWindow(HWND hWnd, LPARAM lParam);
HWND IsGameWindowForeground();
HWND IsGameWindowTopMost();
BOOL IsFGWFullScreen();
BOOL _IsGameWindow(GlobalGameSettings *ggs, HWND hWnd);
void _InitInput(GlobalGameSettings *ggs, HWND hWnd);
BOOL BringGameToFront();
BOOL CheckFullScreen(SessionInfo *session);
HWND RunGame(SessionInfo *session);
BOOL StartSaveFileThread(SessionInfo *session);
BOOL StartWriteFileThread(TCHAR *filePath);
BOOL WaitForWriteFileThread();
DWORD WINAPI WriteFileProc(LPVOID lpParam);
BOOL WaitForWriteFile(TCHAR *filePath);
void AddKeyPress(INPUT *keys, WORD vKey, int *numKeys);
void AddKeyUp(INPUT *keys, WORD vKey, int *numKeys);
void AddKeyDown(INPUT *keys, WORD vKey, int *numKeys);
void AddHotKeyPress(INPUT *keys, WORD vModKey, WORD vKey, int *numKeys);
void PressKeyFast(WORD vKey);
void PressKey(WORD vKey);
void PressKeyDown(WORD vKey, BOOL delay);
void PressKeyUp(WORD vKey, BOOL delay);
void PressHotKey(WORD vModKey, WORD vKey);
void ResetMouse();
void ResetMouseHoriz();
void ResetMouseVert();
void MoveMouse(int horPos, int verPos);
void MoveMouseMod(int horPos, int verPos);
void SetMouseSpeed(int speed);
void SetMousePos(int horPos, int verPos);
void CheckMousePosKeys(int keyPressed);
void SetFixedMouseSpeed(BOOL enable);
void PressLButtonDown();
void PressLButtonUp();
void PressRButtonDown();
void PressRButtonUp();
void PressLButton();
void PressRButton();
void TestMouseSequence();
void DisableInput(BOOL disable);
void RestoreColorDepth();
void SpinUpCDDrive();

TCHAR *GetDOSBoxPath(TCHAR *gameID);
TCHAR *FindDOSBox();
BOOL GetDOSBoxConfPath(TCHAR *outPath);

#endif