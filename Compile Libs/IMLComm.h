
#define HANDSHAKE_NONE          0
#define HANDSHAKE_XON_XOFF      1
#define HANDSHAKE_HARDWARE      2


/** The functions in Serial.c **/

HANDLE _declspec (dllexport)  WINAPI CommOpen ( PCOMMS pcsComms );

DWORD _declspec (dllexport) WINAPI CommFlush ( HANDLE hComPort );


DWORD _declspec (dllexport)  WINAPI CommWrite ( HANDLE hComPort, 
					char * pChars, DWORD dwSize,DWORD * pdwBytesSent );

DWORD _declspec (dllexport)  WINAPI CommRead ( HANDLE hComPort, 
					char * pChars, DWORD dwSize, DWORD* pdwBytesRead );

HANDLE _declspec (dllexport)  WINAPI CommClose ( HANDLE hComPort );

DWORD _declspec (dllexport)  WINAPI CommLastError ( void );

DWORD _declspec (dllexport)  WINAPI CommState ( HANDLE hComPort,
											   LPCOMSTAT pcsComStat,
											   DWORD*  pdwModemStatus );

DWORD _declspec (dllexport) WINAPI CommSetState ( HANDLE hComPort,
									DWORD bDTR, DWORD bRTS );

DWORD _declspec (dllexport) WINAPI GenerateCRC ( WORD unStartValue, char *szData, WORD unLength );

DWORD _declspec (dllexport)  WINAPI CustomSend ( int nType, char* cMsg, int nMsgSize, char* cValue, char* cOutBuf, void* cSpare);


/** The 3 functions in Aidparse **/
DWORD _declspec (dllexport)  WINAPI DLLParseBuffer ( char* pBuf, DWORD dwBytes, char* pszParse, 
						char* pszReply, short* pnChannel );
DWORD _declspec (dllexport)  WINAPI DLLParseSequence ( int nParseIndex );

DWORD _declspec (dllexport)  WINAPI DLLStringToFloat ( char* pszString, char* cFloat);


void _declspec (dllexport)  WINAPI Network_Initialise ( void );
long _declspec (dllexport) WINAPI Network_Name2IP ( char* cName, char* cIPaddress, long* lSize );
long _declspec (dllexport) WINAPI Network_Socket_Open ( char* cIPaddress, DWORD dwPort );
void _declspec (dllexport)  WINAPI Network_Close ( DWORD nSocket );
long _declspec (dllexport) WINAPI Network_Send ( DWORD nSocket, char* cBuffer, DWORD ulSize ); 
long _declspec (dllexport) WINAPI Network_Reply ( DWORD nSocket, char* cBuffer, long dwSize );
long _declspec (dllexport) WINAPI Network_GetError ( void );



// An impossibly large return value for the find function
#define NOT_FOUND 32000
#define SYNTAX_ERROR 32001
#define END_OF_PARSE 32002

