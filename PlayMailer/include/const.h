#ifndef CONST_H
#define CONST_H

extern "C"
{
#include "libconfig.h"
#include "international.h"
#include "libconfig_helper.h"
#include "libetpan_helper.h"
#include "compose_msg.h"
#include "imap.h"
#include "load_msg.h"
#include "data_structures.h"
}

#define APP_TITLE				L"PlayMailer"
#define APP_VERSION				L"1.2b12"
#define AUTHOR_NAME				L"Dhiraj Pallin"
#define AUTHOR_EMAIL			L"raj@playmailer.net"

#define PLAYMAILER_TAG			1029734 // Just some number used to identify mouse and keyboard messages sent by PlayMailer

#define BENCH_CLOCK				2793	// The clock speed of the machine PlayMailer was written on :)
#define BENCH_CPULOAD			83818350 // The number of for loop cycles completed in 1 second on the machine PlayMailer was written on.

// Hotkeys
#define HOTKEY_MODIFIER				(MOD_SHIFT | MOD_CONTROL)
#define HOTKEY_MODIFIER_STRING		L"Ctrl+Shift+"

// Debug hotkeys
#define DEBUG_HOTKEY_MODIFIER				(MOD_SHIFT | MOD_CONTROL | MOD_WIN)
#define DEBUG_HOTKEY_MODIFIER_STRING		L"Shift+Ctrl+Windows+"

// Sounds
#define SOUND_ALERT				L"DINGLING_WAVE"
#define SOUND_MESSAGE			L"HELLO_WAVE"

#define SESSIONS_CONFIG_FILE	L"sessions.cfg" 
#define MAIL_CONFIG_FILE		L"mail.cfg"
#define GAMES_CONFIG_FILE		L"games.cfg"
#define PLAYER_HISTORY_FILE		L"player.history"
#define DOSBOX_CONFIG_FILE		L"dosbox.conf"

#define INCOMING_EMAIL_FOLDER	L"incoming"
#define OUTGOING_EMAIL_FOLDER	L"outgoing"

#define INTERNAL_SAVE_FOLDER	L"save files"
#define FILE_STORE_FOLDER		L"file store"
#define DOSBOX_CONFIGS_FOLDER	L"DOSBox configs"

#define PLAYMAILER_FROM_NAME	L"PlayMailer"

#define SUBJECT_YOUR_TURN		L"Your turn"
#define SUBJECT_CHAT_MESSAGE	L"Chat message"
#define SUBJECT_STATUS_UPDATE	L"Status update"

#define EMAIL_COMMENT_HEADER	L"## This is an internal PlayMailer email. DO NOT DELETE. ##\n"

// Fonts
#define FONT_SMALL				TEXT("Arial")
#define FONT_SIZE_SMALL			7.5 
#define FONT_HEIGHT_SMALL		-12
#define FONT_NORMAL				TEXT("MS Shell Dlg")
#define FONT_SIZE_NORMAL		8	
#define FONT_HEIGHT_NORMAL		-13
#define FONT_LARGE				TEXT("Arial")
#define FONT_SIZE_LARGE			9
#define FONT_HEIGHT_LARGE		-15
#define FONT_EXTRALARGE			TEXT("MS Shell Dlg")
#define FONT_SIZE_EXTRALARGE	10	
#define FONT_HEIGHT_EXTRALARGE	-17

// Main window GUI attributes
#define MAX_LOADSTRING			100				// Max length of window title and window class name
#define MAINLV_WIDTH			1075
#define MAINLV_HEIGHT			260
#define STATUSBAR_HEIGHT		22
#define WINDOW_CLIENT_WIDTH		(MAINLV_WIDTH + 21)
#define WINDOW_CLIENT_HEIGHT	(MAINLV_HEIGHT + STATUSBAR_HEIGHT + 60)
#define SCROLLBAR_WIDTH			20
#define MAINLV_ROW_HEIGHT		22
#define LVBUTTON_WIDTH			60
#define LVBUTTON_HEIGHT			20
#define LVBUTTON_XGAP			63

// Session Settings Dialog attributes
#define PLAYERNAME_EDIT_HEIGHT	22
#define GAME_SETTINGS_PANE_X	6
#define GAME_SETTINGS_PANE_Y	100

// Settings Dialog Attributes
#define SETTINGS_PAGE_X				95
#define GG_SETTINGS_CHILD_PANE_X	8
#define GG_SETTINGS_CHILD_PANE_Y	65

// Edit Players Dialog Attributes
#define EDITPLAYERS_ROW_HEIGHT	20

// Timer defines
#define SECONDS							1000
#define MINUTES							60 * SECONDS
#define GAME_IN_PROGRESS_TIMER			1
#define GAME_IN_PROGRESS_INTERVAL		30 * MINUTES
#define INCOMING_MAIL_TIMER				2
#define IMAP_POLLING_INTERVAL			30 * SECONDS
#define POP3_POLLING_INTERVAL			60 * SECONDS
#define LONG_POLLING_INTERVAL			10 * MINUTES
#define OUTGOING_MAIL_TIMER				3
#define OUTGOING_TIMER_INTERVAL			30 * SECONDS
#define LONG_OUTGOING_TIMER_INTERVAL	10 * MINUTES	
#define SENDEMAIL_DIALOG_TIMER			4
#define SENDEMAIL_DIALOG_INTERVAL		4 * SECONDS
#define CHECK_INTERNET_TIMER			5
#define CHECK_INTERNET_INTERVAL			5 * SECONDS
#define IMAP_IDLE_INTERVAL				5 * MINUTES // 29 minutes according to IMAP RFC -- set to 5 for dumb servers
#define CHECK_TOPMOST_TIMER				6
#define CHECK_TOPMOST_INTERVAL			1 * SECONDS
#define DISABLE_INPUT_TIMER				7
#define DISABLE_INPUT_INTERVAL			500

#define MAX_FAILED_FETCHES		5
#define MAX_FAILED_SENDS		5

#define SMTP_PORT				587
#define SMTP_SSL_PORT			465
#define IMAP_PORT				143
#define IMAP_SSL_PORT			993
#define POP3_PORT				110
#define POP3_SSL_PORT			995

#define NUM_MAIL_PROTOCOLS	2
enum MailProtocol 
{
	PROTOCOL_IMAP,
	PROTOCOL_POP3
};

#define NUM_SECURITY_PROTOCOLS	3
enum SecurityProtocol 
{
	SECURITY_SSL,
	SECURITY_TLS,
	SECURITY_NONE
};

#define DEFAULT_IN_MAIL_PROTOCOL			PROTOCOL_IMAP
#define DEFAULT_IN_SECURITY_PROTOCOL		SECURITY_SSL
#define DEFAULT_OUT_SECURITY_PROTOCOL		SECURITY_TLS

enum ValidationType 
{
	VALIDATE_GUI,
	VALIDATE_FILE,
	VALIDATE_EMAIL,
	VALIDATE_QUIET
};

#define MAX_SETTING				CONFIG_WCHAR_MAXSTRING
#define MAX_SHORTSETTING		27
#define MAX_EMAIL_FIELD			(MAX_SETTING * 3)

#define NUM_RECVEMAIL_EVENTS 4
enum RecvEmailEvents
{
	EVENT_RECVEMAIL_IMAPSOCKET,
	EVENT_RECVEMAIL_CHECK,
	EVENT_RECVEMAIL_LOGOUT,
	EVENT_RECVEMAIL_QUIT
};

#define NUM_SENDEMAIL_EVENTS 2
enum SendEmailEvents
{
	EVENT_SENDEMAIL_CHECK,
	EVENT_SENDEMAIL_QUIT
};

enum SCHTASK_TRIGGERS
{
	SCHTASK_ONRUN,
	SCHTASK_ONLOGON
};

enum CustomWindowMessages 
{
	WM_PARSE_MAIL = WM_USER,
	WM_BRING_TO_FRONT
};

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

// Virtual Keys
enum VirtualKeyCodes 
{
	VK_A = 0x41, VK_B, VK_C, VK_D, VK_E, VK_F, VK_G, VK_H, VK_I, VK_J, VK_K, VK_L, 
	VK_M, VK_N, VK_O, VK_P, VK_Q, VK_R, VK_S, VK_T, VK_U, VK_V, VK_W, VK_X, VK_Y, VK_Z,
};

enum NumberKeyCodes
{
	VK_0 = 0x30, VK_1, VK_2, VK_3, VK_4, VK_5, VK_6, VK_7, VK_8, VK_9 
};

#define VK_SPACE			0x20
#define VK_MINUS			0xBD

#define SET_BIT(X, Y) ((X) |= (1 << (Y)))
#define CLEAR_BIT(X, Y) ((X) &= ~(1 << (Y)))
#define CHECK_BIT(X, Y) (!!((X) & (1 << (Y)))) 
#define TOGGLE_BIT(X, Y) ((X) ^= (1 << (Y)))

// Session state flags
#define SESSION_MODIFY_CURRENTPLAYER	1
#define SESSION_MODIFY_TURNNUMBER		2
#define SESSION_MODIFY_GAMESETTINGS		4
#define SESSION_MODIFY_TEAMSETTINGS		8
#define SESSION_MODIFY_PLAYERLIST		16
#define SESSION_DELETED					32

#define MAX_PLAYER_HISTORY		15

#define TOTAL_PLAYERS(session)	((session)->numPlayers + (session)->numDeletedPlayers) 

// Player state flags
#define PLAYER_ADD				1
#define PLAYER_DELETE			2
#define PLAYER_MODIFY_EMAIL		4
#define PLAYER_MODIFY_NAME		8
#define PLAYER_MODIFY_FACTION	16
#define PLAYER_MODIFY_STATE		32
#define PLAYER_CURRENT			64
#define PLAYER_TEAM_TOKEN		128

// Team state flags
#define TEAM_MODIFY				1

// Special player factions
#define FACTION_RANDOM			-2
#define FACTION_PLAYERS_CHOICE	-1

typedef struct SearchReplace_t
{
	TCHAR *search;
	TCHAR *replace;
} SearchReplace;

#endif

