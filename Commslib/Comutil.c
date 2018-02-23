/** COMUTIL.C		IMLAPPS Communications Library.
***
*** COMSLIB-P1		Nigel Roberts.
***
*** This is a module of the communications library. 
*** It is responsible for the handling of the channel numbers. Allocation and
*** checking. etc.
***
*** Associated Files are:-
***	COMMAIN.C  - Main module.
***	COMMBDP.C  - Binary data Packets
*** COMUTIL.C  - Utility routines
*** MSEC.C     - Millisecond Timer
***	COMDEF.H   - #defined constants - library specific
***	COMPROTO.H - Library specific function prototypes
***
***
*** VERSION HISTORY.
***
*** 21 March 1990 	IMLAPPS-COMMSLIB-P1.0 	- Inception.
***
**/


#define NOMINMAX	1	/** to prevent min max redefinition error **/


/** Include Files 
**/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <winbase.h>


#include <io.h>		/** for open and write **/
#include <sys\types.h>	/** for open **/
#include <sys\stat.h>	/** for open **/
#include <fcntl.h>	/** for open **/


/** include files which are specific to this code
**/

#include "comdef.h"	/**library specific #defined constants and structures**/
#include "\Windmill 32\Compile Libs\imlinter.h"	/** error codes and exported function prototypes **/
#include "comproto.h"	/** library specific function prototypes - non ststic**/





/** externally declared variables and structures which are required by this 
*** module.
**/


/** Array of DeviceInfo structs indexed on device number. These hold
*** information on the Devices with which the library can communicate.
**/

extern IMLDEVICE adiDevice [ MAX_DEVICES ];


/** Bit Map handle usage.**/
extern unsigned long lHandleUsage ;



extern BDPLINK aBDP [ MAX_BDP_LINKS ];


extern HANDLE hMutex [MAX_DEVICES];

/********************************************************/
/** Check that the device number belongs to a valid device.
*** Return TRUE if it does. FALSE else
**/

short IsDeviceOK ( short nDevice )

{

	/** check device is in range **/	
	if ( ( nDevice < 0 ) || ( nDevice >= MAX_DEVICES ) ) {
		return ( FALSE );
	}

	/** check device exists **/
	if ( adiDevice [ nDevice ].bExists != TRUE ) {
		return ( FALSE );
	}

	return (TRUE);
}



/*********************************************/
/** 		AssignIMLHandle ()
***
***
*** This function checks which communication channels are in use and 
*** returns in *pnIMLHandle the first free one..
***.The handle usage is defined by the bits of lHandleUsage.
*** It returns either TRUE or FALSE if it fails
**/

short AssignIMLHandle ( short * pnIMLHandle )

{

	short   nHandle;
	unsigned long lTemplate = 1;

	/** Check through lHandleUsage to see if one is not inuse.
	*** If we find one then mark it as inuse, set the defaults and return
	*** its number
	**/
	for ( nHandle = 0; nHandle < MAX_IML_USERS; ++ nHandle   ) {

		/** if this channel is not in use then we can use it 
		**/
		if ( (lHandleUsage & lTemplate) == 0 ) {

			/** set the bit to show in use **/
			lHandleUsage = lHandleUsage | lTemplate;

			/** return the channel number
			**/

			*pnIMLHandle = nHandle;

			return ( TRUE );
		}
		/** shift to test the next bit **/
		lTemplate = lTemplate << 1;
	}

	/** No handle available so return false.
	**/

	return ( FALSE );
}
	


/*********************************************/
/** 		IsIMLHandleOK ()
***
***
*** This function takes a channel number and checks lHandleUsage
*** to see if we have allocated this number. 
***
*** We return TRUE if the handle has been allocated
*** FALSE if it is unused or invalid
***
**/

short IsIMLHandleOK ( short nIMLHandle )

{
	unsigned long lTemplate = 1;

	
	/** firstly check to see if the number is in range 
	**/
	if ( ( nIMLHandle >= MAX_IML_USERS ) || ( nIMLHandle < 0 ) ) {
		return ( FALSE );
	}
	
	lTemplate = lTemplate << nIMLHandle;

	/** now check to see if we have issued this number. 
	**/
	if ( (lTemplate & lHandleUsage) != 0 ) {

		/** this channel is in use so we must have allocated it
		*** therefore we return TRUE
		**/
		return ( TRUE );
	}
	/** if we get here then number has not been allocated and therefore is 
	*** a dud so return FALSE
	**/

	return ( FALSE );
}




/*********************************************/
/** 		ReleaseIMLHandle ()
***
***
*** This function checks to see if the channel number has been assigned. If it
*** If it has free it up so it can be reused.
***
*** It returns TRUE if it has released it.
*** FALSE otherwise.
**/

short ReleaseIMLHandle ( short nIMLHandle )

{	unsigned long lTemplate = 1;

	// Check if it is issued
	if ( ! IsIMLHandleOK ( nIMLHandle ) ) {
		return ( FALSE );
	}

	// It is issued so reset it
	lTemplate = lTemplate << nIMLHandle;

	lHandleUsage = lHandleUsage - lTemplate;

	return ( TRUE );
}



/************************************************************
*** Read the IMLINI file to obtain the pathnames of the devices
*** Then load the devices.
*** Return OK or
*** IML_NO_DEVICES if we can't find any
*** IML_FATAL_ERROR if we can't load it
***
*** It is called just once by the first copy of commslib to load.
**/


short GetDevicesFromIMLINI ( void )

{

short nDevices = 0;
short	nDev;
char	*szPath;
char	szCommand [20];
static char	szAppName [ 20 ];
static char szMutex [20];
static char 	szNum [ 10 ];
static char	szProfileString [ MAX_PATH];
static char szINIPath [ MAX_PATH];
PROCESS_INFORMATION  mProcess;
STARTUPINFO	mStart;
HANDLE hTimer;
short nTries;
short nNotReady;


	if ( GetIniPath ( szINIPath, MAX_PATH ) ) {
		return ( IML_FATAL_ERROR );
	}

	for ( nDev = 0; nDev < MAX_DEVICES; nDev ++ ) {

		/** reset the appname **/
		szAppName [ 0 ] = '\0';
		strcpy ( szAppName , "DEVICE" );
		strcat ( szAppName , itoa ( nDev , szNum , 10 ) );

		/** OK look for the type for this device.
		**/

		if ( GetPrivateProfileString ( szAppName,  "TYPE", "",
		                szProfileString , 79, szINIPath ) == 0 ) {

			/** We did not find the type for this device so we
			*** can assume that there is no device with this
			*** number so set structs to indicate no microlink
			*** and continue.
			**/

			adiDevice [ nDev ].bExists = FALSE;
			continue;
		}

		/** OK we need to check the type is WINAPP etc. If it is we
		*** need to get the interpreter path in the exe key name.
		**/

		if ( strnicmp ( szProfileString, "WINAPP", 6 ) == 0 ) {

			/** This is a WINAPP type so get the path and store
			*** it in the device structure.
			**/

			if ( GetPrivateProfileString ( szAppName, "EXE",  "",
			                    szProfileString, MAX_PATH, szINIPath ) == 0 ) {

				/** We did not find the exe for this device
				*** so we can assume that there is no device
				*** with this number so set structs to
				*** indicate no microlink and continue.
				**/

				adiDevice [ nDev ].bExists = FALSE;
				continue;
			}

			/** Strip any leading spaces from the path in the
			*** profile string.
			**/

			szPath = szProfileString;

			while ( *szPath == ' ' ) {
				szPath ++;
			}

					
			/** Need to WinExec this one. Create the command line needed
			*** to start the app. This contains a string to show that
			*** the command comes from commslib not a tinkering user.
			*** Thus  "dir\Myapp COMMSLIB 0"  or similar
			**/
			
			strcat ( szPath, " COMMSLIB " );
			strcat ( szPath, itoa ( nDev, szNum, 10 ));

			strcpy ( szCommand, "COMMSLIB ");
			strcat ( szCommand, itoa ( nDev, szNum, 10 ));


			adiDevice [ nDev ].bExists = TRUE;
			/** Not registered yet **/
			adiDevice [ nDev ].hWndInterpreter = NULL;

			mStart.cb = sizeof(mStart);
			mStart.lpReserved = NULL;
			mStart.lpDesktop = NULL;
			mStart.lpTitle = NULL;
			mStart.dwFlags = 0;
			mStart.cbReserved2 = 0;
			mStart.lpReserved2 = NULL;

			if ( CreateProcess( NULL, szPath, NULL, NULL, FALSE, 0,NULL,NULL,&mStart, &mProcess) == 0) {
				
				return ( IML_FATAL_ERROR);
			}


			nDevices ++;
		}
	}

	if ( nDevices == 0 ) {
		return ( IML_NO_DEVICES );
	}

	/** Create a Semaphore. We will use this to wait for the apps to register **/
	hTimer = CreateSemaphore ( NULL, 0, 2, "Waiter" );

	/** Have 10 tries at 500 msec intervals **/
	for ( nTries = 0; nTries < 10; nTries++ ) {

		/** Wait for it to go signalled. It never does **/
		WaitForSingleObject ( hTimer, 500 );

		/** Does it exist but not yet registered **/
		nNotReady = 0;
		for ( nDev = 0; nDev < MAX_DEVICES; nDev ++ ) {
			if ( adiDevice [nDev].bExists ) {
				if ( adiDevice [nDev].hWndInterpreter == NULL ) {
					++nNotReady;
				}
			}
		}

		/** All registered **/
		if ( nNotReady == 0 ) {
			break;
		}

	}


	/** Kill it now **/
	CloseHandle ( hTimer );


	return ( OK );
}


/*********************************************************
*** This routine attempts to gain access to the passed in Device. It
*** will wait for up to 5 seconds for the device to become
*** available. This is done via a mutex with a defined name. 
*** The routine returns TRUE is access is gained.
**/


short ClaimDevice ( short nIMLHandle, short nDevice )

{
	
	char    szMutexName [20];
	char    szNum[20];
	DWORD   dwRet;



	/** If we have not got a mutex for this device **/
	if ( hMutex [ nDevice ] == NULL ) {	
		/** Prepare the mutex name **/	
		strcpy ( szMutexName , "IMLDEVICE" );
		strcat ( szMutexName , itoa ( nDevice , szNum , 10 ) );
	
		/** Get a handle to the mutex **/
		hMutex [nDevice] = CreateMutex ( NULL, FALSE, szMutexName );
	}



	/** Wait for the mutex to be free **/
	dwRet = WaitForSingleObject ( hMutex [nDevice], 25 );

	switch ( dwRet ) {

	case WAIT_OBJECT_0 :		
	case WAIT_ABANDONED :
		/** OK we have the device **/
		adiDevice [nDevice].nInUseBy = nIMLHandle;
		return (TRUE);
	case WAIT_TIMEOUT :
	case WAIT_FAILED :
		return (FALSE);
	default :
		return (FALSE);
	}


	return ( FALSE );
}


/****************************************************
***
**/

short FreeDevice ( short nIMLHandle, short nDevice )

{
	
	
	/** if we were in use tidy up **/
	if ( adiDevice [ nDevice ].nInUseBy == nIMLHandle ) {

		adiDevice [ nDevice ].wReplyType = IML_NO_REPLY;
		adiDevice [ nDevice ].bReplyReceived = FALSE;
		adiDevice [ nDevice ].nInUseBy = DEVICE_FREE;
	}

	ReleaseMutex ( hMutex [nDevice] );

	return ( TRUE);


}


/******************************************************/
/** Read the path for iml.ini from the registry.
*** The returned string is the full path including the filename.
*** The dwSize argument is the length of the supplied buffer.
*** Return OK or an error.
**/ 

short GetIniPath ( char* cFilePath, DWORD dwSize ) 

{
HKEY  myKey;
char * szSubKey = "SOFTWARE\\WINDMILL SOFTWARE\\WINDMILL";
DWORD dwType;
DWORD dwLength;

	dwLength = dwSize;

	/** Read the install dir from the registry. That's where IML.INI is **/
	if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_QUERY_VALUE, &myKey)) {
		return ( 1 );
	}
	RegQueryValueEx ( myKey, "INSTALLDIR", NULL, &dwType, cFilePath, &dwLength );
	RegCloseKey ( myKey );

	if ( (strlen(cFilePath) + 10) > dwSize ) {
		return ( 1 );
	}

	strcat ( cFilePath, "iml.ini" );

	return (OK);

}