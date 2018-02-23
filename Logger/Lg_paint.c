/**
***     LG_PAINT.c              ATK-LOGGER
***
***     Andrew Thomson
***
***     This module is responsible for painting the client area.
***     The client is subdivided into several sections, the logical units
***     of which are stored in the following rectangle structures :
***
***     rectButtonBar   - Area occupied by the button bar.
***     rectStatusBar   - Area used to display logger status.
***     rectChanLabels  - Area used to display the Channel Names
***                       (including Scroll Bar)
***     rectLogData     - Area used to display logged Channel Data
***     rectCurrentTime - Area used to display the current time.
***     rectLogTime     - Area used to display 'logged times'
***
***
***	Associated Files : 	
***
***	Version History :
***
***	LOGGER-P-4.00	 New Version with Improved Features
***	LOGGER-P-4.05	 Data Columns RIGHT justified. This is in-preparation for
***			 IML devices which offer 'fixed decimal' data strings.
***			 Logger will line up each channel's data values. so...
***			
***				 "2.01"
***				"22.30"
***				 "1.00"
***
***	
**/
#define  NOMINMAX
#define  NOKANJI
#define  NOCOMM
#define  STRICT         /* Use Windows 3.1 Strict Style Coding  	*/
#define  CTLAPIS        /* Use Control API Macros 			*/


#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "\Windmill 32\Compile Libs\imslib.h"    /* ...using the IMSLIB Dynalink                  */



#include "\Windmill 32\ATKit\ATkit.h"      /* Toolkit Defns and Prototypes                  */
#include "lg_def.h"     /* Manifest Constants and Structure Declarations */
#include "lg_prot.h"    /* Logger modules Function Prototypes            */



extern	ATKCOLOURINFO	colors;	/* text colors for three alarm states */





extern RECT	rectButtonBar;  /* Area occupied by Push-button Bar         */
extern RECT	rectStatusBar;  /* Area occupied by Status Bar              */
extern RECT	rectChanLabels; /* Area occupied by Channel Names and Units */
extern RECT	rectLoggedData; /* Area occupied by Logged Data             */
extern RECT	rectLoggedTime; /* Area occupied by Logged Time             */
extern RECT	rectScrollBar;  /* Area occupied by Horz-Scroll-bar 	    */
extern RECT	rectRealTimeClk;/* Area used to display current time        */
extern RECT	rectHighlight;  /* Highlighted time/data                    */
extern RECT	rectScroll;     /* Scrolling Rectangle to move data up      */


extern HINSTANCE	hInst;		/** Program Instance Handle **/
extern HWND		hwndMain;	/** Program Window Handle   **/


/**     Array of Rectangles for Status Bar Text
**/
RECT   rectStatusBarItem [ NUM_STATUS_BAR_ITEMS ];

WNDPROC lpfnDefButtonHandler;
WNDPROC lpfnDefEditBoxHandler;
HMENU   hMenu;
HWND    hEditBox;
HWND    hScrollBar;

extern  HFONT   		hSansSerif_8;
extern  HFONT   		hSansSerif_10;
extern  CLIENTINFO      clntInfo;
extern  LOGINFO         logInfo;
extern  CLOCKINFO       clkRealTime;
extern  CONNECTIONS     conninfo;
extern  SETTINGS        settings;
extern  CONTROLS        controls;

BUTTONBAR abutButtonInfo [ NUM_BUTTONS ] = {
	NULL, BS_PUSHBUTTON, "&Start", 0, 0,
	NULL, BS_PUSHBUTTON, "S&top",  0, 0,
	NULL, BS_PUSHBUTTON, "&Pause", 0, 0,
	NULL, BS_PUSHBUTTON, "&Resume",0, 0,
	NULL, BS_PUSHBUTTON, "&Log",    0, 0
  };


/** local function prototype **/
int GetMaxChars ( HDC hdcClient, char* szText );

/**
***	PositionButtonBar
***
***
***	Sizes and positions the controls on the Button bar.
***	The organization and size of the button bar is dependent
***	on the area defined by global: rectButtonBar.
***
***
**/
short PositionButtonBar ( HDC hDC )
{

	HFONT   hOldFont;
	HPEN    hOldPen;
	HBRUSH  hOldBrush;
	DWORD   dwOldTextColor;
	DWORD   dwOldBkColor;

	long    lRectHeight;
	long    lRectWidth;

	long    lXBoarder;
	long    lYBoarder;

	long    lYButtonPos;
	long    lFirstXButtonPos;


	long    lEditBoxXPos;
	long    lEditBoxYPos;

	long    lIndex;


	long    lButtonLength  		= controls.nPushButtonWidth;
	long    lButtonHeight  		= controls.nPushButtonHeight;
	long    lEditBoxLength 		= controls.nEditBoxLength;
	long    lEditBoxHeight		= controls.nEditBoxHeight;

	long    lEditBoxLabelLength 	= controls.nEditBoxLabelLength;


	/**     The button bar may be 'organised' in one of two ways:
	***
	***     1) As a single row of four buttons + edit Box
	***     2) As two rows, one containing the edit box
	***        and the other the buttons
	***
	***
	***     The area assigned for the button bar is stored
	***     in the static rectangle rectButtonBar from this
	***     determine how to organise the bar.
	***
	***
	***     Allow for different fonts.
	**/


	/**     Draw ButtonBar Background, Button Coloured Solid
	***     Interior with a black boarder.
	**/
	hOldPen  = SelectPen ( hDC, CreatePen ( PS_SOLID, 0,
					      RGB ( 0, 0, 0 ) ) );

	hOldBrush= SelectBrush ( hDC, CreateSolidBrush (
			    GetSysColor ( COLOR_BTNFACE ) ) );


	dwOldTextColor = SetTextColor  ( hDC, GetSysColor ( COLOR_BTNTEXT ) );
	dwOldBkColor   = SetBkColor ( hDC, GetSysColor ( COLOR_BTNFACE ) );


	Rectangle ( hDC, rectButtonBar.left,
	                 rectButtonBar.top,
	                 rectButtonBar.right,
	                 rectButtonBar.bottom );


	/**     Draw the edit-box label...
	***     Select the correct helv-8 Font.
	**/
	hOldFont = SelectFont ( hDC, hSansSerif_8 );

	TextOut ( hDC, clntInfo.lAveTextWidth,
	        ( controls.nSingleBarHeight - clntInfo.lTextHeight ) / 2 ,
	        "Data File:", 10 );

	/**     Restore the original font
	**/
	SelectFont ( hDC, hOldFont );




	/**     Calculate useful 'constants'
	**/
	lRectHeight    = rectButtonBar.bottom - rectButtonBar.top;
	lRectWidth     = rectButtonBar.right  - rectButtonBar.left;
	lXBoarder      = controls.nBarBorder;


	if ( lRectWidth > controls.nMinSingleBarWidth  ) {

		/**     Use a single row with the buttons positioned
		***     after the edit box.
		**/
		lYBoarder   = ( lRectHeight - lButtonHeight ) / 2;
		lYButtonPos = rectButtonBar.top + lYBoarder;

		lFirstXButtonPos = lXBoarder + lEditBoxLength +
		                             lEditBoxLabelLength;




	} else {

		/**     Split into two rows with the buttons on the
		***     second row.
		**/
		lYBoarder   = ( lRectHeight - ( 2 * lButtonHeight ) ) / 4;
		lYButtonPos = rectButtonBar.top +
				  lYBoarder + ( lRectHeight / 2 );

		lFirstXButtonPos = lXBoarder;

	}

	/**     All Y coords are the same for each button
	**/
	for ( lIndex = 0; lIndex < NUM_BUTTONS; lIndex++ ) {

		abutButtonInfo [ lIndex ].lYPos = lYButtonPos;
	}



	/**     Buttons Are always placed next to each other
	**/
	abutButtonInfo [ 0 ].lXPos = lFirstXButtonPos;

	abutButtonInfo [ 1 ].lXPos = abutButtonInfo [ 0 ].lXPos + lButtonLength;

	abutButtonInfo [ 2 ].lXPos = abutButtonInfo [ 1 ].lXPos + lButtonLength;

	abutButtonInfo [ 3 ].lXPos = abutButtonInfo [ 2 ].lXPos + lButtonLength;

	abutButtonInfo [ 4 ].lXPos = abutButtonInfo [ 3 ].lXPos + lButtonLength;


	for ( lIndex = 0; lIndex < NUM_BUTTONS; lIndex++ ) {

		MoveWindow ( abutButtonInfo [ lIndex ].hButtonWnd,
		                abutButtonInfo [ lIndex ].lXPos,
		                abutButtonInfo [ lIndex ].lYPos,
		                lButtonLength,
		                lButtonHeight,
		                TRUE );


	}

	/**     Position the Edit Box
	***
	**/
	lEditBoxXPos = rectButtonBar.left + controls.nEditBoxLabelLength;
	lYBoarder    = controls.nBarBorder;
	lEditBoxYPos = rectButtonBar.top + lYBoarder;

	lEditBoxLength = controls.nEditBoxLength;
	lEditBoxHeight = controls.nEditBoxHeight;


	MoveWindow ( hEditBox, lEditBoxXPos, lEditBoxYPos,
	                        lEditBoxLength, lEditBoxHeight, TRUE );



	/**     Tidy Up, restoring old objects
	**/
	DeletePen    ( SelectPen   ( hDC, hOldPen ) );
	DeleteBrush  ( SelectBrush ( hDC, hOldBrush ) );
	SetTextColor ( hDC, dwOldTextColor );
	SetBkColor   ( hDC, dwOldBkColor );

	return ( OK );
}




/**	DrawStatusBar
***
***	Draws the status bar, the size of which is defined
***	in global rectStatusBar. Use UpdateStatusBar to
***	add the status text to the bar.
**/
short DrawStatusBar ( HDC hDC )
{
	HPEN    	hPen, hOldPen;
	HBRUSH          hOldBrush;
	long            lStatusBarLength;
	short           nItem;

	/**     Draw Bor Background, Button Coloured Solid
	***     Interior with a Black border.
	**/
	hOldPen  = SelectPen ( hDC, CreatePen ( PS_SOLID, 0,
	                                   RGB ( 0, 0, 0 ) ) );

	hOldBrush= SelectBrush ( hDC, CreateSolidBrush (
	                     GetSysColor ( COLOR_BTNFACE ) ) );



	Rectangle ( hDC, rectStatusBar.left,
	                 rectStatusBar.top,
	                 rectStatusBar.right,
	                 rectStatusBar.bottom );

	/**     Add Shading, White Lines on Top and Right
	***     Button Border on Top on Left
	**/
	hPen = CreatePen ( PS_SOLID, 0, RGB ( 255, 255, 255 ) );
	DeletePen ( SelectPen ( hDC, hPen ) );


	MoveToEx ( hDC, rectStatusBar.left + 1, rectStatusBar.bottom - 1 , NULL);
	LineTo ( hDC, rectStatusBar.left + 1, rectStatusBar.top + 1 );

	MoveToEx ( hDC, rectStatusBar.left  + 1, rectStatusBar.top + 1, NULL );
	LineTo ( hDC, rectStatusBar.right - 1, rectStatusBar.top + 1 );

	/**     White Shading
	**/
	hPen = CreatePen ( PS_SOLID, 0, GetSysColor ( COLOR_BTNSHADOW ) );
	DeletePen ( SelectPen ( hDC, hPen ) );


	DeletePen    ( SelectPen   ( hDC, hOldPen ) );
	DeleteBrush  ( SelectBrush ( hDC, hOldBrush ) );


	lStatusBarLength = STATUS_BAR_ITEM_LENGTH * clntInfo.lAveTextWidth;

	for ( nItem = 0; nItem < NUM_STATUS_BAR_ITEMS; nItem++ ) {


		rectStatusBarItem [ nItem ].top	   = rectStatusBar.top + 4;
		rectStatusBarItem [ nItem ].bottom = rectStatusBar.bottom - 4;
		rectStatusBarItem [ nItem ].left   = rectStatusBar.left
		                + ( ( lStatusBarLength + 4 ) * nItem ) + 4;

		rectStatusBarItem [ nItem ].right  = rectStatusBarItem [ nItem ].left
		                                              +  lStatusBarLength;

		/**     Is the rectangle visible or partially visible
		**/
		if ( rectStatusBarItem [ nItem ].left <
		                                rectStatusBar.right ) {

			DepressRect ( hDC, &rectStatusBarItem [ nItem ] );

		}
	}


	return ( OK );
}







/**
***	DepressRect
***
***	Called by DrawStatusBar to draw the individual rectangles
***	used to 'contain' one status item.
**/
short DepressRect ( HDC hDC, LPRECT lprectShadedArea )
{
	HPEN     hPen, hOldPen;



	/**	Add Shading,
	***	White Lines on Bottom and Right
	***	Button Border on Top on Left
	**/
	hPen = CreatePen ( PS_SOLID, 0, GetSysColor ( COLOR_BTNSHADOW ) );
	hOldPen = SelectPen ( hDC, hPen );

	MoveToEx ( hDC, lprectShadedArea->left, lprectShadedArea->bottom, NULL );
	LineTo ( hDC, lprectShadedArea->left, lprectShadedArea->top );

	MoveToEx ( hDC, lprectShadedArea->left,  lprectShadedArea->top, NULL );
	LineTo ( hDC, lprectShadedArea->right, lprectShadedArea->top );

	/**     White Shading
	**/
	hPen = CreatePen ( PS_SOLID, 0, RGB ( 255, 255, 255 ) );
	DeletePen ( SelectPen ( hDC, hPen ) );

	MoveToEx ( hDC, lprectShadedArea->right, lprectShadedArea->bottom, NULL );
	LineTo ( hDC, lprectShadedArea->right, lprectShadedArea->top );


	MoveToEx ( hDC, lprectShadedArea->right, lprectShadedArea->bottom, NULL );
	LineTo ( hDC, lprectShadedArea->left,  lprectShadedArea->bottom  );


	DeletePen ( SelectPen ( hDC, hOldPen ) );


	return ( OK );
}









/**	UpdateStatusBar
***
***	Paints the current status text onto the status bar.
***	This function may be called without invalidating the
***	client area.
**/
short UpdateStatusBar ( HWND hwndParent )
{

	HFONT   hOldFont;
	DWORD   dwOldTextColor;
	DWORD   dwOldBkColor;
	HDC     hdcClient;
	RECT    rectText;
	char    aszItemText [ NUM_STATUS_BAR_ITEMS ][ STATUS_BAR_ITEM_LENGTH + 1 ];
	short   nItem;
	char    *szPtrAbbrDirName;
	char    szDrive [ 3 ];
	char	cTemp [ _MAX_FNAME + 1 ];
	int		nIndex;
	
	struct  tm      *ptmGMT;        /* Ptr to Time Structure       */
	                                /* This will be returned from  */
	                                /* a call to gmTime            */

	hdcClient = GetDC ( hwndParent );

	/**
	***     Select the correct Font and
	***     Text Foreground/Background Colors
	**/
	hOldFont = 	 SelectFont ( hdcClient, hSansSerif_10 );
	dwOldTextColor = SetTextColor  ( hdcClient, GetSysColor ( COLOR_BTNTEXT ) );
	dwOldBkColor   = SetBkColor ( hdcClient, GetSysColor ( COLOR_BTNFACE ) );


	/**
	***	First get the text for the first status-box
	***	This is the actual logging status field
	**/

	if ( ( logInfo.bIsLogging ) && ( !logInfo.bIsPaused ) ) {


		strcpy ( aszItemText [ 0 ], "Running" );

	} else if ( logInfo.bIsPaused ) {

		strcpy ( aszItemText [ 0 ], "Paused" );

	} else {

		strcpy ( aszItemText [ 0 ], "Stopped" );
	}


	/**	Secondly get the text for the 'Data file directory'
	***	field, if settings.bLoggingMode == LOG_TO_SCREEN, use the
	***	text  Logging to Screen' otherwise display the full path
	***	name abbreviated as if necessary.
	**/
	if ( !settings.bLogDataToDisk ) {

		strcpy ( aszItemText [ 1 ], "Logging to Screen Only" );

	} else {

		/**	Shorten the Pathname if too long...
		**/
		szPtrAbbrDirName = settings.szDataDirectory;
		strncpy ( szDrive, settings.szDataDirectory, 2 );
		szDrive [ 2 ] = '\0';

		if ( strlen ( szPtrAbbrDirName ) >
					( STATUS_BAR_ITEM_LENGTH / 2 ) ) {

			/**	Remove unwanted components. Leave
			***	room for an addition six chars
			***	for the drive spec and '...'
			**/
			szPtrAbbrDirName =
				strrchr ( settings.szDataDirectory, '\\' );


		}
		if ( szPtrAbbrDirName != settings.szDataDirectory ) {

			/**	Prefix '..' to indicate that the directory
			***	name has been truncated..
			**/
			sprintf ( aszItemText [ 1 ], "%s\\...%s",
			               szDrive, szPtrAbbrDirName );

		} else {
			/**	Use the path name in its original form
			***	this is probably a drive specifiaction
			***	e.g 'e:'
			**/
			strncpy ( aszItemText [ 1 ],
					settings.szDataDirectory,
					STATUS_BAR_ITEM_LENGTH );
		}
	}




	/**     Next the Scan Interval. Adjust the number of decimal
	***     places displayed depending on the magnitude of the
	***     interval.
	**/
	if ( ( settings.dwScanInterval % 60000 ) == 0 ) {

		/**	A whole number of minutes
		**/
		sprintf ( aszItemText [ 3 ], "Interval: %.0lf minutes",
		                (double) settings.dwScanInterval / 60000.0 );

	} else if ( settings.dwScanInterval > 1000 ) {

		/**     1 Decimal Place for intervals greater than 1 sec
		**/
		sprintf ( aszItemText [ 3 ], "Interval: %.1lf seconds",
				(double) settings.dwScanInterval / 1000.0 );

	} else {
		/**	2 Decimal Places for small Intervals ( < 1 sec )
		**/
		sprintf ( aszItemText [ 3 ], "Interval: %.2lf seconds",
				(double) settings.dwScanInterval / 1000.0 );
	}


	/**
	***     Start Time, leave blank if not logging
	***
	**/
	if ( ( logInfo.bIsLogging ) && ( settings.bLogDataToDisk ) &&
					( settings.bPeriodicFiles ) ) {

		/**
		***	Periodic files in use get the extension ????
		**/
		strcpy ( cTemp, settings.szDataFileCurrent );

		strtok ( cTemp, "." );
		
		nIndex = strlen ( cTemp );
		--nIndex;

		while ( cTemp [ nIndex ] != '_' ) {
			--nIndex ;
		}

		strcpy ( aszItemText [ 2 ], "File :   " );
		strcat ( aszItemText [ 2 ], cTemp + nIndex );

	} else if ( logInfo.bIsLogging ) {

		/**	Non-periodic files, report time logging started
		**/
		ptmGMT = gmtime ( (time_t *) &logInfo.tmCalenderTimeStarted );
		strftime ( aszItemText [ 2 ], STATUS_BAR_ITEM_LENGTH,
					"Started at %H:%M:%S", ptmGMT );

	} else {
		aszItemText [ 2 ] [ 0 ] = '\0';
	}



	for ( nItem = 0; nItem < NUM_STATUS_BAR_ITEMS; nItem++ ) {

		/**     Create a clipping rectangle 1-pixel smaller than
		***     the items depressed rectangle...
		**/

		rectText = rectStatusBarItem [ nItem ];

		InsetRect ( &rectText, 1, 1 );

		ExtTextOut ( hdcClient, rectText.left, rectText.top,
			ETO_CLIPPED | ETO_OPAQUE, &rectText,
			aszItemText [ nItem ],
			strlen ( aszItemText [ nItem ] ), NULL );

	}




	SelectFont   ( hdcClient, hOldFont );
	SetTextColor ( hdcClient, dwOldTextColor );
	SetBkColor   ( hdcClient, dwOldBkColor );

	ReleaseDC ( hwndParent, hdcClient );

	return ( OK );

}





short UpdateButtonBar ( HWND hwndParent )
{

	BOOL	bIsEditBoxEditable;



	if ( !settings.bLogDataToDisk ) {

		/**	Logging to screen only so don't
		***	put a data file name in the edit box...
		**/
		Edit_SetText ( hEditBox, "" );

	} else {		
		
		Edit_SetText ( hEditBox, settings.szDataFile );
		
	}

	Button_Enable ( abutButtonInfo [ B_START   ].hButtonWnd,
		( conninfo.nNumConnected > 0 ) && ( !logInfo.bIsLogging ) );

	Button_Enable ( abutButtonInfo [ B_STOP    ].hButtonWnd,
						logInfo.bIsLogging );

	Button_Enable ( abutButtonInfo [ B_PAUSE   ].hButtonWnd,
				( !logInfo.bIsPaused ) && ( logInfo.bIsLogging ) );

	Button_Enable ( abutButtonInfo [ B_RESUME  ].hButtonWnd,
				( logInfo.bIsLogging && logInfo.bIsPaused ) );

	Button_Enable ( abutButtonInfo [ B_LOG ].hButtonWnd,
				( logInfo.bIsLogging && !logInfo.bIsPaused 
				                     && settings.bLogOnKeyPress ) );


	/**	Enable/Disable the filename edit box.
	***	It should be disabled :
	***		a) During Logging
	***		b) If logging to Screen
	**/
	bIsEditBoxEditable = ( ( !logInfo.bIsLogging ) &&
					( settings.bLogDataToDisk ) );

	if ( stricmp (settings.szDataFile, ".WL" ) == 0 ){
		bIsEditBoxEditable = 0;
	}

	EnableWindow ( hEditBox, bIsEditBoxEditable );

	SetFocus ( hEditBox );  /* Make sure Edit box does not lose focus */

	return ( OK );

}


short UpdateMenuBar ( HWND hwndParent )
{
	HMENU        hMenu;     /* Handle of Menu               */
	long         lAction;   /* New State For Menu Items 	*/

	hMenu 	= GetMenu ( hwndParent );


	/**     Disable menu items which we do
	***     not want to be used once logging has commenced
	**/
	lAction = logInfo.bIsLogging ? MF_DISABLED | MF_GRAYED : MF_ENABLED;
	EnableMenuItem  ( hMenu, IDM_LOADIMS, 	    lAction );
	EnableMenuItem  ( hMenu, IDM_RESTORE_SETUP, lAction );



	/**     Disable menu items which we do
	***     not want to be used once logging has commenced
	***	OR if no valid setup is loaded....
	**/
	lAction = ( ( logInfo.bIsLogging ) ||
	 ( conninfo.bIMSSetupLoaded == FALSE ) ) ? MF_DISABLED | MF_GRAYED
								: MF_ENABLED;

	EnableMenuItem	( hMenu, IDM_INPUTS,	 lAction );
	EnableMenuItem	( hMenu, IDM_SAVE_SETUP, lAction );
	EnableMenuItem	( hMenu, IDM_DATAFILE,	 lAction );
	EnableMenuItem  ( hMenu, IDM_FILEOPT,    lAction );

	/**	Disable menu items which we do
	***	not want to be used if no valid setup is loaded....
	**/
	lAction =  ( conninfo.bIMSSetupLoaded == FALSE ) ?
				MF_DISABLED | MF_GRAYED : MF_ENABLED;

	EnableMenuItem	( hMenu, IDM_SETTINGS, lAction );



	DrawMenuBar ( hwndParent );

	return ( OK );

}


/**
***    CreatePushButtons
***
***
***	Creates, positions and sizes push button controls
***	on the Button Bar area.
***
***
***
***
***
***
***
***
***
**/
void CreatePushButtons ( HWND hWndParent )
{

	short           nButton;
	HINSTANCE       hModuleInst;
	WNDPROC         lpfnEditBoxCallBackFn;

	/**     To create the Window we will first need to
	***     determine the modules instnace handle.
	**/
	hModuleInst = (HINSTANCE) GetWindowLong ( hWndParent, GWL_HINSTANCE );


	/**     We need to sub-class the edit-box control
	***     First create a Instance Thunk for the callback
	***     function linking the export func with the
	***     instance's data segment. The callback function
	***     will be used for all buttons.
	**/
	lpfnEditBoxCallBackFn = (WNDPROC) MakeProcInstance ( (FARPROC)
	                                      EditBox_WndProc, hModuleInst );

	/**     Now Create the Four Push Buttons.
	***     The style and button text are stored in the
	***     static array 'abutButtonInfo'.
	***     Initially the controls are not positioned or sized.
	**/

	for ( nButton = 0; nButton < NUM_BUTTONS; nButton++ ) {

		abutButtonInfo [ nButton ].hButtonWnd =
		           CreateWindow ( "button",
					abutButtonInfo [ nButton ].szText,
					WS_CHILD | WS_VISIBLE |
					abutButtonInfo [ nButton ].dwStyle,
					0, 0,
					0, 0,
					hWndParent,
					(HMENU) (IDC_START + nButton ),
					hModuleInst,
					NULL );



		ShowWindow ( abutButtonInfo [ nButton ].hButtonWnd, SW_SHOW );
		UpdateWindow ( abutButtonInfo [ nButton ].hButtonWnd );

		/**     Now we need to specify the callback fn that will
		***     intercept messages to the buttons prior to be
		***     processed by the standard button handler.
		***
		**/
		lpfnDefButtonHandler = NULL; /* Not Implemented */

		InvalidateRect ( abutButtonInfo [ nButton ].hButtonWnd,
								 NULL, TRUE );


	}
	/**     Store the handles in the logger Configuration structure
	***     and the default button handler which must be called by
	***     the ButtonCallBackFn.
	***
	**/

	hEditBox = CreateWindow ( "edit", "*.wl",
					WS_CHILD | WS_VISIBLE |
					WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
					0, 0,
					0, 0,
					hWndParent,
					(HMENU) IDC_FILEEDIT,
					hModuleInst,
					NULL );



	ShowWindow   ( hEditBox, SW_SHOW );
	UpdateWindow ( hEditBox );


	/**     Now we need to specify the callback fn that will
	***     intercept messages to the buttons prior to be
	***     processed by the standard button handler.
	***
	**/

	lpfnDefEditBoxHandler = SubclassWindow ( hEditBox,
					   lpfnEditBoxCallBackFn );

	SendMessage ( hEditBox, WM_SETFONT, (WPARAM) hSansSerif_8,
							(LPARAM) TRUE );


	Edit_LimitText ( hEditBox, MAX_EDITBOX_STRLEN );
	Edit_SetSel    ( hEditBox, 0, -1 );

	InvalidateRect ( hEditBox, NULL, TRUE );


	return;
}











/**     Function                EditBox_WndProc
***
***                             This is a callback function which
***                             may be used to intercept messages
***                             destined for the filename edit box
***                             located on the button bar. We will
***                             use this function to intercept
***                             messages destined for scroll bar
***                             or for the client ( F1 Help ).
***                             Accelerator Messages for the buttons
***                             are processed using WM_GETMENUCHAR
***                             messages.
***
**/
LRESULT CALLBACK EditBox_WndProc ( HWND hWnd, WORD iMessage,
					   WPARAM wParam, LPARAM lParam )
{

	/**
	***     Intercept Alt-Key/key stroke combinations
	***     and pass up to main application, these may
	***     be for the push-buttons, menus, whatever
	**/

	switch ( iMessage ) {

			

		case ( WM_CHAR ) :
		case ( WM_KEYUP ) :


			if ( HIWORD ( wParam ) & 0x2000 ) {
				/**	Accelerator Key, Forward to main Window
				**/
				return ( 0L );

				break; 
			} else {
				  
				/**	All other keys to the edit box
				**/
				return ( CallWindowProc ( lpfnDefEditBoxHandler,
						  hWnd, iMessage, wParam, lParam ) );
			}
		
 
		

		case ( WM_KEYDOWN ) :

			if ( HIWORD ( wParam ) & 0x2000 ) {
				/**	Accelerator Key, Forward to main Window
				**/
				return ( 0L );

				break;
			} 
			
		
			switch ( LOWORD (wParam) ) {

				case VK_F1 :
					SendMessage ( hwndMain, WM_COMMAND,
                				(WPARAM) IDM_HELP_CONTENTS, 0L );
					break;

				case VK_NEXT:

					if ( GetKeyState ( VK_CONTROL ) < 0 ) {

						/**     high bit set means key down
						***     so send pageup rather than lineup
						**/
						SendMessage ( hwndMain, WM_HSCROLL,
							  SB_PAGEUP, 0L );

					} else {
						SendMessage ( hwndMain, WM_HSCROLL,
							  SB_LINEUP, 0L );
					}
					break;


				case VK_PRIOR:

					if ( GetKeyState ( VK_CONTROL ) < 0 ) {

						/**     high bit set means key down
						***     so send pagedown rather than
						***	linedown
						**/
						SendMessage ( hwndMain, WM_HSCROLL,
								SB_PAGEDOWN, 0L );

					} else {
						SendMessage ( hwndMain, WM_HSCROLL,
								SB_LINEDOWN, 0L );
					}
					break;
			

				default :
					/**	All other keys to the edit box
					**/
					return ( CallWindowProc ( lpfnDefEditBoxHandler,
						  hWnd, iMessage, wParam, lParam ) );
						  
			} /* End WM_KEYDOWN */					  
		default : 

			return ( CallWindowProc ( lpfnDefEditBoxHandler,
						   hWnd, iMessage, wParam, lParam ) );
	}
	return 0L;
	
}





/**	Function DrawTimeDataDivision
***
***	Paints a single line which divides the time stamps from
***	the logged data.
***
**/
short   DrawTimeDataDivision ( HDC hdcClient )
{
	HPEN	hOldPen;

	/**
	***  Select a pen which contrasts with the background
	***
	**/
	hOldPen = SelectPen ( hdcClient,
		CreatePen ( PS_SOLID, 0, colors.crNormalText ) );



	MoveToEx ( hdcClient, rectLoggedData.left, rectLoggedData.top, NULL );
	LineTo ( hdcClient, rectLoggedData.left, rectStatusBar.top - 1 );


	DeletePen ( SelectPen ( hdcClient, hOldPen ) );

	return ( OK );

}









/**
***
***
***
***
***
***
***
***
***
***
***
***
***
***
**/
VOID PositionScrollBar ( VOID )
{


	MoveWindow ( hScrollBar,
	             rectScrollBar.left,
	             rectScrollBar.top,
	             rectScrollBar.right  - rectScrollBar.left,
	             rectScrollBar.bottom - rectScrollBar.top,
	             TRUE );

	return;
}






/**
***   CreateScrollBar
***
***
***
***
***
***
***
***
***
***
***
***
***
**/
VOID CreateScrollBar ( HWND hWndParent )
{
	HINSTANCE   	hModuleInst;


	/**     To create the Window we will first need to
	***     determine the modules instnace handle.
	**/
	hModuleInst = (HINSTANCE) GetWindowLong ( hWndParent, GWL_HINSTANCE );




	hScrollBar = CreateWindow ( "scrollbar", "",
	                                WS_CHILD | WS_VISIBLE |
	                                SBS_HORZ,
	                                0, 0,
	                                0, 0,
	                                hWndParent,
	                                (HMENU) IDC_HSCROLL,
	                                hModuleInst,
	                                NULL );

	ShowWindow   ( hScrollBar, SW_SHOW );
	UpdateWindow ( hScrollBar );

	return;
}







short PaintRealTimeClk ( HDC hdcClient )
{

	struct	tm     	*ptmGMT;        /* Ptr to Time Structure       */
	                                /* This will be returned from  */
	                                /* a call to gmTime            */

	char	        szTimeText [ MAX_TIMESTAMP_CHARS + 1 ];
	HFONT           hOldFont;
	COLORREF	crOldTextColor, crOldBkColor;



	/**	Select font and text colours
	**/
	hOldFont = SelectFont ( hdcClient, hSansSerif_8 );

	crOldTextColor = SetTextColor ( hdcClient, colors.crNormalText );
	crOldBkColor   = SetBkColor   ( hdcClient, colors.crNormalBk   );


	ptmGMT = gmtime ( (time_t *) &clkRealTime.timeNow );

	strftime ( szTimeText, MAX_TIMESTAMP_CHARS, "%H:%M:%S", ptmGMT );


	/**     Use ExtTextOut to ensure existing
	***	text is overwritten. To do this we have
	***     to define the rectangle occupied by the
	***	text.
	**/
	ExtTextOut ( hdcClient, rectRealTimeClk.left,
	        rectRealTimeClk.top, ETO_OPAQUE, &rectRealTimeClk,
	           szTimeText, strlen ( szTimeText ), NULL );



	TextOut ( hdcClient, rectRealTimeClk.left +
	            ( clntInfo.lAveTextWidth * 2 ),
	             rectRealTimeClk.top - clntInfo.lTextHeight,
	                "Time", 4 );

	SelectFont ( hdcClient, hOldFont );

	SetTextColor ( hdcClient, crOldTextColor );
	SetBkColor   ( hdcClient, crOldBkColor	);


	return ( OK );

}





/**	PaintLoggedData
***
***
**/
short PaintLoggedData ( HDC hdcClient, RECT *prectInvalidated )
{
	short           nColumn;
	short           nColumnsToDraw;
	short           nChanNo;
	long            lRow;
	long            lXPos;
	long            lYPos;
	HFONT           hOldFont;
	long            lFirstInvldRow;
	long            lLastInvldRow;
	DWORD           dwOldBkColor;
	DWORD           dwOldTextColor;
	HBRUSH          hbrushRect;
	RECT            rectOpaqueText;
	char            szCookedText [ MAX_VALUE + 1 ];
	UINT            unOldTextAlign;
	int				nChars;

	if ( ( conninfo.nNumConnected == 0 ) ||
	     ( conninfo.pciConnections == NULL ) ||
	     ( settings.bScreenDisplayOn == FALSE ) ) {

		/**     No data to Paint
		**/
		return ( OK );
	}

	hOldFont = SelectFont ( hdcClient, hSansSerif_8 );

	/**	Use right aligned text 
	***	Fixed placed IML values will then line up
	**/ 
	unOldTextAlign = SetTextAlign ( hdcClient, TA_RIGHT );
        
        

	/**
	***     Determine which rows have been Invalidated
	**/


	lFirstInvldRow = ( rectLoggedData.bottom -
	           prectInvalidated->bottom ) / clntInfo.lTextHeight;




	lLastInvldRow  = ( rectLoggedData.bottom -
	           prectInvalidated->top ) / clntInfo.lTextHeight;



	/**     Determine which connection will occupy the
	***     first column and how many columns can be seen.
	**/
	nColumnsToDraw = min ( clntInfo.lNumColumnsVisible,
	                                          conninfo.nNumConnected );


	if ( nColumnsToDraw > ( conninfo.nNumConnected -
	                		clntInfo.lFirstChanVisible ) ) {

	        /**     Adjust so we do NOT attempt to draw
	        ***     channels which have been scrolled off of
	        ***     the display.
	        **/
	        nColumnsToDraw = conninfo.nNumConnected -
	                		(short) clntInfo.lFirstChanVisible;

	}



	if ( lFirstInvldRow == 0 ) {

		/**     The first row contains the most recent
		***     data. For this reason the data in this
		***     row is inverted for emphasis.
		***
		***     Select appropriate Pens, and Brushs for
		***     this row...
		**/

		/**	Colors for non-data text...
		***	Remember to store the original colors
		***	and restore them before exiting the fn
                **/
		dwOldTextColor = SetTextColor ( hdcClient,
						colors.crNormalText );

		dwOldBkColor   = SetBkColor ( hdcClient,
						colors.crNormalBk );

		/**	Superimpose the text.
		***
		***     First The Logged time, then the Logged data
		**/
		lXPos = rectLoggedTime.left;

		lYPos = rectLoggedData.bottom -
		          ( ( lFirstInvldRow + 1 ) * clntInfo.lTextHeight );


		/**	Use ExtTextOut to ensure existing
		***	text is overwritten. To do this we have
		***	to define the rectangle occupied by the
		***	text.
		**/

		rectOpaqueText.left = lXPos;
		rectOpaqueText.right = lXPos +
			( clntInfo.lAveTextWidth * MAX_TIMESTAMP_CHARS );
		rectOpaqueText.top    = lYPos;
		rectOpaqueText.bottom = lYPos + clntInfo.lTextHeight;

		ExtTextOut ( hdcClient, 
			  rectOpaqueText.right, rectOpaqueText.top, 
			  ETO_OPAQUE,
			  &rectOpaqueText,
			  clntInfo.alpHistIndx [ 0 ]->szTime,
			  lstrlen ( clntInfo.alpHistIndx [ 0 ]->szTime ),
			  NULL );




		for ( nColumn = 0; nColumn < nColumnsToDraw; nColumn++ ) {



			/**     Establish which channel we are 'drawing'
			***     this will not necessarily be channel 0
			***     if the user has used the scroll-bar.
			**/
			nChanNo =   nColumn + (short) clntInfo.lFirstChanVisible;



			/**     It may be more efficient to use
			***     ExtTextOut here and not erase
			***     the previous text when invalidating
			***     the area.. For now I will simply use
			***     TextOut
			**/
			lXPos = rectLoggedData.left + clntInfo.lAveTextWidth +
			   ( nColumn * clntInfo.lAveTextWidth *
						   clntInfo.lColumnWidth );

			/**     Each row has height nTextHeight. The first
			***     row is the row that touches the bottom of the
			***     logged data area...
			**/
			lYPos = rectLoggedData.bottom -
			  ( ( lFirstInvldRow + 1 ) * clntInfo.lTextHeight );



			/**	Determine the color of the data field
			**/
			switch ( clntInfo.alpHistIndx [ 0 ]->
					anAlarmState [ nChanNo ] ) {

				default 	      :
				case ( ALARM_NORMAL ) :

					SetBkColor   ( hdcClient, colors.crNormalBk );
					SetTextColor ( hdcClient, colors.crNormalText  );
					break;

				case ( ALARM_WARNING ) :

					SetBkColor   ( hdcClient, colors.crWarningBk );
					SetTextColor ( hdcClient, colors.crWarningText	);
					break;

				case ( ALARM_CRITICAL ) :

					SetBkColor   ( hdcClient, colors.crCriticalBk );
					SetTextColor ( hdcClient, colors.crCriticalText );
					break;
			}


			/**     Use ExtTextOut to ensure existing
			***     text is overwritten. To do this we have
			***     to define the rectangle occupied by the
			***     text.
			**/

			rectOpaqueText.left = lXPos;
			rectOpaqueText.right = lXPos +
				( clntInfo.lAveTextWidth *
						clntInfo.lColumnWidth );
			rectOpaqueText.top   = lYPos;
			rectOpaqueText.bottom= lYPos + clntInfo.lTextHeight;


			nChars = GetMaxChars ( hdcClient, clntInfo.alpHistIndx[0]
				                                   ->aszData[nChanNo] );

			/**	If necessary abbreviate the field
			***	data to fit in the current column width
			**/
			if ( lstrlen( clntInfo.alpHistIndx [ 0 ]->aszData [ nChanNo ])
				         > nChars ) {

				/**  String too long copy 'n' chars into
				***  a new buffer before outputing...
				**/
				lstrcpyn ( (LPSTR) szCookedText,
					clntInfo.alpHistIndx [ 0 ]->
					    aszData [ nChanNo ],
						nChars );

				/**  Change the last character to indicate
				***  that the string is abbreviated and
				***  terminate the string
				**/
				szCookedText [ nChars - 1 ] =
								  (char) 187;

				szCookedText [ nChars ] = '\0';


				ExtTextOut ( hdcClient, 
				  rectOpaqueText.right, rectOpaqueText.top,
				  ETO_OPAQUE, &rectOpaqueText,
				  (LPSTR) szCookedText,
				  lstrlen ( (LPSTR) szCookedText ),
				  NULL );

			} else {

				/**	Text is okay, print directly
				***	from the Historic Index array
				**/
				ExtTextOut ( hdcClient, 
				  rectOpaqueText.right, rectOpaqueText.top,
				  ETO_OPAQUE, &rectOpaqueText,
				  clntInfo.alpHistIndx [ 0 ]->
						aszData [ nChanNo ],
				  lstrlen ( clntInfo.alpHistIndx [ 0 ]->
						aszData [ nChanNo ] ),
				  NULL );

			}


		}



		/**	Only paint the rectangle if logging
		***	Use the FrameRect so not to overwrite
		***	the data field text within the the rectangle.
		**/
		if ( logInfo.bIsLogging ) {


			/**	Create a brush for the rect frame
			**/
			hbrushRect = CreateSolidBrush ( colors.crNormalText );

			FrameRect ( hdcClient, (LPRECT) &rectHighlight,
								hbrushRect );

			DeleteBrush  ( hbrushRect );

		}






		/**     Exclude this row from any further processing
		**/
		lFirstInvldRow += 1;

	}
	/**     Now the remaining Rows...
	**/
	for ( lRow = lFirstInvldRow; lRow <= lLastInvldRow; lRow++ ) {


		/**    Logged Time First...
		**/
		lXPos = rectLoggedTime.left;

		lYPos = rectLoggedData.bottom -
			 ( ( lRow + 1 ) * clntInfo.lTextHeight );

		/**     Define the rectangle in which to draw the text
		***     This will ensure any old text will be removed
		**/
		rectOpaqueText.left 	= lXPos;
		rectOpaqueText.right   	= lXPos +
			( clntInfo.lAveTextWidth * MAX_TIMESTAMP_CHARS );
		rectOpaqueText.top    	= lYPos;
		rectOpaqueText.bottom 	= lYPos + clntInfo.lTextHeight;

		/**	Use 'No Alarm' colors for time stamp
		**/
		SetTextColor ( hdcClient, colors.crNormalText );

		SetBkColor   ( hdcClient, colors.crNormalBk );



		ExtTextOut ( hdcClient, rectOpaqueText.right, rectOpaqueText.top, 
		        ETO_OPAQUE,
		        &rectOpaqueText,
		        clntInfo.alpHistIndx [ lRow ]->szTime,
		        lstrlen ( clntInfo.alpHistIndx [ lRow ]->szTime ),
		        NULL );


		/**    Now Logged Data
		**/
		for ( nColumn = 0; nColumn < nColumnsToDraw; nColumn++ ) {


			/**     Establish which channel we are 'drawing'
			***     this will not necessarily be channel 0
			***     if the user has used the scroll-bar.
			**/
			nChanNo = nColumn + (short) clntInfo.lFirstChanVisible;



			/**     It may be more efficient to use
			***     ExtTextOut here and not erase
			***     the previous text when invalidating
			***     the area.. For now I will simply use
			***     TextOut
			**/
			lXPos = rectLoggedData.left + clntInfo.lAveTextWidth +
			   ( nColumn * clntInfo.lAveTextWidth *
						clntInfo.lColumnWidth );

			/**     Each row has height nTextHeight. The first
			***     row is the row that touches the bottom of the
			***     logged data area...
			**/
			lYPos = rectLoggedData.bottom -
			      ( ( lRow + 1 ) * clntInfo.lTextHeight );

			/**     Define the rectangle in which to draw the text
			***     This will ensure any old text will be removed
			**/
			rectOpaqueText.left 	= lXPos;
			rectOpaqueText.right  	= lXPos +
			  ( clntInfo.lAveTextWidth * clntInfo.lColumnWidth );
			rectOpaqueText.top   	= lYPos;
			rectOpaqueText.bottom	= lYPos + clntInfo.lTextHeight;



			/**	Select the foreground and background colors
			***	for this data value.
			**/
			switch ( clntInfo.alpHistIndx [ lRow ]->
					anAlarmState [ nChanNo ] ) {

				default 	      :
				case ( ALARM_NORMAL ) :

					SetBkColor   ( hdcClient, colors.crNormalBk );
					SetTextColor ( hdcClient, colors.crNormalText  );
					break;

				case ( ALARM_WARNING ) :

					SetBkColor   ( hdcClient, colors.crWarningBk );
					SetTextColor ( hdcClient, colors.crWarningText	);
					break;

				case ( ALARM_CRITICAL ) :

					SetBkColor   ( hdcClient, colors.crCriticalBk );
					SetTextColor ( hdcClient, colors.crCriticalText );
					break;
			}

			/**	If necessary abbriviate the field
			***	data to fit in the current column width
			**/
			nChars = GetMaxChars ( hdcClient, clntInfo.alpHistIndx[lRow]
				                              ->aszData[nChanNo] );

			
			if ( lstrlen (
			    clntInfo.alpHistIndx [ lRow ]->aszData
				[ nChanNo ] ) > nChars ) {

				/**  String too long copy 'n' chars into
				***  a new buffer before outputing...
				**/
				lstrcpyn ( (LPSTR) szCookedText,
					clntInfo.alpHistIndx [ lRow ]->
					    aszData [ nChanNo ],
						nChars );

				/**  Change the last character to indicate
				***  that the string is abbreviated and
				***  terminate the string  187 is a double
				***  arrow character.
				**/
				szCookedText [ nChars	- 1 ] =
								  (char) 187;

				szCookedText [ nChars ] = '\0';


				ExtTextOut ( hdcClient, 
					rectOpaqueText.right, rectOpaqueText.top,
					ETO_OPAQUE, &rectOpaqueText,
					(LPSTR) szCookedText,
					lstrlen ( (LPSTR) szCookedText ),
					NULL );

			} else {

				/**	Text is okay, print directly
				***	from
				**/
				ExtTextOut ( hdcClient, 
					rectOpaqueText.right, rectOpaqueText.top,
					ETO_OPAQUE, &rectOpaqueText,
				  	clntInfo.alpHistIndx [ lRow ]->
						aszData [ nChanNo ],
				  	lstrlen ( clntInfo.alpHistIndx [ lRow ]->
						aszData [ nChanNo ] ),
				  	NULL );

			}



		}       /* Next Column */


	}       /* Next Row */


	/**	Reset original text colors and font
	**/
	dwOldTextColor = SetTextColor ( hdcClient, dwOldTextColor );

	dwOldBkColor   = SetBkColor ( hdcClient, dwOldBkColor );

	SelectFont  ( hdcClient, hOldFont );
	SetTextAlign ( hdcClient, unOldTextAlign );
       

	return ( OK );
}







short PaintChanLabels ( HDC hdcClient )
{
	long            lYPos;
	long            lXPos, lXRightPos;
	short           nColumn;
	short           nChanNo;
	short           nColumnsToDraw;
	HFONT		hOldFont;
	COLORREF	crOldTextColor, crOldBkColor;
	UINT		unOldTextAlign;






	if ( ( conninfo.nNumConnected == 0 ) ||
	     ( conninfo.pciConnections == NULL ) ) {

		/**     No data to Paint
		**/
		return ( OK );
	}



	/**	Select font and text colours
	**/
	hOldFont = SelectFont ( hdcClient, hSansSerif_8 ); 
	
	/**	Use right aligned text to match data values
	**/ 
	unOldTextAlign = SetTextAlign ( hdcClient, TA_RIGHT );
    
	
	

	crOldTextColor = SetTextColor ( hdcClient, colors.crNormalText );
	crOldBkColor   = SetBkColor   ( hdcClient, colors.crNormalBk   );





	/**     Caclulate how many columns are actually visible
	***     on the client. This can not exceed the total number
	***     of connected channels.
	**/
	nColumnsToDraw = min ( clntInfo.lNumColumnsVisible,
	                                          conninfo.nNumConnected );

	if ( nColumnsToDraw > ( conninfo.nNumConnected -
	                		clntInfo.lFirstChanVisible ) ) {

		/**     Adjust so we do NOT attempt to draw
		***     channels which have been scrolled off of
		***     the display.
		**/
		nColumnsToDraw = conninfo.nNumConnected -
		        		(short) clntInfo.lFirstChanVisible;

	}


	for ( nColumn = 0; nColumn < nColumnsToDraw; nColumn++ ) {

		lYPos = rectChanLabels.top;
		lXPos = rectChanLabels.left + clntInfo.lAveTextWidth +
		           ( nColumn * clntInfo.lAveTextWidth *
					    clntInfo.lColumnWidth );
					    
		lXRightPos = lXPos + ( clntInfo.lAveTextWidth *
					    clntInfo.lColumnWidth );

		/**     Establish which channel we are 'drawing'
		***     this will not necessarily be channel 0
		***     if the user has used the scroll-bar.
		**/
		nChanNo = nColumn + (short) clntInfo.lFirstChanVisible;




		/**     Write Channel Name
		**/
		TextOut ( hdcClient, 
			lXRightPos,	/* specify Right end for Right Justified Text */ 
			lYPos,
			conninfo.pciConnections [ nChanNo ].szChannel,
			GetMaxChars ( hdcClient, 
			       conninfo.pciConnections[nChanNo].szChannel ) );


		/**     Write Channel Units, on next line down
		**/
		lYPos += clntInfo.lTextHeight;

		TextOut ( hdcClient, 
			lXRightPos,     /* specify Right end for Right Justified Text */
			lYPos,
			conninfo.pciConnections [ nChanNo ].szUnits,
			min ( settings.wCharsPerColumn, strlen ( 
				conninfo.pciConnections [ nChanNo ].szUnits ) ) );


	}
	/**     Reset Text Color/Font and return
	**/
	SetTextColor ( hdcClient, crOldTextColor );
	SetBkColor   ( hdcClient, crOldBkColor );

	SelectFont   ( hdcClient, hOldFont );

	SetTextAlign ( hdcClient, unOldTextAlign );


	return ( OK );
}




/************************************************
/** The column width is selected by the user from the settings
*** dialog as up to 15 average width chars. If the user defines
*** a channel name which is, for example ,all upper case then
*** the selected number of chars may not fit the column. This
*** routine decides the number of chars from a string which will
*** fit the column.
**/


int GetMaxChars ( HDC hdcClient, char* szText )

{
	SIZE sizeRect ;
	int nChars;
	int nNumAverageChars = 255; //start at a large size
	BOOL bSearch = TRUE;

	
	/** The desired number of chars is either the full string
	*** or the dialog selected number of chars whichever is smaller.
	**/
	nChars = min ( settings.wCharsPerColumn, strlen ( szText ) );

	
	
	/** reduce the string as required to make it fit within the 
	*** available number of average size chars.
	**/
	while ( bSearch) {
	
		/** Get the size of the desired text **/
		GetTextExtentPoint ( hdcClient, szText, nChars, &sizeRect );

		/** How many average size chars is this ? **/
		nNumAverageChars = sizeRect.cx / clntInfo.lAveTextWidth;

		if (nNumAverageChars > settings.wCharsPerColumn) {
			--nChars;
		} else {
			bSearch = FALSE;
		}
	}

	/** Return the number of chars that will fit **/
	return ( nChars );


}
