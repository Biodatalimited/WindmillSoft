#define FNPREFIX	_declspec (dllexport) WINAPI

short FNPREFIX ReadModuleIDList ( short nModule, char* cReply );

short	FNPREFIX ReadModuleID ( short nModule, char* cReply );
short	FNPREFIX ReadModuleSoftVersion ( short nModule, char* cReply );
short	FNPREFIX ReadModuleChannel ( short nModule, short nChannel,
												int* ptnReading );
long	FNPREFIX ReadModuleTime ( short nModule );
long	FNPREFIX ReadModuleEpoch ( short nModule );
long	FNPREFIX ReadModuleEpochNumber ( short nModule );
long	FNPREFIX ReadModuleNextTime ( short nModule );
long	FNPREFIX ReadModuleTON ( short nModule );
long	FNPREFIX ReadModuleTOFF ( short nModule );
short	FNPREFIX ReadModuleEpochData ( short nModule, short nEpoch, 
									  short nBytes, char* pcDataArray );

short	FNPREFIX WriteModuleChannel ( short nModule, short nChannel, int nSetting );
short	FNPREFIX WriteModuleChannelRange ( short nModule, short nChannel, int nSetting );

short	FNPREFIX WriteModuleTime ( short nModule, long lTime );
short	FNPREFIX WriteModuleNextTime ( short nModule, long lTime, 
									               long lEpoch );
short	FNPREFIX WriteModuleEpoch ( short nModule, long lEpoch );
short	FNPREFIX WriteModuleTON ( short nModule, long lTON );
short	FNPREFIX WriteModuleTOFF ( short nModule, long lTOFF );
short	FNPREFIX WriteModuleNumber ( short nModule, short nNumber  );
short	FNPREFIX WriteModuleBaudRate ( short nModule, short nBaud  );

short	FNPREFIX  OpenSerialLink ( int nPort, int nBaud );
short	FNPREFIX  CloseSerialLink ( void );
void	FNPREFIX  EnableErrorLog ( char* cFilePath, int nEnable );

long	FNPREFIX  GetLongTime ( void ) ;
void	FNPREFIX LongTimeToString ( long lTime, char* cBuffer );

//Modem Commands
short  FNPREFIX SendATCommand ( char *lpszCommand );
short FNPREFIX	GetModemReply ( short nTimeout );
short FNPREFIX HangUp ( WORD nDelay );
void FNPREFIX SetTimeOut ( long lTimeOut );

#define REPLY_OK 10000
#define REPLY_CONNECT 10001
#define REPLY_BUSY 10002
#define REPLY_NO_CARRIER 10003
#define REPLY_NO_DIALTONE 10004
#define REPLY_NO_ANSWER 10005

