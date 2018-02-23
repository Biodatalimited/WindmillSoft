/**
***	LG_MAIN.c              ATK-LOGGER
***
***	Andrew Thomson
***
***	This module is responsible for creating the Application
***	Window and processing messages for that window.
***	Additionally this module is used to initialize data structures
***	and load/open DLL's used by the application.
***
***
***	Associated Files : 	
***
***	Version History :
***
***	3.00	Alpha test version used internally. December 1991.
***
***	3.01	First Beta Test Version - With all functionallity
***		implemented.
***		Built using The Windows 3.0 SDK, Feb 19th 1992
***		( using 3.0 Window's SDK )
***
***	3.02	Recompiled with 'Global Register Allocation' optimization
***		disabled, this caused problems with the Copy protection
***		routine, CCWIN.obj which does not appear to preserve the
***		registers.
***		No Code changes.
***		Built using the Windows 3.0 SDK, March 5th 1992.
***
***	3.03	-Fn 'LoggerOnCreate' now returns FALSE (abort Window Creation)
***		 when DLL or memory errors are detected which would prevent
***		 the logger working. This replaces the PostQuitMessage call
***		 used previously...
***		-Messages Box in LoggerOnCreate now use DefTitle for the
***		 caption.
***		-Paint functions painting channels or channel labels now
***		 check that connect.pciConnections is valid. This pointer
***		 is unlocked when calling the connections dialog-box but
***		 the handle and 'numConnected' fields still remain valid.
***		-The 'Inputs' menu item is now disabled if no valid
***		 IMS setup is loaded. Previously it checked if hConnections
***		 was NULL ( no inputs ) which meant that if there were no
***		 inputs to begin with the only way of specifying any was to
***		 reload the IMS file ( which calls the connections dialog
***		 directly bypassing the menu).
***		-Status bar now displays 'Logging to Screen Only' instead
***		 of data file directory if NOT logging to disk.  The Edit box
***		 used to enter the filename is also disabled in this mode.
***		-Message Box displayed if SaveAppSetup fails to write the
***		 data file.
***		-Updates menus, titles etc. after restoring setups. This had
***		 been remarked out for 3.02.
***		-Sets 'time stopped' if logging fails in WndMain or OnTimer
***		 this caused UAE in CloseDataFile.
***		-SaveLoggerSetup now destroys any Input entries in an
***		 existing setup file provided they are no longer in use.
***		-Serial Number NOT defaulted on receipt of a WM_CREATE
***		 message. This field will have been assigned BEFORE
***		 CreateWindow
***	3.04	-Relinked with CCWIN Version 1.55, ATK 1.01 and new style
***		 About Box.
***	3.05	-Rebuilt with toolkit; ATK-P-1.02
***
***	4.00	-New 100 channel version with alarm support and
***		 enhanced display and file facilities.
***		 Built with toolkit ATK-P-2.00
***		 Feb-May 1993
***
***	4.01	-DDE support Added
***	4.02	-International Support Added (See  INTERNATIONAL struct
***		 and GetInternationalSettings).  Periodic files can now
***		 be appended to ( next free file is used ). This is
***		 automatically used if started in Auto mode.
***		-Resume key can now be invoked from Accelerator keystroke.
***	4.03	-Toolkit now sends REP if 'AREAD Failed' returned.
***	4.04	-Rebuilt using Copy Control 1.66 for Win95
***		 New time stamp format' option allowing Date stamping in the
***		 data files. This involved :
***
***		 -Adding a new field to HISTORICDATAROW to store the date.
***		 -Adding a new constant TIME_DATE to LOG_DEF.h
***		  (used by the settings dialog and when save/restoring setups)
***		 -Adding a new resource string for the TIME_DATE ID
***		 -New function FormatDate to convert time to date string
***		 -Editing code in FormatTime to recognise TIME_DATE
***		 -New fields/code for the INTLINFO strut
***		  (international settings) allowing the date to be written
***		  in the correct format.   
***
***	4.05	-Rebuilt using C++ 1.51 
***		-Uses Windmill Copy Protection system instead of 'Copy Control' 
***		-Allows selection of logging interval in Minutes as well as seconds
***		-New time stamp format 'Minutes and Seconds since start'
***		-Data values are now right justified on the screen - ready for
***		 decimal aligned IML devices.    
***		-Allows Window title to entered by user.
***		-Fixes bug where new num-columns is not recalculated when restoring setups 
***		
***	4.06	-Fixes bug where data files could not be appended too, introduced in
***		 4.05
***	4.07	-Fixes bug in UpdateTitle which overrides settings.szTitle.
***
*** 5.00  BGO April 99. Converted to 32 bit. Sampling rate now controlled by Windows
***       timers combined with WaitMessage in the message loop to minimise the use
***       of machine time.
***       New copy control arrangements added
*** 5.01  BGO October 99. Fixed NT bug whereby program went to wait state
***       rather than doing logging. 
*** 5.02  BGO Nov 99  Above bug also present on 98. Fix works fine until 
***       long sampling periods are used (minutes). Solved by allowing
***       background timer to check if reading needed as well as 
***       correct timer.
*** 5.03  BGO Dec 99 Solution to above bug found in error inherited from
***       16 bit. Also corrected error due to sscanf function writing
***       DWORDs into WORD variables without the compiler objecting. 
***       This caused misreading of setup files with generation of
***       a new file at every reading.
***       This potential error also corrected in kit files.
*** 5.04  Added arrangement so that when using periodic files the scan is 
***       taken at a round number of secs. eg if you want to scan every
***       10 minutes it will be at 10.00, 20.00, 30.00 minutes etc. 
***       An initial start up reading is taken immediately then a 
***       correct time reading follows.
*** 6.00  Compiled with new kit. Log on keypress/alarm added. New periodic
***       filenames added. Changed message loop.
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
#include <direct.h>
#include <errno.h>

#include <time.h>
#include <string.h>

#include "\Windmill 32\Compile Libs\IMLinter.h"   /* ...using the COMMSLIB Dynalink                */
#include "\Windmill 32\Compile Libs\imslib.h"	/* ...using the IMSLIB Dynalink		  */
#include "\Windmill 32\ATKit\ATKit.h"      /* Application Toolkit Header/Prototypes         */
#include "lg_def.h"     /* Manifest Constants and Structure Declarations */
#include "lg_prot.h"    /* Functions Prototypes */
#include "Resource.h"                         



/**	Application Name and Title
***	Use Conditional Compilation Macros to define title...
**/
char		szAppName      [ ]   = "ATKLOG";





/**	Useful Globals....
**/
HINSTANCE hInst			= NULL;	/* Program Instance Handle	    */
HWND	  hwndMain		= NULL;	/* Program Window Handle	    */
BOOL	  bHelpLoaded		= FALSE;/* Has Application loaded Help	    */
BOOL	  bInitializationOK	= TRUE;	/* WM_CREATE Processed succesfully  */
BOOL	  bNotQuit	= FALSE;      /* True after Create until close    */

int nDDECloseCommand = FALSE;

/**	Fonts used by Logger
**/
HFONT		hSansSerif_8;   /*  8 Point Sans Serif Font */
HFONT		hSansSerif_10;  /* 10 Point Sans Serif Font */


/**	Pens and brushes
**/
HPEN	hpenStandardText;
HBRUSH	hbrushStandardBk;


RECT    rectButtonBar;  /* Area occupied by Push-button Bar             */
RECT    rectStatusBar;  /* Area occupied by Status Bar                  */
RECT    rectChanLabels; /* Area occupied by Channel Names and Units     */
RECT    rectLoggedData; /* Area occupied by Logged Data                 */
RECT    rectLoggedTime; /* Area occupied by Logged Time                 */
RECT    rectScrollBar;  /* Area occupied by Horz-Scroll-bar (if present)*/
RECT    rectRealTimeClk;/* Area used to display current time            */
RECT    rectClockLabel;	/* Used for 'TIME' label above real time clk    */
RECT    rectHighlight;  /* Area Highlighted to show 'last' reading      */
RECT    rectScroll;     /* Scrolling area of Client                     */

LOGINFO         logInfo;       /* Logging Information                   */
CLIENTINFO      clntInfo;      /* Client Area  Details 		 	*/
CLOCKINFO       clkRealTime;   /* 'Real Time Clock' data                */
SETTINGS        settings;      /* Current Logger Settings	        */
CONNECTIONS     conninfo;      /* Channel Connections & IMS Lib Details */
CONTROLS        controls;
ATKCOLOURINFO   colors;        /* COLORREF values for 3 alarm states	*/
INTLINFO        intl;          /* International character information	*/


BOOL bTimerExpired = FALSE;
BOOL bDestroy = FALSE;

/**
***     Handles of client area controls.....
**/
extern  BUTTONBAR abutButtonInfo [ NUM_BUTTONS ];
extern  HWND	  hEditBox;    			 /* The filename edit box */
extern  HWND      hScrollBar;                    /* The horz Scroll Bar   */

void ServiceBackground ( void );



/**
***	WinMain 		Program Entry Point
***	
***	Arguments :
***		hInstance 	Handle to program Insatnce	
***		hPrevInstance	Handle to previous Instance 
***		lpszCmdLine	The command line
***		nCmdShow	-
***
***	Return Value :
***		wMsg.wParam	Standard WinMain Return Value
***
***
***	This is the standard entry function for a Windows Application.
***	This Function registers the Window Class and Extracts any Information
***	from the command line argument string 'lpszCmdLine' 	
***
**/
int PASCAL WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance,
					   LPSTR lpszCmdLine, int nCmdShow )
	
{
	HWND            hwnd;
	MSG             msg;	
	WNDCLASS        wndclass;



	/**     'Reference' formal Parameters to avoid Warning Level 4
	***      messages...
	**/
	hPrevInstance = hPrevInstance;
	lpszCmdLine   = lpszCmdLine;
	nCmdShow      = nCmdShow;


	/**
	***     Determine if the user has a valid release copy
	***     of the software. If not put up MessageBox.
	**/
    if ( !IMSClear ( "Windmill" ) ) {

		/**	Copy protection has been invalidated
		**/
		MessageBox ( (HWND)hInstance, 
		"        This Program is NOT an Authorised Copy. \r\r    It will however function correctly for this Session. \r\r See Confiml Software Section for Authorisation Details",
					"Logger Security Warning",
					 MB_OK );
						   		
	}


	/**     If the Window Class has not already been registered,
	***     register it now. Do not use hPrevInstance to determine
	***     if the class has already been registered support for this
	***     will be redrawn in future versions of the SDK.
	**/


	wndclass.style		= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc	= (WNDPROC) Logger_WndProc;
	wndclass.cbClsExtra	= 0;
	wndclass.cbWndExtra	= 0;
	wndclass.hInstance	= hInstance;
	wndclass.hIcon 		= LoadIcon ( hInstance, 
						(LPSTR) szAppName );
	wndclass.hCursor	= LoadCursor ( NULL, IDC_ARROW );
	wndclass.hbrBackground	= GetStockObject ( WHITE_BRUSH );
	wndclass.lpszMenuName	= (LPSTR) szAppName; 
	wndclass.lpszClassName 	= (LPSTR) szAppName; 

	if ( !RegisterClass ( (LPWNDCLASS) &wndclass ) ) {
		return FALSE;

	}	

	/**	Calculate default area to be one quarter
	***	the total screen size...
	**/


	/**	The CreateWindow function sends WM_CREATE,
	***	WM_GETMINMAXINFO and WM_NCCREATE messages to
	***	the Window.
	**/
	hwnd = CreateWindow ( (LPSTR) szAppName, 
			      (LPSTR) szAppName, 
			      WS_OVERLAPPED |
			      WS_CAPTION     | WS_SYSMENU | WS_MAXIMIZEBOX |
			      WS_MINIMIZEBOX | WS_THICKFRAME,
			      CW_USEDEFAULT, 0,
			      GetSystemMetrics ( SM_CXSCREEN ) / 2, 
			      GetSystemMetrics ( SM_CYSCREEN ) / 2, 
			      NULL,
			      NULL,
			      hInstance,
			      NULL );



	if ( ( bInitializationOK == FALSE ) || ( hwnd == NULL ) ) {

		/**	Window Not Created, Exit the Program
		***	The application has probably detected
		***	and reported an error while processing
		***	WM_CREATE.
		***
		**/
		return ( 0 );

	} else {


		/**	Application created ok, set useful
		***	variables...
		**/
		bNotQuit = TRUE;

		hwndMain = hwnd;	/** Store the Window Handle	**/
		hInst	 = hInstance;	/** Store the Instance		**/

	}

	hbrushStandardBk = CreateSolidBrush ( colors.crNormalBk );
	hpenStandardText = CreatePen ( PS_SOLID, 0, colors.crNormalText );

	/**	Change the Window background colour to
	***	match the colour alarm colour scheme
	***
	***	Hmmm .. don't think you can do this
	**/

	SetClassLong ( hwnd, GCL_HBRBACKGROUND, (WORD) hbrushStandardBk );




	/**	Initialize our 'real time clock'. If autostarting
	***	we need to know the current time.
	**/
	ServiceRealTimeClock ( hwnd );



	/**
	***     If a command line argument has been specified
	***	process this.  This may auto start the logger
	***
	**/
	ExtractCmdLineArg ( hwnd, lpszCmdLine );
	UpdateTitleBar ( hwnd ); 


 

	ShowWindow   ( hwnd, nCmdShow );
	UpdateWindow ( hwnd );



	UpdateButtonBar ( hwnd );
	UpdateMenuBar   ( hwnd );
	UpdateStatusBar ( hwnd );



	/**		The following loop runs all the time between create
	*** and destroy.
	***     Whilst we are logging, implement a peek message loop.
	***	If no messages are outstanding, check the clock if
	***     necessary obtain new data.
	***
	***     NB We MUST remove messages for this application
	***	inorder that bLogging may be set FALSE and the
	***     PEEK message loop escaped...
	**/
	while ( bNotQuit ) {

		// Taken out for 5.01
		WaitMessage ();


		while ( PeekMessage ( (LPMSG) &msg, NULL, 0 , 0,
						   PM_REMOVE ) ) {

			if ( msg.message == WM_QUIT ) {
				bNotQuit = FALSE;
			}

			TranslateMessage ( (LPMSG) &msg );
			DispatchMessage  ( (LPMSG) &msg );

		}


		/** All actual logging is done from this call.
		*** The logging routines in ATK contain their own 
		*** message loops to service the user during logging
		**/
		if ( bTimerExpired && !bDestroy ) {
			ServiceBackground ();
			bTimerExpired = FALSE;
		}

		/** The ATK message loops may process a close request.
		*** We must ensure that we are out of the ServiceBackground
		*** routine before we actually Destroy the window and
		*** the dynamically allocated memory.
		**/

		if ( bDestroy ) {
			DestroyWindow ( hwnd );
			bDestroy = FALSE;
		}

	}

	
	return ( (UINT) msg.wParam );
}






/**
***	Logger_WinProc		Application Message Handling Fn
***	
***	Arguments :
***		hwnd	 	Handle of Window Recieving Message
***		wMsg		The Message To Process 
***		wParam		Additional Message Info (message specific)
***		lParam		Additional Message Info (message specific)
***
***	Return Value :
***		0L    if message Processesd 
***		Return Value from DefWindowProc if passed to the Default Handler
***
***
**/
LRESULT CALLBACK Logger_WndProc ( HWND hwnd, WORD wMsg,
					   WPARAM wParam, LPARAM lParam )
{

	switch ( wMsg ) {

	case WM_CREATE : 
		HANDLE_WM_CREATE ( hwnd, wParam, lParam, Logger_OnCreate );
		break;
		
	case WM_SIZE   : 
		HANDLE_WM_SIZE	 ( hwnd, wParam, lParam, Logger_OnSize );
		break;
		
	case WM_PAINT  : 
		HANDLE_WM_PAINT  ( hwnd, wParam, lParam, Logger_OnPaint );
		break;
		
	case WM_COMMAND: 
		HANDLE_WM_COMMAND( hwnd, wParam, lParam, Logger_OnCommand );
		break;
		
	case WM_HSCROLL:
		HANDLE_WM_HSCROLL( hwnd, wParam, lParam, Logger_OnHScroll );
		break;
		
	case WM_CLOSE  : 
		HANDLE_WM_CLOSE  ( hwnd, wParam, lParam, Logger_OnClose );
		break;
		 
	case WM_DESTROY: 
		HANDLE_WM_DESTROY ( hwnd, wParam, lParam, Logger_OnDestroy );
		break;
		
	case WM_MENUCHAR:
		/**	This handles the Keyboard I/F for the 
		***	Button Bar controls. The return value MUST 
		***	be passed on to inform Windows the 'Menus'
		***	have been processed.
		**/
		return	HANDLE_WM_MENUCHAR ( hwnd, wParam, lParam,
							Logger_OnMenuChar );
		break;
		
	case WM_SETFOCUS:
		HANDLE_WM_SETFOCUS ( hwnd, wParam, lParam, Logger_OnSetFocus );
		break;
		
	case WM_KEYDOWN :
		HANDLE_WM_KEYDOWN  ( hwnd, wParam, lParam, Logger_OnKeyDown );
		break;
		
	case WM_KEYUP :
		HANDLE_WM_KEYUP  ( hwnd, wParam, lParam, Logger_OnKeyUp );
		break;

	case WM_TIMER :
		 HANDLE_WM_TIMER  ( hwnd, wParam, lParam, Logger_OnTimer, );
		 break;


	case WM_GETMINMAXINFO: HANDLE_WM_GETMINMAXINFO
			( hwnd, wParam, lParam, Logger_OnGetMinMaxInfo );
		break;
		
	default :
		return DefWindowProc ( hwnd, wMsg, wParam, lParam );
	}
	return ( 0L );
}




VOID Logger_OnKeyDown ( HWND hwnd, long vk, BOOL fDown,
						long cRepeat, long flags )
{

	/**     Forward keystrokes to the editbox handler which
	***     should be recieving all messages of this type...
	***     (but cannot if it loses the focus)
	**/
	FORWARD_WM_KEYDOWN ( hEditBox, vk, cRepeat, flags, SendMessage );

}



VOID Logger_OnKeyUp ( HWND hwnd, long vk, BOOL fDown,
						long cRepeat, long flags )
{

	/**     Forward keystrokes to the editbox handler which
	***     should be recieving all messages of this type...
	***     (but cannot if it loses the focus)
	**/
	FORWARD_WM_KEYUP ( hEditBox, vk, cRepeat, flags, SendMessage );

}





/**
***  Logger_OnCreate    This function is called on recieving a
***                     WM_CREATE message.
***
***  Arguments
***
***  lpCreateStruct     Points to a CREATESTRUCT containing information about the window
***			being created. The members of the CREATESTRUCT
***			structure are identical to the parameters of the
***			CreateWindowEx function.
***
***  Return Value       TRUE  - Continue with Window Creation
***			FALSE - Abort Window Creation.
***
***			also sets Global bInitializationOK to TRUE
***			if successful or false if not.
***
**/
BOOL Logger_OnCreate ( HWND hwnd, CREATESTRUCT * lpCreateStruct )
{
	short   	nReturn;        /* General Fn Return Value      */
	HINSTANCE   	hinst;          /* Module Instance Handle       */
	HDC             hdcDisplay;     /* Device Context for Screen    */

	short           nHorzPixelsPerInch;
	short           nVertPixelsPerInch;
	short           nFontHeight;
	short           nFontWidth;


	hinst = GetWindowInstance ( hwnd );





	/**	Open the Commslib channel
	***
	**/

	nReturn = OpenIMLInterface ( &( conninfo.nIMLInterface ) ) ;

	if ( nReturn != OK ) {

		ATKMessageBox  ( hinst, hwnd, (WORD) nReturn,
				      "Logger", MB_OK | MB_ICONHAND );

		/**	Abort Window Creation...
		**/
		bInitializationOK = FALSE;
		return ( bInitializationOK );
	}




	/**     Default remaining fields in CONNECTIONS struct
	**/
	conninfo.nNumConnected 	= 0;
	conninfo.nMaxConnections = MAX_LOGGER_CONNECTIONS;
	conninfo.wChanTypes      = TYPE_INPUT;
	conninfo.hConnections    = NULL;
	conninfo.pciConnections  = NULL;
	conninfo.bIMSSetupLoaded = FALSE;

	/**     We may be able to get the Setup Path from the
	***     latter.....
	**/
	conninfo.imsSetup.szSetupPath [ 0 ] = '\0';
	conninfo.imsSetup.szSetupName [ 0 ] = '\0';
	conninfo.imsSetup.nSetupKey         = 0;




	clkRealTime.timeNow 		= (time_t) 0;
	clkRealTime.timeAtLastUpdate 	= (time_t) 0;


	/**	Initialize the settings structure.
	***	but do not assign a default to the
	***	the serail number field , this will
	***	already have been assigned its true
	***	value before CreateWindow was called.
	**/
	settings.szFileSpec [ 0 ]	= '\0'; /* No setup fiel yet	   */
	settings.dwScanInterval		= 1000; /* Default 1 Sec scan rate */
	settings.dwLogDuration = 0;
	settings.bLogOnAlarm = FALSE;
	settings.bLogOnKeyPress = FALSE;
	settings.bLogOnChange = FALSE;
	settings.wFileFormat		= FILEFORMAT_BASE;
	settings.wTimeStampFormat	= TIMESTAMP_BASE;
	settings.szDataFile [ 0 ]       = '\0';
	settings.szDataDirectory [ 0 ]	= '\0';
	settings.szDataFileCurrent [ 0 ]	= '\0';
	settings.szDataFileFull [ 0 ]	= '\0';



	/**     Default data directory is the current working directory
	**/
	//getcwd ( settings.szDataDirectory, _MAX_PATH );


	/**	Default logging Info...
	**/
	logInfo.bIsLogging      = FALSE;
	logInfo.bIsPaused       = FALSE;
	logInfo.bAutoStarted	= FALSE;
	logInfo.bLogPressed     = FALSE;


	/**	Get the International settings info from WIN.INI
	**/
	GetInternationalSettings ( );



	/**	Read the default alarm colour scheme
	***	from the Windmill.ini file. No longer does that 
	*** simply sets defaults
	**/
	ATKLoadINIColours ( &colors );

	/**	The logger can only use Solid colours...
	***
	**/
	hdcDisplay = GetDC ( hwnd );

	colors.crNormalText   =
		GetNearestColor ( hdcDisplay, colors.crNormalText );

	colors.crNormalBk     =
		GetNearestColor ( hdcDisplay, colors.crNormalBk );

	colors.crWarningText  =
		GetNearestColor ( hdcDisplay, colors.crWarningText );

	colors.crWarningBk    =
		GetNearestColor ( hdcDisplay, colors.crWarningBk );

	colors.crCriticalText =
		GetNearestColor ( hdcDisplay, colors.crCriticalText );

	colors.crCriticalBk   =
		GetNearestColor ( hdcDisplay, colors.crCriticalBk );


	ReleaseDC ( hwnd, hdcDisplay );




	/**	Create some standard pens and brushes
	***	These will be destroyed on WM_DESTROY
	**/

	hdcDisplay = GetDC ( hwnd );
	nHorzPixelsPerInch  = GetDeviceCaps ( hdcDisplay, LOGPIXELSX );
	nVertPixelsPerInch  = GetDeviceCaps ( hdcDisplay, LOGPIXELSY );
	ReleaseDC ( hwnd, hdcDisplay );


	/**     Create Useful Fonts
	**/
 
        
	/**	We require an 8-Point Font ( a point is 1/72 inch )
	***	but add 2 extra 'points' for external leading. The  
	***	two has been determined by trial and error and may
	***	not be correct for all monitor types... 
	**/        
	nFontHeight = ( nVertPixelsPerInch * ( 8 + 2 ) ) / 72;
	nFontWidth = 0;		/* Default width */


	hSansSerif_8 = CreateFont (
		nFontHeight,   	/* font height                  */
		nFontWidth,    	/* character width              */
		0,	       	/* escapement of line of text   */
		0,		/* angle of baseline and x-axis */
		700,		/* font weight                  */
		0,		/* flag for italic attribute    */
		0,		/* flag for underline attribute */
		0,		/* flag for strikeout attribute */
		ANSI_CHARSET,   /* character set                */
		OUT_DEFAULT_PRECIS, /* output precision		*/
		CLIP_DEFAULT_PRECIS,/* clipping precision	*/
		DEFAULT_QUALITY,    /* output quality		*/
		FF_SWISS | DEFAULT_PITCH,  /* pitch and family  */
		"MS Sans Serif" );


	/**	10 Point Font ( again with 2 points for external leading )
	**/
	nFontHeight = ( nVertPixelsPerInch * 12 ) / 72;
	nFontWidth  = 0;



	hSansSerif_10 = CreateFont (
		nFontHeight,   	/* font height                  */
		nFontWidth,    	/* character width              */
		0,	       	/* escapement of line of text   */
		0,		/* angle of baseline and x-axis */
		400,		/* font weight                  */
		0,		/* flag for italic attribute    */
		0,		/* flag for underline attribute */
		0,		/* flag for strikeout attribute */
		ANSI_CHARSET,   /* character set                */
		OUT_DEFAULT_PRECIS, /* output precision		*/
		CLIP_DEFAULT_PRECIS,/* clipping precision	*/
		DEFAULT_QUALITY,    /* output quality		*/
		FF_SWISS | DEFAULT_PITCH,  /* pitch and family  */
		"MS Sans Serif" );


	/** Store the text sizes of each font
	*** for use when creating Controls
	***
	***     hSansSerif_8  is used for the button-bar text
	***     hSansSerif_10 is used for the status-bar text
	**/
	GetFontMetrics ( hwnd, hSansSerif_8,
			&( clntInfo.lTextHeight ),
			&( clntInfo.lAveTextWidth ) );

	GetFontMetrics ( hwnd, hSansSerif_10,
			&( clntInfo.lLargeTextHeight ),
			&( clntInfo.lLargeTextWidth ) );



	/**
	*** Calc number visible Rows IF client area is MAXIMIZED.
	*** This is needed BEFORE allocating the Historic
	*** Data Buffer.
	***
	*** NOTE Too many wont do any harm ( just allocate
	*** unused space... )
	**/
	clntInfo.lMaxRowsVisible = 
	        GetSystemMetrics ( SM_CYFULLSCREEN ) / clntInfo.lTextHeight;


	/**
	***	Create a buffer in which to hold recent logged data/time.
	***	It should be capable of holding enough data to update
	***	the client area should the Window be maximised.
	***
	***	This should be created early because it is used when
	***	painting the client area..
	**/
	nReturn = CreateHistoricBuffer ( );
	if ( nReturn != OK ) {

		/**     Report the error
		**/
		ATKMessageBox  ( hInst, hwnd,
		     ERR_NO_LOCAL_MEMORY, settings.szTitle, MB_OK | MB_ICONHAND );

		/**     Request to close the application....
		**/
		bInitializationOK = FALSE;
		return ( bInitializationOK );
	}


	/**     Fill the CONTROLS structure.
	***     This contains sizes for buttons, the edit box,
	***     scroll bar and the button and status bars etc.
	***     The dimensions are determined by sizes of
	***     the above fonts, which will be used in the controls.
	**/
	SizeControls ( );


	/**
	***     Create Client Area Controls...
	**/
	CreatePushButtons ( hwnd );
	CreateScrollBar   ( hwnd );


	/**     Set Initial Button/Menu States
	***
	**/
	UpdateButtonBar ( hwnd );
	UpdateMenuBar   ( hwnd );
	UpdateStatusBar ( hwnd );


	/**	Give the FOCUS to the Edit box. This control
	***	should always have the focus unless disabled 
	***	(e.g. when logging)
	**/
	SetFocus ( hEditBox );



	/**     Initialise the scroll bar range to zero.
	***     The scroll-bar will be set up properly once we
	***     have established the number of connections
	**/
	SetScrollRange ( hScrollBar, SB_CTL, 0, 0, TRUE );
	SetScrollPos   ( hScrollBar, SB_CTL, 0, TRUE );
	ShowWindow ( hScrollBar, SW_HIDE );



	settings.bScreenDisplayOn	= TRUE;
	settings.bIndicateEmptyScans	= TRUE;
	settings.bDigitalAsText 	= TRUE;

	settings.bLogDataToDisk		= TRUE;
	settings.bPeriodicFiles 	= FALSE;
	settings.wPeriodType		= PERIOD_EVERY_HOUR;

	settings.wCharsPerColumn	= DEFAULT_COLUMN_WIDTH;
	clntInfo.lColumnWidth		= settings.wCharsPerColumn + 2;

	LoadString ( hInst, IDS_MAINTITLE, settings.szTitle,
					   MAX_TITLE );
        
        
        
	/**	New for version 4.01, DDE Support
	***	Register DDE support
	**/
	strcpy ( settings.szDDEServiceName, "Logger" );
	nReturn = PTKInitializeDDEService ( hwnd, settings.szDDEServiceName );

	if ( nReturn != OK ) {

		/**	No DDE available for this instance of Logger.
		**/


	}


	SetTimer ( hwnd, BACKGROUND_TIMER, 1000, NULL);

	/**	Return TRUE, allowing Window creation to be continued
	**/
	bInitializationOK = TRUE;



	return ( bInitializationOK );

}









/**     Logger_OnSize
***
***     This function is called on a WM_SIZE message.
***
***     Arguments :
***
***       state	SIZEFULLSCREEN  Window has been maximized.
***
***		SIZEICONIC      Window has been minimized.
***		SIZENORMAL      Window has been resized, but neither
***				SIZEICONIC nor SIZEFULLSCREEN applies.
***
***		SIZEZOOMHIDE    Message is sent to all pop-up windows when
***				some other window is maximized.
***
***		SIZEZOOMSHOW    Message is sent to all pop-up windows when some
***				other window has been restored to its former
***				size.
***
***       cx	The new client area Width
***       cy	The new client area Height
***
***     Return Value : void
***
**/
VOID Logger_OnSize ( HWND hwnd, long state, long cx, long cy )
{
	HDC        hdcClient;   /* Client Area Device Context Handle    */



	hdcClient = GetDC ( hwnd );     /* Get The DC for later         */

	clntInfo.lWidth 	= cx;
	clntInfo.lHeight	= cy;


	/**     Using the new client area size
	***     assign the various areas used by the logger
	***
	***     These are :
	***
	***	     The Push Button Bar
	***          The Status Bar
	***          Channel Labels
	***          The Logged Data Area
	***          The Logged Time Area
	***          The Current Time
	***          The Scroll Bar
	**/
	rectButtonBar.left 	= 0;
	rectButtonBar.right 	= cx;
	rectButtonBar.top       = 0;
	rectButtonBar.bottom    = ( cx > controls.nMinSingleBarWidth ) ?
			controls.nSingleBarHeight : controls.nDoubleBarHeight;



	rectStatusBar.left      = 0;
	rectStatusBar.right     = cx;
	rectStatusBar.bottom    = cy;
	rectStatusBar.top = rectStatusBar.bottom - controls.nStatusBarHeight;




	rectScrollBar.left      = clntInfo.lAveTextWidth *
					  ( MAX_TIMESTAMP_CHARS + 3 )  + 1;
	rectScrollBar.right     = cx;
	rectScrollBar.bottom    = rectStatusBar.top;
	rectScrollBar.top       = rectScrollBar.bottom -
						controls.nScrollBarHeight;



	rectChanLabels.left     = rectScrollBar.left;
	rectChanLabels.right    = rectScrollBar.right;
	rectChanLabels.bottom   = rectScrollBar.top     - 1;
	rectChanLabels.top      = rectChanLabels.bottom -
					 ( 2 * clntInfo.lTextHeight );



	rectLoggedTime.left     = clntInfo.lAveTextWidth * 1;
	rectLoggedTime.right    = clntInfo.lAveTextWidth *
					  ( MAX_TIMESTAMP_CHARS + 2 );
	rectLoggedTime.bottom   = rectChanLabels.top   - 1;
	rectLoggedTime.top      = rectButtonBar.bottom;




	rectLoggedData.left     = rectChanLabels.left;
	rectLoggedData.right    = rectChanLabels.right;
	rectLoggedData.bottom   = rectChanLabels.top    - 1;
	rectLoggedData.top      = rectButtonBar.bottom;



	rectHighlight.left      = 0;
	rectHighlight.right     = cx;
	rectHighlight.bottom    = rectChanLabels.top    - 1;
	rectHighlight.top       = rectHighlight.bottom  -
	                                        clntInfo.lTextHeight;




	/**     The Scrolling Rectangle used to scroll data
	***     up the screen. Note we dont scroll the bottom
	***     line which we have to update seperately.
	**/
	rectScroll.left         = rectLoggedTime.left;
	rectScroll.right        = rectLoggedData.right;
	rectScroll.top          = rectLoggedData.top;
	rectScroll.bottom       = rectLoggedData.bottom -
	                                  ( clntInfo.lTextHeight );

	/**	The real time clock 
	***
	**/	
	rectRealTimeClk.left    = rectLoggedTime.left;
	rectRealTimeClk.right   = rectLoggedTime.right;
	rectRealTimeClk.bottom  = rectStatusBar.top - clntInfo.lTextHeight;
	rectRealTimeClk.top     = rectRealTimeClk.bottom -
	                           ( clntInfo.lTextHeight );

	/**	The label above the Real Time Clock
	**/
	rectClockLabel.left 	= rectRealTimeClk.left;
	rectClockLabel.right	= rectRealTimeClk.right;
	rectClockLabel.bottom	= rectRealTimeClk.top;
	rectClockLabel.top	= rectRealTimeClk.top - clntInfo.lTextHeight;


	/**
	*** Number of Visible Data Columns
	**/
	clntInfo.lNumColumnsVisible =
	               ( rectLoggedData.right - rectLoggedData.left )
	                 / ( clntInfo.lAveTextWidth * clntInfo.lColumnWidth );

	/**     Include ONE partial Column
	**/
	if ( ( rectLoggedData.right - rectLoggedData.left )
		% ( clntInfo.lAveTextWidth * clntInfo.lColumnWidth ) != 0 ) {

		clntInfo.lNumColumnsVisible++;
	}


	/**
	*** Number of Visible data Rows
	**/
	clntInfo.lNumRowsVisible =
	         ( rectLoggedData.bottom - rectLoggedData.top )
		                               / ( clntInfo.lTextHeight );


	/**
	*** Visible Rows IF client area is MAXIMIZED.
	*** This is needed BEFORE allocating the Historic
	*** Data Buffer.
	***
	*** NOTE Too many wont do any harm just allocate
	*** unused space...
	**/
	clntInfo.lMaxRowsVisible =
	        GetSystemMetrics ( SM_CYFULLSCREEN ) / clntInfo.lTextHeight;



	ReleaseDC ( hwnd, hdcClient );


	return;

}






/**
***  Logger_OnPaint     This function is called on receiving a
***                     WM_PAINT message.
***
***  Arguments
***
***  hwnd               Handle of Window owning Client
***
***  Return Value       Void
***
**/
VOID Logger_OnPaint ( HWND hwnd )
{
	HDC		hdcClient;
	PAINTSTRUCT     psPaint;
	RECT            rectIntersect;    /* Intersection of two RECTs */
	RECT            rectUnion;        /* Union of two RECTS        */

	hdcClient = BeginPaint ( hwnd, (LPPAINTSTRUCT) &psPaint );



	if ( IsIconic ( hwnd ) ) {

		/**     Application is iconised, do not paint anything
		**/
		EndPaint ( hwnd , (LPPAINTSTRUCT) &psPaint );
		return;

	}


	/**
	***     Determine which rectangles need repainting
	***	First the Real Time Clock and associated label
	**/
	UnionRect ( &rectUnion, &rectRealTimeClk, &rectClockLabel ); 
	if ( IntersectRect ( &rectIntersect, &psPaint.rcPaint, &rectUnion ) ) {

		/**     Clock Rect Invalidated ( or part thereof )
		***	...Paint the Clock
		**/
		PaintRealTimeClk ( hdcClient );

	}



	if ( IntersectRect ( &rectIntersect,
	              &psPaint.rcPaint, &rectStatusBar ) ) {


		/**     Status Bar Rect Invalidated ( or part thereof )
		***  ...Paint the Status Bar
		**/
		DrawStatusBar ( hdcClient );
		UpdateStatusBar ( hwnd );

	}



	/**     Paint Logged Data is also responsible for painting
	***     the logged times and the highlighted text row, ands
	***     and needs calling if any of the rects are
	***     invalidated. Therefore find the union of all areas
	***     within the invalidated area....
	**/
	UnionRect ( &rectUnion, &rectLoggedData, &rectLoggedTime );
	UnionRect ( &rectUnion, &rectUnion,	 &rectHighlight  );

	if ( IntersectRect ( &rectIntersect, &psPaint.rcPaint, &rectUnion ) ) {


		/**     Logged Data Rect Invalidated ( or part thereof )
		***  ...Paint the Logged Data
		**/
		PaintLoggedData ( hdcClient, &rectIntersect );
		DrawTimeDataDivision ( hdcClient );


	}


	if ( IntersectRect ( &rectIntersect,
		&psPaint.rcPaint, &rectChanLabels )  ) {


		/**     Channel Names/Units Invalidated ( or part thereof )
		***  ...Paint the Logged Data
		**/


		PaintChanLabels ( hdcClient );
		DrawTimeDataDivision ( hdcClient );


	}



	/**     Do the button bar after the Logged Data, since the
	***     logged data may overshoot its area slightly...
	**/
	if ( IntersectRect ( &rectIntersect,
			&psPaint.rcPaint, &rectButtonBar ) ) {


		/**     Button Bar Rect Invalidated ( or part thereof )
		***   ..Paint the Status Bar
		**/
		PositionButtonBar ( hdcClient );   /* ??? only when resized */
		PositionScrollBar ( );

	}






	EndPaint ( hwnd , (LPPAINTSTRUCT) &psPaint );
	return;
}








/**
***  Logger_OnCommand   This function is called on recieving a
***			WM_COMMAND message.
***			The WM_COMMAND message is sent to a window
***			when the user selects an item from a menu, when
***			a control sends a notification message to its
***			parent window, or when an accelerator key stroke
***			is translated.
***
***
***
***  Arguments
***
***  hwnd               Handle of Window recieving the Command
***  id                 Identifies the menuitem or control
***  hwndCtl            Handle of control sending message ( Controls Only )
***  codeNotify         The notification code. ( Controls Only )
***
***  Return Value       Void
***
**/
VOID Logger_OnCommand ( HWND hwnd, long id, HWND hwndCtl, long codeNotify )
{

	short           nReturn;        /* General Fn Return Value      */
	HINSTANCE       hinst;          /* Module Instance Handle       */
	char            szAppSetupName  [ _MAX_PATH + 1 ];

	short           nChar;
	char            *szPtr;
	char            szSetup [ _MAX_FNAME + 1 ];

	DWORD			dwLastTime;
	DWORD			dwTemp;
	RECT            rectWindow;
	static OPENFILENAME	ofnData;      /* for GetSaveFileName Func */
	char szTemp [ MAX_PATH + 1];
	static BOOL bDirSpecified = FALSE;
	char  szDataFilter [30];
	short   nResult;
	char szExt [4];
   
	hinst = GetWindowInstance ( hwnd );


	switch ( id ) {


		case IDC_FILEEDIT :

			/**  Notification Codes to act upon : EN_CHANGE
			**/
			if ( ( hwndCtl == NULL ) || ( codeNotify != EN_CHANGE ) ) {
				/**     Edit box not created yet
				**/
				break;
			}


			/**     Seperate drive/directory from
			***     filename component.
			**/
			{
				char    szDrive 	[ _MAX_DRIVE + 1 ];
				char    szDir   	[ _MAX_DIR   + 1 ];
				char    szExt   	[ _MAX_EXT   + 1 ];
				char    szName		[ _MAX_FNAME + 1 ];
				char    szEditBoxText	[ _MAX_PATH  + 1 ];
				char    szNewPath       [ _MAX_PATH  + 1 ];

				Edit_GetText ( hwndCtl, szEditBoxText, _MAX_PATH );
				_splitpath ( szEditBoxText, szDrive, szDir, szName, szExt );

				/**     Add default extension
				**/
				if ( strlen ( szExt ) == 0 ) {

					switch ( settings.wFileFormat ) {
						case ( COMMA_SEPARATED ) :
							sprintf ( szExt, ".%s", DEF_DATAFILE_EXT );
							break;
						default :
							sprintf ( szExt, ".%s", DEF_COMBINED_EXT );
							break;
					}
				}

				sprintf ( szNewPath 	     , "%s%s",   szDrive, szDir );
				sprintf ( settings.szDataFile, "%s%s"  , szName,  szExt );
				/** ???? **/


				if ( strlen ( szNewPath ) > 0 ) {

					/**     A new path has also be entered which
					***     should override the current path.
					**/
					strcpy ( settings.szDataDirectory, szNewPath );
				}
			}
			/**     Remove any trailing '\' from the directory name
			**     I have chosen NOT to store this...
			**/
			if ( settings.szDataDirectory
			      [ strlen ( settings.szDataDirectory ) - 1 ] == '\\' ) {


				settings.szDataDirectory [ strlen
				       ( settings.szDataDirectory ) - 1 ] = '\0';

			}

			UpdateStatusBar ( hwnd );

			break;


		case IDC_START :

			/**  Notification Codes to act upon : BN_CLICKED
			**/

			/**     Record Time Started both as
			***     calender time which may be converted
			***     to hours/min/secs and as millisecs using
			***     GetMillisecondCount, this will be used
			***     to determine when a new scan should commence.
			***
			***     Get calender time from our 'clkRealTime' struct
			***     The time stored in this structure compensates
			***     for GMT and daylight saving.
			**/
			logInfo.tmCalenderTimeStarted = clkRealTime.timeNow;


			/**     Prepare the data file and header ready for
			***     storage of the logged data....
			***
			**/
			if ( settings.bLogDataToDisk == TRUE ) {


				nReturn = CreateDataFile ( TRUE );
				if ( nReturn != OK ) {

					/**     File Error...
					**/
					if ( nReturn != ERR_USER_ABORTED ) {

						ATKMessageBox ( hinst, hwnd,
						nReturn, NULL,
						     MB_OK | MB_ICONEXCLAMATION );
					}
					/**     Exit the IDC_START case
					***     statement, we will NOT
					***     be logging any data...
					**/
					break;
				}
			}


			/**     Set 'Time Started' after creating the file,
			***     otherwise the logger will miss a few scans
			***     while the user decides whether to overwrite
			***     append etc....
			**/
			logInfo.dwTimeStarted  = GetIMLMillisecondCount ( );


			/**     Explicitly Set the Time of Last Scan
			***     to force the log routine to start
			***     logging as soon as possible
			**/

			logInfo.dwTimeLastScan = logInfo.dwTimeStarted -
			                                settings.dwScanInterval;


			/** 5.04 mod. If we are using periodic files then try and
			*** force readings to be taken on the hour or minute as
			*** appropriate.
			**/

			if ( settings.bPeriodicFiles == TRUE ) {
			

				/** Scan Intervals of less than 1 sec caused problems
				*** because of divide by zero. Anything less than
				*** 1 sec treat as 1 sec.
				**/

				if ( settings.dwScanInterval < 1000 ) {
					dwTemp = 1000;
				} else {
					dwTemp = settings.dwScanInterval;
				}
			
				/** Can only do it if the scan interval divides exactly
				*** into the day length.
				**/
				
				if ( SECS_PER_DAY % ( dwTemp / 1000 ) == 0 ) {
					
					/** Force a clock update **/
					ServiceRealTimeClock ( hwnd);
					
					/** Calculate when the last reading should have
					*** been taken for a round number situation.
					**/
					dwLastTime = clkRealTime.timeNow % 
						   ( dwTemp / 1000 );

					/** Set the last scan time to one whole scan ago +
					*** the residual amount. This causes an immediate
					*** reading then followed by round number scans.
					**/
					logInfo.dwTimeLastScan = logInfo.dwTimeLastScan
						                  - 1000 * dwLastTime ;

				}

			}
			

			/**     Do NOT allow the Hardware configuration
			***     to change while we are logging............
			**/
			nReturn = PreventSetupConfiguration
					( hwnd, &conninfo.imsSetup );

			if (  nReturn != OK )  {
				/**     Cannot continue without setup.
				***     'PreventSetupConfiguaration' will
				***     have reported the error to the user.
				**/
				break;

			} else if ( conninfo.nNumConnected == 0 ) {

				/**     All the connected channels have been
				***     disabled. Report the error and prevent
				***     the user attempting to restart logging.
				**/
				ATKMessageBox ( hInst, hwnd,
					ERR_ALL_CONNECTIONS_MISSING,
						   NULL, MB_OK | MB_ICONEXCLAMATION );

				if ( settings.bLogDataToDisk == TRUE ) {

					/**     Close the file again...
					**/
					CloseDataFile ( TRUE );
				}
				/**     It is NOW safe to edit the Hardware
				***     configuration
				**/
				AllowSetupConfiguration ( hwnd, &conninfo.imsSetup );

				break;

			}


			/**     Allow logging by setting the appropriate
			***     flag in the logOnfo structure. This is checked
			***     in the GetMessage loop of WinMain.
			**/
			logInfo.bIsLogging = TRUE;

			
			/**     If we are logging on change initialise
			***     the stored value.
			**/
			if ( settings.bLogOnChange ) {
				ReadFirstChannelChange ( hwnd );
			}


			/**     Remove any existing data.
			**/
			ClearHistoricBuffer ( );
			InvalidateRect ( hwnd, &rectLoggedData, FALSE );
			InvalidateRect ( hwnd, &rectLoggedTime, FALSE );


			/**     Turn the highlight On...
			**/
			InvalidateRect ( hwnd, &rectHighlight, FALSE );

			UpdateButtonBar ( hwnd );
			UpdateMenuBar   ( hwnd );
			UpdateStatusBar ( hwnd );


			/**	Give the Main Window the Focus since the
			***	Edit box has been disabled. We can pass the
			***	messages into the editbox from LoggerWndProc
			***	if need be.
			**/
			SetFocus ( hwnd );


			/** Start the timer **/
			SetTimer ( hwnd, SAMPLE_TIMER,settings.dwScanInterval,NULL);
			/** Force the first sample **/
			Logger_OnTimer ( hwnd, SAMPLE_TIMER );
			
			break;


		case IDC_STOP :

			/**  Notification Codes to act upon : BN_CLICKED
			**/
			KillTimer ( hwnd, SAMPLE_TIMER );


			logInfo.bIsLogging   = FALSE;
			logInfo.bIsPaused    = FALSE;

			/** Stop further auto-start
			*** facilities being used
			*** on subsequent 'Starts'
			**/
			logInfo.bAutoStarted = FALSE;


			logInfo.tmCalenderTimeStopped = clkRealTime.timeNow;



			if ( settings.bLogDataToDisk == TRUE ) {

				CloseDataFile ( TRUE );
			}

			/**     It is NOW safe to edit the Hardware
			***     configuration
			**/
			AllowSetupConfiguration ( hwnd, &conninfo.imsSetup );


			/**     Turn the highlight Off...
			**/
			InvalidateRect ( hwnd, &rectHighlight, FALSE );



			UpdateButtonBar ( hwnd );
			UpdateMenuBar   ( hwnd );
			UpdateStatusBar ( hwnd );

			/**	Give the EditBox the Focus since it is now
			**	re-enabled.
			**/
			SetFocus ( hEditBox );

			break;


		case IDC_PAUSE :

			/**  Notification Codes to act upon : BN_CLICKED
			**/
			logInfo.bIsPaused = TRUE;

			UpdateButtonBar ( hwnd );
			UpdateMenuBar   ( hwnd );
			UpdateStatusBar ( hwnd );

			break;

		case IDC_RESUME :

			/**  Notification Codes to act upon : BN_CLICKED
			**/


			/**     We do not want hr LogData rountine
			***     to panic when it sees how many scans have
			***     been missed whilst logging has been paused.
			***     For this reason reset 'dwTimeLastScan' to
			***     current time minus one scan interval. LogData
			***     will then believe the next scan is due now.
			**/
			logInfo.dwTimeLastScan = GetIMLMillisecondCount ( ) -
			                               settings.dwScanInterval;

			logInfo.bIsPaused = FALSE;


			UpdateButtonBar ( hwnd );
			UpdateMenuBar   ( hwnd );
			UpdateStatusBar ( hwnd );

			break;
	
		case IDC_LOG :

			logInfo.bLogPressed = TRUE;

			break;

		case IDM_LOADIMS :


			/**     Close the existing Setup...
			**/
			if ( conninfo.bIMSSetupLoaded == TRUE ) {

			/**     Free the IMS setup and IMSLib.DLL
			**/
				IMSUnlockSetup ( conninfo.imsSetup.szSetupName,
						conninfo.imsSetup.nSetupKey );

				conninfo.bIMSSetupLoaded 	   = FALSE;
				conninfo.imsSetup.szSetupPath [ 0 ] = '\0';
				conninfo.imsSetup.szSetupName [ 0 ] = '\0';
				conninfo.imsSetup.nSetupKey         = 0;

			}


			/**     Free the connections array
			**/
			if ( conninfo.hConnections != NULL ) {

				LocalUnlock ( conninfo.hConnections );
				LocalFree   ( conninfo.hConnections );

				conninfo.hConnections  = NULL;
				conninfo.nNumConnected = 0;

			}



			/**     Load a new Setup..........
			***     Assume that the setup path and name, if known,
			***     has already been copied into the IMSSETUP
			***     structure. If not known then the strings
			***     should be null terminated..
			**/
			nReturn = ATKLoadIMSSetup ( hinst, hwnd,
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
				break;

			} else {
				conninfo.bIMSSetupLoaded = TRUE;

				LoadString ( hInst, IDS_MAINTITLE, settings.szTitle,
					   MAX_TITLE );

				
				strcat ( settings.szTitle, " - " );
				strcat ( settings.szTitle, 
					    strupr ( conninfo.imsSetup.szSetupName ) );
				strcat ( settings.szTitle, ".IMS" );
				SetWindowText ( hwnd, settings.szTitle );
			
			
			}


			/**     Allow the Setup to be edited....
			***
			**/
			nReturn = AllowSetupConfiguration ( hwnd, &conninfo.imsSetup );



			/**     Now force the user to specify Connections..
			***     Drop thru......
			**/


		case IDM_INPUTS :


			nReturn = PreventSetupConfiguration ( hwnd,
							&conninfo.imsSetup );
			if ( nReturn != OK ) {

				/**     Cannot continue without setup.
				**/
				break;
			}


			/**     We may be changing the Connections Array
			***     so unlock it ready to be destroyed
			**/
			if ( conninfo.hConnections != NULL ) {

				LocalUnlock ( conninfo.hConnections );
				conninfo.pciConnections = NULL;

				/**     Clear the Buffer containing the
				***     historic data for the old connections
				**/
				ClearHistoricBuffer ( );
				InvalidateRect ( hwnd, &rectLoggedData, FALSE );
				InvalidateRect ( hwnd, &rectLoggedTime, FALSE );

			}


			ATKSelectChannels ( hinst,
				hwnd,
				conninfo.imsSetup.szSetupName,
				&conninfo.hConnections,
				&conninfo.nNumConnected,
				conninfo.nMaxConnections,
				conninfo.wChanTypes );

			/**     Lock the connections array
			***
			**/
			if ( conninfo.hConnections != NULL ) {

				conninfo.pciConnections = (PATKCHANINFO)
					LocalLock ( conninfo.hConnections );

			}


			/**     Establish the units of each connected channel
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



			/**     Allow the Setup to be edited....
			***
			**/
			nReturn = AllowSetupConfiguration ( hwnd, &conninfo.imsSetup );



			/**     Update the Channel Units on the Client
			**/
			InvalidateRect ( hwnd, &rectChanLabels, TRUE );


			/**     Remove any existing 'historic' data from the client
			**/
			ClearHistoricBuffer ( );
			InvalidateRect ( hwnd, &rectLoggedData, FALSE );
			InvalidateRect ( hwnd, &rectLoggedTime, FALSE );


			/**     Enable the Connections, Settings Menu Items etc.
			**/
			UpdateTitleBar  ( hwnd );
			UpdateButtonBar ( hwnd );
			UpdateMenuBar   ( hwnd );


			/**     Enable the scroll-bar with the correct range
			***     and initial position.
			**/
			ShowWindow ( hScrollBar, SW_SHOW );

			SetScrollRange ( hScrollBar, SB_CTL, 0,
 				( conninfo.nNumConnected - 1 ), TRUE );

			SetScrollPos   ( hScrollBar, SB_CTL,
				clntInfo.lFirstChanVisible, TRUE );

			break;



		case IDM_SETTINGS :


			DialogBox ( hinst, "SETTINGS",
		 		 hwnd, (DLGPROC) Settings_DlgProc );


			/**	Has the chars per column changed ?
			***	If it has changed the column width to
			***	accomodate the new size. nColumnWidth
			***	is the width in average characters.
			***	therefore accomodate 2 additional ave
			***	width chars incase its a 'fat' string.
			***	This should also ensures a space between
			***	adjacent columns..
			**/
			clntInfo.lColumnWidth = settings.wCharsPerColumn + 2;

			/**
			*** New Number of Visible Data Columns
			**/
			clntInfo.lNumColumnsVisible =
				( rectLoggedData.right - rectLoggedData.left )
				 / ( clntInfo.lAveTextWidth * clntInfo.lColumnWidth );

			/**	Include ONE partial Column
			**/
			if ( ( rectLoggedData.right - rectLoggedData.left )
				% ( clntInfo.lAveTextWidth *
					clntInfo.lColumnWidth ) != 0 ) {

				clntInfo.lNumColumnsVisible++;
			}



			/**	Invalidate the data to display it using the new
			***	column width.
			**/
			InvalidateRect ( hwnd, &rectLoggedData, FALSE );
			InvalidateRect ( hwnd, &rectLoggedTime, TRUE  );
			InvalidateRect ( hwnd, &rectHighlight,	FALSE );
			InvalidateRect ( hwnd, &rectChanLabels, TRUE );


			UpdateButtonBar ( hwnd );
			UpdateStatusBar ( hwnd );
                                        
                                        
			/**	The Window Title may have been changed by the user
			**/                                        
			SetWindowText ( hwnd,  settings.szTitle );

			break;


		case IDM_DATAFILE :


			/** Envoke the Common File Save Dialog
			*** First initialise the OPENFILENAME structure
			*** members.
			**/

			/** Form the 'list' of filter descs/filters
			*** seperating with \0 chars and terminating with
			*** a additional \0 character. We of course will
			*** only use a single filter...
			**/
			sprintf ( szDataFilter, "%s%c%s%c%c",
	              "Logger Files (*.WL)", '\0', "*.WL",'\0','\0' );

			/**	At first use we get the Working dir from the registry and
			*** use the passed in default file name. Otherwise the last used 
			*** values are in the variables.
			**/

			
			/** We may have got the dir from a SetUp **/
			if ( strlen ( settings.szDataDirectory ) > 0 ) {
				bDirSpecified = TRUE;
			}


			if ( bDirSpecified == FALSE ) {
				GetIMLDirPath ( settings.szDataDirectory, MAX_PATH );
				strcat ( settings.szDataDirectory, "\\Data");
				strcpy ( settings.szDataFile, "*.WL");
			}

			/** Make a local copy ???? **/
			strcpy ( szTemp, settings.szDataFile );

			/** Find the file extension **/
			GetFileExtension ( szExt );

			ofnData.lStructSize         = sizeof ( OPENFILENAME );
			ofnData.hwndOwner           = hwnd;
			ofnData.hInstance           = hInst;
			ofnData.lpstrFilter         = (LPSTR) szDataFilter;
			ofnData.lpstrCustomFilter   = (LPSTR) NULL;
			ofnData.nMaxCustFilter      = ATK_MAX_FILTER_SIZE;
			ofnData.nFilterIndex        = 0L;
			ofnData.lpstrFile           = szTemp;
			ofnData.nMaxFile            = sizeof ( szTemp );
			ofnData.lpstrFileTitle      = settings.szDataFile;
			ofnData.nMaxFileTitle       = sizeof (settings.szDataFile);
			ofnData.lpstrInitialDir     = settings.szDataDirectory;
			ofnData.lpstrTitle          = (LPSTR) NULL;
			ofnData.Flags               = OFN_HIDEREADONLY ;
			ofnData.nFileOffset         = 0;
			ofnData.nFileExtension      = 0;
			ofnData.lpstrDefExt         = szExt;
			ofnData.lCustData           = 0L;
			ofnData.lpfnHook            = NULL;
			ofnData.lpTemplateName      = NULL;

			nResult = GetSaveFileName ( &ofnData );

			if ( nResult == FALSE ) {
				break;
			}

			/** The file name has already been set. But we
			*** must dig out the directory.
			**/
			szTemp [ofnData.nFileOffset - 1] = 0;
			strcpy ( settings.szDataDirectory, szTemp );
			
			StripPeriodicPostFix ();
	
			bDirSpecified = TRUE;

			

			UpdateButtonBar ( hwnd );
			UpdateStatusBar ( hwnd );

			break;

		case IDM_FILEOPT :


			/**	Invoke 'Periodic Files' sub-dialog
			**/

			DialogBox ( hinst, "ADVANCED_FILE",
				hwnd, (DLGPROC) PeriodicFiles_DlgProc );


			UpdateButtonBar ( hwnd );
			UpdateStatusBar ( hwnd );

			break;




		case IDM_SAVE_SETUP :

			/**	Default to "*.WLG", without specifying
			***	a full path specification. The toolkit fn 
			***	should then use the current working directory.
			**/
			if ( strlen ( settings.szFileSpec ) == 0 ) {

				strcpy ( szAppSetupName, "*.WLG" );
			} else {
				strcpy ( szAppSetupName, settings.szFileSpec );

			}

			ATKSaveApplicationSetup ( hInst,
				hwnd,
				szAppSetupName,
				"wlg",
				"Logger Setup (*.wlg)",
				"*.wlg" );


			break;


		case IDM_RESTORE_SETUP :

			/**	Terminate the current Service name
			***	in preparation for a new service name
			***	stored in the Setup file.
			**/
			PTKTerminateDDEService ( );




			/**     Default to "*.WLG", without specifying
			***	a full path specification. The toolkit fn 
			***	should then use the current working directory.
			**/
			strcpy ( szAppSetupName, "*.WLG" );

			ATKLoadApplicationSetup ( hInst,
				hwnd,
				szAppSetupName,
				"wtb",
				"Logger Setup (*.wlg)",
				"*.wlg" );




			/**     Remove any existing logged data, chan names etc
			**/
			ClearHistoricBuffer ( );
			InvalidateRect ( hwnd, &rectLoggedData, TRUE );
			InvalidateRect ( hwnd, &rectLoggedTime, TRUE );
			InvalidateRect ( hwnd, &rectChanLabels, TRUE );



			/** Update Status, Buttons etc...
			**/
			UpdateTitleBar	( hwnd );
			UpdateButtonBar ( hwnd );
			UpdateMenuBar	( hwnd );
			UpdateStatusBar ( hwnd );

			/**	The Window Title may have been changed 
			**/                                        
			SetWindowText ( hwnd,  settings.szTitle );


			/**     Enable the scroll-bar with the correct range
			***     and initial position.
			**/
			ShowWindow ( hScrollBar, SW_SHOW );

			SetScrollRange ( hScrollBar, SB_CTL, 0,
					( conninfo.nNumConnected - 1 ), TRUE );

			SetScrollPos   ( hScrollBar, SB_CTL,
						clntInfo.lFirstChanVisible, TRUE );



			/**	Register the new DDE Service name...
			***
			**/
			nReturn = PTKInitializeDDEService ( hwnd,
					settings.szDDEServiceName );

			if ( nReturn != OK ) {

				ATKMessageBox ( hinst, hwnd,
					nReturn,
					(LPSTR) NULL,
					MB_OK | MB_ICONEXCLAMATION );

			}
                
			//
			//	Invoke WM_SIZE, This will ensure that all the variables
			//	used to paint the client have been correctly setup, including
			//	the number of columns which is derived from information from
			//	the restored setup.
			//
			GetWindowRect ( hwnd, &rectWindow );
			MoveWindow ( hwnd, rectWindow.left, rectWindow.top,
						( rectWindow.right  - rectWindow.left ) + 1,
						( rectWindow.bottom - rectWindow.top )  + 1, FALSE ); 
						
			MoveWindow ( hwnd, rectWindow.left, rectWindow.top,
						( rectWindow.right  - rectWindow.left ),
						( rectWindow.bottom - rectWindow.top ), FALSE );
               

		/**
		***	Invalidate AND clear all areas
		**/
		InvalidateRect ( hwnd, NULL, TRUE );
		break;



		case IDM_DDE :

			/**	Do we want this instance to support DDE
			***	and if so using what service name.
			**/


			/**	Terminate old service ( we can reconnect
			***	later if they cancel the operation )
			**/
			PTKTerminateDDEService ( );



			/**	Obtain a copy of the setup name
			***	Strip off the path and extension
			***	(reverse search for the last '\' character
			***	then copy upto the '.' of the exetension
			**/
			nChar = 0;
			if ( strlen ( settings.szFileSpec ) > 0 ) {

				szPtr = strrchr ( settings.szFileSpec, '\\' );
				if ( szPtr == (char *) NULL ) {

					/**	No path specified, must just be a fname
					**/
					szPtr = settings.szFileSpec;

				} else {

					/**	Skip the '\' character to advacnce on to
					***	the fname
					**/
					szPtr++;
				}

				/**	Now copy the fname upto the extension.
				**/
				while ( ( *szPtr != '\0' ) && ( *szPtr != '.' ) ) {

					szSetup [ nChar++ ] = *szPtr++;
				}
			}
			szSetup [ nChar ] = '\0';   /* Terminate our copy */


			/**	Obtain the DDE Service name and enabled flag.
			***
			**/
			nReturn = ATKGetDDEServiceName ( hinst, hwnd, "Logger",
					szSetup, settings.szDDEServiceName );

			/**	Register the new service name
			**/
			nReturn = PTKInitializeDDEService ( hwnd,
						settings.szDDEServiceName );

			if ( nReturn != OK ) {

				ATKMessageBox ( hinst, hwnd,
					nReturn,
					(LPSTR) NULL,
					MB_OK | MB_ICONEXCLAMATION );

			}
			break;


		case IDM_ABOUT :


			DialogBox ( hinst, "ABOUT",
					 hwnd, (DLGPROC) About_DlgProc );


			break;


		case IDM_HELP_CONTENTS  :

			bHelpLoaded = InvokeWinHelp ( hwndMain, HELP_CONTENTS,
									(DWORD) 0L );
			break;



		case IDM_EXIT  :


			FORWARD_WM_CLOSE ( hwnd, SendMessage );
			break;

		default :
			break;
	}


	return;

}










/**
***  Logger_OnClose     This function is called on recieving a
***			WM_CLOSE message. An application can prompt the
***			user for confirmation, prior to destroying the
***			window, by processing the WM_CLOSE message and
***			calling the DestroyWindow function only if the user
***			confirms the choice.
***
***
***  Arguments
***
***  hwnd               Handle of Window which may be called
***
***  Return Value       Void
***
**/
VOID Logger_OnClose ( HWND hwnd )
{

    if ( !nDDECloseCommand ) {
		if ( MessageBox ( hwnd, "Do you wish to close Logger ?",
					"Logger - Close",
					MB_YESNO | MB_APPLMODAL  ) == IDNO) {
			return ;
		}
	}

	if ( logInfo.bIsLogging ) {

		/**     We are still logging. Take any
		***     necessary action to shut down
		***     cleanly....
		**/

		/** Emulate Press of 'Stop' Button
		**/
		SendMessage ( hwnd, WM_COMMAND, (WPARAM) IDC_STOP,
				(LPARAM) MAKELONG (
				abutButtonInfo [ B_STOP ].hButtonWnd,
				BN_CLICKED ) );
	}


	if ( bHelpLoaded ) {

		/**    	Inform Help Application that we
		***	no longer require Help.
		**/
		InvokeWinHelp ( hwnd, HELP_QUIT, (DWORD) 0L );
	}

	bDestroy = TRUE;

	return;
}



/**
***  Logger_OnDestroy   This function is called on recieving a
***                     WM_DESTROY message.
***
***  Arguments
***
***  hwnd               Handle of Window being Destroyed.
***                     It may be assumed that whilst processing this
***			message any child Windows still exist.
***
***  Return Value       Void
***
**/
VOID Logger_OnDestroy ( HWND hwnd )
{



	/**	Revoke our DDE services
	**/
	PTKTerminateDDEService ( );
	FreeDDECallBackProc ( );

	/**	Free global memory that be in use the application
	***
	**/
	ATKDestroyMonitorTable ( );  /* Destroy toolkit table if exists */



	/**     Delete any Objects.
	***
	**/
	DeleteFont ( hSansSerif_8  );
	DeleteFont ( hSansSerif_10 );

	DeleteObject ( (HGDIOBJ) hpenStandardText );

	/**	Release brush used for background before destroying
	**/
	SetClassLong ( hwnd, GCL_HBRBACKGROUND, ( COLOR_WINDOW + 1 ) );

	DeleteObject ( (HGDIOBJ) hbrushStandardBk );


	/**     Free the connections array
	**/
	if ( conninfo.nNumConnected > 0 ) {

		LocalUnlock ( conninfo.hConnections );
		LocalFree   ( conninfo.hConnections );
	}

	/**	Free the IMS setup and IMSLib.DLL
	**/
	IMSUnlockSetup ( (LPSTR) conninfo.imsSetup.szSetupName,
					conninfo.imsSetup.nSetupKey );

	CloseIMLInterface ( conninfo.nIMLInterface );

	DestroyHistoricBuffer ( );

	PostQuitMessage ( 0 );
	return;
}



/**
***  Logger_OnHScroll   This function is called on recieving a
***                     WM_HSCROLL message.
***
***  Arguments
***
***  hwnd               Handle of parent Window
***  hwndctl            Handle of Scroll-bar
***  code               SB_BOTTOM         Scroll to lower right.
***
***                     SB_ENDSCROLL      End scroll.
***
***                     SB_LINEDOWN       Scroll one line down.
***
***                     SB_LINEUP         Scroll one line up.
***
***                     SB_PAGEDOWN       Scroll one page down.
***
***                     SB_PAGEUP         Scroll one page up.
***
***                     SB_THUMBPOSITION  Scroll to absolute position.
***                                       The current position is provided
***                                       in 'pos'
***
***                     SB_THUMBTRACK     Drag thumb to specified position.
***                                       The current position is provided
***                                       in 'pos'
***
***     pos             The current position (see above)
***
***  Return Value       Void
***
**/
VOID Logger_OnHScroll ( HWND hwnd, HWND hwndCtl, long code, long pos )
{

	long   lOriginalPos;


	/**     Remember the current Position of the thumb
	**/
	lOriginalPos = clntInfo.lFirstChanVisible;




	switch ( code ) {

		case SB_TOP :

			/**     Channel 0 should be displayed in the
			***     first column of the client.
			**/
			clntInfo.lFirstChanVisible = 0;
			break;

		case SB_BOTTOM :

			/**     Last Channel should be displayed in the
			***     first column of the client.
			**/
			clntInfo.lFirstChanVisible = conninfo.nNumConnected - 1;
			break;

		case SB_PAGEDOWN :

			/**     Scroll NEXT PAGE into view.
			***     Include end check to prevent scrolling of the end.
			**/
			clntInfo.lFirstChanVisible =
				min ( clntInfo.lFirstChanVisible +
						clntInfo.lNumColumnsVisible,
				 			conninfo.nNumConnected - 1 );
			break;

		case SB_LINEDOWN :

			/**     Scroll NEXT channel into view.
			***     Include end check to prevent scrolling of the end.
			**/
			clntInfo.lFirstChanVisible =
					min ( clntInfo.lFirstChanVisible + 1,
							 conninfo.nNumConnected - 1 );

			break;

		case SB_PAGEUP :

			/**     Scroll NEXT PAGE into view.
			***     Include end check to prevent scrolling of the end.
			**/
			clntInfo.lFirstChanVisible =
				max ( 0, clntInfo.lFirstChanVisible -
						clntInfo.lNumColumnsVisible );
			break;

		case SB_LINEUP :

			/**     Scroll PREVIOUS channel into view.
			***     Include end check to prevent scrolling of
			***	the beginning.
			**/
			clntInfo.lFirstChanVisible =
					max ( 0, clntInfo.lFirstChanVisible - 1 );

			break;


		case SB_THUMBPOSITION :
		case SB_THUMBTRACK :

			clntInfo.lFirstChanVisible = (short) pos;

		default :
			break;
	}

	SetScrollPos ( hScrollBar, SB_CTL, clntInfo.lFirstChanVisible, TRUE );

	/**     Invalidate the client which has been 'scrolled'
	***     Check first that the thumb HAS moved.
	**/
	if ( lOriginalPos != clntInfo.lFirstChanVisible ) {

		InvalidateRect ( hwndMain, &rectChanLabels, TRUE );
		InvalidateRect ( hwndMain, &rectLoggedData, TRUE );
	}
}




DWORD Logger_OnMenuChar ( HWND hwnd, long ch, long flags, HMENU hMenu )
{
	switch ( (char) ch ) {

		case 's' :
		case 'S' :
			if ( IsWindowEnabled
					( abutButtonInfo [ B_START ].hButtonWnd  ) ) {

				SendMessage ( hwnd, WM_COMMAND,
						(WPARAM) IDC_START, 0L );
			}
			return ( MAKELONG ( 0, 1 ) );

		case 't' :
		case 'T' :
			if ( IsWindowEnabled
				( abutButtonInfo [ B_STOP ].hButtonWnd ) ) {

				SendMessage ( hwnd, WM_COMMAND,
						(WPARAM) IDC_STOP, 0L );

			}
			return ( MAKELONG ( 0, 1 ) );

		case 'p' :
		case 'P' :      
			if ( IsWindowEnabled
				( abutButtonInfo [ B_PAUSE ].hButtonWnd ) ) {

				SendMessage ( hwnd, WM_COMMAND,
						(WPARAM) IDC_PAUSE, 0L );

			}
			return ( MAKELONG ( 0, 1 ) );

		case 'r' :
		case 'R' :
			if ( IsWindowEnabled
				( abutButtonInfo [ B_RESUME ].hButtonWnd ) ) {

				SendMessage ( hwnd, WM_COMMAND,
						(WPARAM) IDC_RESUME, 0L );

			}
			return ( MAKELONG ( 0, 1 ) );

		case 'l' :
		case 'L' :
			if ( IsWindowEnabled
				( abutButtonInfo [ B_LOG ].hButtonWnd ) ) {

				SendMessage ( hwnd, WM_COMMAND,
						(WPARAM) IDC_LOG, 0L );

			}
			return ( MAKELONG ( 0, 1 ) );



		default :       /** Discard and bleep **/
			return ( MAKELONG ( 0, 0 ) );
	}

}


/**     Logger_OnSetFocus
***
***     This function is called on a WM_SETFOCUS message.
***     We will use it to Prevent the edit box losing
***     the Focus to the main application window.
***
***	Arguments :
***		hwnd - 		Handle of Window receiving focus.
***		hwndOldFocus -  Handle of Window losing the focus.
***
***	Return Value : void
***
**/
VOID Logger_OnSetFocus ( HWND hwnd, HWND hwndOldFocus )
{
	/**	Only the Edit box is permitted the Focus whilst not logging.
	***	If this is the case return the focus back to the edit control.
	***	If logging, the edit box is disabled  so messages must be 
	***	recieved by the main window and either processed or passed 
	***	indirectly to the edit box. 
	**/
	if ( !logInfo.bIsLogging ) {
		SetFocus ( hEditBox );
	}
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
***                          If the argument is a logger setup file
***                         identified by the extension '.WLG' then
***                          the fn: LoadAppSetup, will be envoked.
***
***     Arguments :
***
***      hwndMain            Handle of Application Window. May be NULL.
***     lpszCmdLine          The command line, excluding the program name.
***                          This is the string passed into WinMain.
***
***     Return Value :
***                          VOID
***
**/
void ExtractCmdLineArg ( HWND hwndMain, LPSTR lpszCmdLine )
{
	char	szPath  [ _MAX_PATH  + 1 ];
	char	szDrive [ _MAX_DRIVE + 1 ];
	char	szDir   [ _MAX_DIR   + 1 ];
	char	szFName [ _MAX_FNAME + 1 ];
	char	szExt   [ _MAX_EXT   + 1 ];
	char	szDummy [ _MAX_FNAME + 1 ];
	char*   ptszPath;

	OFSTRUCT        ofCmdLineFile;
	short           nReturn;
	HINSTANCE       hInstance;

	char    *szFlag;
	char    *szEndFileSpec;
	BOOL    bAutoStartRequested = FALSE;



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



	/**	Search for flags appended onto
	***	the end of the command line
	**/
	szFlag = strchr ( szPath, '/' );
	if ( szFlag != NULL ) {

		if ( (char) toupper ( (int) szFlag [ 1 ] ) == 'A' ) {

			bAutoStartRequested = TRUE;
		}


	}


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
	***	a file does actually exist.
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




		SendMessage ( hwndMain, WM_COMMAND,
				(WPARAM) IDM_LOADIMS, (LPARAM) 0L );


		return;

	} else {

		PTKTerminateDDEService ( );




		/**	Load the specified Logger setup file
		***
		**/
		nReturn = LoadAppSetup ( szPath );



		/**	If DDE support is required, register now...
		***
		**/
		nReturn = PTKInitializeDDEService ( hwndMain,
					settings.szDDEServiceName );

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
		ShowWindow ( hScrollBar, SW_SHOW );

		SetScrollRange ( hScrollBar, SB_CTL, 0,
					( conninfo.nNumConnected - 1 ), TRUE );


		clntInfo.lFirstChanVisible = 0;
		SetScrollPos   ( hScrollBar, SB_CTL,
					clntInfo.lFirstChanVisible, TRUE );

		/**	Did the user specify /Auto on the end of
		***	the command line. If so Commence logging
		***	immediately
		**/
		if ( bAutoStartRequested == TRUE ) {


			/**	Note we have been autostarted
			**/
			logInfo.bAutoStarted = TRUE;


			/**	Emulate a 'Start' button press
			**/
			PostMessage ( hwndMain, WM_COMMAND,
				(WPARAM) IDC_START, (LPARAM) 0L );
		}



	}

	/**	Menus, Client and Title should be updated by caller
	***	and return of this function....
	**/
	return;
}









/**
***     Function :      UpdateTitleBar
***
***                     Originally Added Hardware Setup file to Application Name
***                     on the Title bar.
***                     Now just displays 'settings.szTitle';
***
***     Arguments :
***     hwndMain      Handle of the Application Window.
***
***     Return :        VOID
***
**/
VOID    UpdateTitleBar ( HWND hwndMain )
{

	SetWindowText ( hwndMain, settings.szTitle );

	return;
}




VOID Logger_OnGetMinMaxInfo ( HWND hwnd, MINMAXINFO *lpMinMaxInfo )
{
	short   nMinWidth, nMinHeight;


	/**     Minimum width should display Time + 1 column of data.
	***     Minimum height should display atleast one row of data.
	**/
	nMinWidth  = ( clntInfo.lAveTextWidth * MAX_DATA_CHARS * 3 );
	nMinHeight = ( clntInfo.lTextHeight   * 15 );

	lpMinMaxInfo->ptMinTrackSize.x = nMinWidth;
	lpMinMaxInfo->ptMinTrackSize.y = nMinHeight;

	return;
}



/*********************************************
*** Either the sample timer or the 1 second background
*** timer can bring us here
**/


VOID Logger_OnTimer ( HWND hwnd, long id )
{

	
	bTimerExpired = TRUE;
				
}


void ServiceBackground ( void )

{
	short nReturn;

	ServiceRealTimeClock ( hwndMain );
				
								
	if ( ( logInfo.bIsLogging ) && ( !logInfo.bIsPaused ) ) {

		/**     Log New Data if Required.
		***     Shut things down if the attempt to log fails
		**/
		if ( ( nReturn = LogData ( hwndMain ) ) != OK ) {

			/**  Shut down the logging Process
			**/
			logInfo.bIsLogging = FALSE;

			logInfo.tmCalenderTimeStopped =
						clkRealTime.timeNow;

			if ( settings.bLogDataToDisk == TRUE ) {

				CloseDataFile ( TRUE );
			}


			/**     It is NOW safe to edit the Hardware
			***     configuration
			**/
			AllowSetupConfiguration ( hwndMain, &conninfo.imsSetup );

			/**     Turn the highlight Off...
			**/
			InvalidateRect ( hwndMain, &rectHighlight, FALSE );
 


			UpdateButtonBar ( hwndMain );
			UpdateMenuBar   ( hwndMain );
			UpdateStatusBar ( hwndMain );

			/**	The Window Title may have been changed by the user
			**/                                        
			SetWindowText( hwndMain, settings.szTitle );


			/**  Finally, now we have safely
			***  closed down the logging...
			***  tell the user whats happened
			**/
			ATKMessageBox  ( hInst, hwndMain,
			     (WORD) nReturn,
			      NULL, MB_OK | MB_ICONHAND );
		}
	}

}