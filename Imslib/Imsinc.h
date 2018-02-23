/** 	IMSINC.H	Nigel Roberts. 		29 August, 1990
***
***	Private include file for IMSLIB. Contains prototypes of non-exported
***	functions and #defined constants and data structures etc.
***
**/


#define MAX_SETUPS	10
#define MAX_SETUP_KEYS	10

#define SETUP_KEY_FREE		0x00
#define SETUP_KEY_INUSE 	0x01
#define SETUP_KEY_CONFIG	0x02
#define SETUP_KEY_CHANGED	0x03
#define SETUP_KEY_EDIT      0x04
#define SETUP_KEY_LOADED    0x05

/** setup summary structure. **/

typedef struct SetupDetailsTag { 
	char	szSetupName [ MAX_NAME_LENGTH + 1 ];
	char	szDescription [ MAX_DESCRIPTION_LENGTH + 1 ];
	short	nChannels;     /** Channels in the set up **/
	char    szFileSpec [ _MAX_PATH ];
	short   bSetupChanged; /** Flag to say that set up has been modified **/

} SETUPDETAILS;


#define MAX_DEVICES         32  /** Allow for 32 devices **/
#define MAX_MONITOR_MODS    10  /** Each with up to 10 monitor modules **/
#define MAX_MONITOR_CHANS   20  /** Each with up to 20 channels **/


/** Structure used to hold details of the monitor channels within a
*** single device. We hold an array of these in the shared data area.
**/

typedef struct MonDevTag {
	short   nFirstModule;                       
	long    lModuleMap;
	long    lChannelMap [ MAX_MONITOR_MODS ];
} MON_DEV;




/** Functions in IMSINTER **/

short GetIndexToSetup ( char * lpszSetupName , short * nIndex );
short AddChannelToIndexedSetup ( short nSetupIndex, PCHANNELINFO lpciChannel );
short RemoveChannelFromIndexedSetup ( short nSetupIndex, PCHANNELINFO 
								lpciChannel );

short ModifyChannelInIndexedSetup ( short nSetupIndex, PCHANNELINFO 
								lpciChannel );

short GetChannelIndexInIndexedSetup ( short nSetupIndex , PCHANNELINFO 
					lpciChannel , short * pnChanIndex );
					
short AddSetupToList ( char * lpszSetupName );
short RemoveSetupFromList ( short nIndex );

short ModifyIndexedSetupDescription ( short nIndex, char * lpszDescription);

short GetSetupState ( short nIndex, short * nLockCount );
short GetSetupCount ( void );

/** functions is IMSFILE **/

short LoadSetupFile ( char *szFullPath, char *szSetupName );
short WriteSetupToFile ( short nIndex , short nFileHandle );
short GetSummaryFromFile ( short nSetupIndex );
short IsThisChannelAvailable ( short nSetupIndex, short nFrame, short
								  nChannel );

void CreateChannelNumberString ( PCHANNELINFO pciChannel, char* szName );




short GetReply ( short nCommsChannel, short nDev, char * szBuf,	short nSize );





/** Functions in IMSMON.C
**/

short   RequestMonitorChannel ( PCHANNELINFO lpchMon, short nIMLHandle );
short   ReleaseMonitorChannel ( PCHANNELINFO lpchMon, short nIMLHandle );
