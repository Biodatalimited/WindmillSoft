/**
***	WPDO_UTL.c		ATK-PANELS
***				Miscellaneous Support Fns for
***				Windmill Panels Application Shell.
***     Andrew Thomson
***
***	This module holds miscellaneous functions used to support
***	other modules.
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
#define  CTLAPIS
#define  IML_APPLICATION

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <htmlhelp.h>
#include <string.h>


#include "\Windmill 32\Compile Libs\IMLinter.h"   /* ...using the COMMSLIB Dynalink		 */
#include "\Windmill 32\Compile Libs\imslib.h"     /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATkit\ATKit.h"      /* Application Toolkit Header/Prototypes         */



#include "WPDO_def.h"	/* Manifest Constants and Structure Declarations */
			/* Functions Prototypes				 */


/**	Externally defined Globals Variables and structures
**/
extern HINSTANCE  hInst;		/* Program Instance Handle	    */
extern HWND	  hwndMain;		/* Program Window Handle	    */

/**	Fonts used by Application
**/
extern HFONT	hSansSerif_8;	/*  8 Point Sans Serif Font */
extern HFONT	hSansSerif_10;	/* 10 Point Sans Serif Font */



extern RECT rectButtonBar;  /* Area occupied by Push-button Bar		   */
extern RECT rectStatusBar;  /* Area occupied by Status Bar		   */
extern RECT rectData;	    /* Area occupied by Channel Name, Data & Units */
extern RECT rectScrollBar;  /* Area occupied by Vertical Scroll Bar	   */


/**	Further divide the data area into fields
**/
extern RECT rectNames;	    /* Area occupied by Channel names		    */
extern RECT rectValues;	    /* Area occupied by Data Values		    */
extern RECT rectUnits;	    /* Area occupied by Channel units		    */

extern RECT rectStatusBarItem [ NUM_STATUS_BAR_ITEMS  ];



/**	Application Details
**/
extern PANELINFO	panDigitalOut;

/**	Channel Connection Information
**/
extern CONNECTIONS conninfo;


/**
***     Handles of client area controls.....
**/
extern HANDLE	hScrollBar;


/**	The following is a lock count used by PreventSetupConfiguration and
***	AllowSetupConfiguration. One function increments the count, the other
***	decrements it. When greater than zero one or more processes have
***	disabled the setup from being edited.  Initialize to 1, since the
***	setup will be disabled by default until AllowSetupConfiguration
***	is called.
***
**/
static short	nSetupConfigureDisableCount = 1;




/**
***     AllowSetupConfiguration
***
***	This function allows other applications, and inparticular
***	SetupIML to edit the setup we are using. It should be called
***	before any idle period when no data is being collected/written.
***	Once called, we can no longer assume the setup is correct.
***
***
***	The sister function; PreventSetupConfiguration, should be used
***	to reclaim the setup before data transfers or using any of the
***	information contained within the setup.
***
***	NOTE:
***	A static variable; nSetupConfigureEnabledCount, has been added to this
***	module to allow multiple processes to disable setup configuration.
***	For example both Auto mode and starting a DDE conversation will
***	want to disable the setup (from being edited ). We will only allow
***	the setup to be edited when ALL processes requiring the setup
***	have finished.
***
***
***
**/
short AllowSetupConfiguration ( HWND hwnd, PIMSSETUP pimssSetup )
{




	/**	Decrement the Disable Configuring Count, if this is
	***	now zero no 'process' needs the setup and it is safe to
	***	allow configuration.
	***
	**/
	nSetupConfigureDisableCount--;	/* Dec the count */

	/**	Prevent the count going negative
	**/
	nSetupConfigureDisableCount =
			max ( 0, nSetupConfigureDisableCount );


	if ( nSetupConfigureDisableCount > 0 ) {

		/**	Other 'process' i.e. DDE still need
		***	the setup locked.
		**/
		return ( OK );

	}


        return ( IMSEnableConfiguring ( pimssSetup->szSetupName,
		                                pimssSetup->nSetupKey ) );

}





/**
***     PreventSetupConfiguration
***
***	This function prevents other applications, and inparticular
***	SetupIML editing the setup we are using. It should be called
***	before collect/writting data to any of the channels in the
***	setup or accessing any of the channel's details.
***	Once called, we can assume the setup is correct and will not change.
***
***
***	The sister function; AllowSetupConfiguration, should be used
***	to release the setup during periods when we will are not using
***	setup information.
***
***	NOTE:
***	A static variable; nSetupConfigureEnabledCount, has been added to this
***	module to allow multiple processes to disable setup configuration.
***	For example both Auto mode and starting a DDE conversation will
***	want to disable the setup (from being edited ). We will only allow
***	the setup to be edited when ALL processes requiring the setup
***	have finished.
***
***
***
**/
short PreventSetupConfiguration ( HWND hwnd, PIMSSETUP pimssSetup )
{


	short           nReturn;
	HINSTANCE       hinst;
	short           nConnection;
	short           nChannel;
	short           nIndex;
	GLOBALHANDLE    ghSummary;
	PCHANNELINFO   lpSummary;
	short           nNumSummaryItems;
	BOOL            bChannelInSummary;
	BOOL            bUserInformed = FALSE;
	long            lRange;
	BOOL            bConnectionsMissing;




	/**	Check our static count, if this is zero
	***	disabled the setup for configuration. If non-zero
	***	the setup is already locked and no further action
	***	is needed.
	**/
	if ( nSetupConfigureDisableCount != 0 ) {

		/**	Subsequent request to be disabled. Already done
		***	just allow the count to be incremented.
		**/
		return ( OK );
	}


        hinst = GetWindowInstance ( hwnd );


	/**	Disable the Configuration, If SetupIML is already
	***	editing the setup, pass up the error and give up.
	***
	***	The old panels used to give the user the option of
	***	retrying, but since we cannot stop/start cycling
	***	this will result in an error everytime we attempt
	***	to obtain new readings.
	**/
	nReturn = IMSDisableConfiguring (
                        	pimssSetup->szSetupName,
                        	pimssSetup->nSetupKey );


	/**	The setup will either have changed, not changed
	***	or could not be disabled for configuration.
	***
	***	If OK increment the setup locked count and return okay
	***	otherwise pass the error up to the caller.
	**/


	switch ( nReturn ) {


	case OK :

		/**	Successfully locked, increment the lock count
		**/
		nSetupConfigureDisableCount++;

		return ( OK );




	case  IMS_CONFIGURING :

		/**	The setup is being edited by SetupIML NOW
		***	The setup could not be locked and it the application
		***	should not attempt to use the channels.
		**/




		return ( nReturn );




	case IMS_SETUP_CHANGED :



		/**
		*** The Setup has changed
		*** but we may still be able to use
		*** it if the changes are minor
                ***
                **/


		/**	Successfully locked, increment the lock count
		**/
		nSetupConfigureDisableCount++;




                /** Check Setup
                *** using IMSGetSetupSummary
                *** Changing units & limits has no effect
                *** changing names requires a warning
                *** disabling a channel is serious.
                **/

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





		bUserInformed		= FALSE;
		bConnectionsMissing	= FALSE;


                for ( nConnection = 0;
                        nConnection < conninfo.nNumConnected;
		                                        nConnection++ ) {

                	/**     First ensure the connection still exists
	        	***	in the IMS Summary. If it does not we cannot
        		***     use the channel anymore...
	        	**/
        		bChannelInSummary = FALSE;
	        	for ( nChannel = 0;
                                 nChannel < nNumSummaryItems; nChannel++ ) {

                		if ( lstrcmp  ( conninfo.pciConnections
                                     [ nConnection ].szChannel,
                                        lpSummary [ nChannel ].szName ) == 0 ) {


	                        	/**     Corresponding entry found
        	                	***     in the setup summary
	        	        	**/
                                        if ( lpSummary [ nChannel ].bEnabled ) {
                                                /**     Channel found and
                                                ***     enabled....
                                                **/
                        			bChannelInSummary = TRUE;
                                        }
                        		break;
                                }
                	}
                        if ( !bChannelInSummary ) {
                                /**
                                ***     Delete the connection by
                                ***     shifting the rest of the
                                ***     of the connections down in
                                ***     the array
                                **/
                                for ( nIndex = nConnection; nIndex <
                                           ( conninfo.nNumConnected - 1 );
	                                                       	 nIndex++ ) {

                                        /**   Use structure assignement to
                                        ***   shift up each array element
                                        ***   into the gap left by the
                                        ***   'deleted' connection
                                        **/
                                        conninfo.pciConnections [ nIndex ] =
                                         conninfo.pciConnections [ nIndex + 1 ];
                                }
                        	/**	Adjust vars affected by removing
                                ***     the array element.
                                **/
                                nConnection--;
                                conninfo.nNumConnected--;



	                        if ( !bUserInformed ) {

			      		ATKMessageBox ( hinst, hwnd,
                		     		ERR_CHAN_NOT_IN_SUMMARY,
                        	                NULL,
                                                MB_OK | MB_ICONEXCLAMATION );


                        		bUserInformed = TRUE;
				}

				/**	Flag that the number of connections
				***	has changed...
				**/
				bConnectionsMissing = TRUE;

                        }

		}

		if ( conninfo.nNumConnected == 0 ) {


			/** No connections left...
			**/
			ATKMessageBox ( hInst, hwnd, ERR_ALL_CONNECTIONS_MISSING,
                  			   NULL, MB_OK | MB_ICONEXCLAMATION );

		} else if ( bConnectionsMissing == TRUE ) {



			/**	Resize the OUTPUTDATA array to match the
			***	new number of connections.
			***	structures, one element per connection. The
			***	OUTPUTDATA structure stores information above
			***	and beyond that stored in the toolkit's
			***	CHANINFO structure.
			**/
			conninfo.lpodOutputData = GlobalReAllocPtr (
				conninfo.lpodOutputData,
					(DWORD) sizeof ( OUTPUTDATA )
						* conninfo.nNumConnected,
							   GMEM_MOVEABLE );


			if ( conninfo.lpodOutputData != NULL ) {

				GlobalLockPtr ( conninfo.lpodOutputData );

				/** Default ALL fields in the array
				**/
				for ( nConnection = 0;
				    nConnection < conninfo.nNumConnected;
							    nConnection++ ) {

					conninfo.lpodOutputData
					  [ nConnection ].bWriteData = FALSE;



					conninfo.lpodOutputData
					  [ nConnection ].nNewState = 0;
				}

			} else {

				/**	Failed to assign and lock the memory
				***	Warn the user and fail the operation
				**/
				ATKMessageBox  ( hInst, hwnd,
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


		}




                /**     Establish the new units of each connected channel
                ***
                **/
                ATKChannelUnits ( hwnd, conninfo.nIMLInterface,
                        conninfo.pciConnections, conninfo.nNumConnected );


		/**	Establish the new limits of each connected channel
		***	(required by DDE module)
                **/
		ATKChannelLimits ( hwnd, conninfo.nIMLInterface,
                        conninfo.pciConnections, conninfo.nNumConnected );



	        /**
        	***     Put all connected channels in 'read' list
	        **/
        	ATKSetReadList ( hwnd, conninfo.nIMLInterface,
                	conninfo.pciConnections, conninfo.nNumConnected );


		/**
		***	Put all connected channels in 'write' list
	        **/
		ATKSetWriteList ( hwnd, conninfo.nIMLInterface,
                	conninfo.pciConnections, conninfo.nNumConnected );


		/**	Build the 'Monitor' table. If this exists
		***	we may use the AREAD command to determine
		***	if channels are in an alarm state.
		**/
		ATKBuildMonitorTable ( conninfo.imsSetup.szSetupName );


		/**	Draw the new channels names/units on the
		***	client and remove any old data. Recalculate
		***	the client area sections to accomodate new
		***	connections and allow space for a scollbar
		***	if required
		**/
		ResizeClientRects ( hwnd, panDigitalOut.lWidth,
						panDigitalOut.lHeight );

		InvalidateRect ( hwnd, &rectData, TRUE );


                /**     Update Status, Buttons etc...
                **/
                UpdateButtonBar ( hwnd );
                UpdateMenuBar   ( hwnd );
                UpdateStatusBar ( hwnd );
		UpdateScrollBar ( hwnd );

		/**	Enable the scroll-bar with the correct range
		***	and initial position.
		**/
		panDigitalOut.lFirstChanVisible = 0;


		/**	Finally enable the scroll-bar with the correct range
		***	and initial position. ( If visible at all )
		**/
		lRange = max ( 0, conninfo.nNumConnected -
					panDigitalOut.lNumRowsVisible );

		SetScrollRange ( hScrollBar, SB_CTL, 0, lRange, TRUE );

		SetScrollPos ( hScrollBar, SB_CTL, 0, TRUE );


		/**	Only allow the user to see the Scrollbar
		***	if it has a valid range.
		**/
		if ( lRange > 0 ) {

			ShowWindow ( hScrollBar, SW_SHOW );
		} else {

			ShowWindow ( hScrollBar, SW_HIDE );
		}




                /**     Destroy the summary
	        **/
        	GlobalUnlock ( ghSummary );
	        GlobalFree   ( ghSummary );





		/** Pass up the IMS_SETUP_HAS_CHANGED ret code
		*** to inform the caller that the arrays have changed
		**/
		return ( nReturn );



	default :

		/**	Serious problem ?
                ***     Inform the User...
                **/
                ATKMessageBox ( hinst, hwnd, nReturn, NULL,
							MB_ICONHAND );

		return ( nReturn );
	}



}












/**
***     AddConnectionToArray
***
***     This function will add a new ATKCHANINFO structure to
***     the array pointed to by 'conninfo.pciConnections'
***     (handle: conninfo.hConnections ). If no array exists, the
***     array will be created with the new connection being the
***	first element.
***
***     Arguments :
***
***
***
***     Return Value :
***
***
**/
short   AddConnectionToArray ( char *szConnection, PCHANNELINFO lpSummary,
						     short nNumSummaryChans )
{
        short   nChannel;               /* Index into Summary Array       */
	short	nConnection;    	/* Index to new array element     */
        BOOL    bChannelInSummary;      /* TRUE if channel in IMS summary */


        /**     First ensure the connection still exists
        ***	in the IMS Summary. If it does not we cannot
        ***     use the channel anymore...
        **/
        bChannelInSummary = FALSE;
        for ( nChannel = 0; nChannel < nNumSummaryChans; nChannel++ ) {

                if ( lstrcmp  ( szConnection,
                        lpSummary [ nChannel ].szName ) == 0 ) {


                        /**     Corresponding entry found
                        ***     in the setup summary, but is
                        ***     it enabled ????
	                **/
                        if ( lpSummary [ nChannel ].bEnabled ) {

                        	bChannelInSummary = TRUE;

                        }
                        break;
                }
        }

        if ( !bChannelInSummary ) {

                /**     Abort fn now, the channel cannot be used...
                **/
                return ( ERR_CHAN_NOT_IN_SUMMARY );
        }


        /**
        ***     Okay, now we know we have something to add we
        ***     had better make room for it in the connections
        ***     array.....
        ***
        **/
        if ( conninfo.hConnections == NULL ) {

                /**     No array currently exists.
	        ***     Allocate a new array, with a single element
	        **/
        	conninfo.hConnections = LocalAlloc ( LMEM_MOVEABLE,
		                                sizeof ( ATKCHANINFO ) );

	        if ( conninfo.hConnections == NULL ) {

                        conninfo.nNumConnected  = 0;
			conninfo.pciConnections = NULL;

        	        return ( ERR_NO_LOCAL_MEMORY );
                }

        } else {
                /**     The array exists but we need to enlarge
                ***     the array to accomodate the new element.
		**/
		LocalUnlock ( conninfo.hConnections );

                conninfo.hConnections = LocalReAlloc ( conninfo.hConnections,
                  ( ( conninfo.nNumConnected + 1 ) * sizeof ( ATKCHANINFO ) ),
                      					    LMEM_MOVEABLE );

                if ( conninfo.hConnections == NULL ) {

                        conninfo.nNumConnected  = 0;
			conninfo.pciConnections = NULL;

        	        return ( ERR_NO_LOCAL_MEMORY );
                }
        }

        /**     Get the pointer to the new array and store in the
        ***     'connect' structure allowing the array to be accessed
        ***     directly.
        **/
        conninfo.pciConnections = (PATKCHANINFO)
					LocalLock ( conninfo.hConnections );

        if ( conninfo.pciConnections == NULL ) {

                LocalFree ( conninfo.hConnections );
                conninfo.nNumConnected  = 0;

                return ( ERR_NO_LOCAL_MEMORY );
        }

        /**     Connection sucessfully added. Increment the Connections
        ***     count, stored in the 'connect' structure.
        **/
        conninfo.nNumConnected++;         /* Store the new total connections */




        /**     Initialise the structure, defaulting unknown fields.
        ***     Remember 'nChannel' indexes the channel in the summary
        ***     and will use 'nConnection' as our index to the new
        ***     element in the connections array.
        **/
        nConnection = conninfo.nNumConnected - 1;  /* Calc index to new chan */


        /**     Enter the defaults etc....
        **/
        lstrcpy ( conninfo.pciConnections [ nConnection ].szChannel,
        					 szConnection );

        conninfo.pciConnections [ nConnection ].nChannelNumber =
	                              lpSummary [ nChannel ].nChannel;

        conninfo.pciConnections [ nConnection ].nDeviceNumber =
                                      lpSummary [ nChannel ].nFrame;

	/**	Is the channel digital
	**/
	switch ( lpSummary [ nChannel ].cTypeCode ) {

	case 'B' :
	case 'R' :
	case 'T' : conninfo.pciConnections [ nConnection ].bDigital = TRUE;
		   break;

	default  : conninfo.pciConnections [ nConnection ].bDigital = FALSE;
		   break;
	}



        /**     The application must use ATKChannelUnits
        ***	to obtain the units string even though this
        ***	is available in the summary.
        **/
        conninfo.pciConnections [ nConnection ].szUnits [ 0 ] = '\0';


        conninfo.pciConnections [ nConnection ].nError		= 0;
        conninfo.pciConnections [ nConnection ].szValue [ 0 ]	= '\0';
        conninfo.pciConnections [ nConnection ].ulTime		= 0L;
        conninfo.pciConnections [ nConnection ].nMSecs		= 0;
        conninfo.pciConnections [ nConnection ].szMin [ 0 ]	= '\0';
        conninfo.pciConnections [ nConnection ].szMax [ 0 ]	= '\0';



        return ( OK );
}



/**  This is the routine which brings up the standard Windows HELP system when
***  the user presses a HELP button, or selects one of the "Help..." menu items.
***  The type of help and the correct page etc. are passed in as the wCommand
***  and dwData params for "WinHelp" function (q.v.). The point of this code
***  is to track down the full path of the SETUPIML.HLP file, which is assumed
***  to be in the same directory as the .EXE file. So it finds the drive &
***  directory of our own EXE file, and uses this same drive/dir for specifying
***  the .HLP file to the WinHelp function. No return value.
**/

BOOL	InvokeWinHelp ( HWND hWnd, WORD wCommand, DWORD dwData )
{
	char		szFullPath [ _MAX_PATH ];
	char		szExeDrive [ _MAX_DRIVE ];
	char		szExeDir   [ _MAX_DIR ];
	char		szExeName  [ _MAX_FNAME ];
	char		szExeExtn  [ _MAX_EXT ];
    HINSTANCE   	hInstance;
	BOOL		bRet;


	switch ( wCommand ) {

	case HELP_INDEX:
		wCommand = HH_DISPLAY_TOPIC;
		bRet = TRUE;
		break;
	case HELP_QUIT:
		wCommand = HH_CLOSE_ALL;
		bRet = FALSE;
		break;
	}
    
		
	hInstance = GetWindowInstance ( hWnd );


	GetModuleFileName ( hInstance, (LPSTR) szFullPath, _MAX_PATH );
	
	_splitpath ( szFullPath, szExeDrive, szExeDir, szExeName, szExeExtn );
	
	_makepath  ( szFullPath, szExeDrive, szExeDir, HELP_FILE_NAME,
				".CHM" );

	HtmlHelp ( hWnd, (LPSTR) szFullPath, wCommand, dwData );
	
	return ( bRet );
	

}




/**
***     ExtractCmdLineArg    This function will determine if an
***                          optional argument has been specified
***                          on the command line.
***
***                          If the argument is a IMS filename identified
***                          by the '.IMS' extension. The path and name
***                          are seperated and stored in the 'Connect'
***                          structure.
***
***			     If the argument is a WinApp setup file
***			     identified by the extension '.WLG' then
***                          the fn: LoadAppSetup, will be envoked.
***
***     Arguments :
***
***	hwndMain	     Handle of Application Window. May be NULL.
***     lpszCmdLine          The command line, excluding the program name.
***                          This is the string passed into WinMain.
***
***     Return Value :
***                          VOID
***
**/
void ExtractCmdLineArg ( HWND hwndMain, LPSTR lpszCmdLine )
{
	char            szPath  [ _MAX_PATH  + 1 ];
	char            szDrive [ _MAX_DRIVE + 1 ];
	char            szDir   [ _MAX_DIR   + 1 ];
	char            szFName [ _MAX_FNAME + 1 ];
	char            szExt   [ _MAX_EXT   + 1 ];
	char            szDummy [ _MAX_FNAME + 1 ];
	char*			ptszPath;
	char*			szEndFileSpec;
	OFSTRUCT        ofCmdLineFile;
	short           nReturn;
	HINSTANCE       hInstance;


	long            lRange;



	if ( lstrlen ( lpszCmdLine ) == 0 ) {

		/**     No argument specified
		**/
		return;
	}


	/**     Obtain the module instance.
	**/
	hInstance = GetWindowInstance ( hwndMain );



	/**     Make a local copy of the string
	**/
	lstrcpy ( szPath, lpszCmdLine );


	/** The original code searched for the first space on the line and
	*** treated all before it as file name. However file names acquired
	*** spaces and Microsoft enclosed them in quotes to avoid confusion
	*** on command lines. If the first character of the string is " then
	*** the end of the file name will be indicated by the next ". 
	*** Otherwise use the old system of looking for spaces.
	**/

	if ( szPath[0] == '"') {

		szEndFileSpec = strchr ( szPath + 1, '"' );

		if ( szEndFileSpec != NULL ) {
			*szEndFileSpec = '\0';	/* Move the terminator */
		}
		ptszPath = szPath + 1;
	
	} else {
	
		szEndFileSpec = strchr ( szPath, ' ' );

		if ( szEndFileSpec != NULL ) {
			*szEndFileSpec = '\0';	/* Move the terminator */
		}
		ptszPath = szPath;
	}

	/**     Split into path components
	**/
	_splitpath ( ptszPath, szDrive, szDir, szFName, szExt );


	/**     If no path is specified assume we are using
	***     the same path from which the program was loaded.
	**/
	if ( ( strlen ( szDrive ) == 0 ) && ( strlen ( szDir ) == 0 ) ) {

		GetModuleFileName ( hInstance, (LPSTR) szPath, _MAX_PATH );

		/**     Extract the new drive /directory but NOT
		***     the filename or Extension
		**/
		_splitpath ( szPath, szDrive, szDir, szDummy, szDummy );

	}

	/**     Reform the path with any changes made and verify
	***     a file does actually exist.
	**/
	_makepath ( szPath, szDrive, szDir, szFName, szExt );

	if ( OpenFile ( szPath, &ofCmdLineFile, OF_EXIST ) == (HFILE) -1 ) {

		ATKMessageBox  ( hInstance, hwndMain,
		      ERR_INVALID_CMDLINE_ARG, NULL, MB_OK | MB_ICONINFORMATION );

		return;

	}

	/**     Examine Extension to determine type of file specified
	**/
	if ( stricmp ( szExt, ".IMS" ) == 0 ) {


		/**     Copy path specifiaction into IMSSetup Structure
		**/
		strcpy ( conninfo.imsSetup.szSetupPath, szPath );



		FORWARD_WM_COMMAND ( hwndMain, IDM_LOADIMS,
						NULL, 0, SendMessage );

		return;

	} else {
		PTKTerminateDDEService ( );


		nReturn = LoadAppSetup ( szPath );


		/**	Register new DDE service...
		***
		**/
		nReturn = PTKInitializeDDEService ( hwndMain,
					panDigitalOut.szDDEServiceName );

		if ( nReturn != OK ) {

			ATKMessageBox ( hInstance, hwndMain,
				nReturn,
				(LPSTR) NULL,
				MB_OK | MB_ICONEXCLAMATION );

		}


		/** Any errors will already have been reported
		*** by the LoadAppSetup fn. We can act on the value
		*** if we desire. I am not going to bother.
		**/


		/**     Enable the scroll-bar with the correct range
		***     and initial position.
		**/
		lRange = max ( 0, conninfo.nNumConnected -
					panDigitalOut.lNumRowsVisible );

		SetScrollRange ( hScrollBar, SB_CTL, 0, lRange, TRUE );

		SetScrollPos ( hScrollBar, SB_CTL, 0, TRUE );

		/**	Only allow the user to see the Scrollbar
		***	if it has a valid reange.
		**/
		if ( lRange > 0 ) {

			ShowWindow ( hScrollBar, SW_SHOW );
		} else {

			ShowWindow ( hScrollBar, SW_HIDE );
		}


	}

	/**    Menus, Client and Title should be updated by caller
	***    and return of this function....
	**/
	return;
}














/**
***   The remaining functions are used to convert between
***   channel number,	client text lines and physical units.
***
***   Text lines map directly onto physical units. Channel
***   numbers map onto to text lines but will have an offset
***   if the first channel is not the first channel displayed.
**/






/**
***	ChannelNumberToTextLine		Converts a Channel Number 
***					in the range 0 to 
***					conninfo.NumConnected - 1 to
***					its present text line position
***					on the client area.
***	Arguments :
***		nChannelNumber		The Channel whose text-line
***					we are trying to find.
***	
***	Return Value :
***		short	The Text Line Number on the Client Area
***
**/
long ChannelNumberToTextLine ( long lChannelNumber ) 
{

	return ( lChannelNumber - panDigitalOut.lFirstChanVisible );
}




/**
***	TextLineToChannelNumber		Converts a Client Area Text Line
***					into the channel number that the
***					text represents
***	Arguments :
***		nTextLine		The Text Line on the Client Area
***					
***	
***	Return Value :
***		short	The Channel Number represented by the Text Line 
***
**/
long TextLineToChannelNumber ( long lTextLine ) 
{

	return ( lTextLine + panDigitalOut.lFirstChanVisible );
}



/**
***	PixelPositionToChannelNumber	Converts a Mouse or RECT Coordinate
***					into the Channel Number of the
***					channel 'occupying' that pixel 
***
***					Only the Y coord of the pixel is 
***					specified
***
***	Arguments :
***		wYPos			The Y coordinate of the mouse pointer	
***	
***	Return Value :
***		short	The Channel Number Pointed To By The Mouse
***
**/

long PixelPositionToChannelNumber ( WORD wYPos )
{
	long	lTextLine;

	/**	Convert Y-Position to A Text Line then
	***	the Text Line to Channel Number....
	**/

	lTextLine = ( wYPos - rectData.top ) / panDigitalOut.lRowHeight;
	return ( TextLineToChannelNumber ( lTextLine ) );
}



/**
***	ChannelToMouseCoords - Returns the top right y-coord
***			       of the specified channel
***
***	Return Value : The Y-physical coord of the specified channel
***		       within the client area.
**/
LONG ChannelToMouseCoords ( short nChannel )
{
	long	lTextLine;

	/**	Convert Y-Position to A Text Line then
	***	the Text Line to Channel Number....
	**/

	lTextLine = ChannelNumberToTextLine ( nChannel );
	return ( MAKELONG ( 0, rectData.top +
				   ( lTextLine * panDigitalOut.lRowHeight ) ) );
}



VOID InvalidateChannel ( HWND hwnd, long lChannel, short nChanFlags )
{
	long	lInvalidTextLine;
	RECT	rectInvalid;



	lInvalidTextLine = ChannelNumberToTextLine ( lChannel );

	rectInvalid.top    = rectData.top +
				( lInvalidTextLine * panDigitalOut.lRowHeight );

	rectInvalid.bottom = rectInvalid.top + panDigitalOut.lRowHeight;


	/**	Assume initially that no fields are invalid
	***	Set the left to the maximum possible value and
	***	the right to the minimum. For each field with
	***	the appropriate flag set, we will then reduce
	***	the left value and increase the right to include
	***	that area.
	**/
	rectInvalid.left   = rectData.right;	/* Initially set too high */
	rectInvalid.right  = rectData.left;	/* initially set too low  */


	if ( ( nChanFlags & CHANFIELD_NAME ) > 0 ) {

		rectInvalid.left   = min ( rectInvalid.left, rectNames.left );
		rectInvalid.right  = max ( rectInvalid.right,rectNames.right );

	}

	if ( ( nChanFlags & CHANFIELD_VALUE ) > 0 ) {

		rectInvalid.left   = min ( rectInvalid.left, rectValues.left );
		rectInvalid.right  = max ( rectInvalid.right,rectValues.right );

	}

	if ( ( nChanFlags & CHANFIELD_UNITS ) > 0 ) {


		rectInvalid.left   = min ( rectInvalid.left, rectUnits.left );
		rectInvalid.right  = max ( rectInvalid.right,rectUnits.right );
	}


	/**	Inform Window's that the area is invalid
	**/
	InvalidateRect ( hwnd, (LPRECT) &rectInvalid, TRUE );
	return;

}
