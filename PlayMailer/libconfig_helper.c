#include "libconfig_helper.h"

TCHAR ConfigErrorString[CONFIG_ERROR_MAXSTRING];
BOOL ConfigError = FALSE;

config_setting_t *cfgGetMember(const config_setting_t *setting, const TCHAR *name)
{
	config_setting_t *member;
	char uName[CONFIG_UTF8_MAXSTRING];

	UTF8_Encode(name, uName, CONFIG_UTF8_MAXSTRING);

	if(!(member = config_setting_get_member(setting, uName)))
	{
		wcscpy_s(ConfigErrorString, CONFIG_ERROR_MAXSTRING, name);
		ConfigError = TRUE;
		return NULL;
	}
	return member;
}

int cfgGetBool(const config_setting_t *setting, const TCHAR *name)
{
	int value; 
	char uName[CONFIG_UTF8_MAXSTRING];

	UTF8_Encode(name, uName, CONFIG_UTF8_MAXSTRING);
	
	if(!config_setting_lookup_bool(setting, uName, &value))
	{
		wcscpy_s(ConfigErrorString, CONFIG_ERROR_MAXSTRING, name);
		ConfigError = TRUE;
		return 0;
	}
	return value;
}

int cfgGetIntD(const config_setting_t *setting, const TCHAR *name, int def)
{
	int value; 
	char uName[CONFIG_UTF8_MAXSTRING];

	UTF8_Encode(name, uName, CONFIG_UTF8_MAXSTRING);
	
	if(!config_setting_lookup_int(setting, uName, &value))
		return def;

	return value;
}

int cfgGetInt(const config_setting_t *setting, const TCHAR *name)
{
	int value; 
	char uName[CONFIG_UTF8_MAXSTRING];

	UTF8_Encode(name, uName, CONFIG_UTF8_MAXSTRING);
	
	if(!config_setting_lookup_int(setting, uName, &value))
	{
		wcscpy_s(ConfigErrorString, CONFIG_ERROR_MAXSTRING, name);
		ConfigError = TRUE;
		return 0;
	}
	return value;
}

long long cfgGetInt64(const config_setting_t *setting, const TCHAR *name)
{
	long long value; 
	char uName[CONFIG_UTF8_MAXSTRING];

	UTF8_Encode(name, uName, CONFIG_UTF8_MAXSTRING);
	
	if(!config_setting_lookup_int64(setting, uName, &value))
	{
		wcscpy_s(ConfigErrorString, CONFIG_ERROR_MAXSTRING, name);
		ConfigError = TRUE;
		return 0;
	}
	return value;
}

TCHAR *cfgGetStringDyn(const config_setting_t *setting, const TCHAR *name)
{
	return cfgGetString(setting, name, NULL);
}

TCHAR *cfgGetString(const config_setting_t *setting, const TCHAR *name, TCHAR *outStr)
{
	char *value;
	char uName[CONFIG_UTF8_MAXSTRING];

	UTF8_Encode(name, uName, CONFIG_UTF8_MAXSTRING);

	if(!config_setting_lookup_string(setting, uName, &value))
	{
		wcscpy_s(ConfigErrorString, CONFIG_ERROR_MAXSTRING, name);
		ConfigError = TRUE;
		return NULL;
	}

	if(!outStr)
		return UTF8_Decode_Dyn(value);
	
	return UTF8_Decode(value, outStr, CONFIG_WCHAR_MAXSTRING);
}

config_setting_t *cfgSetBool(config_setting_t *parent, const TCHAR *name, int value)
{
	config_setting_t *setting;
	char uName[CONFIG_UTF8_MAXSTRING];

	UTF8_Encode(name, uName, CONFIG_UTF8_MAXSTRING);

	setting = config_setting_add(parent, uName, CONFIG_TYPE_BOOL);
	config_setting_set_bool(setting, value);

	return setting;
}

config_setting_t *cfgSetInt64(config_setting_t *parent, const TCHAR *name, long long value)
{
	config_setting_t *setting;
	char uName[CONFIG_UTF8_MAXSTRING];

	UTF8_Encode(name, uName, CONFIG_UTF8_MAXSTRING);

	setting = config_setting_add(parent, uName, CONFIG_TYPE_INT64);
	config_setting_set_int64(setting, value);

	return setting;
}

config_setting_t *cfgSetInt(config_setting_t *parent, const TCHAR *name, int value)
{
	config_setting_t *setting;
	char uName[CONFIG_UTF8_MAXSTRING];

	UTF8_Encode(name, uName, CONFIG_UTF8_MAXSTRING);

	setting = config_setting_add(parent, uName, CONFIG_TYPE_INT);
	config_setting_set_int(setting, value);

	return setting;
}

config_setting_t *cfgSetString(config_setting_t *parent, const TCHAR *name, const TCHAR *value)
{
	config_setting_t *setting;
	char *cfgStr;
	char uName[CONFIG_UTF8_MAXSTRING];

	UTF8_Encode(name, uName, CONFIG_UTF8_MAXSTRING);

	setting = config_setting_add(parent, uName, CONFIG_TYPE_STRING);

	cfgStr = UTF8_Encode_Dyn(value);
	config_setting_set_string(setting, cfgStr);
	free(cfgStr);

	return setting;
}

int cfgReadFile(config_t *config, const TCHAR *filename)
{
	char uFileName[MAX_PATH];

	UTF8_Encode(filename, uFileName, MAX_PATH);

	return config_read_file(config, uFileName);
}

int cfgWriteFile(config_t *config, const TCHAR *filename)
{
	char uFileName[MAX_PATH];

	UTF8_Encode(filename, uFileName, MAX_PATH);
	return config_write_file(config, uFileName);
}

config_setting_t *cfgLookup(const config_t *config, const TCHAR *path)
{
	char uPath[MAX_PATH];

	UTF8_Encode(path, uPath, MAX_PATH);
	return config_lookup(config, uPath);
}

config_setting_t *cfgSettingAdd(config_setting_t *parent, const TCHAR *name, int type)
{
	char uName[CONFIG_UTF8_MAXSTRING];

	if(name) UTF8_Encode(name, uName, CONFIG_UTF8_MAXSTRING);
	return config_setting_add(parent, uName, type);
}

config_setting_t *cfgAddString(config_setting_t * setting, const TCHAR * value)
{
	char cfgStr[CONFIG_UTF8_MAXSTRING];

	return config_setting_set_string_elem(setting, -1, UTF8_Encode(value, cfgStr, CONFIG_UTF8_MAXSTRING));
}

TCHAR *cfgGetStringElem(const config_setting_t * setting, int index, TCHAR *outStr)
{
	const char *value;

	value = config_setting_get_string_elem(setting, index);
	return UTF8_Decode(value, outStr, CONFIG_WCHAR_MAXSTRING);
}


