/**
***	LG_SETUP.c              ATK-LOGGER
***
***     Andrew Thomson
***
***     This module is responsible for Saving and Restoring
***     Logger Setups.
***
***
***	Associated Files : 	
***
***	Version History :
***	
**/
#define  NOMINMAX
#define  NOKANJI
#define  NOCOMM
#define  STRICT
#define  IML_APPLICATION

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "\Windmill 32\Compile Libs\IMLinter.h"   /* ...using the COMMSLIB Dynalink                */
#include "\Windmill 32\Compile Libs\imslib.h"     /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATKit\ATKit.h"      /* Application Toolkit Header/Prototypes         */
#include "lg_def.h"     /* Manifest Constants and Structure Declarations */
#include "lg_prot.h"    /* Functions Prototypes                          */


/**     Externally Declared Structures
**/
extern CLIENTINFO	clntInfo;	/* Client Area  Details		*/
extern CONNECTIONS	conninfo;	/* Channel Connections		*/
extern SETTINGS         settings;	/* Logger Settings              */

extern HINSTANCE	hInst;		/* Program Instance Handle	*/
extern HWND		hwndMain; 	/* Program Window Handle	*/









/**
***   SaveAppSetup
***
***   Saves the details held in the 'settings' structure,
***   the Window height and position and the current connections,
***   detailed in the 'connections' structure array, a setup file.
***
**/
short SaveAppSetup ( char *szAppSetup )
{
	char    szSectionName    [ ] = APPL_SETUP_SECTIONNAME;
	char    szKey  		 [ MAX_KEY_LENGTH + 1 ];
	char    szKeyName        [ MAX_KEY_LENGTH + 1 ];
	WORD    wKeyValue;
	short   nConnection;
	RECT    rectWindow;
	BOOL	bSucceeded;

	double	dPercentageXPos;
	double  dPercentageYPos;
	double	dPercentageWidth;
	double  dPercentageHeight;
	


	/**	Store the filename in which the details will be stored
	***
	**/
	strcpy ( settings.szFileSpec, szAppSetup );




	/**     General Stuff first, thats not implemented in this
	***     version
	**/
	WritePrivateProfileString ( szSectionName,
					"Type", "Logger", szAppSetup );


	/**     Exe Filename
	**/
	GetModuleFileName ( hInst, szKey, MAX_KEY_LENGTH );
	WritePrivateProfileString ( szSectionName,
					"Exe", szKey, szAppSetup );


	/**     Window Size and Position
	***
	***     Parameters are in order of occurence :
	***
	***       X Position of Window ( Top Left )
	***       Y Position of Window ( Top Left )
	***       Width  of Window
	***       Height of Window
	**/
	GetWindowRect ( hwndMain, &rectWindow );
        
	/**	Calculate Positions and Dimensions as 
	***	percentage of current screen dimensions. This will
	***	make the fiqures 'device independant'
	**/
	dPercentageXPos  =  ( rectWindow.left * 100.0 ) / 
				 	GetSystemMetrics ( SM_CXSCREEN );
				 	
	dPercentageYPos  =  ( rectWindow.top * 100.0 ) /
					GetSystemMetrics ( SM_CYSCREEN );
				
	dPercentageWidth = ( ( rectWindow.right  - rectWindow.left ) * 100.0 )
				      / GetSystemMetrics ( SM_CXSCREEN );
	
	dPercentageHeight = ( ( rectWindow.bottom - rectWindow.top ) * 100.0 )
				     / GetSystemMetrics ( SM_CYSCREEN );
				
	sprintf ( szKey, "%5.2f %5.2f %5.2f %5.2f",
	                        dPercentageXPos,
	                        dPercentageYPos,
	                        dPercentageWidth,
	                        dPercentageHeight );

	WritePrivateProfileString ( szSectionName,
					"Window", szKey, szAppSetup );




	/**     IMS Filename. This MUST be obtained from IMSLIB DLL
	***	since the setup may have been initially loaded
	***	from another application or resaved elsewhere
	***	by SetupIML
	**/
	IMSGetSetupFileSpec ( conninfo.imsSetup.szSetupName, szKey, 
						MAX_KEY_LENGTH );
	        					
	WritePrivateProfileString ( szSectionName, "IMSFile",
						szKey, szAppSetup );


         
	/**	User assigned Window Title
	**/         
	WritePrivateProfileString ( szSectionName, "Title", 
			settings.szTitle, szAppSetup );
 
         
         
	/**	DDE Support
	**/
	wKeyValue = (WORD) settings.bDDEEnabled;
	sprintf ( szKey, "%u", wKeyValue );

	/**	..are we allowing DDE support
	**/
	WritePrivateProfileString ( szSectionName, "DDEEnabled",
						szKey, szAppSetup );

	/**	...and the service name to be used for DDE conncetion
	**/
	WritePrivateProfileString ( szSectionName, "DDEServiceName",
				     settings.szDDEServiceName, szAppSetup );




	/**     Sample Interval, stored in millisecs
	**/
	sprintf ( szKey, "%lu", settings.dwScanInterval );

	WritePrivateProfileString ( szSectionName, "Interval",
						szKey, szAppSetup );



	/**     File Format
	**/
	wKeyValue = settings.wFileFormat - FILEFORMAT_BASE;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "FileFormat",
					szKey, szAppSetup );



	/**     Time Stamp Format
	**/
	wKeyValue = settings.wTimeStampFormat - TIMESTAMP_BASE;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "TimeFormat",
					szKey, szAppSetup );


	/**	Display data on Screen ?
	**/
	wKeyValue = ( settings.bScreenDisplayOn ) == 0 ? 0 : 1;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "DisplayOn",
		    			szKey, szAppSetup );


	/**	Indicate Empty Scans ?
	**/
	wKeyValue = ( settings.bIndicateEmptyScans ) == 0 ? 0 : 1;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "LogEmptyScans",
		    			szKey, szAppSetup );


	/**	Digital data as text ?
	**/
	wKeyValue = ( settings.bDigitalAsText ) == 0 ? 0 : 1;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "DigitalAsText",
		    			szKey, szAppSetup );


	/**	Column Width ( Characters )
	**/
	sprintf ( szKey, "%u", settings.wCharsPerColumn );


	WritePrivateProfileString ( szSectionName, "ColumnWidth",
		    			szKey, szAppSetup );




	/**	Log to File ?
	**/
	wKeyValue = ( settings.bLogDataToDisk ) == 0 ? 0 : 1;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "LogToFile",
		    			szKey, szAppSetup );



	/**	Log On Alarm ?
	**/
	wKeyValue = ( settings.bLogOnAlarm ) == 0 ? 0 : 1;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "LogOnAlarm",
		    			szKey, szAppSetup );

	/**	Log On Key Press ?
	**/
	wKeyValue = ( settings.bLogOnKeyPress ) == 0 ? 0 : 1;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "LogOnKeyPress",
		    			szKey, szAppSetup );

	/**	Log On Change ?
	**/
	wKeyValue = ( settings.bLogOnChange ) == 0 ? 0 : 1;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "LogOnChange",
		    			szKey, szAppSetup );

	/**  Log Duration, stored in millisecs
	**/
	sprintf ( szKey, "%lu", settings.dwLogDuration );

	WritePrivateProfileString ( szSectionName, "LogDuration",
						szKey, szAppSetup );


	/**     Data Directory
	**/
	WritePrivateProfileString ( szSectionName, "DataDirectory",
		         	     settings.szDataDirectory, szAppSetup );

	/**     Data File Name
	**/
	WritePrivateProfileString ( szSectionName, "DataFile",
				     settings.szDataFile, szAppSetup );


	/**	Periodic Files ?
	**/
	wKeyValue = ( settings.bPeriodicFiles ) == 0 ? 0 : 1;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "PeriodicFiles",
					szKey, szAppSetup );


	/**	Period Type
	**/
	wKeyValue = settings.wPeriodType - PERIOD_TYPE_BASE;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "PeriodType",
		    			szKey, szAppSetup );


	/**	Postfix Type
	**/
	wKeyValue = settings.wPostFixType - POSTFIX_TYPE_BASE;
	sprintf ( szKey, "%u", wKeyValue );


	WritePrivateProfileString ( szSectionName, "PostFix",
		    			szKey, szAppSetup );



	/**	Finally an entry for each connected channel....
	**/
	for ( nConnection = 0; nConnection < conninfo.nNumConnected;
	                                                nConnection++ ) {

		sprintf ( szKeyName, "Input%d", nConnection + 1 );

		bSucceeded = WritePrivateProfileString ( szSectionName,
			szKeyName,
  			conninfo.pciConnections [ nConnection ].szChannel,
			szAppSetup );


	}
	/**	Give the remain possible inputs channels a NULL key entry.
	***	This will delete unwanted channels already existing in the
	***	file
	**/
	for ( nConnection = conninfo.nNumConnected;
			nConnection < MAX_LOGGER_CONNECTIONS;
		                                   nConnection++ ) {

		sprintf ( szKeyName, "Input%d", nConnection + 1 );

		/**	If param 3 is NULL the line is deleted completely
		***	if it points to NULL only the key value is left blank.
		***	We want to remove the whole line.
		**/
		WritePrivateProfileString ( szSectionName, szKeyName,
							NULL, szAppSetup );


	}




	if ( !bSucceeded ) {
	    ATKMessageBox ( hInst, hwndMain,
			ERR_UNABLE_TO_SAVE_SETUP,
		                  NULL, MB_OK | MB_ICONEXCLAMATION );

	}
	return ( bSucceeded ? OK : ERR_UNABLE_TO_SAVE_SETUP );
}





/**
***	LoadAppSetup
***
***	Loads details from a setup file and stores/handles them
***	as appropriate. This may include resizing and moving the
***	window, internalizing new 'settings' and connecting new
***	channels.
***
***
**/
short LoadAppSetup ( char *szAppSetup )
{

	char            szSectionName    [ ] = APPL_SETUP_SECTIONNAME;
	char            szKey  [ MAX_KEY_LENGTH + 1 ];
	char            szWorkingDir [ _MAX_DIR + 1 ];
	DWORD           dwKeyValue;
	GLOBALHANDLE    ghSummary;
	PCHANNELINFO   lpSummary;
	short           nNumSummaryItems;
	short           nReturn;
	long            lCopied;
	short           nConnection;
	char            szConnection [ MAX_NAME + 1 ];
	char            szKeyName    [ MAX_KEY_LENGTH + 1 ];
	BOOL            bErrReported;
	
	short           nWindowXPos;
	short           nWindowYPos;
	short           nWindowWidth;
	short           nWindowHeight;
	double          dPercentageXPos;
	double          dPercentageYPos;
	double          dPercentageWidth;
	double          dPercentageHeight;

	
	

	/**	Store the filename from whence we got the setup details
	***
	**/
	strcpy ( settings.szFileSpec, szAppSetup );



	/**     IMS Filename
	**/
	GetPrivateProfileString ( szSectionName, "IMSFile", "",
	        conninfo.imsSetup.szSetupPath, _MAX_PATH, szAppSetup );


	/**     Load the Specified Setup File....
	***     Close the existing Setup if this exists
	**/



	/**     Free the connections array
	**/
	if ( conninfo.hConnections != NULL ) {

		LocalUnlock ( conninfo.hConnections );
		LocalFree   ( conninfo.hConnections );

		conninfo.hConnections  = NULL;
		conninfo.nNumConnected = 0;

	}
	if ( conninfo.bIMSSetupLoaded == TRUE ) {

		/**     Free the IMS setup and IMSLib.DLL
		**/
		IMSUnlockSetup ( conninfo.imsSetup.szSetupName,
					conninfo.imsSetup.nSetupKey );

		/**     Reset the vars assoicated with the
		***     old IMS file. Do NOT delete the newly
		***     acquired path name though.
		**/
		conninfo.imsSetup.szSetupName [ 0 ] = '\0';
		conninfo.bIMSSetupLoaded 	   = FALSE;
		conninfo.imsSetup.nSetupKey         = 0;

	}


	/**     Load a new Setup..........
	***     Assume that the setup path and name, if known,
	***     has already been copied into the IMSSETUP
	***     structure. If not known then the strings
	***     should be null terminated..
	**/
	nReturn = ATKLoadIMSSetup ( hInst, hwndMain,
				&( conninfo.imsSetup ), 0 );
	if ( nReturn != OK ) {

		/**     Unable to load the setup, so dont
		***     attempt to make any connections
		**/
		conninfo.hConnections  	= NULL;
		conninfo.nNumConnected 	= 0;
		conninfo.bIMSSetupLoaded = FALSE;
		conninfo.imsSetup.szSetupPath [ 0 ] = '\0';
		conninfo.imsSetup.szSetupName [ 0 ] = '\0';

		/**     The toolkit will have already
		***     reported the error to the user.
		**/
		return ( nReturn );

	} else {
		conninfo.bIMSSetupLoaded = TRUE;
	}



	/**     Request a summary from IMSLib
	**/
	ghSummary = IMSGetSetupSummary (
	               (LPSTR) conninfo.imsSetup.szSetupName,
	                                    &nNumSummaryItems );

	if ( ghSummary == NULL ) {

		/**     The summary could not be obtained, in this
		***     case, nSummaryItems contains an error code
		**/
		return ( nNumSummaryItems );

	}



	/**     Lock the summary array
	**/
	lpSummary = (PCHANNELINFO) GlobalLock ( ghSummary );
	if ( lpSummary == NULL ) {

		/**     No memory...
		**/
		GlobalFree ( ghSummary );

		return ( ERR_NO_LOCAL_MEMORY );

	}


	/**     Next the Window Size and Position...
	***
	**/
	GetPrivateProfileString ( szSectionName, "Window", "0 0 0 0",
				szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%lg %lg %lg %lg",
				&dPercentageXPos,
				&dPercentageYPos,
				&dPercentageWidth,
				&dPercentageHeight );

	/**	Convert Percentage sizes to Physical Units
	***	for the current screen device..
	**/
	nWindowXPos = (short) ( ( ( dPercentageXPos *
			 GetSystemMetrics ( SM_CXSCREEN ) ) / 100.0 ) + 0.5 );
				 
	nWindowYPos = (short) ( ( ( dPercentageYPos *
			 GetSystemMetrics ( SM_CYSCREEN ) ) / 100.0 ) + 0.5 ); 
	
	nWindowWidth = (short) ( ( ( dPercentageWidth *
			 GetSystemMetrics ( SM_CXSCREEN ) ) / 100.0 ) + 0.5 );
				 
	nWindowHeight = (short) ( ( ( dPercentageHeight *
			 GetSystemMetrics ( SM_CYSCREEN ) ) / 100.0 ) + 0.5 );


	if ( ( nWindowHeight != 0 ) && ( nWindowWidth  != 0 ) ) {

		MoveWindow ( hwndMain, nWindowXPos, nWindowYPos,
				nWindowWidth, nWindowHeight, TRUE );

		/**     Invalidate the Window as well, MoveWindow
		***     doesn't have a facility for clearing the
		***     screen before repainting.
		**/
		InvalidateRect ( hwndMain, NULL, TRUE );


	}

         
          
	/**	User define Window Title
	**/ 
	GetPrivateProfileString ( szSectionName,
				    "Title",
				    "Logger",
				    settings.szTitle,
				    MAX_TITLE,
				    szAppSetup );
 

	/**	DDE support
	**/
	settings.bDDEEnabled = (BOOL) GetPrivateProfileInt (
						szSectionName,
						(LPCSTR) "DDEEnabled",
						0,
						(LPCSTR) szAppSetup );


	GetPrivateProfileString ( szSectionName,
				    "DDEServiceName",
				    "Logger",
				    settings.szDDEServiceName,
				    ATK_MAX_DDE_SERVICE_NAME,
				    szAppSetup );




	/**     Sample Interval, stored in millisecs
	**/
	GetPrivateProfileString ( szSectionName, "Interval", "1000",
				szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%lu", &settings.dwScanInterval );



	/**     File Format
	**/
	GetPrivateProfileString ( szSectionName, "FileFormat", "0",
			    szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.wFileFormat = FILEFORMAT_BASE + (WORD)dwKeyValue;



	/**     Time Stamp Format
	**/
	GetPrivateProfileString ( szSectionName, "TimeFormat", "0",
			szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.wTimeStampFormat = TIMESTAMP_BASE + (WORD)dwKeyValue;





	/**	Display data on screen
	**/
	GetPrivateProfileString ( szSectionName, "DisplayOn", "1",
			    szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.bScreenDisplayOn =
			( (BOOL) dwKeyValue == FALSE ) ? FALSE : TRUE;


	/**	Indicate empty scans
	**/
	GetPrivateProfileString ( szSectionName, "LogEmptyScans", "1",
				szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.bIndicateEmptyScans =
			( (BOOL) dwKeyValue == FALSE ) ? FALSE : TRUE;




	/**	Digital Data As Text
	**/
	GetPrivateProfileString ( szSectionName, "DigitalAsText", "1",
			    szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.bDigitalAsText =
			( (BOOL) dwKeyValue == FALSE ) ? FALSE : TRUE;



	/**	Column Width
	**/
	GetPrivateProfileString ( szSectionName, "ColumnWidth",
	    DEFAULT_COLUMN_WIDTH_TEXT, szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.wCharsPerColumn = (WORD)dwKeyValue;	  /* ...as Characaters	   */
	clntInfo.lColumnWidth	 = dwKeyValue + 2; /* ...as Ave Width Chars */




	/**	Log to file
	**/
	GetPrivateProfileString ( szSectionName, "LogToFile", "1",
			szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.bLogDataToDisk =
			( (BOOL) dwKeyValue == FALSE ) ? FALSE : TRUE;


	/**	Log On Alarm **/
	GetPrivateProfileString ( szSectionName, "LogOnAlarm", "0",
			    szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.bLogOnAlarm =
			( (BOOL) dwKeyValue == FALSE ) ? FALSE : TRUE;

	/**	Log On Key Press **/
	GetPrivateProfileString ( szSectionName, "LogOnKeyPress", "0",
			    szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.bLogOnKeyPress =
			( (BOOL) dwKeyValue == FALSE ) ? FALSE : TRUE;

	/**	Log On Change **/
	GetPrivateProfileString ( szSectionName, "LogOnChange", "0",
			    szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.bLogOnChange =
			( (BOOL) dwKeyValue == FALSE ) ? FALSE : TRUE;


	/** Log Duration, stored in millisecs **/
	GetPrivateProfileString ( szSectionName, "LogDuration", "0",
				szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%lu", &settings.dwLogDuration );


	/**     Data Directory, use current directory for default.
	**/

	/** Get the current working directory.**/
		
	GetIMLDirPath ( szWorkingDir, _MAX_DIR  );
	strcat ( szWorkingDir, "\\SetUps");

	GetPrivateProfileString ( szSectionName, "DataDirectory",
	   szWorkingDir,  settings.szDataDirectory, _MAX_PATH, szAppSetup );



	/**     The data file name.
	**/
	GetPrivateProfileString ( szSectionName, "DataFile",
		"*.wl",  settings.szDataFile,
				( _MAX_FNAME + _MAX_EXT ), szAppSetup );


	/**	Periodic files
	**/
	GetPrivateProfileString ( szSectionName, "PeriodicFiles", "0",
			    szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.bPeriodicFiles =
			( (BOOL) dwKeyValue == FALSE ) ? FALSE : TRUE;



	/**	Period Type
	**/
	GetPrivateProfileString ( szSectionName, "PeriodType", "0",
			    szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.wPeriodType = PERIOD_TYPE_BASE + (WORD)dwKeyValue;


	/**	Postfix Type
	**/
	GetPrivateProfileString ( szSectionName, "PostFix", "0",
			    szKey, MAX_KEY_LENGTH, szAppSetup );

	sscanf ( szKey, "%u", &dwKeyValue );
	settings.wPostFixType = POSTFIX_TYPE_BASE + (WORD)dwKeyValue;



	/**	Finally any entries for connected channels....
	**/
	nConnection  = 0;
	bErrReported = FALSE;   /* Report errors ONLY ONCE      */
	do {

		sprintf ( szKeyName, "Input%d", nConnection + 1 );

		lCopied = GetPrivateProfileString ( szSectionName,
		      szKeyName, "", szConnection, MAX_NAME, szAppSetup );


		if ( lCopied == 0 ) {

			/**     Input not in Setup file,
			***	exit do while loop....
			**/
			break;
		}




		nReturn = AddConnectionToArray ( szConnection, lpSummary,
						   	nNumSummaryItems );

		if ( nReturn == ERR_CHAN_NOT_IN_SUMMARY ) {

			if ( !bErrReported ) {

				ATKMessageBox ( hInst, hwndMain,
					nReturn,
					NULL, MB_OK | MB_ICONEXCLAMATION );


				bErrReported = TRUE;
			}

		} else if ( nReturn != OK ) {

			/**      Display error and abort....
			**/
			ATKMessageBox ( hInst, hwndMain,
			         nReturn, NULL, MB_OK | MB_ICONSTOP );

			break;
		}

		/**     If checking for errors destroy the summary
		***     here before returning control to caller.
		**/
		nConnection++; /* Increment connection for next iteration */



	} while ( TRUE );  /* Exit if no more 'Inputn' entries    */





	/**     Destroy the summary
	**/
	GlobalUnlock ( ghSummary );
	GlobalFree   ( ghSummary );


	if ( conninfo.nNumConnected == 0 ) {


		/** ????
		**/
		ATKMessageBox ( hInst, hwndMain, ERR_ALL_CONNECTIONS_MISSING,
						   NULL, MB_OK | MB_ICONEXCLAMATION );

	} else {

		/**     Establish the units of each connected channel
		***
		**/
		ATKChannelUnits ( hwndMain, conninfo.nIMLInterface,
			conninfo.pciConnections, conninfo.nNumConnected );




		/**
		***     Put all connected channels in 'read' list
		**/
		ATKSetReadList ( hwndMain, conninfo.nIMLInterface,
			conninfo.pciConnections, conninfo.nNumConnected );


		/**	Build the 'Monitor' table. If this exists
		***	we may use the AREAD command to determine
		***	if channels are in an alarm state.
		**/
		ATKBuildMonitorTable ( conninfo.imsSetup.szSetupName );
	}




	/**     Allow the Setup to be edited....
	***
	**/
	nReturn = AllowSetupConfiguration ( hwndMain, &conninfo.imsSetup );


	return ( nReturn );
}
