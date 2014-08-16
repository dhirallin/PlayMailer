#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#ifdef UNICODE

// Convert a null-terminated Wide Character string to a dynamically allocated UTF8 string
LPSTR UTF8_Encode_Dyn( LPCWSTR inStr )
{
    LPSTR outStr;
	int outSize;
	
	outSize = WideCharToMultiByte(CP_UTF8, 0, inStr, -1, NULL, 0, NULL, NULL);
    outStr = (LPSTR)malloc(outSize);
    WideCharToMultiByte(CP_UTF8, 0, inStr, -1, outStr, outSize, NULL, NULL);
    return outStr;
}

// Convert a null-terminated Wide Character string to a statically allocated UTF8 string
LPSTR UTF8_Encode( LPCWSTR inStr, LPSTR outStr, int outSize )
{
    WideCharToMultiByte(CP_UTF8, 0, inStr, -1, outStr, outSize, NULL, NULL);
    return outStr;
}

// Convert a null-terminated UTF8 string to a dynamically allocated Wide Character string
LPWSTR UTF8_Decode_Dyn( LPCSTR inStr )
{
    LPWSTR outStr;
	int outSize;

	outSize = MultiByteToWideChar(CP_UTF8, 0, inStr, -1, NULL, 0);
    outStr = (LPWSTR)malloc(sizeof(WCHAR) * outSize);
    MultiByteToWideChar(CP_UTF8, 0, inStr, -1, outStr, outSize);
    return outStr;
}

// Convert a null-terminated UTF8 string to a statically allocated Wide Character string
LPWSTR UTF8_Decode( LPCSTR inStr, LPWSTR outStr, int outSize )
{
    MultiByteToWideChar(CP_UTF8, 0, inStr, -1, outStr, outSize);
    return outStr;
}

#else

/** These functions included for TCHAR support **/

// Convert a null-terminated ASCII string to a dynamically allocated UTF8 string
LPSTR UTF8_Encode_Dyn( LPCSTR inStr )
{
    LPSTR outStr;
	
    outStr = (LPSTR)malloc(strlen(inStr));
    strcpy(outStr, inStr);
    return outStr;
}

// Convert a null-terminated ASCII string to a statically allocated UTF8 string
LPSTR UTF8_Encode( LPCSTR inStr, LPSTR outStr, int outSize )
{
    strncpy(outStr, inStr, outSize);
    return outStr;
}

// Convert a null-terminated UTF8 string to a dynamically allocated ASCII string
LPSTR UTF8_Decode_Dyn( LPCSTR inStr )
{
    LPSTR outStr;

	inSize = strlen(inStr) + 1;
    outStr = (LPSTR)malloc(inSize);
	
	for(int i = 0; i < inSize; i++)
	{
		if(*inStr < 0x80) outStr[i] = *inStr;
		if(!*inStr) break;
		inStr++;
	}
    return outStr;
}

// Convert a null-terminated UTF8 string to a statically allocated ASCII string
LPSTR UTF8_Decode( LPCSTR inStr, LPSTR outStr, int outSize )
{
	for(int i = 0; i < outSize; i++)
	{
		if(*inStr < 0x80) outStr[i] = *inStr;
		if(!*inStr) break;
		inStr++;
	}
    return outStr;
}

#endif
