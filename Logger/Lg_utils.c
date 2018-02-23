/**
***	LG_UTILS.c              ATK-LOGGER
***
***	Andrew Thomson
***
***	This module is responsible for miscellaneous fns
***
***
***
***	Associated Files : 	
***
***	Version History :
***
***	ATKLOG - 4.01		Prevent/AllowSetupConfiguration
***				enhanced to allow nested calls to
***				these routines. This is needed as the
***				new DDE module may see fit to prevent
***				the configuration as well as the main
***				application. ( These routines are now
***				the same as the version 4.00 panels )
***
***	4.02			New Function GetInternationalSettings added.
***	4.04			New function FormatDate to convert time
***				to date string.
***				Date support added to GetInternationalSettings
***
***
**/
#define  NOMINMAX
#define  NOKANJI
#define  NOCOMM
#define  STRICT         /* Use Windows 3.1 Strict Style Coding  */
#define  CTLAPIS        /* Use Control API Macros 		*/

#include <windows.h>
#include <windowsx.h>
/**#include <toolhelp.h>	**//* for TimerCount etc in GetMillisecondCount  */
#include <conio.h>	/* for inp() and outp() */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys\timeb.h>
#include <dos.h>
#include <htmlhelp.h>


#include "\Windmill 32\Compile Libs\IMLinter.h"  /* ...using the COMMSLIB Dynalink                */
#include "\Windmill 32\Compile Libs\imslib.h"    /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATKit\ATKit.h"      /* Application Toolkit Header/Prototypes         */
#include "lg_def.h"     /* Manifest Constants and Structure Declarations */
#include "lg_prot.h"    /* Logger modules Function Prototypes            */

/**     External Rectangles
**/
extern  RECT		rectRealTimeClk; /* Rect in which to draw R.T.clock */
extern  RECT		rectChanLabels;  /* Area occupied by Channel Names  */
extern  RECT		rectLoggedData;  /* Area occupied by Logged Data    */
extern  RECT		rectLoggedTime;  /* Area occupied by Logged Time    */

/**     External Structures
**/
extern  CLOCKINFO       clkRealTime;     /* Real Time Clock Info            */
extern  LOGINFO         logInfo;
extern  CONTROLS        controls;
extern  CLIENTINFO      clntInfo;
extern  CONNECTIONS	conninfo;
extern  SETTINGS        settings;
extern  INTLINFO	intl;


/**     External Handles\Variables
**/
extern  HWND            hScrollBar;
extern  HWND            hwndMain;
extern  HINSTANCE       hInst;



/**	The following is a lock count used by PreventSetupConfiguration and
***	AllowSetupConfiguration. One function increments the count, the other
***	decrements it. When zero ( or less ) one or more processes have
***	disabled the setup from being edited.  Initialize to zero, the setup will
***	be disabled by default until AllowSetupConfiguration is called.
***
**/
static short	nSetupConfigureEnabledCount = 0;



/**
***
***     GetFontMetrics
***
***     Returns by Reference the tmHeight and tmAveCharWidth fields
***	of the TEXTMETRIC structure for the specified Font.
***
***
***     hWndClient      The Window owning the client Area.
***
***
***     pnHeight        The Text Height field returned by reference
***     pnWidth         The Average Text Character Width returned by
***                     reference.
***
***     Return Value    Void
***
***
**/
void GetFontMetrics ( HWND hwndClient, HFONT hFont,
				long *plHeight, long *plWidth )
{
	HDC             hClientDC;
	HFONT           hOldFont;
	TEXTMETRIC      tmTextMetrics;



	hClientDC = GetDC ( hwndClient );

	hOldFont = SelectFont ( hClientDC, hFont );
	GetTextMetrics ( hClientDC, &tmTextMetrics );

	*plHeight = tmTextMetrics.tmHeight;
	*plWidth  = tmTextMetrics.tmAveCharWidth;

	/**     Do not destroy Fonts, these are created and destroyed
	***     by the application
	**/
	SelectFont ( hClientDC, hOldFont );

	ReleaseDC ( hwndClient, hClientDC );

}










/**
***	AllowSetupConfiguration
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

	if ( nSetupConfigureEnabledCount++ != 0 ) {

		/**	The setup has already been enabled for
		***	configuration. Exit now...
		**/
		return ( OK );
	}


	return ( IMSEnableConfiguring ( pimssSetup->szSetupName,
		                                pimssSetup->nSetupKey ) );

}





/**
***	PreventSetupConfiguration
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
	BOOL            bUserInformed;
	short           nReply;


	/**	Decrement our static count, if this is now zero
	***	(i.e. No process wants the setup enabled ) disabled the
	***	setup for configuration.
	**/
	if ( --nSetupConfigureEnabledCount != 0 ) {

		return ( OK );
	}


	hinst = GetWindowInstance ( hwnd );

	do {

		/**     Disable the Configuration, allow the
		***     user to retry as many times as he wishes...
		**/
		nReturn = IMSDisableConfiguring (
				pimssSetup->szSetupName,
				pimssSetup->nSetupKey );

		if ( nReturn == IMS_CONFIGURING ) {


			/**     Remind the user, he is editing the
			***     the hardware setup.
			**/
			nReply = ATKMessageBox ( hinst, hwnd, nReturn, NULL,
				       MB_ICONHAND | MB_RETRYCANCEL );

			if ( nReply == IDCANCEL ) {

				/**	Abort if cancelled, the caller
				***	will also have to abort the
				***	operation it is trying to start.
				***
				***	As we have failed, increment the
				***	static count before returning.
				**/
				nSetupConfigureEnabledCount++;

				return ( nReturn );
			}

		}

	} while ( nReturn == IMS_CONFIGURING );



	/**     The setup will either have changed, not changed
	***     or could not be disabled for configuration. Handle
	***     the setup changed case first...
	**/
	if ( nReturn == IMS_SETUP_CHANGED ) {


		/**
		*** The Setup has changed
		*** but we may still be able to use
		*** it if the changes are minor
		***
		**/

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
		bUserInformed = FALSE;
		for ( nConnection = 0;
		        nConnection < conninfo.nNumConnected;
		                                        nConnection++ ) {

			/**	First ensure the connection still exists
			***	in the IMS Summary. If it does not we cannot
			***	use the channel anymore...
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
				***	the array element.
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
			}

		}

		/**     Establish the new units of each connected channel
		***
		**/
		ATKChannelUnits ( hwnd, conninfo.nIMLInterface,
		       conninfo.pciConnections, conninfo.nNumConnected );




		/**
		***     Put all connected channels in 'read' list
		**/
		ATKSetReadList ( hwnd, conninfo.nIMLInterface,
			conninfo.pciConnections, conninfo.nNumConnected );



		/**	Build the 'Monitor' table. If this exists
		***	we may use the AREAD command to determine
		***	if channels are in an alarm state.
		**/
		ATKBuildMonitorTable ( conninfo.imsSetup.szSetupName );


		/**     Draw the new channels names/units on the
		***     client and remove any old data.
		**/
		ClearHistoricBuffer ( );
		InvalidateRect ( hwnd, &rectLoggedData, TRUE );
		InvalidateRect ( hwnd, &rectLoggedTime, TRUE );
		InvalidateRect ( hwnd, &rectChanLabels, TRUE );

		/**     Update Status, Buttons etc...
		**/
		UpdateButtonBar ( hwnd );
		UpdateMenuBar   ( hwnd );
		UpdateStatusBar ( hwnd );

		/**     Enable the scroll-bar with the correct range
		***     and initial position.
		**/
		ShowWindow ( hScrollBar, SW_SHOW );

		SetScrollRange ( hScrollBar, SB_CTL, 0,
					( conninfo.nNumConnected - 1 ), TRUE );

		clntInfo.lFirstChanVisible = 0;
		SetScrollPos   ( hScrollBar, SB_CTL,
					clntInfo.lFirstChanVisible, TRUE );


		/**     Destroy the summary
		**/
		GlobalUnlock ( ghSummary );
		GlobalFree   ( ghSummary );

		nReturn = OK;


	}


	/**     Now decide if  it is safe to continue
	***     with the setup. If it is return OK to the
	***     caller otherwise inform the user and return
	***     up the error.
	**/
	if ( nReturn != OK ) {

		/**
		***     Inform the User...
		**/
		ATKMessageBox ( hinst, hwnd, nReturn, NULL,
						MB_ICONHAND );

	}



	return ( nReturn );

}





short ServiceRealTimeClock ( HWND hwndParent )
{


	struct timeb timSystem;


	/**     Fill the timeb structure with information
	***     pertaining to the current System Clock time.
	**/
	ftime ( &timSystem );


	/** Time in Secs since 00:00:00 GMT, January 1st, 1970
	**/
	clkRealTime.timeNow = timSystem.time;

	/**	Adjust from Local Time
	***	( which is by default in the Pacific Time Zone )
	**/
	clkRealTime.timeNow -= ( timSystem.timezone * 60L );
	if ( timSystem.dstflag ) {

		/**     Daylight saving in operation
		***     Add 3600 Seconds to convert
		***     GMT to BST
		***
		**/
		clkRealTime.timeNow += ( 60L * 60L );
	}


	if ( clkRealTime.timeNow != clkRealTime.timeAtLastUpdate ) {


		/**
		***	Force a repaint of the logger clock
		***	and note the time now displayed on the
		***	screen so we may detect any further
		***	variation between the system clock and
		***	the logger clock.
		**/
		clkRealTime.timeAtLastUpdate = clkRealTime.timeNow;

		InvalidateRect ( hwndParent, &rectRealTimeClk, FALSE );

	}

	return ( OK );
}



void TimeStructToText ( struct tm *ptmGMT, char *szFormatedText,
						    WORD wTimeFormat )
{


	switch ( wTimeFormat ) {


		case SECS_SINCE_START  :

			sprintf ( szFormatedText, "%d", ptmGMT->tm_sec );
			break;


		case SECS_AND_HUNDRETHS   :


			strftime ( szFormatedText,
				MAX_TIMESTAMP_CHARS, "%H:%M:%S", ptmGMT );
			break;


		case HOURS_MINS_SECS   :

		default :

			strftime ( szFormatedText,
				MAX_TIMESTAMP_CHARS, "%H:%M:%S", ptmGMT );
			break;
	}

}




/**
***     dwTime may be Seconds since Jan 1st 1970 or
***     mSecs since computer started...
***     depending on value of wTimeFormat..
***
**/

void FormatTime ( char *szFormatedText, DWORD dwTime,  WORD wTimeFormat )
{
	DWORD   	dwElapsedMins;
	DWORD   	dwElapsedSecs;
	DWORD		dwElapsedTenths;
	DWORD		dwElapsedHundredths;
	struct  tm	*ptmGMT;


	switch ( wTimeFormat ) {

		case MINUTES_AND_SECS :

			dwElapsedMins = ( dwTime - logInfo.dwTimeStarted ) / 60000L;  

			dwElapsedSecs = ( ( dwTime - logInfo.dwTimeStarted ) % 60000L ) / 1000L;
			if ( ( ( dwTime - logInfo.dwTimeStarted ) % 1000L ) >= 500 ) {

				/**     Round upto nearest second
				**/
				dwElapsedSecs++;
			}
			sprintf ( szFormatedText, "%lu:%02lu", dwElapsedMins, dwElapsedSecs );
			break;

        

		case SECS_SINCE_START  :


			dwElapsedSecs = ( dwTime - logInfo.dwTimeStarted ) / 1000L;
			if ( ( ( dwTime - logInfo.dwTimeStarted ) % 1000L ) >= 500 ) {

				/**     Round upto nearest second
				**/
				dwElapsedSecs++;
			}
			sprintf ( szFormatedText, "%lu", dwElapsedSecs );
			break;


		case SECS_AND_HUNDRETHS   :  /* Version 4 also includes hundredths */



			dwElapsedSecs   = ( dwTime - logInfo.dwTimeStarted ) / 1000L;

			dwElapsedTenths = ( ( dwTime - logInfo.dwTimeStarted )
								 % 1000L )  / 100L;

			dwElapsedHundredths = ( ( dwTime - logInfo.dwTimeStarted )
								 % 100L )  / 10L;

			if ( ( ( dwTime - logInfo.dwTimeStarted ) % 10L ) >= 5 ) {

				/** Round Upto nearest hundredth
				**/
				if ( ++dwElapsedHundredths == 10 ) {

					if ( ++dwElapsedTenths == 10 ) {

						dwElapsedSecs++;

						dwElapsedTenths = 0L;
					}
					dwElapsedHundredths = 0L;
				}
			}
			sprintf ( szFormatedText, "%lu.%1lu%1lu",
				dwElapsedSecs, dwElapsedTenths, dwElapsedHundredths );
			break;


		case HOURS_MINS_SECS   :
		case DATE_TIME	       :

			/**     Convert time_t to a tm structure
			***     and use the c-library time formating
			***     fn 'strftime'
			**/
			ptmGMT = gmtime ( (time_t *) &dwTime );

			strftime ( szFormatedText,
				MAX_TIMESTAMP_CHARS, "%H:%M:%S", ptmGMT );
			break;

	}

}



/**
***	dwTime may be Seconds since Jan 1st 1970 or
***	mSecs since computer started...
***	depending on value of wTimeFormat..
***
**/

void FormatDate ( char *szFormatedText, DWORD dwTime,  WORD wTimeFormat )
{
	struct	tm	*ptmGMT;
	short		nChar;

	switch ( wTimeFormat ) {


		case DATE_TIME :


			/**     Convert time_t to a tm structure
			***     and use the c-library time formating
			***     fn 'strftime'
			**/
			ptmGMT = gmtime ( (time_t *) &dwTime );

			if ( intl.bAmericanDateFormat ) {

				strftime ( szFormatedText,
					  MAX_DATESTAMP_CHARS,
					  "%m/%d/%y",
					  ptmGMT );
			} else {

				strftime ( szFormatedText,
					  MAX_DATESTAMP_CHARS,
					  "%d/%m/%y",
					  ptmGMT );
			}

			/**	If necessary replace the component
			***	seperators with those set up using
			//	control-panel.
			**/
			if ( intl.cDateSeparator != '/' ) {

				/**	Search for default separators to be
				***	replaced.
				**/
				for ( nChar = 0;
				    nChar < lstrlen ( szFormatedText );
								nChar++ ) {


					if ( szFormatedText [ nChar ] == '/' ) {

						szFormatedText [ nChar ] =
							intl.cDateSeparator;
					}
				}

			}

			break;


		default :

			/**	Date field not supported for this time format
			**/
			lstrcpy ( szFormatedText, "" );
	}
	return;
}








/**
***     Button Bar & Status Bar Definitions
***     These specify the dimensions of buttons etc.
***	We may have to replace these with dimensions
***     calculated from the font size !
***
**/
VOID SizeControls ( VOID )
{
	controls.nPushButtonHeight   = ( clntInfo.lTextHeight * 7 ) / 4;
	controls.nPushButtonWidth    = clntInfo.lAveTextWidth * 10;
	controls.nEditBoxLength      = ( controls.nPushButtonWidth * 3 ) / 2;
	controls.nEditBoxHeight      = controls.nPushButtonHeight;
	controls.nEditBoxLabelLength = clntInfo.lAveTextWidth * 15;
	controls.nBarBorder          = 2;
	controls.nSingleBarHeight    = controls.nPushButtonHeight +
	                               ( 2 * controls.nBarBorder );

	controls.nDoubleBarHeight    = ( 2 * controls.nPushButtonHeight ) +
	                               ( 4 * controls.nBarBorder );

	controls.nMinSingleBarWidth  = ( controls.nPushButtonWidth *
	                                 NUM_BUTTONS ) +
	                                 controls.nEditBoxLength +
	                                 controls.nEditBoxLabelLength + 5;

	controls.nStatusBarHeight    =  clntInfo.lLargeTextHeight + 10;
	controls.nScrollBarHeight    =  ( 3 * clntInfo.lTextHeight ) / 2;
}













/**
***     AddConnectionToArray
***
***     This function will add a new ATKCHANINFO structure to
***     the array pointed to by 'connect.pciConnections'
***     (handle: connect.hConnections ). If no array exists, the
***     array will be created with the new connection being the
***     first element.
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
	short   nConnection;            /* Index to new array element     */
	BOOL    bChannelInSummary;      /* TRUE if channel in IMS summary */


	/**     First ensure the connection still exists
	***     in the IMS Summary. If it does not we cannot
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

	/**	Is this channel digital ?
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
	conninfo.pciConnections [ nConnection ].szUnits [ 0 ]	= '\0';
	conninfo.pciConnections [ nConnection ].szValue [ 0 ]	= '\0';
	conninfo.pciConnections [ nConnection ].szMin   [ 0 ]	= '\0';
	conninfo.pciConnections [ nConnection ].szMax   [ 0 ]	= '\0';
	conninfo.pciConnections [ nConnection ].szOn    [ 0 ]	= '\0';
	conninfo.pciConnections [ nConnection ].szOff   [ 0 ]	= '\0';
	conninfo.pciConnections [ nConnection ].ulTime		= 0L;
	conninfo.pciConnections [ nConnection ].nMSecs		= 0;
	conninfo.pciConnections [ nConnection ].nError		= 0;


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
		dwData = 0;
		bRet = TRUE;
		break;
	case HELP_QUIT:
		wCommand = HH_CLOSE_ALL;
		bRet = FALSE;
		break;
	case HELP_CONTEXT:
		wCommand = HH_HELP_CONTEXT;
		bRet = TRUE;
		break;
	default:
	//	wCommand = HH_DISPLAY_INDEX;
		bRet = TRUE;
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
***	GetInternationalSettings
***
***	Reads [intl] section of WIN.ini and stores the decimal, list and
***	time separators in the global structure 'intl'.
***
***
**/
void GetInternationalSettings ( void )
{
	char	szDest [ 25 + 1 ];

	/**	The character which separates the fractional and
	***	non-fractional parts of a number.
	**/
	GetProfileString ( "intl", "sDecimal", ".", szDest, 2 );
	intl.cDecimalSeparator = szDest [ 0 ];


	/**	The character which separates list items
	***
	**/
	GetProfileString ( "intl", "sList", ",", szDest, 2 );
	intl.cListSeparator = szDest [ 0 ];


	/**	The character which separates components in time
	***	strings
	**/
	GetProfileString ( "intl", "sTime", ":", szDest, 2 );
	intl.cTimeSeparator = szDest [ 0 ];


	/**	The character which separates components in short date
	***	strings
	**/
	GetProfileString ( "intl", "sDate", "/", szDest, 2 );
	intl.cDateSeparator = szDest [ 0 ];


	/**	The short data format  we will only implement
	***	MM DD YY or DD MM YY ignoring long-year (YYYY) and
	***	stuff like that.
	**/
	GetProfileString ( "intl", "sShortDate", "dd/MM/yy", szDest, 25 );
	if ( ( szDest [ 0 ] == 'M' ) ||	( szDest [ 0 ] == 'm' ) ) {

		/**	American Format
		**/
		intl.bAmericanDateFormat = TRUE;
	} else {
		intl.bAmericanDateFormat = FALSE;

	}


	return;



}
