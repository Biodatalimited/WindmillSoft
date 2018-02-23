/**
***	ATkit.h                	GENAPPS -  Applications Toolkit
***
***	IMLAPPS-ATK-P-3.02	Andrew Thomson
***
***	This file contains the following :
***
***
***             Toolkit Structure Declarations.
***             Prototypes for Toolkit functions.
***             Manifest constants used by the Toolkit.
***
***	Associated Files :
***
***
***	Version History  :
***
***	ATK-P-1.00      17th Sept 91    First Version
***	ATK-P-1.01	5th  Oct  92	Additional error codes added.
***	ATK-P-2.00	18th Feb  93	Alarm Support Added
***	ATK-P-2.01	1st  Aug  93	Correction to ATKGetAlarmLimits
***					which only worked woith device 0
***					Added TYPE_N ( Monitor definition )
***					Added message for Error 22 in RScipt
***	ATK-P-2.02	15th Oct  93	Add new DDE Service name function.
***	ATK-P-3.00	21st Jan  94    No message-boxes on comms failures.
***					Reads each channel even if comms errs
***	ATK-P-3.01	19th Jan  95	Performs REP only if READ failed
***					received. Removes one last messagebox
***					displayed if REP failed.
***	ATK-P-3.02	15th Apr  96	New implementation of GetMillisecondCount fn
***					in module ATMSEC.c
***
**/

#if defined ( __cplusplus )

extern "C" {
#endif


#define TOOLKIT_VERSION   "IMLAPPS-ATK-P-3.02"




/** Addt Include files containing Dialog Control IDs
**/
#include "ATLodset.h"   /* Load Setup Dialog Control ID's		 */
#include "ATSelect.h"   /* Channel Connections Dialog Control ID's 	 */
#include "ATDDE.h"	/* DDE Service Name Control ID's                 */



#define NOCOMM          /* No Comm's support      */
#define NOKANJI         /* No Kanji support       */


/**	Toolkit Manifest Constants
**/
#define ATK_MAX_PATH_LENGTH	128	/* Setup file path length */
#define ATK_MAX_NAME_LENGTH	  8	/* Setup file name length */


#define ATK_MAX_DEVICES 	 62	/* Maximum number of devices */

/**     The following are used by Common dialogs...
**/
#define ATK_MAX_FILTER_SIZE         128
#define ATK_IMS_FILTER_DESC	    "Hardware Setup (*.IMS)"
#define ATK_IMS_FILTER              "*.ims"
#define ATK_DEF_IMS_EXT             "IMS"
#define ATK_MAX_SETUP_NAME_LEN      9



/**     ERROR CODES/RESOURCE STRINGTABLE ID's
**/
#ifndef OK
#define OK 				0  /* No error                      */
#endif
#define ATK_ERR_USER_ABORTED         2000  /* User Aborted the operation    */
#define ATK_ERR_NO_FILENAME          2001  /* No filename in edit box 	    */
#define ATK_ERR_NO_MEMORY_IN_DLG     2002  /* Unobtainable Setups Info      */
#define ATK_ERR_TOO_MANY_CONNECTIONS 2003  /* Too channels connected        */
#define ATK_ERR_NO_CONNECTIONS       2004  /* No connections selected       */
#define ATK_ERR_INVALID_IML_REPLY    2005  /* REPORT reply invalid          */
#define ATK_ERR_NO_LOCAL_MEMORY      2006  /* Out of Heap Space             */
#define ATK_ERR_COMMON_DIALOG        2007  /* Common dlg Failed             */
#define ATK_ERR_NO_SUITABLE_CHANS    2008  /* No suitable chans to connect  */
#define ATK_ERR_REPLY_TIMEOUT	     2009  /* ATKIMLCommand reply timeout   */
#define ATK_ERR_NO_GLOBAL_MEMORY     2010  /* No global memory		    */
#define DESTROY_RECEIVED             2020

/**     Define the Error Code Ranges, used by different
***     components of the IML system.
**/
#define FIRST_GPIB_ERR                 1
#define LAST_GPIB_ERR                 15
#define FIRST_COMMSLIB_ERR            16
#define LAST_COMMSLIB_ERR             49
#define FIRST_IMSLIB_ERR              50
#define LAST_IMSLIB_ERR               99
#define FIRST_IML_EXEC_ERR	     100
#define LAST_IML_EXEC_ERR            199
#define FIRST_IML_DEFINE_ERR         200
#define LAST_IML_DEFINE_ERR          299
#define FIRST_IML_OTHERS_ERR         300
#define LAST_IML_OTHERS_ERR          999
#define FIRST_APPL_ERR              1000
#define LAST_APPL_ERR               1999
#define FIRST_TOOLKIT_ERR           2000
#define LAST_TOOLKIT_ERR            2999


/**     Manifest Constants used by ATKChanInfo Structure fields.
**/
#define MAX_NAME                15
#define MAX_VALUE               15
#define MAX_LIMIT               15
#define MAX_UNITS               15
#define MAX_STATE		15

#define ALARM_NORMAL		0	/* Three supported Alarm States */
#define ALARM_WARNING		1
#define ALARM_CRITICAL		2

#define ALARM_LOW		4	/* Two supported Alarm Types	*/
#define ALARM_HIGH		8	/* (direction)			*/

/**     Default ATKChanInfo structure field values
**/
#define ATK_DEFAULT_MIN_LIMIT   "0"
#define ATK_DEFAULT_MAX_LIMIT   "100"



/**    Manifest Contants used to describe Channel Types
***    These must be bit fields since types may be combined (OR'ed)
**/
#define TYPE_A  0x0001   	/* Multiplexed Analog Input 	*/
#define TYPE_I  0x0002          /* Value Input Channel          */
#define TYPE_O  0x0004          /* Output Channel               */
#define TYPE_V  0x0008          /* Value Input & Output Channel */
#define TYPE_B  0x0010          /* Bit Digital Input            */
#define TYPE_R  0x0020          /* Bit Digital Output           */
#define TYPE_T  0x0040          /* Bit Digital Input & Output   */
#define TYPE_M  0x0080          /* Multiplexer Control Channel  */
#define TYPE_U  0x0100          /* Unused Dummy Channel         */
#define TYPE_N	0x0200		/* MoNitor Channel		*/

#define TYPE_INPUT  ( TYPE_A | TYPE_I | TYPE_V | TYPE_B | TYPE_T )
#define TYPE_OUTPUT ( TYPE_O | TYPE_V | TYPE_R | TYPE_T )



/**     Miscellanous manfest Constants
**/
#define ATK_DESC_LEN     80    /* Max Len of IMS desc in Connections Dialog */
#define ATK_DLG_DESC_LEN 80    /* Max Len of Dlg desc in Connections Dialog */

#define ATK_READ_CMD_LEN       ( MAX_NAME + 4 )  /* Size of IML Command Buf */
#define ATK_REP_CMD_REPLY_LEN  ( 80 )     	 /* Size of IML Reply Buf   */
#define ATK_READ_CMD_REPLY_LEN ( 80 )		 /* Max READ cmd Reply Len  */
#define ATK_SHOW_CMD_LEN       ( MAX_NAME + 5 )  /* Size of SHOW cmd        */
#define ATK_SHOW_REPLY_LEN     ( 80 )            /* Max Size of SHOW Reply  */
#define ATK_LIMITS_REPLY_LEN   ( 80 )            /* Max Size of LIMITS Reply*/
#define ATK_MON_REPLY_LEN      ( 100 )		 /* Max Size of MON Reply   */


#define ATK_MAX_RETRYS	       1000	  /* No of trys to access IML bus   */
#define ATK_SEND_TIMEOUT       20000	  /* Timeout waiting for commslib   */
#define ATK_REPLY_TIMEOUT      5000	  /* millisec timeout for IML reply */
#define DOS_TICK_INTV	       54.92549	  /* System Timer interval division */


#define ATK_MAX_DDE_SERVICE_NAME ( 80 )   /* Max size of DDE service name   */


/**     Toolkit Structure Declarations
**/


/**     IMS_CONFIG may be passed into LoadIMSSetup for the wFlags argument.
***     If it is LoadIMSSetup opens the setup allowing it to be edited.
***     This should only be done by SetupIML. Normally a wFlags value of 0
***	should be specified.
***
**/
#define IMS_CONFIG      1


/**    The IMSSETUP structure:
***
***    szSetupPath is the file spec of the IMS file
***    szSetupName is the name of the IMS setup
***    nSetupKey   is the reference key used by the IMSLIB
**/
typedef struct IMSSetupTag {

        char    szSetupPath [ ATK_MAX_PATH_LENGTH + 1 ];
        char    szSetupName [ ATK_MAX_NAME_LENGTH + 1 ];
        short   nSetupKey;

} IMSSETUP, *PIMSSETUP;



/**     The ATKCHANINFO structure
***
***     This structure holds the details for one 'Connected' channel.
***     Most toolkit functions accept an array of ATKCHANINFO structs
***		detailing the channels to process.
***
***     szChannel 	-  The Channel Name
***		nDeviceNumber	-  The Channel's Device Number
***     nChannelNumber  -  The 4 digit Channel Number
***     nError          -  Result of last operation
***     szValue         -  The reading or value to write
***     ulTime          -  Time in secs since 01 Jan 1980
***                        when the last reading was taken
***     nMSecs          -  The number of millisecs which when
***                        added to ulTime gives the 'actual' time
***                        of the reading.
***     szUnits         -  The channel units string
***     szMax           -  The maximum value (upper limit) string
***     szMin           -  The minimum value (lower limit) string
***
***		bDigital	-  Value may be one of two states: 0 and 1
***		szOn		-  String representing 1 state
***		szOff		-  String representing 0 state
***
***		nAlarmState	-  ALARM_NORMAL  - Last reading not in alarm
***				   ALARM_WARNING - Last reading in warning alarm
***				   ALARM_CRITICAL- Last reading in critical alarm
***
**/
typedef struct ATKChanInfoTag {

	char    szChannel [ MAX_NAME + 1 ];
	short   nDeviceNumber;
	short   nChannelNumber;
	short   nError;
	char    szValue [ MAX_VALUE + 1 ];
	DWORD   ulTime;
	short   nMSecs;
	char	szUnits [ MAX_UNITS + 1 ];

	char    szMax   [ MAX_LIMIT + 1 ];
	char	szMin	[ MAX_LIMIT + 1 ];

	BOOL	bDigital;
	char	szOn	[ MAX_STATE + 1 ];
	char	szOff	[ MAX_STATE + 1 ];

	short	nAlarmState; /* ALARM_NORMAL | ALARM_WARNING | ALARM_CRITICAL */

} ATKCHANINFO, *PATKCHANINFO;



/**	The ATKCOLORINFO structure
***
***	This structure holds the RGB colour values for
***	the alarm states; currently Normal, Warning and
***	Critical.
***
***	A ATKCOLOURINFO structure may be defaulted by
***	passing it into the toolkit function: 'ATKLoadINIColours'
***	This function reads the colors from the 'WINDMILL.ini' file
***	if it exists, or sets it to default colours if not.
***
***	ATKSaveINIColours may be called to save the colours
***	stored in a ATKColourInfo structure into the 'WINDMILL.ini'
***	file, creating it if it does not exist.
**/
typedef struct ATKColourTag {

	COLORREF	crNormalText;	 /* RGB values when no alarm	   */
	COLORREF	crNormalBk;

	COLORREF	crWarningText;	 /* RGB values when warning alarm  */
	COLORREF	crWarningBk;

	COLORREF	crCriticalText;  /* RGB values when critical alarm */
	COLORREF	crCriticalBk;

} ATKCOLOURINFO, *PATKCOLOURINFO;



/**	ATKMONITORCHAN
***
***	This structure holds details on an IML Monitor channel ( cType == N )
***	The name is simply the channel name from the N attribute.
***
***
**/
typedef struct ATKMonitorChanTag {

	char	lpszMonitorName [ MAX_NAME + 1 ];
	short	nAlarmType; /* ALARM_NORMAL | ALARM_WARNING | ALARM_CRITICAL */

} ATKMONITORCHAN, *LPATKMONITORCHAN;





/**     Function Prototypes
***     The following functions are available to the Application
**/
short ATKLoadIMSSetup ( HANDLE hInst, HWND hWndParent,
					  PIMSSETUP pimsIMS, WORD wFlags );

short ATKMessageBox ( HANDLE hInstance,  HWND hWndParent,  WORD wStringID,
		       			     char *szCaption,  WORD wType );

short ATKSelectChannels ( HANDLE hInst, HWND hWndParent, char *szSetupName,
	   PHANDLE phConnected, short *pnChans, short nMaxChans, WORD wType );

short WINAPI ATKSelectChannelsDialogFn ( HWND hDlg, DWORD iMessage,
						WPARAM wParam, LPARAM lParam );

short ATKReadChannel ( HWND hWndParent, short nIMLHandle,
			 PATKCHANINFO pciRead, BOOL bReadAlarmState );


short ATKReadList ( HWND hWndParent, short nIMLHandle,
			PATKCHANINFO pciReadList, BOOL bReadAlarmStates	);

short ATKReadAlarmList ( HWND hWndParent, short nIMLHandle,
						PATKCHANINFO pciReadList );


short ATKSetWriteList ( HWND hWndParent,
                 short nIMLHandle, PATKCHANINFO pciWriteList, short nChans );

short ATKWriteChannel ( HWND hWndParent, short nIMLHandle,
						 PATKCHANINFO pciWrite );

short ATKWriteList ( HWND hWndParent, short nIMLHandle,
						PATKCHANINFO pciWriteList );

short ATKSetReadList ( HWND hWndParent,
                 short nIMLHandle, PATKCHANINFO pciReadList, short nChans );

short ATKProcessIMLCommand (  HWND hWndParent, short nIMLHandle,
	  char	*szCommand, short nIMLDev,  char * szReply,
			       		short nReplySize, WORD wFlags );

short ATKChannelUnits ( HWND hWndParent, short nIMLHandle,
				PATKCHANINFO pciChanList, short nChans );

short ATKChannelLimits ( HWND hWndParent, short nIMLHandle,
				PATKCHANINFO pciChanList, short nChans );


short ATKSaveApplicationSetup ( HANDLE hInst, HWND hWndParent,
        char *szAppSet, char *szDefExtn, char *szFilterDesc, char *szFilter );


short ATKLoadApplicationSetup ( HANDLE hInst, HWND hWndParent,
        char *szAppSet, char *szDefExtn, char *szFilterDesc, char *szFilter );


short ATKLoadINIColours ( PATKCOLOURINFO pcolColours );

short ATKSaveINIColours ( PATKCOLOURINFO pcolColours );


short ATKBuildMonitorTable ( char *szIMSSetup );


short ATKDestroyMonitorTable ( VOID );


DWORD GetMillisecondCount ( VOID );	/* ( For ATMSEC.c ) */



/**
***     Prototypes for LoadAppSetup and SaveAppSetup
***     These MUSt be written by the application writer
***     when using the ATappset module.
**/
short LoadAppSetup ( char *szAppSet );
short SaveAppSetup ( char *szAppSet );



typedef struct ATKALARMLEVELStag {


	BOOL	bWarningLevel;
	BOOL	bCriticalLevel;

	double	dWarningLevel;
	double	dCriticalLevel;

	short	nWarningType;	/* ALARM_LOW | ALARM_HIGH */
	short	nCriticalType;


} ATKALARMLEVELS, *PATKALARMLEVELS;



short ATKGetChannelAlarmLevels ( HWND hWndParent, short nIMLHandle,
			PATKCHANINFO pciChan, PATKALARMLEVELS palLevels );




short ATKGetDDEServiceName ( HANDLE hInst, HWND hWndParent,
	char *szApplicationName, char *szSetupName, char *szDDEServiceName );

#if defined ( __cplusplus )

}
#endif
