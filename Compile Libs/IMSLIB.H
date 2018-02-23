/**	IMSLIB.H	Nigel Roberts.		29 August, 1990.
***
***	Include file for the Intelligent Microlink Applications Setup Management
***	Library IMSLIB.
***
***	This file contains the prototypes of all the exported functions and
***	all the #defined constants and data structures required by an 
***	application to use the library.
***
**/


#ifdef __cplusplus
extern "C" {
#endif 



//#ifdef IMSLIB

	#define IMSFNPREFIX     _declspec (dllexport)  WINAPI
//#else
 //       #define IMSFNPREFIX     _cdecl
//#endif





#define MAX_DESCRIPTION_LENGTH 	80
#define MAX_NAME_LENGTH		12
#define MAX_SETUP_NAME_LENGTH	9


/** Data structures **/

/** channel information structure **/

typedef struct ChannelInfoTag {
	short   nFrame;
	short   nChannel;
	char    cTypeCode;
	char    szName  [ MAX_NAME_LENGTH + 1 ];
	char    szUnits [ MAX_NAME_LENGTH + 1 ];
	short   bEnabled;
} CHANNELINFO;

typedef CHANNELINFO*  PCHANNELINFO;


/** Setup information structure. **/

typedef struct SetupInfoTag {
	char    szSetupName   [ MAX_NAME_LENGTH + 1 ];
	char    szDescription [ MAX_DESCRIPTION_LENGTH + 1 ];
	char    szFileSpec [ _MAX_PATH ];
} SETUPINFO;

typedef SETUPINFO * PSETUPINFO;






/** Function prototypes. **/

short IMSFNPREFIX LoadIMSLibrary ( HANDLE *phLibrary );

short IMSFNPREFIX FreeIMSLibrary ( void );

short IMSFNPREFIX IMSLoadSetup ( LPSTR lpszSetupPath, LPSTR lpszSetupName,
                                                       short nConfigFlag );
					
short IMSFNPREFIX IMSReleaseSetup ( char * lpszSetupName );

HANDLE IMSFNPREFIX IMSGetLoadedSetupList ( short far * lpnNumSetups );

HANDLE IMSFNPREFIX IMSGetSetupSummary ( LPSTR lpszSetupName, short far *
						lpnChannels );

short IMSFNPREFIX IMSSaveSetup ( LPSTR lpszSetupName, LPSTR lpszPath, short
								nMode );

short IMSFNPREFIX IMSLibVersion ( LPSTR lpszVersion, short nMaxChars );

short IMSFNPREFIX IMSRequestPartitioningMode ( void );

short IMSFNPREFIX IMSReleasePartitioningMode ( void );

short IMSFNPREFIX IMSModifyLoadedSetupList ( LPSTR lpszSetupName, short nOperation );

short IMSFNPREFIX IMSModifySetupSummary ( LPSTR lpszSetupName, PCHANNELINFO
                                            lpciChannel, short nOperation );

short IMSFNPREFIX IMSModifySetupDescription ( LPSTR lpszSetupName, LPSTR
							lpszDescription);

short IMSFNPREFIX IMSGetSetupDescription ( LPSTR lpszSetupName, LPSTR
                                       lpszDescription, short nMaxChars );

short IMSFNPREFIX IMSLoadAndLockSetup ( LPSTR lpszPathName, LPSTR
                                       lpszSetupName, short far *lpnKey );

short IMSFNPREFIX IMSUnlockSetup ( LPSTR lpszSetupName, short nKey );

short IMSFNPREFIX IMSEnableConfiguring ( LPSTR lpszSetupName, short nKey );

short IMSFNPREFIX IMSDisableConfiguring ( LPSTR lpszSetupName, short nKey );

short IMSFNPREFIX IMSFinishedConfiguring ( LPSTR lpszSetupName );

short IMSFNPREFIX IMSGetSetupFileSpec ( LPSTR lpszSetupName, LPSTR
					lpszBuffer, short nMaxLenght );



short IMSFNPREFIX IMSRequestMonitorChannel ( char far * lpszSetupName,
						PCHANNELINFO lpciChannel );

short IMSFNPREFIX IMSReleaseMonitorChannel ( char far * lpszSetupName,
						PCHANNELINFO lpciChannel );


short IMSFNPREFIX IMSClear ( char * cProgramTitle );


#ifndef OK
#define OK	0
#endif


#define IMS_NEW     1
#define IMS_APPEND  2
#define IMS_ADD     3
#define IMS_REMOVE  4
#define IMS_UPDATE  5


#define IMS_PARTITIONING        0x0032
#define IMS_CONFIGURING         0x0033
#define IMS_CHANNEL_IN_USE      0x0034
#define IMS_FILE_NOT_FOUND      0x0035
#define IMS_FILE_FORMAT_ERROR   0x0036
#define IMS_OUT_OF_MEMORY       0x0037
#define IMS_LOAD_ERRORS         0x0038
#define IMS_ALREADY_LOADED      0x0039
#define IMS_SUCCESS             0x003A
#define IMS_BAD_NAME            0x003B
#define IMS_LOCK_UNDERFLOW      0x003C
#define IMS_FILE_EXISTS         0x003D
#define IMS_CANNOT_CREATE       0x003E
#define IMS_DISK_FULL           0x003F
#define IMS_CHANNEL_ERROR       0x0040
#define IMS_NOT_PARTITIONING    0x0041
#define IMS_BAD_OPERATION       0x0042
#define IMS_NOT_CONFIGURING     0x0043
#define IMS_BAD_CHANNEL         0x0044
#define IMS_DUPLICATE_CHANNEL   0x0045
#define IMS_TOO_MANY_SETUPS     0x0046
#define IMS_BAD_KEY             0x0047
#define IMS_SETUP_CHANGED       0x0048
#define IMS_LIB_NOT_LOADED      0x0049
#define IMS_LIB_VERSION_ERROR   0x004A
#define IMS_BUFFER_TOO_SMALL    0x004B

#define IMS_TOO_MANY_MONITORS   0x004C

#ifdef __cplusplus
}
#endif 
