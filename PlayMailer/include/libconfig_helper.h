#ifndef _LIBCONFIG_HELPER_H
#define _LIBCONFIG_HELPER_H

#include <Windows.h>
#include "international.h"
#include "libconfig.h"

#define CONFIG_ERROR_MAXSTRING		2048
#define CONFIG_WCHAR_MAXSTRING		260
#define CONFIG_UTF8_MAXSTRING		(CONFIG_WCHAR_MAXSTRING * 4)

extern BOOL ConfigError;
extern TCHAR ConfigErrorString[CONFIG_ERROR_MAXSTRING];

config_setting_t *cfgGetMember(const config_setting_t *setting, const TCHAR *name);
int cfgGetInt(const config_setting_t *setting, const TCHAR *name);
int cfgGetIntD(const config_setting_t *setting, const TCHAR *name, int def);
long long cfgGetInt64(const config_setting_t *setting, const TCHAR *name);
int cfgGetBool(const config_setting_t *setting, const TCHAR *name);
TCHAR *cfgGetString(const config_setting_t *setting, const TCHAR *name, TCHAR *outStr);
TCHAR *cfgGetStringDyn(const config_setting_t *setting, const TCHAR *name);
config_setting_t *cfgSetString(config_setting_t *parent, const TCHAR *name, const TCHAR *value);
config_setting_t *cfgSetInt(config_setting_t *parent, const TCHAR *name, int value);
config_setting_t *cfgSetInt64(config_setting_t *parent, const TCHAR *name, long long value);
config_setting_t *cfgSetBool(config_setting_t *parent, const TCHAR *name, int value);

int cfgWriteFile(config_t *config, const TCHAR *filename);
int cfgReadFile(config_t *config, const TCHAR *filename);
config_setting_t *cfgLookup(const config_t *config, const TCHAR *path);
config_setting_t *cfgSettingAdd(config_setting_t *parent, const TCHAR *name, int type);
config_setting_t *cfgAddString(config_setting_t * setting, const TCHAR * value);
TCHAR *cfgGetStringElem(const config_setting_t * setting, int index, TCHAR *outStr);

#endif