#ifndef _INTERNATIONAL_H
#define _INTERNATIONAL_H

#ifdef UNICODE
LPSTR UTF8_Encode_Dyn( LPCWSTR inStr );
LPSTR UTF8_Encode( LPCWSTR inStr, LPSTR outStr, int outSize );
LPWSTR UTF8_Decode_Dyn( LPCSTR inStr );
LPWSTR UTF8_Decode( LPCSTR inStr, LPWSTR outStr, int outSize );
#else
/** These functions included for TCHAR support **/
LPSTR UTF8_Encode_Dyn( LPCSTR inStr );
LPSTR UTF8_Encode( LPCSTR inStr, LPSTR outStr, int outSize );
LPSTR UTF8_Decode_Dyn( LPCSTR inStr );
LPSTR UTF8_Decode( LPCSTR inStr, LPSTR outStr, int outSize );
#endif 

#endif
