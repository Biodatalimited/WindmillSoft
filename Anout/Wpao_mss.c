/**
***	WPAO_MSS.c		ATK-PANELS
***				Window Creation & Message Handling
***				for AnalogOut Program.
***
***	Andrew Thomson
***
***	This module is responsible for creating the Application
***	Window and processing messages for that window.
***	Additionally this module is used to initialize data structures
***	and load/open DLL's used by the application.
***
***
***	Associated Files :	Source file with the prefix 'WPAO'
***				Application toolkit files.
***				CCWIN.obj ( copy protection)
***				COMMSLIB/IMSLIB library files.
***
***	Version :		GENAPPS-PANELS-P-4.07
***
***
***	Version History :	2.00 First version of new style panels
***				2.01 Beta Test Version
***				2.02 Minor changes following tests.
***				2.03 Rebuilt with ATK-P-1.02
***
***				4.00 Cycling added to read inputs
***				     periodically incase they have
***				     been changed on a IML alarm.
***				     Built using ATKIT-P-2.00
***
***				     Uses CopyControl V1.57 Copy protection
***				     with improved Windows Support
***
***				4.01 DDE Executes and Alarm support added
***				4.02 Correction to MoveWindow on Restore
***				     Correction to draw routine to prevent
***				     editbox being updated with new value
***				     IF it has or is being edited.
***				     Does not now check if setup has changed
***				     when reading.
***				4.03 New code to allow/disable editing of
***				     IMS setup consistant with new DDE panel.
***				4.04 Performs REP if AREAD fails.
***				4.05 Further correction to prevent edited
***				     values being updated which was missed
***				     on 4.02 ( see UpdateChannelEdittext fn )
***				4.06 Rebuilt using Copy Control 1.66
***				     for Win95 - No Other Changes 
***				4.07 Rebuilt using C1.51
***				     New Windmill Software Copy-protection 
***				     replaces 'Copy Control' (see Protect.c)
***				     New Timer function replaces 'GetMillisecCount'
***				     (see ATKTimer.c)
***				5.00 Converted to 32 bit.
***             5.01 Rebuilt with corrected kit files
***
***             6.00 Compiled with new kit
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

#include "\Windmill 32\Compile Libs\IMLinter.h"	/* ...using the COMMSLIB Dynalink		 */
#include "\Windmill 32\Compile Libs\imslib.h"	/* ...using the IMSLIB Dynalink 		 */
#include "\Windmill 32\ATkit\ATKit.h"	/* Application Toolkit Header/Prototypes	 */
#include "WPAO_def.h"	/* Manifest Constants and Structure Declarations */
			/* Functions Prototypes 			 */



/**	Application Name and Title
***	Use Conditional Compilation Macros to define title...
**/
char		szAppName      [ ]   = "AnalogOut";

#if defined ( DEBUG_VERSION )
char		szDefTitle     [ ]   = "AnalogOut (Debug)";
#elif defined ( DEMO_VERSION )
char		szDefTitle     [ ]   = "Demonstration AnalogOut";
#else
char		szDefTitle     [ ]   = "AnalogOut";
#endif




/**	Useful Globals....
**/
HINSTANCE hInst 		= NULL; /* Program Instance Handle	    */
HWND	  hwndMain		= NULL; /* Program Window Handle	    */
BOOL	  bHelpLoaded		= FALSE;/* Has Application loaded Help	    */
BOOL	  bInitializationOK	= TRUE; /* WM_CREATE Processed succesfully  */
BOOL      bDestroy = FALSE;



/**	Fonts/Brushes and Bitmaps used by Application
**/
HFONT	hSansSerif_8;		/*  8 Point Sans Serif Font	*/
HFONT	hSansSerif_10;		/* 10 Point Sans Serif Font	*/
HBRUSH	hCtlBrush = NULL;	/* Brush for WM_CTRCOLOR	*/
HBITMAP hbmpExclaimation;	/* Bit map for Exclamation mask */
HBITMAP hbmpExclaimationMask;	/* ... and its mask.		*/
HPEN	hpenStandardText = NULL;/* Def Brush/pen for client	*/
HBRUSH	hbrushStandardBk = NULL;



/**	Divide the client area into regions.
**/
RECT	rectButtonBar;	/* Area occupied by Push-button Bar		    */
RECT	rectStatusBar;	/* Area occupied by Status Bar			    */
RECT	rectData;	/* Area occupied by Channel Name, Data & Units	    */
RECT	rectScrollBar;	/* Area occupied by Vertical Scroll Bar 	    */
RECT	rectStatusBarItem [ NUM_STATUS_BAR_ITEMS  ];   /* One RECT per Item */


/**	Further divide the data area into fields
**/
RECT	rectNames;	/* Area occupied by Channel names		    */
RECT	rectSymbols;	/* Exclamation Bitmap				    */
RECT	rectValues;	/* Area occupied by Data Values 		    */
RECT	rectUnits;	/* Area occupied by Channel units		    */






/**	Application Details
**/
PANELINFO	panAnalogOut;	/* General application Information	*/
CONTROLS	controls;	/* Details on Client Area Control sizes */
ATKCOLOURINFO	colors; 	/* COLORREF values for 3 alarm states	*/



/**	Channel Connection Information
**/
CONNECTIONS conninfo;


/**
***	Handles of client area controls.....
**/
HWND	hScrollBar;
HWND	hEditBox;
HWND	hModeText;
extern	BUTTONBAR abutButtonInfo [ NUM_BUTTONS ];
extern	HWND aheditTextRow [ MAX_SUPPORTED_DATAROWS ];



int nDDECloseCommand = FALSE; /* Indicates source of closure */

int bTimerExpired = 0; /* Set to 1 when the windows timer expires **/

/**	Export Function Pointers created by MakeProcInstance
**/
FARPROC lpfnAbout_DlgProc;





/**
***	WinMain 		Program Entry Point
***	
***	Arguments :
***		hInstance	Handle to program Insatnce	
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
	HWND		hwnd;
	MSG		msg;	
	WNDCLASS	wndclass;
	BOOL bNotQuit = TRUE;


	/**
	***	Determine if the user has a valid release copy
	***	of the software. If not put up MessageBox.
	**/
    if ( !IMSClear ( "Windmill" ) ) {

		/**	Copy protection has been invalidated
		**/
		MessageBox ( (HWND)hInstance, 
		"        This Program is NOT an Authorised Copy. \r\r    It will however function correctly for this Session. \r\r See Confiml Software Section for Authorisation Details",
					"Windmill AnalogOut Security Warning",
					 MB_OK );
						    		
	}





	/**	If the Window Class has not already been registered,
	***	register it now. Do not use hPrevInstance to determine
	***	if the class has already been registered support for this
	***	will be redrawn in future versions of the SDK.
	**/
	

	wndclass.style		    = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc	= (WNDPROC) AnalogOut_WndProc;
	wndclass.cbClsExtra	    = 0;
	wndclass.cbWndExtra	    = 0;
	wndclass.hInstance	    = hInstance;
	wndclass.hIcon		    = LoadIcon ( hInstance, 
							     (LPSTR) szAppName );
	wndclass.hCursor	    = LoadCursor ( NULL, IDC_ARROW );
	wndclass.hbrBackground	= (HBRUSH) ( COLOR_WINDOW + 1 );
	wndclass.lpszMenuName	= (LPSTR) szAppName; 
	wndclass.lpszClassName	= (LPSTR) szAppName; 

	if ( !RegisterClass ( (LPWNDCLASS) &wndclass ) ) {
		return FALSE;

	}

		

	/**	Calculate default area to be one slightly
	***	greater than one half of the screen width ( AnalogOut
	***	needs to be wider than the other panels )
	***	and one half the screen height...
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
			      ( GetSystemMetrics ( SM_CXSCREEN ) * 7 ) / 12,
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

	}

	hwndMain = hwnd;	/** Store the Window Handle	**/
	hInst	 = hInstance;	/** Store the Instance		**/


	/**
	***	If a command line argument has been specified
	***	process this.
	***
	**/
	ExtractCmdLineArg ( hwnd, lpszCmdLine );
	UpdateTitleBar ( hwnd ); 


 

	ShowWindow   ( hwnd, nCmdShow );
	UpdateWindow ( hwnd );



	UpdateButtonBar ( hwnd );
    UpdateMenuBar   ( hwnd );
    UpdateStatusBar ( hwnd );
	UpdateScrollBar ( hwnd );



	/**	Remove all outstanding messages from the
	***	message queue and yield control to other
	***	applications. Cyclic updating of screen performed
	***	by windows timer routine
	**/
	while ( bNotQuit ) {

		WaitMessage ();

		/** Service messages until the queue is empty **/
		while ( PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE )) {
			
			if ( msg.message == WM_QUIT ) { 
				bNotQuit = FALSE;
			}
			TranslateMessage ( &msg );
			DispatchMessage  ( &msg );
		
		}

		/** If one of the messages was the timer expiring
		*** we can now service it at a time when the queue is
		*** empty.
		**/
		if (bTimerExpired == 1) {
			ServiceAutoCycle ();
			bTimerExpired = 0;
		}

		if ( bDestroy ) {
			DestroyWindow ( hwnd );			
		}

	}

	
	return ( msg.wParam );


}





/**
***	AnalogOut_WinProc		Application Message Handling Fn
***	
***	Arguments :
***		hwnd		Handle of Window Receiving Message
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
LRESULT CALLBACK AnalogOut_WndProc ( HWND hwnd, WORD wMsg,
					   WPARAM wParam, LPARAM lParam )
{
	LRESULT 	lresResult;


	switch ( wMsg ) {


	case WM_CREATE : 
		HANDLE_WM_CREATE ( hwnd, wParam, lParam, AnalogOut_OnCreate );
		break;
		
	case WM_SIZE   : 
		HANDLE_WM_SIZE	 ( hwnd, wParam, lParam, AnalogOut_OnSize );
		break;
		
	case WM_PAINT  : 
		HANDLE_WM_PAINT  ( hwnd, wParam, lParam, AnalogOut_OnPaint );
		break;
		
	case WM_COMMAND: 
		HANDLE_WM_COMMAND( hwnd, wParam, lParam, AnalogOut_OnCommand );
		break;
		
	case WM_VSCROLL:
		HANDLE_WM_VSCROLL( hwnd, wParam, lParam, AnalogOut_OnVScroll );
		break;


	case WM_CTLCOLORDLG :

		/**  Return the brush handle is used...
		**/
		lresResult = HANDLE_WM_CTLCOLORDLG ( hwnd, wParam, lParam,
							AnalogOut_OnCtlColor );

		if ( lresResult == (LRESULT) NULL ) {

			/**	Not processed, pass onto DefWindowProc
			**/
			return DefWindowProc ( hwnd, wMsg, wParam, lParam );
		} else {

			return ( lresResult );
		}
		break;

	case WM_CLOSE  : 
		HANDLE_WM_CLOSE  ( hwnd, wParam, lParam, AnalogOut_OnClose );
		break;
		 
	case WM_DESTROY: 
		HANDLE_WM_DESTROY ( hwnd, wParam, lParam, AnalogOut_OnDestroy);
		break;
		
	case WM_MENUCHAR:
		/**	This handles the Keyboard I/F for the 
		***	Button Bar controls. The return value MUST 
		***	be passed on the default handler to inform
		***	Windows the 'Menus' have been processed.
		**/
		return	HANDLE_WM_MENUCHAR ( hwnd, wParam, lParam,
							AnalogOut_OnMenuChar );
		break;
		
	case WM_SETFOCUS:
		HANDLE_WM_SETFOCUS ( hwnd, wParam, lParam,
							AnalogOut_OnSetFocus );
		break;
		
	case WM_KEYDOWN :
		HANDLE_WM_KEYDOWN  ( hwnd, wParam, lParam,
						AnalogOut_OnKeyDown );
		break;
		
	case WM_KEYUP :
		HANDLE_WM_KEYUP  ( hwnd, wParam, lParam,
						 AnalogOut_OnKeyUp );
		break;



	case WM_GETMINMAXINFO: HANDLE_WM_GETMINMAXINFO
			( hwnd, wParam, lParam, AnalogOut_OnGetMinMaxInfo );
		break;


	case WM_READCHAN : HANDLE_WM_READCHAN
			( hwnd, wParam, lParam, AnalogOut_OnReadChan );

		break;


	case WM_WRITECHAN : HANDLE_WM_WRITECHAN
			( hwnd, wParam, lParam, AnalogOut_OnWriteChan );

		break;

	case WM_TIMER :
		bTimerExpired = 1;
		break;

    default :
		return DefWindowProc ( hwnd, wMsg, wParam, lParam );
	}
	return ( 0L );
}






/**
***  Function		AnalogOut_OnCreate
***
***  Description	This function is called on recieving a
***			WM_CREATE message. It defaults static structure
***			fields and other globals, establishes links to
***			the IMS and COMMS libraries and creates fonts, brushes
***			and client area controls.
***
***  Arguments
***
***  lpCreateStruct	Points to a CREATESTRUCT containing information about the window
***			being created. The members of the CREATESTRUCT
***			structure are identical to the parameters of the
***			CreateWindowEx function.
***
***  Return Value	TRUE  - Continue with Window Creation
***			FALSE - Abort Window Creation.
***
***			also sets Global bInitializationOK to TRUE
***			if successful or false if not.
***
**/
BOOL AnalogOut_OnCreate ( HWND hwnd, CREATESTRUCT * lpCreateStruct )
{
	short		nReturn;	/* General Fn Return Value	*/
	HINSTANCE	hinst;		/* Module Instance Handle	*/
	HDC		hdcDisplay;	/* Device Context for Screen	*/

	short		nHorzPixelsPerInch;
	short		nVertPixelsPerInch;
	short		nFontHeight;
	short		nFontWidth;


	LOGBRUSH	lbrCtlBrush;	/* For Control Background color */





	hinst = GetWindowInstance ( hwnd );



	/**	Default structure fields
	**/
	conninfo.nNumConnected	= 0;
	conninfo.nMaxConnections = MAX_APPLICATION_CONNECTIONS;
	conninfo.wChanTypes	= TYPE_O | TYPE_V;
	conninfo.hConnections	= NULL;
	conninfo.pciConnections = NULL;
	conninfo.lpodOutputData = NULL;

	conninfo.bIMSSetupLoaded = FALSE;

	/**	We may be able to get the Setup Path from the
	***	latter.....
	**/
	conninfo.imsSetup.szSetupPath [ 0 ] = '\0';
	conninfo.imsSetup.szSetupName [ 0 ] = '\0';
	conninfo.imsSetup.nSetupKey	    = 0;



	/**	The application's status etc.
	**/
	panAnalogOut.nStatus		 = STATUS_NOSETUP;
	panAnalogOut.nNumDDELinks	 = 0;
	panAnalogOut.nFocusChannel	 = 0; /* Give first channel the focus */
	panAnalogOut.dwMillisecsInterval = 5000L; /* Default 5 second auto */
	panAnalogOut.nMode		 = MODE_IMMEDIATE;
	panAnalogOut.szFileSpec [ 0 ]	 = '\0';  /* No .WAO setup loaded  */


	/**	New for version 4.01, DDE Support
	**/
	panAnalogOut.bDDEEnabled = FALSE;	    /* No DDE support required */
	panAnalogOut.szDDEServiceName [ 0 ] = '\0'; /* No DDE Service name yet */



	/**	Open The IMS Library ready for use by
	***	toolkit functions
	***
	**/




	nReturn = OpenIMLInterface ( &( conninfo.nIMLInterface ) ) ;

	if ( nReturn != OK ) {

		ATKMessageBox  ( hinst, hwnd, (WORD) nReturn,
				      szDefTitle, MB_OK | MB_ICONHAND );

		/**	Abort Window Creation...
		**/
		bInitializationOK = FALSE;
		return ( bInitializationOK );
	}




	/**	Establish the number of logical pixels units
	***
	**/
	hdcDisplay = GetDC ( hwnd );
	nHorzPixelsPerInch  = GetDeviceCaps ( hdcDisplay, LOGPIXELSX );
	nVertPixelsPerInch  = GetDeviceCaps ( hdcDisplay, LOGPIXELSY );
	ReleaseDC ( hwnd, hdcDisplay );



	/**	Read the default alarm colour scheme
	***	from the Windmill.ini file
	**/
	ATKLoadINIColours ( &colors );

	/**	The Panels can only use Solid colours...
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




	/**	Change the Window background colour to
	***	match the colour alarm colour scheme
	***
	***	Hmmm .. don't think you can do this
	**/
	hbrushStandardBk = CreateSolidBrush ( colors.crNormalBk );
	hpenStandardText = CreatePen ( PS_SOLID, 0, colors.crNormalText );

	SetClassLong ( hwnd, GCL_HBRBACKGROUND, (WORD) hbrushStandardBk );





	/**	Create Useful Fonts
	**/
 
	
	/**	We require an 8-Point Font ( a point is 1/72 inch )
	***	but add 2 extra 'points' for external leading. The  
	***	two has been determined by trial and error and may
	***	not be correct for all monitor types... 
	**/	   
	nFontHeight = ( nVertPixelsPerInch * ( 8 + 2 ) ) / 72;
	nFontWidth = 0; 	/* Default width */


	hSansSerif_8 = CreateFont (
		nFontHeight,	/* font height			*/
		nFontWidth,	/* character width		*/
		0,		/* escapement of line of text	*/
		0,		/* angle of baseline and x-axis */
		700,		/* font weight			*/
		0,		/* flag for italic attribute	*/
		0,		/* flag for underline attribute */
		0,		/* flag for strikeout attribute */
		ANSI_CHARSET,	/* character set		*/
		OUT_DEFAULT_PRECIS, /* output precision 	*/
		CLIP_DEFAULT_PRECIS,/* clipping precision	*/
		DEFAULT_QUALITY,    /* output quality		*/
		FF_SWISS | DEFAULT_PITCH,  /* pitch and family	*/
		"MS Sans Serif" );


	/**	10 Point Font ( again with 2 points for external leading )
	**/
	nFontHeight = ( nVertPixelsPerInch * 12 ) / 72;
	nFontWidth  = 0;



	hSansSerif_10 = CreateFont (
		nFontHeight,	/* font height			*/
		nFontWidth,	/* character width		*/
		0,		/* escapement of line of text	*/
		0,		/* angle of baseline and x-axis */
		400,		/* font weight			*/
		0,		/* flag for italic attribute	*/
		0,		/* flag for underline attribute */
		0,		/* flag for strikeout attribute */
		ANSI_CHARSET,	/* character set		*/
		OUT_DEFAULT_PRECIS, /* output precision 	*/
		CLIP_DEFAULT_PRECIS,/* clipping precision	*/
		DEFAULT_QUALITY,    /* output quality		*/
		FF_SWISS | DEFAULT_PITCH,  /* pitch and family	*/
		"MS Sans Serif" );


	/**	Store the text sizes of each font
	***	for use when creating Controls
	***
	***	hSansSerif_8  is used for the button-bar text
	***	hSansSerif_10 is used for the status-bar text
	**/
	GetFontMetrics ( hwnd, hSansSerif_8,
			&( panAnalogOut.lTextHeight ),
			&( panAnalogOut.lAveTextWidth ) );

	GetFontMetrics ( hwnd, hSansSerif_10,
			&( panAnalogOut.lLargeTextHeight ),
			&( panAnalogOut.lLargeTextWidth ) );

	/**	Calculate the height required for each data row.
	***	Since a channel button will be 1.5 chars allow
	***	two and a quarter chars for the row.
	**/
	panAnalogOut.lRowHeight = ( ( panAnalogOut.lTextHeight * 9 ) / 4 );

	/**   Determine the maximum number of rows which would be visible
	***   if the client were to be maximized. This number may then be
	***   used to create sufficient 'data area' push-buttons/editboxs.
	***   It is of little consequent if the fiqure is to large, this will
	***   create additional controls which will never be used.
	**/
	panAnalogOut.nMaxRowsVisible = (short) ( GetSystemMetrics ( SM_CYFULLSCREEN )
						   / panAnalogOut.lRowHeight );



	/**	Create a brush for client area control backgrounds
	***	(see WM_CTLCOLOR message processing)
	**/
	lbrCtlBrush.lbStyle =  BS_SOLID;
	lbrCtlBrush.lbColor =  GetSysColor ( COLOR_BTNFACE );
	lbrCtlBrush.lbHatch =  0;

	hCtlBrush = CreateBrushIndirect ( (LPLOGBRUSH) &lbrCtlBrush );



	/**	Load the Exclaimation bitmaps which will be used to
	***	inform the user that the value displayed is an edited
	***	value and not the true output.
	**/

	hbmpExclaimation =  LoadBitmap ( (HINSTANCE) GetWindowLong ( hwnd,
		      GWL_HINSTANCE ), (LPSTR) "ExclaimationBmap" );


	hbmpExclaimationMask = LoadBitmap ( (HINSTANCE) GetWindowLong ( hwnd,
		       GWL_HINSTANCE ), (LPSTR) "ExclaimationMaskBmap" );



	/**	Fill the CONTROLS structure.
	***	This contains sizes for buttons, the edit box,
	***	scroll bar and the button and status bars etc.
	***	The dimensions are determined by sizes of
	***	the above fonts, which will be used in the controls.
	**/
	SizeControls ( );


	/**
	***	Create Client Area Controls...
	**/
	CreateButtonBar ( hwnd );  /* Creates button bar controls   */
	CreateScrollBar ( hwnd );  /* Creates status text controls  */
	CreateDataArea	( hwnd );  /* Creates data area controls    */



	/**	Set Initial Button/Menu States
	***
	**/
	UpdateButtonBar ( hwnd );
	UpdateMenuBar	( hwnd );
	UpdateStatusBar ( hwnd );





	/**	Initialise the scroll bar range to zero.
	***	The scroll-bar will be set up properly once we
	***	have established the number of connections
	**/

	SetScrollRange ( hScrollBar, SB_CTL, 0, 0, FALSE );
	SetScrollPos   ( hScrollBar, SB_CTL, 0, FALSE );

	ShowWindow ( hScrollBar, SW_HIDE );


	/**	Register DDE support
	**/
	strcpy ( panAnalogOut.szDDEServiceName, DDE_SERVICE_NAME );
	nReturn = PTKInitializeDDEService ( hwnd,
					panAnalogOut.szDDEServiceName );


	if ( nReturn != OK ) {

		/**	No DDE available for this instance of AnalogOut.
		**/


	}


	/**	Start the background 5 second autocycle. This wont take
	***	effect until the application enters its idle mode i.e it
	***	has a loaded setup, and busy doing nothing in particular.
	**/
	//StartAutoCycle ( );

	SetTimer ( hwnd, 1, panAnalogOut.dwMillisecsInterval, NULL);

    /**     Return TRUE, allowing Window creation to be continued
	**/
	bInitializationOK = TRUE;
	return ( bInitializationOK );

}









/**	AnalogOut_OnSize
***
***	This function is called on a WM_SIZE message.
***
***	Arguments :
***
***	  state SIZEFULLSCREEN	Window has been maximized.
***
***		SIZEICONIC	Window has been minimized.
***		SIZENORMAL	Window has been resized, but neither
***				SIZEICONIC nor SIZEFULLSCREEN applies.
***
***		SIZEZOOMHIDE	Message is sent to all pop-up windows when
***				some other window is maximized.
***
***		SIZEZOOMSHOW	Message is sent to all pop-up windows when some
***				other window has been restored to its former
***				size.
***
***	  cx	The new client area Width
***	  cy	The new client area Height
***
***	Return Value : void
***
**/
VOID AnalogOut_OnSize ( HWND hwnd, long state, long cx, long cy )
{
	short	nRange; 	/* Recalcs scroll bar range		*/




	if ( state == SIZEICONIC ) {

		return; /* No action required if we can't see it */
	}



	panAnalogOut.lWidth	 = cx;
	panAnalogOut.lHeight = cy;



	/**	Using the new client area size
	***	determine the sizes of each sub-area of the client area.
	***	This are stored in globals: rectNames, rectButtonBar etc.
	***	and are used to selectivly invalidate and paint the client.
	**/
	ResizeClientRects ( hwnd, panAnalogOut.lWidth, panAnalogOut.lHeight );


	/**	Finally enable the scroll-bar with the correct range
	***	and initial position. ( If visible at all )
	**/
	nRange = max ( 0, conninfo.nNumConnected -
					panAnalogOut.nNumRowsVisible );


	SetScrollRange ( hScrollBar, SB_CTL, 0, nRange, TRUE );

	SetScrollPos ( hScrollBar, SB_CTL, 0, TRUE );


	/**	Only allow the user to see the Scrollbar
	***	if it has a valid range.
	**/
	if ( nRange > 0 ) {

		ShowWindow ( hScrollBar, SW_SHOW );
	} else {

		ShowWindow ( hScrollBar, SW_HIDE );
	}


	/**	Update the channel name text in the 'data area' controls
	***
	**/
	UpdateDataArea ( hwnd );



	return;

}




/**
***  Function		AnalogOut_OnCommand
***
***			This function is called on recieving a
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
***  hwnd		Handle of Window recieving the Command
***  id 		Identifies the menuitem or control
***  hwndCtl		Handle of control sending message ( Controls Only )
***  codeNotify 	The notification code. ( Controls Only )
***
***  Return Value	Void
***
**/
VOID AnalogOut_OnCommand ( HWND hwnd, long id, HWND hwndCtl, long codeNotify )
{

	short		nReturn;	/* General Fn Return Value	*/
	HINSTANCE	hinst;		/* Module Instance Handle	*/
	char		szAppSetupName	[ _MAX_PATH + 1 ];
	short		nRange;
	FARPROC 	lpfnAbout_DlgProc;
	short		nChannel;
	HCURSOR 	hcursorOld;
	short		nRowIndex;

	short		nChar;
	char		*szPtr;
	char		szSetup [ _MAX_FNAME + 1 ];
	HWND		hwndFocus;


	hinst = GetWindowInstance ( hwnd );


	switch ( id ) {


	case IDC_MODE_BUTTON :

		/**	Toggle the mode...
		**/
		panAnalogOut.nMode =
			( panAnalogOut.nMode == MODE_IMMEDIATE ) ?
					MODE_DEFERRED : MODE_IMMEDIATE;


		/**	If exiting 'Deferred mode' send any outstanding
		***	channel data to the hardware
		**/
		if ( panAnalogOut.nMode == MODE_IMMEDIATE ) {

			/**	Write the data...
			**/
			FORWARD_WM_WRITECHAN ( hwnd, 0, SendMessage );

		}


		UpdateButtonBar ( hwnd );

		/**	Prevent the button control keeping the focus if
		***	it recieves it, otherwise keyboard messages
		***	go the button handler and not the application.
		**/
		if ( GetFocus ( ) == abutButtonInfo [ B_MODE ].hButtonWnd ) {

			/**	Return the focus to the data area...
			**/
			SetChannelFocus ( hwnd, panAnalogOut.nFocusChannel );
		}


		break;


	case IDM_LOADIMS :

		/**	Close the existing Setup...
		**/
	    if ( conninfo.bIMSSetupLoaded == TRUE ) {


			/**  If a setup already exists, it will be
			***  enabled for editing by SetupIML. Stop
			***  this to keep things tidy and prevent
			***  configuration errors
			**/
			nReturn = PreventSetupConfiguration ( hwnd,
							&conninfo.imsSetup );
			if ( nReturn != OK ) {

				/**	Cannot continue without setup.
				**/
				break;
			}



			/**	Free the IMS setup and IMSLib.DLL
			**/
			IMSUnlockSetup ( conninfo.imsSetup.szSetupName,
						conninfo.imsSetup.nSetupKey );

			conninfo.bIMSSetupLoaded	   = FALSE;
			conninfo.imsSetup.szSetupPath [ 0 ] = '\0';
			conninfo.imsSetup.szSetupName [ 0 ] = '\0';
			conninfo.imsSetup.nSetupKey	   = 0;


			panAnalogOut.nStatus = STATUS_NOSETUP;



		}


		/**	Free the connections array
		**/
		if ( conninfo.hConnections != NULL ) {

			LocalUnlock ( conninfo.hConnections );
			LocalFree   ( conninfo.hConnections );

			conninfo.hConnections  = NULL;
			conninfo.nNumConnected = 0;

		}



		/**	Load a new Setup..........
		***	Assume that the setup path and name, if known,
		***	has already been copied into the IMSSETUP
		***	structure. If not known then the strings
		***	should be null terminated..
		**/
		nReturn = ATKLoadIMSSetup ( hinst, hwnd,
					&( conninfo.imsSetup ), 0 );
		if ( nReturn != OK ) {

			/**	Unable to load the setup, so dont
			***	attempt to make any connections
			**/
			conninfo.hConnections	= NULL;
			conninfo.nNumConnected	= 0;
			conninfo.bIMSSetupLoaded = FALSE;
			conninfo.imsSetup.szSetupPath [ 0 ] = '\0';
			conninfo.imsSetup.szSetupName [ 0 ] = '\0';
		/**	Remove old data and update the button
		***	status and scroll bars. Remember first
		***	to reallocate the areas occupied by the
		***	data to accomodate the new connections.
		**/
		ResizeClientRects ( hwnd, panAnalogOut.lWidth,
						panAnalogOut.lHeight );



			InvalidateRect ( hwnd, &rectData, TRUE );


			/**	Update Status, Button and Scrollbars etc...
			***	(These do not necessary require a redraw).
			**/
			UpdateTitleBar	( hwnd );
			UpdateButtonBar ( hwnd );
			UpdateMenuBar	( hwnd );
			UpdateStatusBar ( hwnd );
			UpdateScrollBar ( hwnd );

			/**	Update the channel name text in the
			***	'data area' controls
			***
			**/
			UpdateDataArea ( hwnd );

			break;

		} else {
			
			conninfo.bIMSSetupLoaded = TRUE;
			/**	Update the application's status
			**/
			panAnalogOut.nStatus = STATUS_BUSY;


		}


		/**	Allow the Setup to be edited....
		***
		**/
		nReturn = AllowSetupConfiguration ( hwnd, &conninfo.imsSetup );


		
		/**	Now force the user to specify Connections..
		***	Drop thru......
		**/


	case IDM_OUTPUTS :

		panAnalogOut.nStatus = STATUS_BUSY;

		nReturn = PreventSetupConfiguration ( hwnd,
							&conninfo.imsSetup );

		if ( nReturn != OK ) {

			/**	Cannot continue without setup.
			**/
			break;
		}


		/**	We may be changing the Connections Array
		***	so unlock it ready to be destroyed
		**/
		if ( conninfo.hConnections != NULL ) {

			LocalUnlock ( conninfo.hConnections );
			conninfo.pciConnections = NULL;


		}


		/**	Also  free the memory used for storing output
		***	data, we can recreate this once we have established
		***	how many connections have been made by the user
		**/
		if ( conninfo.lpodOutputData != NULL ) {

			GlobalUnlockPtr ( conninfo.lpodOutputData );
			GlobalFreePtr	( conninfo.lpodOutputData );
		}


		ATKSelectChannels ( hinst,
			hwnd,
			conninfo.imsSetup.szSetupName,
			&conninfo.hConnections,
			&conninfo.nNumConnected,
			conninfo.nMaxConnections,
			conninfo.wChanTypes );

		/**	Lock the connections array
		***
		**/
		if ( conninfo.hConnections != NULL ) {

			conninfo.pciConnections = (PATKCHANINFO)
					LocalLock ( conninfo.hConnections );

		}



		/**	For each connected channel we need an
		***	OUTPUTDATA structure to store new data awaiting
		***	transmission.
		**/
		if ( conninfo.nNumConnected != 0 ) {


			conninfo.lpodOutputData = GlobalAllocPtr (
			       GMEM_MOVEABLE, (DWORD) sizeof ( OUTPUTDATA )
						    * conninfo.nNumConnected );


			if ( conninfo.lpodOutputData != NULL ) {

				GlobalLockPtr ( conninfo.lpodOutputData );

				/** Default ALL fields in the array
				**/
				for ( nChannel = 0;
					nChannel < conninfo.nNumConnected;
								nChannel++ ) {

					conninfo.lpodOutputData [ nChannel ].
							bWriteData = FALSE;

					conninfo.lpodOutputData [ nChannel ].
							nWriteError = OK;

					conninfo.lpodOutputData [ nChannel ].
							szNewValue [ 0 ] = '\0';
				}

			} else {

				/**	Failed to assign and lock the memory
				***	Warn the user and fail the operation
				**/
				ATKMessageBox  ( hinst, hwnd,
					ERR_NO_GLOBAL_MEMORY,
					szDefTitle, MB_OK | MB_ICONHAND );

				/**	Destroy the connection array
				***	its no good without the
				***	OUTPUTDATA array.
				**/
				conninfo.nNumConnected = 0;

				LocalUnlock ( conninfo.hConnections );
				LocalFree   ( conninfo.hConnections );

				break;
			}

		}


		/**	Display an Hourglass while we extract
		***	limits/units, read all connections and
		***	move/hide/show all the channel pushbuttons.
		**/
		hcursorOld = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) );



		/**	Reset the first visible channel and focus channel's
		***	to be the first connected channel.
		***
		**/
		panAnalogOut.nFirstChanVisible	= 0;
		panAnalogOut.nFocusChannel	= 0;




		/**	Establish the units and limits of each
		***	connected channel.
		**/
		ATKChannelUnits ( hwnd, conninfo.nIMLInterface,
			conninfo.pciConnections, conninfo.nNumConnected );


		ATKChannelLimits ( hwnd, conninfo.nIMLInterface,
			conninfo.pciConnections, conninfo.nNumConnected );



		/**	Build the 'Monitor' table. If this exists
		***	we may use the AREAD command to determine
		***	if channels are in an alarm state.
		**/
		ATKBuildMonitorTable ( conninfo.imsSetup.szSetupName );

		/**
		***	Put all connected channels in
		***	both a 'read' and 'write' list.
		***	The read list will be needed when
		***	IML supports background control and
		***	alarm handling.
		**/
		ATKSetReadList ( hwnd, conninfo.nIMLInterface,
			conninfo.pciConnections, conninfo.nNumConnected );

		ATKSetWriteList ( hwnd, conninfo.nIMLInterface,
			conninfo.pciConnections, conninfo.nNumConnected );

		/**	Build a table stating which devices
		***	have alarm support. This is required 
		***	by the toolkit Read functions.
		**/
		ATKBuildMonitorTable ( conninfo.imsSetup.szSetupName );


		/**	Perform an initial read on ALLCHANNELS
		***	('AllChannels' = Last Connected Channel + 1 )
		**/
		FORWARD_WM_READCHAN ( hwnd,
				conninfo.nNumConnected, SendMessage );




		/**	Allow the Setup to be edited....
		***
		**/
		nReturn = AllowSetupConfiguration ( hwnd, &conninfo.imsSetup );



		panAnalogOut.nStatus = STATUS_IDLE;


		/**	Remove old data and update the button
		***	status and scroll bars. Remember first
		***	to reallocate the areas occupied by the
		***	data to accomodate the new connections.
		**/
		ResizeClientRects ( hwnd, panAnalogOut.lWidth,
						panAnalogOut.lHeight );



		InvalidateRect ( hwnd, &rectData, TRUE );


		/**	Update Status, Button and Scrollbars etc...
		***	(These do not necessary require a redraw).
		**/
		UpdateTitleBar	( hwnd );
		UpdateButtonBar ( hwnd );
		UpdateMenuBar	( hwnd );
		UpdateStatusBar ( hwnd );
		UpdateScrollBar ( hwnd );

		/**	Update the channel name text in the
		***	'data area' controls
		***
		**/
		UpdateDataArea ( hwnd );

		SetChannelFocus ( hwnd, panAnalogOut.nFocusChannel );


		/**	Enable the scroll-bar with the correct range
		***	and initial position.
		**/
		nRange = max ( 0, conninfo.nNumConnected -
					panAnalogOut.nNumRowsVisible );

		SetScrollRange ( hScrollBar, SB_CTL, 0, nRange, TRUE );

		SetScrollPos ( hScrollBar, SB_CTL, 0, TRUE );



		/**	Only allow the user to see the Scrollbar
		***	if it has a valid reange.
		**/
		if ( nRange > 0 ) {

			ShowWindow ( hScrollBar, SW_SHOW );
		} else {

			ShowWindow ( hScrollBar, SW_HIDE );
		}


		/**	Remove the hour-glass
		**/
		SetCursor ( hcursorOld );


		break;



	case IDM_SAVE_SETUP :

		/**	Default to "*.WAO", without specifying
		***	a full path specification. The toolkit fn 
		***	should then use the current working directory.
		**/
		if ( strlen ( panAnalogOut.szFileSpec ) == 0 ) {

			strcpy ( szAppSetupName, "*.wao" );
		} else {
			strcpy ( szAppSetupName, panAnalogOut.szFileSpec );

		}

		ATKSaveApplicationSetup ( hInst,
			hwnd,
			szAppSetupName,
			"wao",
			"AnalogOut Setup (*.wao)",
			"*.wao" );

		break;


	case IDM_RESTORE_SETUP :


		/**	Terminate the current Service name
		***	in preparation for a new service name
		***	stored in the Setup file.
		**/
		PTKTerminateDDEService ( );


		/**	Default to "*.W__", without specifying
		***	a full path specification. The toolkit fn 
		***	should then use the current working directory.
		**/
		strcpy ( szAppSetupName, "*.wao" );


		ATKLoadApplicationSetup ( hInst,
			hwnd,
			szAppSetupName,
			"wao",
			"AnalogOut Setup (*.wao)",
			"*.wao" );


		/**	Fall thru to initialize appl for new setup
		***			:
		***			:
		***			V
		**/

	case IDM_INIT_NEW_SETUP :   /* ... also called from DDE module */

		/** Reset the focus to the first channel and
		*** ensure this channel is visible by putting
		*** it on the top row.
		**/
		panAnalogOut.nFirstChanVisible = 0;
		panAnalogOut.nFocusChannel     = 0;

		/**	Remove old data and update the button
		***	status and scroll bars ( Invalidating these
		***	areas causing them to be updated without
		***	explicitly calling UpdateStatus etc..
		**/
		ResizeClientRects ( hwnd, panAnalogOut.lWidth,
						panAnalogOut.lHeight );

		/**	Remove old data and update the button
		***	and status bars ( Invalidating these
		***	areas causing them to be updated without
		***	explicitly calling UpdateStatus etc..
		**/
		InvalidateRect ( hwnd, (LPRECT) NULL, TRUE );
		UpdateDataArea ( hwnd );


		/** Also update the title and menubar
		*** now we have a new setup file.
		**/
		UpdateTitleBar	( hwnd );
		UpdateMenuBar	( hwnd );




		/**	Enable the scroll-bar with the correct range
		***	and initial position.
		**/
		nRange = max ( 0, conninfo.nNumConnected -
					panAnalogOut.nNumRowsVisible );

		SetScrollRange ( hScrollBar, SB_CTL, 0, nRange, TRUE );

		SetScrollPos ( hScrollBar, SB_CTL, 0, TRUE );

		/**	Only allow the user to see the Scrollbar
		***	if it has a valid reange.
		**/
		if ( nRange > 0 ) {

			ShowWindow ( hScrollBar, SW_SHOW );
		} else {

			ShowWindow ( hScrollBar, SW_HIDE );
		}


		/**	Register the new DDE Service name...
		***
		**/
		nReturn = PTKInitializeDDEService ( hwnd,
				panAnalogOut.szDDEServiceName );

		if ( nReturn != OK ) {

			ATKMessageBox ( hinst, hwnd,
				nReturn,
				(LPSTR) NULL,
				MB_OK | MB_ICONEXCLAMATION );

		}
		break;

	case IDM_DDE :


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
		if ( strlen ( panAnalogOut.szFileSpec ) > 0 ) {

		     szPtr = strrchr ( panAnalogOut.szFileSpec, '\\' );
		     if ( szPtr == (char *) NULL ) {

			/**	No path specified, must just be a fname
			**/
			szPtr = panAnalogOut.szFileSpec;

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
		nReturn = ATKGetDDEServiceName ( hinst, hwnd, "AnalogOut",
				szSetup, panAnalogOut.szDDEServiceName );

		/**	Register the new service name
		**/
		nReturn = PTKInitializeDDEService ( hwnd,
					panAnalogOut.szDDEServiceName );

		if ( nReturn != OK ) {

			ATKMessageBox ( hinst, hwnd,
				nReturn,
				(LPSTR) NULL,
				MB_OK | MB_ICONEXCLAMATION );

		}
		break;



	case IDM_ABOUT :


		/**   Bind exported functions to instance data segment
		**/
		lpfnAbout_DlgProc =  MakeProcInstance (
			      (FARPROC) About_DlgProc, hinst );

		DialogBox ( hinst, "ABOUT",
				 hwnd, (DLGPROC) lpfnAbout_DlgProc );


		FreeProcInstance ( lpfnAbout_DlgProc );

		break;


	case IDM_HELP_CONTENTS	:

		bHelpLoaded = InvokeWinHelp ( hwndMain, HELP_CONTENTS,
								(DWORD) 0L );
		break;



	case IDM_EXIT  :


		FORWARD_WM_CLOSE ( hwnd, SendMessage );
		break;

	default :

		/**	This could a message from one of the
		***	channel editboxs. To determine which
		***	editbox we will have to search the array of editbox
		***	handles. The handle of the control sending the
		***	message is stored in the low-order word of lParam
		**/


		/**	Ensure also that the editbox has an
		***	associated channel i.e. that channel
		***	connections have been made. The editbox's
		***	are created on WM_CREATE. Channels however
		***	will not exist until IDM_OUTPUTS command has
		***	been successfully processed.
		**/
		if ( ( conninfo.nNumConnected == 0 ) ) {

			break;

		}


		/**	Determine which editbox (if any) has sent
		***	the notification.
		**/
		for ( nRowIndex = 0;
		    nRowIndex < panAnalogOut.nMaxRowsVisible;
							nRowIndex++ ) {

			if ( ( aheditTextRow [ nRowIndex ] == hwndCtl ) &&
						  ( hwndCtl != NULL ) ) {

				/** Control Located, which channel
				*** currently owns the control ? We can
				*** calculate this because we know which
				*** channel is displayed on the first row
				*** where nRowIndex == 0.
				**/
				nChannel = panAnalogOut.nFirstChanVisible +
								   nRowIndex;

				/**	Determine what has happened to the
				***	editbox from the notification code.
				***
				***	If textline editbox loses the focus
				***	(notification == EN_KILLFOCUS), we
				***	assume the user has finished editing
				***	a new output value. We can either
				***	send this value now, or, if in
				***	deferred mode save it till later.
				***	In either case store the value
				***	in the channel's OUTDATA structure
				***	and set the OUTDATA structure's
				***	'bWriteData' flag.
				***
				**/
				switch ( codeNotify ) {


				case EN_SETFOCUS  :

					/** Select the existing text
					*** contained in the editbox
					*** allowing it to be changed
					*** without having to
					*** backspace/delete over the
					*** old entry.
					***
					*** NB Post this message to allow
					***    the editbox handler to
					***    finish processing the focus
					***    command.
					**/

					PostMessage ( hwndCtl, EM_SETSEL,
						  0, MAKELONG ( 0, 32767 ) );

					break;

				case EN_KILLFOCUS :


					/**	This test ensures that when
					***	we lose the focus, it has been
					***	lost to another editbox and NOT
					***	a dialog box or another application
					***
					***	If we lose the focus to another
					***	edit box we assume the user has
					***	finished editing the first and
					***	wishes it sent ( unless in refered
					***	mode )
					**/
					hwndFocus = GetFocus ( );
					if ( hwndFocus == NULL ) {

						break;	/* No Focus */
					}

					if ( GetParent ( hwndFocus )
							   != hwndMain ) {

						 break;  /* lost to applc */
					}

					/**	Another editbox now has
					***	the focus ...
					**/
					if ( conninfo.lpodOutputData
					  [ nChannel ].bWriteData == FALSE ) {

						/**	Edit control has
						***	lost the focus but
						***	recieved no changes
						***	while it owned it.
						***	No action required.
						**/
						break;
					}


					/**	Copy the new output value to
					***	toolkit Write List ready for
					***	transmission.
					**/
					Edit_GetText ( hwndCtl,
					   conninfo.lpodOutputData
						 [ nChannel ].szNewValue,
								MAX_VALUE );


					/**  Set the Write flag to indicate
					***  that this data is outstanding.
					**/
					conninfo.lpodOutputData
					      [ nChannel ].bWriteData = TRUE;




					if ( panAnalogOut.nMode ==
							   MODE_IMMEDIATE ) {

						/**	Immediate mode so,
						***	send the data now.
						***	If not immediate mode
						***	wait until deferred
						***	mode canceled before
						***	writing to hardware
						**/
						FORWARD_WM_WRITECHAN (
							hwnd, nChannel,
							SendMessage );



					} else {


						InvalidateChannel ( hwnd,
						  nChannel, CHANFIELD_BITMAP );
					}
					break;


				default :
					/** No action
					**/
					break;

				} /* End Switch */

			} /* End If Channel Editbox */

		} /* End For Loop */

		break;

	}

	return;

}





/**
***	Specify minimum and maximum sizes of
***	application window.
***
***
**/
VOID AnalogOut_OnGetMinMaxInfo ( HWND hwnd, MINMAXINFO	*lpMinMaxInfo )
{
	long	lMinWidth, lMinHeight;
	short	nRange;



	if ( hwndMain != hwnd ) {

		/**	WM_CREATE Not processed so we don't
		***	know how big our controls are yet.
		***	Dont impose any limits yet assuming
		***	the original size is acceptable.
		**/
		return;

	}


	/**	Minimum width should display Names, Values and ScrollBar
	***	Minimum height should display atleast one channel.
	***
	***	The Names, Symbols and Values rectangles are seperated
	***	by one average text width to improve clarity.
	***	(see WM_SIZE for assignment of RECT structures)
	**/
	lMinHeight =  ( 1 * GetSystemMetrics ( SM_CYCAPTION ) ) +
		      ( 1 * GetSystemMetrics ( SM_CYMENU ) ) +
		      ( 1 * controls.nStatusBarHeight + 2 ) +
		      ( 1 * controls.nSingleBarHeight + 2 ) +
		      ( 2 * GetSystemMetrics ( SM_CYBORDER ) ) +
		      ( 1 * panAnalogOut.lRowHeight  + 2 );

	lMinWidth  =  ( 2 * GetSystemMetrics ( SM_CXBORDER ) ) +
		      ( rectNames.right     - rectNames.left ) +
		      ( rectSymbols.right   - rectSymbols.left ) +
		      ( rectValues.right    - rectValues.left ) +
		      ( 4 * panAnalogOut.lAveTextWidth );


	/**	Allow additional room for scrollbar if visible
	**/
	nRange = max ( 0, conninfo.nNumConnected -
					panAnalogOut.nNumRowsVisible );
	if ( nRange > 0 ) {

		lMinWidth += ( rectScrollBar.right - rectScrollBar.left );
	}





	lpMinMaxInfo->ptMinTrackSize.x = lMinWidth;
	lpMinMaxInfo->ptMinTrackSize.y = lMinHeight;
	return;
}



/**
***  AnalogOut_OnPaint	This function is called on recieving a
***			WM_PAINT message.
***
***  Arguments
***
***  hwnd		Handle of Window owning Client
***
***  Return Value	Void
***
**/
VOID AnalogOut_OnPaint ( HWND hwnd )
{
	HDC		hdcClient;
	PAINTSTRUCT	psPaint;
	RECT		rectIntersect;	  /* Intersection of two RECTs */

	hdcClient = BeginPaint ( hwnd, (LPPAINTSTRUCT) &psPaint );


	if ( IsIconic ( hwnd ) ) {

		/**	Application is iconised, do not paint anything
		**/
		EndPaint ( hwnd , (LPPAINTSTRUCT) &psPaint );
		return;

	}



	if ( panAnalogOut.nStatus == STATUS_BUSY ) {
		
		/** If we are doing a dialog don't try to repaint **/
		EndPaint ( hwnd, (LPPAINTSTRUCT) &psPaint );
		return;
	}

	/**
	***	Determine which rectangles need repainting
	***	First the data area of the client. This may
	***	overwrite the status area slightly so must be
	***	drawn first.
	**/

	/**   The Data Area
	**/
	if ( IntersectRect ( &rectIntersect,
			&psPaint.rcPaint, &rectData ) ) {


		/**
		**/
		DrawDataArea ( hdcClient, &psPaint );

	}


	/**	The Status bar
	**/
	if ( IntersectRect ( &rectIntersect,
			&psPaint.rcPaint, &rectStatusBar ) ) {


		/**	Status Bar Rect Invalidated ( or part thereof )
		***	...Paint the Status Bar
		**/
		DrawStatusBar ( hdcClient );
		UpdateStatusBar ( hwnd );

	}



	/**   The Button Bar
	**/
	if ( IntersectRect ( &rectIntersect,
			&psPaint.rcPaint, &rectButtonBar ) ) {


		/**	Button Bar Rect Invalidated ( or part thereof )
		***	...Paint the Button Bar
		**/
		DrawButtonBar ( hdcClient );
		UpdateButtonBar ( hwnd );

	}

	/**   The Vertical Scroll Bar
	**/
	if ( IntersectRect ( &rectIntersect,
			&psPaint.rcPaint, &rectScrollBar ) ) {


		/**
		**/
		UpdateScrollBar ( hwnd );

	}




	EndPaint ( hwnd , (LPPAINTSTRUCT) &psPaint );
	return;
}


















/**
***  Function		AnalogOut_OnVScroll
***
***  Description	This function is called on recieving a
***			WM_VSCROLL message.
***
***  Arguments
***
***  hwnd		Handle of parent Window
***  hwndctl		Handle of Scroll-bar
***  code		SB_BOTTOM	  Scroll to lower right.
***
***			SB_ENDSCROLL	  End scroll.
***
***			SB_LINEDOWN	  Scroll one line down.
***
***			SB_LINEUP	  Scroll one line up.
***
***			SB_PAGEDOWN	  Scroll one page down.
***
***			SB_PAGEUP	  Scroll one page up.
***
***			SB_THUMBPOSITION  Scroll to absolute position.
***					  The current position is provided
***					  in 'pos'
***
***			SB_THUMBTRACK	  Drag thumb to specified position.
***					  The current position is provided
***					  in 'pos'
***
***	pos		The current position (see above)
***
***  Return Value	Void
***
**/
VOID AnalogOut_OnVScroll ( HWND hwnd, HWND hwndCtl, long code, long pos )
{
	short	nCurrentPos;


	/**	Note the old thumb position
	**/
	nCurrentPos = panAnalogOut.nFirstChanVisible;


	/**	Before giving the new Channel the focus,
	***	'remove the focus' from the old channel.
	***	This will ensure that the old channel recieves
	***	an EN_KILLFOCUS notification, even if the new
	***	channel will be using the same control. At the
	***	end of this function the focus will be reassigned
	***	to a new edit box control.
	**/
	KillChannelFocus ( hwnd, panAnalogOut.nFocusChannel );


	switch ( code ) {

	case SB_TOP :

		/**	The first row should contain channel 0
		***	Give this channel the highlight.
		**/
		panAnalogOut.nFirstChanVisible	= 0;
		panAnalogOut.nFocusChannel	= 0;

		break;

	case SB_BOTTOM :

		/**	The last row should contain the last channel
		***	this is more complicated.  There may be less
		***	channels than rows.
		**/
		panAnalogOut.nFirstChanVisible = max ( 0,
		    ( conninfo.nNumConnected - panAnalogOut.nNumRowsVisible ) );

		/**	Give the last channel the focus.
		**/
		panAnalogOut.nFocusChannel = conninfo.nNumConnected - 1;


		break;

	case SB_PAGEDOWN :

		/**	Move down to next page.
		***
		**/

		panAnalogOut.nFirstChanVisible =
		   min ( panAnalogOut.nFirstChanVisible
				+ panAnalogOut.nNumRowsVisible,
						conninfo.nNumConnected - 1 );

		/**	Move the focus down one page as well.
		***
		**/
		panAnalogOut.nFocusChannel =
		   min ( panAnalogOut.nFocusChannel
				+ panAnalogOut.nNumRowsVisible,
						conninfo.nNumConnected - 1 );

		break;

	case SB_LINEDOWN :

		panAnalogOut.nFirstChanVisible =
				min ( ( 1 + panAnalogOut.nFirstChanVisible ),
						conninfo.nNumConnected - 1 );

		/**	Move the focus down one-line
		**/
		panAnalogOut.nFocusChannel =
				min ( ( 1 + panAnalogOut.nFocusChannel ),
						conninfo.nNumConnected - 1 );

		break;


	case SB_PAGEUP :

		panAnalogOut.nFirstChanVisible =
				max ( 0, ( panAnalogOut.nFirstChanVisible
					  - panAnalogOut.nNumRowsVisible ) );

		/**	Move the focus up one-page
		**/
		panAnalogOut.nFocusChannel =
				max ( 0, ( panAnalogOut.nFocusChannel
					  - panAnalogOut.nNumRowsVisible ) );

		break;


	case SB_LINEUP :

		panAnalogOut.nFirstChanVisible =
			      max ( 0, ( panAnalogOut.nFirstChanVisible - 1 ) );


		/**	Move the focus up one-line
		**/
		panAnalogOut.nFocusChannel =
			      max ( 0, ( panAnalogOut.nFocusChannel - 1 ) );

		break;


	case SB_THUMBPOSITION :
	case SB_THUMBTRACK :



		panAnalogOut.nFirstChanVisible = (short) pos;
		panAnalogOut.nFocusChannel     = (short) pos;

	default :
		break;
	}






	/**	Invalidate the data area which has been 'scrolled'
	***	Check first that the thumb HAS moved.....
	**/
	if ( nCurrentPos != panAnalogOut.nFirstChanVisible ) {

		SetScrollPos ( hScrollBar, SB_CTL,
				panAnalogOut.nFirstChanVisible, TRUE );


		/**	Invalidate the client area ( GDI )
		**/
		InvalidateRect ( hwnd, (LPRECT) &rectData, TRUE );

		/**	Update the channel editbox text in the
		***	'data area' controls. Each editbox will now
		***	be assigned to a new channel and will therefore
		***	need to display that channel's value.
		***
		**/
		UpdateDataArea ( hwnd );




	}


	SetChannelFocus ( hwnd, panAnalogOut.nFocusChannel );


	return;


}









/**	This message is sent to the parent window of a predefined control or
***	message box when the control or message box is about to be drawn. By
***	responding to this message, the parent window can set the text and
***	background colors of the child window by using the given display-
***	context. We will use this to the background color of the static
***	text controls on the button bar.
***
***	Parameters :
***		hwnd	  - Handle of controls parent.
***		hwndChild - Handle of control.
***		hdc	  - Device Context handle of control.
***		type	  - Type of control e.g.  CTLCOLOR_STATIC
***
***	NOTE
***	When processing the WM_CTLCOLOR message, the application must
***	align the origin of the intended brush with the window coordinates
***	by first calling the UnrealizeObject function for the brush, and
***	then setting the brush origin to the upper-left corner of the
***	window.
***
***	Return Value : A handle to a brush to be used for painting the
***	client control
***
**/
HBRUSH AnalogOut_OnCtlColor ( HWND hwnd, HDC hdc, HWND hwndChild, long type)
{




	if ( type == CTLCOLOR_STATIC ) {


		SetBkColor ( hdc, GetSysColor ( COLOR_BTNFACE ) );


		/**	Here we assume All static text controls are
		***	on the button or status bars and require
		***	a background color change.
		**/
		UnrealizeObject ( hCtlBrush );
		SetBrushOrgEx ( hdc, 0, 0, NULL );
		return ( hCtlBrush );


	} else {

		return ( (HBRUSH) NULL );
	}
}




/**	AnalogOut_OnSetFocus
***
***	This function is called on a WM_SETFOCUS message.
***	We will use it to Prevent the client area push-button controls
***	losing the Focus to the main application window.
***
***	Arguments :
***		hwnd -		Handle of Window receiving focus.
***		hwndOldFocus -	Handle of Window losing the focus.
***
***	Return Value : void
***
**/
VOID AnalogOut_OnSetFocus ( HWND hwnd, HWND hwndOldFocus )
{

	SetChannelFocus ( hwnd, panAnalogOut.nFocusChannel );
}






DWORD AnalogOut_OnMenuChar ( HWND hwnd, long ch, long flags, HMENU hMenu )
{


	switch ( ch ) {




	case 'm' :
	case 'M' : /* Mode Button pressed */

		   if ( IsWindowEnabled
				( abutButtonInfo [ B_MODE ].hButtonWnd	) ) {

			FORWARD_WM_COMMAND ( hwnd, IDC_MODE_BUTTON,
							NULL, 0, SendMessage );

			return ( MAKELONG ( 0, 1 ) );

		   } else {

			/** Discard Character and Bleep
			**/
			return ( MAKELONG ( 0, 0 ) );

		   }


	}



	/** Discard Character and Bleep
	**/
	return ( MAKELONG ( 0, 0 ) );
}




/**
***	AnalogOut_OnKeyDown	: This function processes a subset
***				  of the virtual keycodes. The messages
***				  will have been forwarded from the
***				  Editbox callback function since these cntls
***				  maintain ownership of the system focus.
***				  The callback will have filtered out
***				  any keycodes not 'belonging' to the
***				  editbox passing thenm upto the parent (here)
***				  For example VK_F1 will be recieved by
***				  the editbox but forwarded here from where
***				  the help engine will be invoked.
***
**/
VOID AnalogOut_OnKeyDown ( HWND hwnd, long vk, BOOL fDown,
						long cRepeat, long flags )
{
	short	nNewFocusChannel;
	short	nChannelIndex;
	short	nRowIndex;
	HWND	hwndEditbox;

	/**	The main application Window should receive all
	***	keyboard input other than that directed at the edit box
	***
	***	The edit-box should return focus as soon as the user
	***	has finished 'editing' ( indicated by a RETURN, ESC or TAB
	***	or shift TAB key )
	***
	***	Client area Button controls should also submit the focus
	***	to the main application as soon as they receive it.
	***
	***
	***	The Main application processes the following keys :
	***
	***	VK_TAB/shift VK_TAB	-  Move focus to edit
	***	VK_RETURN		-  Write Selected Value
	***	VK_F1			-  Invoke Help
	***
	***	VK_CUP			-
	***	VK_CDOWN		-  Map onto Scroll-bar commands
	***	VK_NEXT 		-
	***	VK_PRIOR		-
	***
	***	VK_HOME 		-  Move focus to first on current page
	***	shift VK_HOME		-  Move focus to first channel
	***
	***	VK_END			-  Move focus to last on current page
	***	shift VK_END		-  Move focus to last channel
	***
	***	VK_ESCAPE		-  Cancels any changes made in
	***				   'Deferred' mode.
	**/

	switch ( vk ) {


	case VK_RETURN :

		/**	Accept changes to the selected Channel.
		***	Translate this key stroke to a WM_COMMAND
		***	notification message for the editbox with the focus.
		***	This will determine the new value and
		***	correctly handle deferred/immediate modes.
		**/
		nRowIndex = panAnalogOut.nFocusChannel -
					panAnalogOut.nFirstChanVisible;

		hwndEditbox = aheditTextRow [ nRowIndex ];

		/**	Emulate a EN_KILLFOCUS notification 'from' the
		***	editbox control. This will cause the application
		***	to assume that the control has lost the focus
		***	indicating that the user has finished editing
		***	the value
		**/
		FORWARD_WM_COMMAND ( hwnd, 0,
				      hwndEditbox, EN_KILLFOCUS, SendMessage );

		break;

	case VK_TAB :

		/**	Move the focus between button-bar controls
		***	and channel controls.  As the button-bar
		***	push-buttons surrending the focus to the
		***	client anyway, it is pointless moving the
		***	focus. This will change if we add an auto
		***	editbox which we will allow to keep the
		***	focus
		**/
		if ( GetKeyState ( VK_SHIFT ) < 0 ) {

			/**	Move focus to edit-box
			**/

		} else {
			/**	Move focus to the 'focus channel' button
			***
			**/
		}

		break;


	case VK_F1 :

		FORWARD_WM_COMMAND ( hwnd, IDM_HELP_CONTENTS,
						NULL, 0, SendMessage );

		break;


	case VK_NEXT:


		SendMessage ( hwnd, WM_VSCROLL, SB_PAGEDOWN, 0L );

		break;




	case VK_PRIOR:


		SendMessage ( hwnd, WM_VSCROLL, SB_PAGEUP, 0L );

		break;



	case VK_HOME :

		if ( GetKeyState ( VK_CONTROL ) < 0 ) {

			/**	Before giving the new Channel the focus,
			***	'remove the focus' from the old channel.
			***	This will ensure that the old channel recieves
			***	an EN_KILLFOCUS notification, even if the new
			***	channel will be using the same control. At the
			***	end of this function the focus will be reassigned
			***	to a new edit box control.
			**/
			KillChannelFocus ( hwnd, panAnalogOut.nFocusChannel );



			/**	High-order bit set so ctrl key down
			***	Move focus to very first channel. This
			***	may involve 'scrolling' to the top
			**/
			panAnalogOut.nFirstChanVisible	= 0;

			panAnalogOut.nFocusChannel	= 0;

			InvalidateRect ( hwnd, (LPRECT) &rectData, TRUE );

			/**	Update the channel name text in the
			***	'data area' controls
			***
			**/
			UpdateDataArea ( hwnd );

			SetScrollPos ( hScrollBar, SB_CTL,
				panAnalogOut.nFirstChanVisible, TRUE );

		} else {

			/**	Move focus to top of client
			**/
			panAnalogOut.nFocusChannel =
				panAnalogOut.nFirstChanVisible;

		}
		SetChannelFocus ( hwnd, panAnalogOut.nFocusChannel );


		break;


	case VK_END :

		if ( GetKeyState ( VK_CONTROL ) < 0 ) {


			/**	Before giving the new Channel the focus,
			***	'remove the focus' from the old channel.
			***	This will ensure that the old channel recieves
			***	an EN_KILLFOCUS notification, even if the new
			***	channel will be using the same control. At the
			***	end of this function the focus will be reassigned
			***	to a new edit box control.
			**/
			KillChannelFocus ( hwnd, panAnalogOut.nFocusChannel );



			/**	High-order bit set so ctrl key down
			***	Move focus to very last channel. This
			***	may involve 'scrolling' to the bottom.
			**/
			panAnalogOut.nFirstChanVisible = max ( 0,
				( conninfo.nNumConnected -
					panAnalogOut.nNumRowsVisible ) );

			panAnalogOut.nFocusChannel =
					  conninfo.nNumConnected - 1;

			InvalidateRect ( hwnd, (LPRECT) &rectData, TRUE );

			/**	Update the channel name text in the
			***	'data area' controls
			***
			**/
			UpdateDataArea ( hwnd );


			SetScrollPos ( hScrollBar, SB_CTL,
				panAnalogOut.nFirstChanVisible, TRUE );

		} else {

			/**	Move focus to bottom of client, or the last
			***	channel if th eclient is not full...
			**/
			panAnalogOut.nFocusChannel = min (
				conninfo.nNumConnected - 1,
				   panAnalogOut.nFirstChanVisible +
					  panAnalogOut.nNumRowsVisible - 1 );

		}
		SetChannelFocus ( hwnd, panAnalogOut.nFocusChannel );


		break;	;



	case VK_UP :



		/**	Set the new Focus Channel. Prevent wrapround
		**/
		nNewFocusChannel = max ( 0, panAnalogOut.nFocusChannel - 1 );

		if ( nNewFocusChannel < panAnalogOut.nFirstChanVisible ) {


			/** Moved the focus of the beginning of the
			*** client data area. Send a scroll message
			*** to scroll the new focus channel onto the
			*** data area. The scroll function will
			*** automatically assign the focus to this
			*** channel and set 'nFocusChannel' field.
			**/

			SendMessage ( hwnd, WM_VSCROLL, SB_LINEUP, 0L );

		} else {

			/**	The new focus channel is already visible
			***	Store the new index in the 'nFocusChannel'
			***	field then give that channel the Window's
			***	input Focus.
			**/
			panAnalogOut.nFocusChannel = nNewFocusChannel;

			SetChannelFocus ( hwnd, panAnalogOut.nFocusChannel );


		}
		break;

	case VK_DOWN :



		/**	Set the new Focus Channel. Prevent wrapround
		**/
		nNewFocusChannel =  min ( ( panAnalogOut.nFocusChannel + 1 ),
					( conninfo.nNumConnected - 1 ) );


		if ( nNewFocusChannel >=
				( panAnalogOut.nFirstChanVisible
					+ panAnalogOut.nNumRowsVisible ) ) {


			/** Moved of the end of the client data area.
			*** Scroll the new focus channel back onto the
			*** display. The scroll message will assign the
			*** new focus channel index into the 'nFocusChannel'
			*** field of the panAnalogOut structure.
			**/
			SendMessage ( hwnd, WM_VSCROLL, SB_LINEDOWN, 0L );

		} else {
			/**	The new focus channel is already visible
			***	Copy the index into the 'nFocusChannel'
			***	of the panAnalogOut structure and give
			***	it the Window's input Focus.
			**/
			panAnalogOut.nFocusChannel = nNewFocusChannel;

			SetChannelFocus ( hwnd, panAnalogOut.nFocusChannel );


		}
		break;


	case VK_ESCAPE :

			/**	The Escape key is used to cancel any
			***	'deferred' channel settings, which have
			***	not yet been transfered to the hardware.
			**/
			for ( nChannelIndex = 0;
				nChannelIndex < conninfo.nNumConnected;
							nChannelIndex++ ) {

				/**	Set the Write flag FALSE
				***	invalidating the new data state.
				**/
				conninfo.lpodOutputData
					 [ nChannelIndex ].bWriteData = FALSE;

			}
			/**	Invalidate the client to remove the
			***	'edited values' and show the existing
			***	output values...
			**/
			InvalidateRect ( hwnd, (LPRECT) &rectSymbols, TRUE );
			UpdateDataArea ( hwnd );

			break;

	}

	return;


}







VOID AnalogOut_OnKeyUp ( HWND hwnd, long vk, BOOL fDown,
						long cRepeat, long flags )
{


}






/**
***	Function :	AnalogOut_OnReadChan
***
***	Description :	This function is called on receipt of the
***			user defined message WM_READCHAN. It is responsible
***			for updating one or all of the CHANINFO strucures,
***			held in the 'conninfo.pciConnections' array, with a
***			new data value string.
***
***			unChannelIndex identifies the element in the
***			conninfo.pciConnections array, which is to be updated.
***			If unChannelIndex is equivalent to the number of
***			connectiions held in the array, then ALL elements
***			should be udated ( i.e. 'AllChannels' channel )
***
**/
VOID AnalogOut_OnReadChan ( HWND hwnd, long lChannelIndex )
{

	short	nFailedToRead;	/* How many Channels could NOT be Read */
	short	nReturn;



	/**	Before proceeding check that the setup has not
	***	changed and prevent the setup from being changed.
	***
	***	If we are in a DDE conversation, this will already
	***	have been done, and the PreventSetupConfiguration fn
	***	will return directly without checking the setup etc.
	**/

	nReturn = PreventSetupConfiguration ( hwnd, &conninfo.imsSetup );


	switch ( nReturn ) {

	case OK :

		break;		/* Continue */

	case IMS_CONFIGURING :

		/**  Setup IML is editing the setup
		***  DO NOT Take new readings and inform
		***  user when we know the extent the of
		***  the changes. ( IMS_SETUP_CHANGED )
		**/
		return;


	case IMS_SETUP_CHANGED :

		/**  Since we may have changed the size
		***  of the connections array.	The
		***  channel(s) we were asked  to read
		***  may not now exist, so fail this request.
		***
		***  However, since the setup was actually
		***  locked ( IMS_SETUP_CHANGED is not an error )
		***  we should call AllowSetupConfiguration as
		***  normal.
		**/


		/**	Allow the Setup to be edited....
		***
		**/
		nReturn = AllowSetupConfiguration ( hwnd, &conninfo.imsSetup );

		/* ... then drop thru */


	default :

		/**  Any other error is more serious
		***  so do no attempt to 'Read' without the setup.
		**/

		return;
	}






	/**	Are we reading all channels OR a single channel
	**/
	if ( (short) lChannelIndex == conninfo.nNumConnected ) {

		/**	All Channels specified. Read without Alarms.
		**/

		nFailedToRead = ATKReadList ( hwnd,
				    conninfo.nIMLInterface,
					conninfo.pciConnections, FALSE );

		if ( nFailedToRead != 0 ) {

			/**	Failed to read one or more channels
			***	May need to Abort cycling....
			***	Examine the nError fields to determine
			***	the type(s) of errors.
			**/


			/**	Invalidate both the values and unit fields
			***	Channels in error do not display units
			**/
			InvalidateRect ( hwnd, (LPRECT) &rectUnits, TRUE );
		}

		/**	Invalidate the entire data value area.
		**/
		InvalidateRect ( hwnd, (LPRECT) &rectSymbols, TRUE );
		UpdateDataArea ( hwnd );


	 } else {


		/**	Read Single data value ( without Alarm status )
		**/
		if ( !ATKReadChannel ( hwnd, conninfo.nIMLInterface,
			&( conninfo.pciConnections [ lChannelIndex ] )
								,FALSE ) ) {

			/**	Failed to read channel
			**/


			/**	In a future version we may wish
			***	to invalidate the units to remove or
			***	replace them on going into or out off
			***	error
			**/



		}

		/**	Invalidate the editbox text and exclaimation
		***	symbol, regardless of the error status
		**/
		InvalidateChannel ( hwnd, lChannelIndex, CHANFIELD_BITMAP );
		UpdateChannelEdittext (  hwnd, lChannelIndex );


	}





	/**	A DDE Client may be interested in
	***	this channel.
	**/
	PTKAdviseDDE ( (short) lChannelIndex );


	/**	Allow the Setup to be edited....
	***
	**/
	nReturn = AllowSetupConfiguration ( hwnd, &conninfo.imsSetup );


	return;
}




/**
***	Function :	AnalogOut_OnWriteChan
***
***	Description :	This function is called on receipt of the
***			user defined message WM_WRITECHAN. It is responsible
***			for updating one or more hardware channels
***			(detailed in the conninfo.pciConnections
***			structure array ) with new digital states.
***
***			Each new state is stored in a OUTPUTDATA structure
***			An array of which ( one per connection ) is pointed
***			to by conninfo.lpodOutputData.
***			The 'szNewValue' field holds the new value in string
***			format. The 'bWriteData' field tells us if this value
***			needs to be sent to the hardware. We can transverse
***			this array checking each 'bWriteData' field to see
***			if we have valid data for that channel. Once sent
***			the 'bWriteData' flag should be set FALSE to prevent
***			the data being resent on subsequent calls.
***
***			unChannelIndex identifies the element in the
***			conninfo.pciConnections array, which is to be updated.
***			If unChannelIndex is equivalent to the number of
***			connectiions held in the array, then ALL elements
***			should be udated ( i.e. 'AllChannels' channel )
***
***
***
**/
VOID AnalogOut_OnWriteChan ( HWND hwnd, long lChannelIndex	)
{

	short	nFailedToWrite; /* How many Channels could NOT be Write */
	short	nFailedToRead;
	short	nChannel;
	short	nReturn;

	HINSTANCE	hinst;


	hinst = (HINSTANCE) GetWindowLong ( hwnd, GWL_HINSTANCE );


	/**	Before proceeding check that the setup has not
	***	changed and prevent the setup from being changed.
	***
	***	If we are in a DDE conversation, this will already
	***	have been done, and the PreventSetupConfiguration fn
	***	will return directly without checking the setup etc.
	**/

	nReturn = PreventSetupConfiguration ( hwnd, &conninfo.imsSetup );


	switch ( nReturn ) {

	case OK :


		break;		/* Continue */

	case IMS_CONFIGURING :

		/**  Setup IML is editing the setup
		***  DO NOT Take new readings and inform
		***  user when we know the extent the of
		***  the changes. ( IMS_SETUP_CHANGED )
		**/


		/**	Since the user has actually performed
		***	some action to output this data, we should
		***	tell him it wont work.
		**/
		ATKMessageBox  ( hinst, hwnd, (WORD) nReturn,
				      szDefTitle, MB_OK | MB_ICONHAND );

		/* drop thru */


	case IMS_SETUP_CHANGED :

		/**  Since we may have changed the size
		***  of the connections array.	The
		***  channel(s) we were asked  to read
		***  may not now exist, so fail this request.
		***
		***  However, since the setup was actually
		***  locked ( IMS_SETUP_CHANGED is not an error )
		***  we should call AllowSetupConfiguration as
		***  normal.
		**/


		/**	Allow the Setup to be edited....
		***
		**/
		nReturn = AllowSetupConfiguration ( hwnd, &conninfo.imsSetup );

		/* ... then drop thru */


	default :

		/**  Any other error is more serious
		***  so do no attempt to 'Read' without the setup.
		**/


		return;
	}





	/**	Are we writing ALL channels OR a subset of the
	***	channels. If  unChannelIndex is the last channel
	***	index + 1, then write all connectedchannels using
	***	the toolkit WriteList function. Otherwise examine
	***	the OUTDATA structure for each channel to determine
	***	if the output state needs updating.
	**/
	if ( (short) lChannelIndex == conninfo.nNumConnected ) {

		/**	All Channels specified. Copy the new data
		***	into the write list ready for transmission
		***	to the hardware.
		**/

		for ( nChannel = 0; nChannel < conninfo.nNumConnected;
							  nChannel++ ) {


			/**	Copy the new output value to
			***	toolkit Write List ready for transmission.
			**/
			lstrcpy ( (LPSTR)
			   conninfo.pciConnections [ nChannel ].szValue,
			     conninfo.lpodOutputData [ nChannel ].szNewValue );


			/**  Reset the Write flag to prevent this data
			***  being resent on further WM_WRITECHAN
			***  messages....
			**/
			conninfo.lpodOutputData [ nChannel ].bWriteData
								  = FALSE;

		}





		nFailedToWrite = ATKWriteList ( hwnd,
					    conninfo.nIMLInterface,
						conninfo.pciConnections );

		if ( nFailedToWrite != 0 ) {

			/**	Failed to read one or more channels
			***	May need to Abort cycling....
			***	Examine the nError fields to determine
			***	the type(s) of errors.
			**/


		}

		/**	Read back the values, ( no Alarm status )
		***
		**/
		nFailedToRead = ATKReadList ( hwnd, conninfo.nIMLInterface,
					    conninfo.pciConnections, FALSE );



		/**	All values are now up-to-date.
		***	reset the write flag to prevent the
		***	values being resent on further WM_WRITECHAN
		***	messages and update the text in each channels
		***	editbox with the value returned from the hardware
		***	(or error code if unsucessful)
		**/
		for ( nChannel = 0; nChannel < conninfo.nNumConnected;
							  nChannel++ ) {

			conninfo.lpodOutputData [ nChannel ].bWriteData
								  = FALSE;

			/**	Copy the error value into the OUTPUTDATA
			***	structure. Do this now, before reading back
			***	the output values. The Readback will overwrite
			***	the write error codes with read error codes
			**/
			conninfo.lpodOutputData [ nChannel ].nWriteError =
				conninfo.pciConnections [ nChannel ].nError;

		}

		/**	Invalidate the entire data value area
		***	including the 'exclamation' symbol bitmaps
		***	and the units ( which will be removed from
		***	any channels in error )
		**/
		UpdateDataArea ( hwnd );
		InvalidateRect ( hwnd, (LPRECT) &rectSymbols, TRUE );
		InvalidateRect ( hwnd, (LPRECT) &rectUnits,   TRUE );


		/**	A DDE Client may be interested in
		***	this channel.
		**/
		PTKAdviseDDE ( (short) lChannelIndex );




	} else {    /* Writing selected Channels */



		/**   This section is executed when nChannel is not
		***   'AllChannels'.  Each connected channel's
		***   OUTPUTDATA structure is checked to see if new
		***   data is outstanding.If it is, the data is copied
		***   to the channel's CHANDATA structure ( in the toolkit
		***   list ) and written using ATKWriteChannel. Potentially
		***   a new  toolkit list of 'outstanding' channels could
		***   be generated and the data sent 'simultaneously' using
		***   the ATKWriteList function but generating the list would
		***   be complicated and seems excessive when weighed against
		***   the advantages.
		**/
		for ( nChannel = 0; nChannel < conninfo.nNumConnected;
								nChannel++ ) {


			if ( conninfo.lpodOutputData [ nChannel ].bWriteData
								== FALSE ) {

				/**	This channel does NOT require updating
				***	and may be skipped....
				**/
				continue;
			}


			/**	Copy the new output value to
			***	toolkit Write List ready for transmission.
			**/
			lstrcpy ( (LPSTR)
			   conninfo.pciConnections [ nChannel ].szValue,
			     conninfo.lpodOutputData [ nChannel ].szNewValue );





			/**	Write single data value to this channel...
			**/
			if ( !ATKWriteChannel ( hwnd, conninfo.nIMLInterface,
				&( conninfo.pciConnections [ nChannel ] ) ) ) {

				/**	Failed to write channel
				**/

			}




			/**	Store the error code in the
			***	channel's OUTPUTDATA structure
			***	ready for displaying in the client.
			***
			***	Do this now. Reading back
			***	the output value will overwrite the
			***	error codes in the toolkit list.
			**/
			conninfo.lpodOutputData [ nChannel ].nWriteError =
			       conninfo.pciConnections [ nChannel ].nError;




			/**	Now perform the readback...
			***
			**/
			if ( !ATKReadChannel ( hwnd, conninfo.nIMLInterface,
				&( conninfo.pciConnections [ nChannel ] ),
								FALSE ) ) {

				/**	Failed to write channel
				**/



			}

			/**	Invalidate the old value on the client area.
			***	including the exclaimation symbol and units
			***	(which will be removed if channel in error)
			**/
			UpdateChannelEdittext ( hwnd, nChannel );

			InvalidateChannel ( hwnd, nChannel,
					CHANFIELD_BITMAP | CHANFIELD_UNITS );


			/**  Finally reset the Write flag to prevent this data
			***  being resent on further WM_WRITECHAN messages.
			***  WE MUST DO THIS AFTER updating the editbox text
			***  The process of updating the editbox will itself
			***  cause bWriteData to be set, since the value has
			***  changed. We must therefore reset it after setting
			***  the editbox text.
			**/
			conninfo.lpodOutputData [ nChannel ].bWriteData = FALSE;


			/**	A DDE Client may be interested in
			***	this channel.
			**/
			PTKAdviseDDE ( nChannel );




		} /* Next channel */
	}





	/**	Allow the Setup to be edited....
	***
	**/
	nReturn = AllowSetupConfiguration ( hwnd, &conninfo.imsSetup );


	return;
}






/**
***  AnalogOut_OnClose	This function is called on recieving a
***			WM_CLOSE message. An application can prompt the
***			user for confirmation, prior to destroying the
***			window, by processing the WM_CLOSE message and
***			calling the DestroyWindow function only if the user
***			confirms the choice.
***
***
***  Arguments
***
***  hwnd		Handle of Window which may be called
***
***  Return Value	Void
***
**/
VOID AnalogOut_OnClose ( HWND hwnd )
{

	/**	CODE to Terminate any current operations
	**/
	if ( nDDECloseCommand == FALSE ) {
		if ( MessageBox ( hwnd, "Do you wish to close AnalogOut ?",
					"Windmill AnalogOut - Close",
					MB_YESNO  ) == IDNO) {
		return ;
		}
	}

    if ( bHelpLoaded ) {

		/**	Inform Help Application that we
		***	no longer require Help.
		**/
		InvokeWinHelp ( hwndMain, HELP_QUIT, (DWORD) 0L );
	}

	bDestroy = TRUE;

	return;
}



/**
***  Function		AnalogOut_OnDestroy
***
***  Description	This function is called on recieving a
***			WM_DESTROY message.
***
***  Arguments
***
***  hwnd		Handle of Window being Destroyed.
***			It may be assumed that whilst processing this
***			message any child Windows still exist.
***
***  Return Value	Void
***
**/
VOID AnalogOut_OnDestroy ( HWND hwnd )
{

	/**	Shut down any outstanding processes
	**/
	StopAutoCycle ( );




	/**	Destroy the toolkit's Monitor table.
	**/
	ATKDestroyMonitorTable (  );



	/**	Free the IMS setup and IMSLib.DLL
	**/
	IMSUnlockSetup ( (LPSTR) conninfo.imsSetup.szSetupName,
					conninfo.imsSetup.nSetupKey );


	CloseIMLInterface ( conninfo.nIMLInterface );





	/**	Kill outstanding DDE Service.
	**/
	PTKTerminateDDEService ( );

	FreeDDECallBackProc ( );


	/**	Delete any Objects.
	***
	**/

	DeleteFont ( hSansSerif_8  );
	DeleteFont ( hSansSerif_10 );

	DeleteObject ( hCtlBrush );		/* WM_CTRCOLOR Brush */
						/* and Bitmaps	     */

	DeleteObject ( hbmpExclaimation );
	DeleteObject ( hbmpExclaimationMask );


	DeleteObject ( (HGDIOBJ) hpenStandardText );

	/**	Release background brush before destroying
	**/
	SetClassLong ( hwnd, GCL_HBRBACKGROUND, ( COLOR_WINDOW + 1 ) );
	DeleteObject ( (HGDIOBJ) hbrushStandardBk );


	/**	Free the connection's and outputdata arrays
	**/
	if ( conninfo.nNumConnected > 0 ) {

		LocalUnlock ( conninfo.hConnections );
		LocalFree   ( conninfo.hConnections );

		GlobalUnlockPtr ( conninfo.lpodOutputData );
		GlobalFreePtr	( conninfo.lpodOutputData );
	}


	PostQuitMessage ( 0 );

	return;
}



/**
***	Function :	UpdateTitleBar
***
***			Adds Hardware Setup file to Application Name
***			on the Title bar.
***	Arguments :
***	  hwndMain	Handle of the Application Window.
***
***	Return :	VOID
***
**/
VOID	UpdateTitleBar ( HWND hwndMain )
{
	char	szTitle [ 128 ];
	char	szUCSetup [ MAX_NAME_LENGTH + 1 ];
	short	nChar;


	if ( conninfo.bIMSSetupLoaded ) {

		/**	Uppercase the setup name
		***
		**/
		strcpy ( szUCSetup, conninfo.imsSetup.szSetupName );

		for ( nChar = 0; nChar < (short) strlen ( szUCSetup );
								nChar++ ) {

			szUCSetup [ nChar ] = (char)
				toupper ( (int) szUCSetup [ nChar ] );

		}

		/**	We only need to update the title
		***	if a setup has been loaded.
		**/
		sprintf ( szTitle, "%s_%s", szDefTitle, szUCSetup );

		SetWindowText ( hwndMain, szTitle );
	} else {
		SetWindowText ( hwndMain, szDefTitle );
	}
	return;
}




/**
***	Function	SetChannelFocus
***
***	Description	Determines which Editbox control holds
***			the text of the 'focus' channel and gives that
***			editbox the system focus.
***
***	Parameters:
***	   hwndParent		The window handle of the parent owning
***				the the push button controls
***
***	   nFocusChannel	An index into the CHANINFO structure array
***				'conninfo.pciConncetions' whose channel we
***				wish to be given the focus.
***	Return		void
***
**/
void SetChannelFocus ( HWND hwndParent, short nFocusChannel )
{
	short	nTextLine;
	HWND	hEditboxWithFocus;


	/**	Determine which editbox on the client belongs
	***	to the focus channel. The button structure array
	***	has an element for each possible text-line on the
	***	client. We therefore need to determine which textline
	***	is occupied by our focus channel and give the focus to
	***	the push-button control for that textline.
	**/

	nTextLine = ChannelNumberToTextLine ( panAnalogOut.nFocusChannel );


	hEditboxWithFocus = aheditTextRow [ nTextLine ];



	SetFocus ( hEditboxWithFocus );


	/**	Select the text in the editbox. This will allow
	***	the user to overwrite the existing text directly
	***	without having to backspace or delete it.
	**/
	Edit_SetSel ( hEditboxWithFocus, 0, 32767 );

	return;

}



/**
***	Function	KillChannelFocus
***
***	Description	Determines which Editbox control holds
***			the text of the 'focus' channel and sends
***			an EN_KILLFOCUS notification code to that
***			control.
***			This is important since the application
***			acts on EN_KILLFOCUS and it is possible
***			for a channel to lose the focus without
***			the edit control losing it ( i.e. the same
***			edit control is used for the new focus channel)
***
***	Parameters:
***	   hwndParent		The window handle of the parent owning
***				the push button controls
***
***	   nFocusChannel	An index into the CHANINFO structure array
***				'conninfo.pciConncetions' whose channel we
***				wish to be given the focus.
***	Return		void
***
**/
void KillChannelFocus ( HWND hwndParent, short nFocusChannel )
{
	short	nTextLine;
	HWND	hEditboxWithFocus;


	/**	Determine which editbox on the client belongs
	***	to the focus channel. The button structure array
	***	has an element for each possible text-line on the
	***	client. We therefore need to determine which textline
	***	is occupied by our focus channel and give the focus to
	***	the push-button control for that textline.
	**/

	nTextLine = ChannelNumberToTextLine ( panAnalogOut.nFocusChannel );


	hEditboxWithFocus = aheditTextRow [ nTextLine ];


	FORWARD_WM_COMMAND ( hwndParent, 0, hEditboxWithFocus,
					EN_KILLFOCUS, SendMessage );


	/**	Select the text in the editbox. This will allow
	***	the user to overwrite the existing text directly
	***	without having to backspace or delete it.
	**/
	Edit_SetSel ( hEditboxWithFocus, 0, 32767 );

	return;

}



/**
***	Function	ResizeClientRects
***
***	This function is used to set the sizes of the global
***	RECT structures which are used to segment the client
***	area. These are:
***
***		rectButtonBar
***		rectStatusBar
***		rectScrollBar
***		rectNames
***		rectValues
***		rectUnits
***		rectSymbols ( if applicable )
***		rectData
***
***	Typically, this function will be called on a WM_SIZE
***	message, but should also be called if the number of
***	connections changes when we may wish to allocate addition
***	space for the scrollbar.
***
***	NOTE This function may actually generate a WM_SIZE message
***	increasing the Window to accomodate a scrollbar without
***	overwriting the 'rectValues' area. This will happen if
***	the panel is at its mininmum width but more channels are
***	added or the Window height reduced.
***
***
***	Parameters:
***	 hwnd		Handle of Owner of Client
***	 cx		Width of client  ( this is obtained on WM_SIZE )
***	 cy		Height of client ( this is obtained on WM_SIZE )
***
***	Return:
***	 void
**/
void  ResizeClientRects ( HWND hwnd, long cx, long cy	)
{

	RECT	rectWindow;
	long	lDataAreaTop, lDataAreaBottom;
	BOOL	bScrollbarRequired;



	rectButtonBar.left	    = 0;
	rectButtonBar.right	= cx;
	rectButtonBar.top	= 0;
	rectButtonBar.bottom	= controls.nSingleBarHeight;



	rectStatusBar.left	= 0;
	rectStatusBar.right	= cx;
	rectStatusBar.bottom	= cy;
	rectStatusBar.top	    = rectStatusBar.bottom -
						    ( 2 * panAnalogOut.lTextHeight );

	/**	We may use the area between the button
	***	and status bars defined above, for
	***	displaying data.
	**/
	lDataAreaTop		= rectButtonBar.bottom;
	lDataAreaBottom 	= rectStatusBar.top;


	/** Determine the number of rows which will be visible between
	*** the button bar and the status bar.
	***
	*** This will always round down to give a whole number of rows.
	*** Partial rows not yet implemented.
	***
	*** Store the number of visible rows in the PANEL structure.
	***
	**/
	panAnalogOut.nNumRowsVisible = (short)
		(( lDataAreaBottom - lDataAreaTop ) / panAnalogOut.lRowHeight);




	/**	Will we need a scrollbar ?
	**/
	bScrollbarRequired =
		( panAnalogOut.nNumRowsVisible < conninfo.nNumConnected )
							   ? TRUE : FALSE;

	/**	The scrollbar.	If visible, reserve space for this control
	***	( 3 chars wide ) otherwise make it zero width. This area
	***	is itself not invalidated, but is reserved to prevent it
	***	from being invalidated by other areas.
	**/
	if ( bScrollbarRequired ) {

		rectScrollBar.left   = cx - ( 3 * panAnalogOut.lAveTextWidth );

	} else {
		/**	Zero width while not in use
		**/
		rectScrollBar.left   = cx;
	}
	rectScrollBar.right  = cx;
	rectScrollBar.top    = lDataAreaTop;
	rectScrollBar.bottom = lDataAreaBottom;



	/**	Subdivide the 'Data' area up into four columns
	***	used to display the four channel fields..
	**/
	rectNames.left		= 0;
	rectNames.right 	= ( MAX_NAME + 2 )
					     * panAnalogOut.lAveTextWidth;

	rectNames.bottom	= lDataAreaBottom;
	rectNames.top		= lDataAreaTop;


	rectSymbols.left	= rectNames.right  +
							panAnalogOut.lAveTextWidth;
	rectSymbols.right	= rectSymbols.left + BITMAP_WIDTH;

	rectSymbols.bottom	= lDataAreaBottom;
	rectSymbols.top 	= lDataAreaTop;


	rectValues.left 	= rectSymbols.right +
						panAnalogOut.lAveTextWidth;

	rectValues.right	= rectValues.left + ( ( MAX_VALUE + 2 )
					      * panAnalogOut.lAveTextWidth );

	rectValues.bottom	= lDataAreaBottom;
	rectValues.top		= lDataAreaTop;


	rectUnits.left		= rectValues.right;
	rectUnits.right 	= rectValues.right + ( ( MAX_UNITS + 2 )
					    * panAnalogOut.lAveTextWidth );

	/**	Dont allow the units to encroach on the area
	***	reserved for the scrollbar ( if visible )
	**/
	rectUnits.right = min ( rectUnits.right, rectScrollBar.left );

	rectUnits.bottom	= lDataAreaBottom;
	rectUnits.top		= lDataAreaTop;





	/**	The data area. This is the union of the above rectangles
	***	We can use this to invalidate ALL data on the client
	***	e.g. when scrolling...
	**/
	rectData.left	     = rectNames.left;
	rectData.right	     = rectUnits.right;
	rectData.bottom      = lDataAreaBottom;
	rectData.top	     = lDataAreaTop;



	/**	Before proceeding, check that the scrollbar and
	***	data value areas do not overlap. Although we process
	***	WM_MINMAXINFO to prevent this, it is still possible
	***	if the user simultanously changes both the vertical
	***	and horizontal extents. Since we do not know what the
	***	new size will be, we cannot predict if a scrollbar will
	***	be needed. Hence the check here...
	**/
	if ( ( !IsIconic ( hwnd ) ) &&
			( rectValues.right > rectScrollBar.left ) ) {

		/**	Increase the window size to accomodate a
		***	a scrollbar.
		**/
		GetWindowRect ( hwnd, (LPRECT) &rectWindow );
		MoveWindow ( hwnd,
			    rectWindow.left,
			    rectWindow.top,
			    ( rectWindow.right	- rectWindow.left ) +
				( rectScrollBar.right - rectScrollBar.left ),
			    rectWindow.bottom - rectWindow.top,
			    TRUE );
		return;
	}





	return;
}
