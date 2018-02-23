/**
***	WPAO_SET.c		ATK-PANELS
***				AnalogOut Application Setup File Support
***
***     Andrew Thomson
***
***	This module holds the SaveAppSetup and LoadAppSetup required by
***	the application toolkit.
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
#define  CTLAPIS
#define  IML_APPLICATION

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>


#include "\Windmill 32\Compile Libs\IMLinter.h"	/* ...using the COMMSLIB Dynalink		 */
#include "\Windmill 32\Compile Libs\imslib.h"     /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATkit\ATKit.h"      /* Application Toolkit Header/Prototypes         */



#include "WPAO_def.h"	/* Manifest Constants and Structure Declarations */
			/* Functions Prototypes				 */







/**	Externally declared variables used by this module
***
**/
extern HINSTANCE hInst;		/* Program Instance Handle	    */
extern HWND	 hwndMain;	/* Program Window Handle	    */


/**	Application Details
**/
extern PANELINFO	panAnalogOut;


/**	Channel Connection Information
**/
extern CONNECTIONS	conninfo;






/**
***	Function	SaveAppSetup
***
***	Description	This function is called by the application
***			toolkit. It is passed a setup filename in which
***			all application setup information should be stored
***			including the IMS setup name and current connections.
***
***	Return		See #defined Error codes (WPAO_DEF.h)
***
**/
short SaveAppSetup ( char *szAppSetup )
{
        char    szSectionName    [ ] = APPL_SETUP_SECTIONNAME;
        char    szKey  		 [ MAX_KEY_LENGTH + 1 ];
	char	szKeyName	 [ MAX_KEY_LENGTH + 1 ];

	short	nConnection;
	RECT    rectWindow;
	BOOL	bSucceeded;
	long	lCopied;

	double	dPercentageXPos;
	double  dPercentageYPos;
	double	dPercentageWidth;
	double  dPercentageHeight;
	

	/**	Store the filename in which the details will be stored
	***
	**/
	strcpy ( panAnalogOut.szFileSpec, szAppSetup );


        /**     General Stuff first, thats not implemented in this
        ***     version
        **/
        WritePrivateProfileString ( szSectionName,
					"Type", "AnalogOut", szAppSetup );


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


	/**	DDE Support
	**/
	sprintf ( szKey, "%u", (WORD) panAnalogOut.bDDEEnabled );

	/**	..are we allowing DDE support
	**/
	WritePrivateProfileString ( szSectionName, "DDEEnabled",
                		    			szKey, szAppSetup );

	/**	...and the service name to be used for DDE conncetion
	**/
	WritePrivateProfileString ( szSectionName, "DDEServiceName",
				  panAnalogOut.szDDEServiceName, szAppSetup );





	/**	Finally an entry for each connected channel....
	**/
        for ( nConnection = 0; nConnection < conninfo.nNumConnected;
                                                        nConnection++ ) {

		sprintf ( szKeyName, "Output%d", nConnection + 1 );

		bSucceeded = WritePrivateProfileString ( szSectionName,
			szKeyName,
  			conninfo.pciConnections [ nConnection ].szChannel,
                        szAppSetup );


	}
	/**	Give any remaining outputs channels a NULL key entry.
	***	This will delete unwanted channels already existing in the
	***	file
	**/
	for ( nConnection = conninfo.nNumConnected;
			nConnection < MAX_APPLICATION_CONNECTIONS;
                                                        nConnection++ ) {

		sprintf ( szKeyName, "Output%d", nConnection + 1 );

		/**	Verify that the 'unwanted' output exists
		***	by reading it. If we can't read it then
		***	we may assume no further existing entries
		***	exist and that we have finished.
		**/
		lCopied = GetPrivateProfileString ( szSectionName,
		      szKeyName, "", szKey, MAX_NAME, szAppSetup );


                if ( lCopied == 0 ) {

			/**	No more Outputs in Setup file to remove,
			***	exit for loop....
                        **/
                        break;
		}

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
***	Function	LoadAppSetup
***
***	Description	This function is called by the application
***			toolkit. It is passed a setup filename from which
***			all application setup information should be restored
***			including the IMS setup name and current connections.
***
***	Return		See #defined Error codes (WPAO_DEF.h)
***
**/
short LoadAppSetup ( char *szAppSetup )
{

	char           szSectionName    [ ] = APPL_SETUP_SECTIONNAME;
	char           szKey  		 [ MAX_KEY_LENGTH + 1 ];
	GLOBALHANDLE   ghSummary;
	PCHANNELINFO   lpSummary;
	short          nNumSummaryItems;
	short          nReturn;
	long           lCopied;
	short          nConnection;
	char           szConnection [ MAX_NAME + 1 ];
	char    	   szKeyName    [ MAX_KEY_LENGTH + 1 ];
	BOOL           bErrReported;
	
	short          nWindowXPos;
	short          nWindowYPos;
	short          nWindowWidth;
	short          nWindowHeight;
	double         dPercentageXPos;
	double         dPercentageYPos;
	double         dPercentageWidth;
	double         dPercentageHeight;
	


	/**	Store the filename from whence we got the setup details
	***
	**/
	strcpy ( panAnalogOut.szFileSpec, szAppSetup );




	/**	Prevent the current setup from being edited.
	***	before it is released. If the setup is being
	***	edited, abort the load.
	**/
	if ( conninfo.bIMSSetupLoaded == TRUE ) {
		nReturn = PreventSetupConfiguration ( hwndMain,
							&conninfo.imsSetup );
		if ( nReturn != OK ) {

			/**	Cannot continue without setup.
			**/
			return ( nReturn );
		}
	}



	/**     IMS Filename  **/
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


	/**	Also  free the memory used for storing output
	***	data, we can recreate this once we have established
	***	how many connections have been made by the user
	**/
	if ( conninfo.lpodOutputData != NULL ) {

		GlobalUnlockPtr ( conninfo.lpodOutputData );
		GlobalFreePtr	( conninfo.lpodOutputData );
	}

	if ( conninfo.bIMSSetupLoaded == TRUE ) {

		/**     Free the IMS setup and IMSLib.DLL **/
		IMSUnlockSetup ( conninfo.imsSetup.szSetupName,
					conninfo.imsSetup.nSetupKey );

		/**     Reset the vars assoicated with the
		***     old IMS file. Do NOT delete the newly
		***     acquired path name though.
		**/
		conninfo.imsSetup.szSetupName [ 0 ] = '\0';
		conninfo.bIMSSetupLoaded 	   = FALSE;
		conninfo.imsSetup.nSetupKey	   = 0;

		panAnalogOut.nStatus = STATUS_NOSETUP;

	}

	/**     Load a new Setup..........
	***     Assume that the setup path and name, if known,
	***     has already been copied into the IMSSETUP
	***	structure. If not known then the strings
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
		***	reported the error to the user.
		**/
		return ( nReturn );

	} else {
		conninfo.bIMSSetupLoaded = TRUE;
		panAnalogOut.nStatus = STATUS_IDLE;

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

		/**     No memory...**/
		GlobalFree ( ghSummary );
		return ( ERR_NO_LOCAL_MEMORY );

	}

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
	}









	/**	DDE support
	**/
	panAnalogOut.bDDEEnabled = (BOOL) GetPrivateProfileInt (
						szSectionName,
						(LPCSTR) "DDEEnabled",
						0,
						(LPCSTR) szAppSetup );


	GetPrivateProfileString ( szSectionName,
				    "DDEServiceName",
				    "AnalogOut",
				    panAnalogOut.szDDEServiceName,
				    ATK_MAX_DDE_SERVICE_NAME,
				    szAppSetup );






	/**     FInally any entries for connected channels....
	**/
	nConnection  = 0;
	bErrReported = FALSE;   /* Report errors ONLY ONCE      */
	do {

		sprintf ( szKeyName, "Output%d", nConnection + 1 );

		lCopied = GetPrivateProfileString ( szSectionName,
                      szKeyName, "", szConnection, MAX_NAME, szAppSetup );


		if ( lCopied == 0 ) {

			/**	Output not in Setup file,
			***	exit do while loop....
			**/
			break;
		}

		nReturn = AddConnectionToArray ( szConnection, lpSummary,
						   	nNumSummaryItems );

		if ( nReturn == ERR_CHAN_NOT_IN_SUMMARY ) {

			if ( !bErrReported ) {

		      	ATKMessageBox ( hInst, hwndMain, nReturn,
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

	} while ( TRUE );  /* Exit if no more 'Outputn' entries	*/





	/**     Destroy the summary  **/
	GlobalUnlock ( ghSummary );
	GlobalFree   ( ghSummary );


	if ( conninfo.nNumConnected == 0 ) {

		/** No connections specified  **/
		ATKMessageBox ( hInst, hwndMain, ERR_ALL_CONNECTIONS_MISSING,
                  			   NULL, MB_OK | MB_ICONEXCLAMATION );

	} else {

		/**	Create and initialize an array of OUTPUTDATA
		***	structures, one element per connection. The
		***	OUTPUTDATA structure stores information above
		***	and beyond that stored in the toolkit's
		***	CHANINFO structure.
		**/
		conninfo.lpodOutputData = GlobalAllocPtr (
			       GMEM_MOVEABLE, (DWORD) sizeof ( OUTPUTDATA )
						    * conninfo.nNumConnected );


		if ( conninfo.lpodOutputData != NULL ) {

			GlobalLockPtr ( conninfo.lpodOutputData );

			/** Default ALL fields in the array
			**/
			for ( nConnection = 0;
				nConnection < conninfo.nNumConnected;
								nConnection++ ) {

				conninfo.lpodOutputData [ nConnection ].
							bWriteData = FALSE;

				conninfo.lpodOutputData [ nConnection ].
							nWriteError = OK;

				conninfo.lpodOutputData [ nConnection ].
							szNewValue [ 0 ] = '\0';
			}

		} else {

			/**	Failed to assign and lock the memory
			***	Warn the user and fail the operation
			**/
			ATKMessageBox  ( hInst, hwndMain,
				ERR_NO_GLOBAL_MEMORY,
				NULL, MB_OK | MB_ICONHAND );

			/**	Destroy the connection array
			***	its no good without the
			***	OUTPUTDATA array.
			**/
			conninfo.nNumConnected = 0;

			LocalUnlock ( conninfo.hConnections );
			LocalFree   ( conninfo.hConnections );


		}




		/**     Establish the units of each connected channel
        ***
		**/
        ATKChannelUnits ( hwndMain, conninfo.nIMLInterface,
                	conninfo.pciConnections, conninfo.nNumConnected );


		/**	Establish the limits of each connected channel
		***	( althoug not directly used, these may be
		***	requested Over DDE )
	        **/
		ATKChannelLimits ( hwndMain, conninfo.nIMLInterface,
                	conninfo.pciConnections, conninfo.nNumConnected );


		/**	Build the 'Monitor' table. If this exists
		***	we may use the AREAD command to determine
		***	if channels are in an alarm state.
		**/
		ATKBuildMonitorTable ( conninfo.imsSetup.szSetupName );


		/**
        ***     Put all connected channels in 'read' list
		**/
        ATKSetReadList ( hwndMain, conninfo.nIMLInterface,
        		conninfo.pciConnections, conninfo.nNumConnected );


		/**
		***	Put all connected channels in 'write' list
		**/
		ATKSetWriteList ( hwndMain, conninfo.nIMLInterface,
        		conninfo.pciConnections, conninfo.nNumConnected );


		/**	Perform an initial read on ALLCHANNELS
		***	('AllChannels' = Last Connected Channel + 1 )
		**/
		FORWARD_WM_READCHAN ( hwndMain,	conninfo.nNumConnected,
							   SendMessage );
	}

	/**	Give the focus to one of the editboxs
	**/
	SetChannelFocus ( hwndMain, panAnalogOut.nFocusChannel );


	/**     Allow the Setup to be edited....
	***
	**/
	nReturn = AllowSetupConfiguration ( hwndMain, &conninfo.imsSetup );


	return ( nReturn );
}
