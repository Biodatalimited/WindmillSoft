/**
***	WPDO_DRW.c		ATK-PANELS
***				Paint routines for Windmill Panels
***				Application Shell.
***     Andrew Thomson
***
***	This module is responsible for painting the client area
***	including the status and button bars.
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

#include <string.h>

#include "\Windmill 32\Compile Libs\IMLinter.h"   /* ...using the COMMSLIB Dynalink		 */
#include "\Windmill 32\Compile Libs\imslib.h"     /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATkit\ATKit.h"      /* Application Toolkit Header/Prototypes         */

#include "WPDO_def.h"	/* Manifest Constants and Structure Declarations */
          /* Functions Prototypes*/



/**	Externally defined Globals Variables and structures
**/
extern HINSTANCE  hInst;    /* Program Instance Handle	    */
extern HWND	  hwndMain;     /* Program Window Handle	    */

/**	Fonts used by Application
**/
extern HFONT    hSansSerif_8;          /*  8 Point Sans Serif Font */
extern HFONT    hSansSerif_10;         /* 10 Point Sans Serif Font */
extern HBITMAP  hbmpLEDOn;             /* LED bitmap (on state)    */
extern HBITMAP  hbmpLEDOff;            /* LED bitmap (off state)   */
extern HBITMAP  hbmpLEDMask;           /* LED bitmaps' common mask */
extern HBITMAP  hbmpExclaimationMask;
extern HBITMAP  hbmpExclaimation;

extern RECT rectButtonBar;  /* Area occupied by Push-button Bar		   */
extern RECT rectStatusBar;  /* Area occupied by Status Bar		   */
extern RECT rectData;	    /* Area occupied by Channel Name, Data & Units */
extern RECT rectScrollBar;  /* Area occupied by Vertical Scroll Bar	   */

/**	Further divide the data area into fields
**/
extern RECT rectNames;      /* Area occupied by Channel names  */
extern RECT rectValues;     /* Area occupied by Data Values    */
extern RECT rectUnits;      /* Area occupied by Channel units  */


/**	Individual rectangles for status bar text.
***	These allow long text to be clipped correctly by
***	using the ExtTextOut fn.
**/
extern RECT rectStatusBarItem [ NUM_STATUS_BAR_ITEMS  ];


/**	Application Details
**/
extern PANELINFO	panDigitalOut;
extern CONTROLS		controls;
extern ATKCOLOURINFO	colors;	      /* text colors for three alarm states */



/**	Channel Connection Information
**/
extern CONNECTIONS conninfo;




BUTTONBAR abutButtonInfo [ NUM_BUTTONS ] = {

	NULL, BS_PUSHBUTTON, "All O&n",	  0, 0,
	NULL, BS_PUSHBUTTON, "All &Off",  0, 0,
	NULL, BS_PUSHBUTTON, "&Mode",	  0, 0,


};


/**
***     Handles of client area controls.....
**/
extern HWND	hScrollBar;
extern HWND	hModeText;

BUTTONBAR abutChanButton [ MAX_CHAN_BUTTONS ];



/**	Text strings for Application Status
***	This array may be indexed by STATUS_
***	manifest constants.
**/
char *aszStatusText [ NUM_STATES ] = {

	"No Setup",
	"Ready",
	"Auto",
	"Busy",

};



/**	The following are pointers to default control handlers
***	These will be used when sub-classing the controls.
**/
WNDPROC	lpfnDefButtonHandler;




/**
***	Function	CreateStatusBar
***
***	This function 'creates' the Status bar.
***	This may involve creating text controls to be
***	displayed on the bar
***
***
**/
short CreateStatusBar ( HWND hWndMain )
{
	return ( OK );
}




/**	Function	DrawStatusBar
***
***	This function should be called if the status bar has
***	been invalidated. This may happen if the Window has been
***	resized or another Window has covered all of part of the
***	StatusBar Area.	The complete bar will be redrawn.
***
***	This function is different from UpdateStatusBar which
***	merely changes the text in the bar's text controls to
***	notify the operator of a change of status..
***
**/
short DrawStatusBar ( HDC hDC )
{

	HPEN            hPen, hOldPen;
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


	MoveToEx ( hDC, rectStatusBar.left + 1, rectStatusBar.bottom - 1, NULL );
	LineTo ( hDC, rectStatusBar.left + 1, rectStatusBar.top + 1 );

	MoveToEx ( hDC, rectStatusBar.left  + 1, rectStatusBar.top + 1, NULL );
	LineTo ( hDC, rectStatusBar.right - 1, rectStatusBar.top + 1 );

	/**     White Shading
	**/
	hPen = CreatePen ( PS_SOLID, 0, GetSysColor ( COLOR_BTNSHADOW ) );
	DeletePen ( SelectPen ( hDC, hPen ) );


	DeletePen    ( SelectPen   ( hDC, hOldPen ) );
	DeleteBrush  ( SelectBrush ( hDC, hOldBrush ) );



	lStatusBarLength = STATUS_BAR_ITEM_LENGTH * panDigitalOut.lAveTextWidth;

	for ( nItem = 0; nItem < NUM_STATUS_BAR_ITEMS; nItem++ ) {


		rectStatusBarItem [ nItem ].top	   = rectStatusBar.top + 4;
		rectStatusBarItem [ nItem ].bottom = rectStatusBar.bottom - 4;
		rectStatusBarItem [ nItem ].left   = rectStatusBar.left
		                    + ( ( lStatusBarLength + 4 ) * nItem ) + 4;

		rectStatusBarItem [ nItem ].right  = rectStatusBarItem [ nItem ].left
						     +	lStatusBarLength;

		/**     Is the rectangle visible or partially visible
		**/
		if ( rectStatusBarItem [ nItem ].left <
		                                rectStatusBar.right ) {

			DepressRect ( hDC, &rectStatusBarItem [ nItem ] );

		}
	}

	return ( OK );
}

/**    Function 	DepressRect
***
***    Description	This function is called exclusively from
***			DrawStatusBar. It draws a rectangle of the
***			stated size and adds shading to give the
***			appearance of being recessed.
***
**/
static short DepressRect ( HDC hDC, LPRECT lprectShadedArea )
{
	HPEN     hPen, hOldPen;



        /**     Add Shading,
        ***	White Lines on Bottom and Right
        ***     Button Border on Top on Left
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




/**	Function	UpdateStatusBar
***
***	This function wil change the text on the status bar
***	and should be called if the status bar text changes.
***	It will achieve this without invalidating the Status bar
***	area.
***
***
***	This function is different from the DrawStatusBar function
***	since it WILL NOT actually repaint the status bar area.
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





	hdcClient = GetDC ( hwndParent );

	/**
	***     Select the correct Font and
	***	Text Foreground/Background Colors
	**/
	hOldFont = 	 SelectFont ( hdcClient, hSansSerif_10 );
	dwOldTextColor = SetTextColor  ( hdcClient, GetSysColor ( COLOR_BTNTEXT ) );
	dwOldBkColor   = SetBkColor ( hdcClient, GetSysColor ( COLOR_BTNFACE ) );



	/**	PUT CURRENT TEXT INTO aszItemText ARRAY
	***
	***			:
	***			:
	***			:
	**/
	strncpy ( aszItemText [ 0 ], aszStatusText [ panDigitalOut.nStatus ],
						   STATUS_BAR_ITEM_LENGTH );



	/**  Report number of DDE links currently connected.
	***
	**/
	switch ( panDigitalOut.nNumDDELinks ) {

		case 0 : strncpy ( aszItemText [ 1 ], "No DDE Links",
							   STATUS_BAR_ITEM_LENGTH );
			 break;

		case 1 : sprintf ( aszItemText [ 1 ], "1 DDE Link" );

			 break;

		default:

			/**	Report 'n' Number of links
			**/
			sprintf ( aszItemText [ 1 ], "%d DDE Links",
							panDigitalOut.nNumDDELinks );
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







/**
***	Function	CreateButtonBar
***
***	This function 'creates' the Button bar.
***	This may involve creating edit/text/push button	controls to be
***	displayed on the bar
***
***
**/
short CreateButtonBar ( HWND hWndParent )
{


	short        nButton;
	HINSTANCE    hModuleInst;

	/**     To create the Controls we will first need to
	***     determine the modules instance handle.
	**/
	hModuleInst = (HINSTANCE) GetWindowLong ( hWndParent, GWL_HINSTANCE );


	/**     Now Create the Push-Buttons.
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
					(HMENU) (IDC_BUTTON_START + nButton ),
					hModuleInst,
					NULL );



		/**	Note, we are not going to subclass the menu button
		***	controls since we do not allow them to keep the focus
		***	and hence they do not receive any keyboard input
		***	intended for the main application.
		***	Menu buttons simply notify their parent of button presses
		***	using the standard DefButtonHandler.
		***
		***	The keyboard interface to these buttons is via accelarator
		***	keys. These are 'caught' by Window's and sent to the
		***	applications WndProc via a WM_MENUCHAR message i.e
		***	they are considered to be an extension to the applications
		***	menu-bar.
		**/

		ShowWindow ( abutButtonInfo [ nButton ].hButtonWnd, SW_SHOW );
		UpdateWindow ( abutButtonInfo [ nButton ].hButtonWnd );



		InvalidateRect ( abutButtonInfo [ nButton ].hButtonWnd,
								 NULL, TRUE );


	}



	/**	Lastly create the static text control which informs
	***	the user whether the application is in immediate
	***	or deffered mode
	**/
	hModeText = CreateWindow ( "static", "Mode : Immediate",
					WS_CHILD | WS_VISIBLE,
					0, 0,
					0, 0,
					hWndParent,
					(HMENU) IDC_EDITBOX,
					hModuleInst,
					NULL );


	ShowWindow   ( hModeText, SW_SHOW );
	UpdateWindow ( hModeText );

	return ( OK );
}



/**	Function	DrawButtonBar
***
***	This function should be called if the Button bar has
***	been invalidated. This may happen if the Window has been
***	resized or another Window has covered all of part of the
***	StatusBar Area.	The complete bar will be redrawn.
***
***	This function is different from UpdateButtonBar which
***	performs only updates the control on the bar but does
***	not repaint the bar itself.
**/
short DrawButtonBar ( HDC hDC )
{
	HPEN    hOldPen;
	HBRUSH  hOldBrush;
	DWORD   dwOldTextColor;
	DWORD   dwOldBkColor;


	long    lRectHeight;
	long    lRectWidth;

	short   nXBorder;
	short   nYBorder;

	short   nYButtonPos;
	short   nFirstXButtonPos;



	short   nIndex;


	short   nButtonLength  = controls.nPushButtonWidth;
	long    lButtonHeight  = controls.nPushButtonHeight;


	long   lTextLength;  /* Mode description text control */
	long   lTextHeight;
	long   lTextXPos;
	long   lTextYPos;


	/**	Draw ButtonBar Background, Button Coloured Solid
	***	Interior with a black Border.
	**/
	hOldPen  = SelectPen ( hDC, CreatePen ( PS_SOLID, 0,
	                                   RGB ( 0, 0, 0 ) ) );

	hOldBrush= SelectBrush ( hDC, CreateSolidBrush (
				    GetSysColor ( COLOR_BTNFACE ) ) );


	dwOldTextColor = SetTextColor  ( hDC, GetSysColor ( COLOR_BTNTEXT ) );
	dwOldBkColor   = SetBkColor    ( hDC, GetSysColor ( COLOR_BTNFACE ) );


	Rectangle ( hDC, rectButtonBar.left,
	                 rectButtonBar.top,
	                 rectButtonBar.right,
			 rectButtonBar.bottom );



	/**     The button bar is 'organised' as follows:
	***
	***     A row of two buttons and a row with one button
	***     and one static text control
	***
	***
	***     The area assigned for the button bar is stored
	***     in the static rectangle rectButtonBar from this
	***     determine how to organise the bar.
	***
	***
	***     Allow for different fonts.
	**/






	/**     Calculate useful 'constants'
	**/
	lRectHeight    = rectButtonBar.bottom - rectButtonBar.top;
	lRectWidth     = rectButtonBar.right  - rectButtonBar.left;
	nXBorder       = controls.nBarBorder;



	/**	Use a double row Button Bar
	**/
	nYBorder   = ( lRectHeight - ( 2 * lButtonHeight ) ) / 3;
	nYButtonPos = rectButtonBar.top + nYBorder;

	nFirstXButtonPos = nXBorder;







	/**	Buttons are arranged in two rows
	***	The first row contains the AllOn and AllOff buttons
	***	The second row contains the Mode button. The
	***	Auto Edit box, Auto and Stop buttons will be implemented
	***	in Version 3
	***
	**/
	abutButtonInfo [ B_ALLON ].nXPos = nFirstXButtonPos;
	abutButtonInfo [ B_ALLON ].nYPos = nYButtonPos;



	abutButtonInfo [ B_ALLOFF ].nXPos = abutButtonInfo [ 0 ].nXPos +
						nButtonLength + nXBorder;

	abutButtonInfo [ B_ALLOFF ].nYPos = nYButtonPos;



	abutButtonInfo [ B_MODE ].nXPos = nFirstXButtonPos;
	abutButtonInfo [ B_MODE ].nYPos = nYButtonPos + lButtonHeight +
								nYBorder;



	for ( nIndex = 0; nIndex < NUM_BUTTONS; nIndex++ ) {

		MoveWindow ( abutButtonInfo [ nIndex ].hButtonWnd,
				abutButtonInfo [ nIndex ].nXPos,
				abutButtonInfo [ nIndex ].nYPos,
				nButtonLength,
				lButtonHeight,
				TRUE );


	}

	/**	Position the Mode Text to the right of thh 'Mode' button
	***
	**/
	lTextXPos = abutButtonInfo [ B_MODE ].nXPos +
						nButtonLength + nXBorder;

	/**	Vertically centre the text to align with the mode button
	**/
	lTextYPos = abutButtonInfo [ B_MODE ].nYPos +
			( lButtonHeight - panDigitalOut.lTextHeight ) / 2;


	lTextLength = rectButtonBar.right - lTextXPos - nXBorder;
	lTextHeight = panDigitalOut.lTextHeight;


	MoveWindow ( hModeText, lTextXPos, lTextYPos,
				lTextLength, lTextHeight, TRUE );






	/**     Tidy Up, restoring old objects
	**/
	DeletePen    ( SelectPen   ( hDC, hOldPen ) );
	DeleteBrush  ( SelectBrush ( hDC, hOldBrush ) );
	SetTextColor ( hDC, dwOldTextColor );
	SetBkColor   ( hDC, dwOldBkColor );

	return ( OK );
}




/**	Function	UpdateButtonBar
***
***	This function wil change the text on the button bar
***	and should be called if any of the controls need changing
***	e,g enabling \ disabling buttons, changing text etc.
***	It will achieve this without invalidating the button bar
***	area.
***
***
***	This function is different from the DrawButtonBar function
***	since it WILL NOT actually repaint the button bar area.
**/
short UpdateButtonBar ( HWND hWndMain )
{

	char	szModeText [ MAX_MODE_TEXT + 1 ];


	/**	'AllOn' button enabled if connections available
	**/
	Button_Enable ( abutButtonInfo [ B_ALLON ].hButtonWnd,
					( conninfo.nNumConnected > 0 ) );


	/**	'AllOff' button enabled if connections available
	**/
	Button_Enable ( abutButtonInfo [ B_ALLOFF ].hButtonWnd,
					( conninfo.nNumConnected > 0 ) );

	/**	'Mode' button enabled if connections available
	**/
	Button_Enable ( abutButtonInfo [ B_MODE ].hButtonWnd,
					( conninfo.nNumConnected > 0 ) );




	/**	Update the static text detailing the mode of operation
	***	as being immediate or deffered....
	**/
	switch ( panDigitalOut.nMode ) {

	default :
	case MODE_IMMEDIATE :

		strcpy ( szModeText, "Mode: Immediate" );
		break;

	case MODE_DEFERRED :


		strcpy ( szModeText, "Mode: Deferred" );
		break;
	}


	Static_SetText ( hModeText, (LPSTR) szModeText );




	return ( OK );
}




/**	Function	CreateDataArea
***
***	This function should be called to perform any initialization
***	required by the data area. e.g. creating client area controls etc.
***
**/
short CreateDataArea ( HWND hwndParent )
{
	short		nChanButton;
	WNDPROC 	lpfnButtonCallBackFn;
	HINSTANCE	hModuleInst;
	HCURSOR 	hcursorOld;


	/**	To create the Window we will first need to
	***	determine the modules instance handle.
	**/
	hModuleInst = (HINSTANCE) GetWindowLong ( hwndParent, GWL_HINSTANCE );

	/**	Create an Instance thunk for PushButton_WndProc
	***	Each push-button created will be subclassed using
	***	this procedure.
	**/
	lpfnButtonCallBackFn = (WNDPROC) MakeProcInstance ( (FARPROC)
					    PushButton_WndProc, hModuleInst );

	/**	Creating a screenfull of push-buttons may take
	***	several seconds, so display the hour glass cursor
	**/

	hcursorOld = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) );


	/**	Create a push-button control for each potential 'text row'
	***	visible.
	**/
	for ( nChanButton = 0; nChanButton < panDigitalOut.lMaxRowsVisible;
							   nChanButton++ ) {

		abutChanButton [ nChanButton ].hButtonWnd =
				CreateWindow ( "button",
					NULL,
					WS_CHILD | WS_VISIBLE |
					BS_PUSHBUTTON,
					0, 0,
					0, 0,
					hwndParent,
					(HMENU) ( 500 + nChanButton ),
					hModuleInst,
					NULL );





		/**	Subclass the push buttons control.
		***	This will allow us to intercept its
		***	keyboard messages and redirect them
		***	to DigitalOut_WndProc for processing.
		**/

		lpfnDefButtonHandler = SubclassWindow (
				abutChanButton [ nChanButton ].hButtonWnd,
				lpfnButtonCallBackFn );




	}


	/**	Remove the Hour glass cursor.
	***	NB Loaded Cursors do have to be deleted.
	**/
	SetCursor ( hcursorOld );
	return ( OK );
}






/**	Function	DrawDataArea
***
***	This function should be called if the Data Area bar has
***	been invalidated. This may happen if the Window has been
***	resized, another Window has covered all of part of the
***	Data Area or the scrollbar has been used.
***
***	The whole area of part-thereof will be redrawn depending
***	on the area invalidated.
***
***
**/
short DrawDataArea ( HDC hdcClient, LPPAINTSTRUCT lpps )
{

	long	lChanIndex;		/* Channel Index		*/
	long	lTextYPos;		/* Y Position (all fields)	*/

	long	lLEDXPos;		/* X Position of bitmaps	*/
	long	lExclaimationXPos;	/* and Text fields		*/
	long	lTextXPos;

	BOOL	bDrawNames;		/* Name Invalidated ?		*/
	BOOL	bDrawValues;		/* Values Invalidated ? 	*/
	BOOL	bDrawUnits;		/* Units Invalidated  ? 	*/

	long	lNumChansVisible;
	long	lFirstInvalidRow;
	long	lLastInvalidRow;
	long	lRow;
	RECT	rectIntersect;

	HFONT	hfontOld;
	DWORD	dwOldTextColor;
	DWORD	dwOldBkColor;


	long	lJustifiedXPos;
	long	lCentredYPos;

	char	szValueText [ MAX_VALUE + 1 ];

	short	nLEDState;	       /* 0 if LED off, otherwise non-zero */


	/**	Determine how much of the client is invalid.
	***	We need to know :
	***
	***	1) Which rows are invalidated
	***	2) Which fields are invalidated
	**/

	bDrawValues = TRUE;		/* Always redraw data values */

	bDrawNames  = ( IntersectRect ( (LPRECT) &rectIntersect,
			  &lpps->rcPaint, (LPRECT) &rectNames ) == 0 )
							       ? FALSE : TRUE;

	bDrawUnits  = ( IntersectRect ( (LPRECT) &rectIntersect,
			  &lpps->rcPaint, (LPRECT) &rectUnits ) == 0 )
							       ? FALSE : TRUE;



	/**	Remember when calculating the invalidated rows
	***	that the rectNames area is not at the top of
	***	client but below the button bar.
	**/
	lFirstInvalidRow = max ( 0, ( lpps->rcPaint.top - rectNames.top ) )
						/ panDigitalOut.lRowHeight;

	lLastInvalidRow  = max ( 0, ( lpps->rcPaint.bottom - rectNames.top ) )
						/ panDigitalOut.lRowHeight;




	/**	Do not update more channels than are visible
	***	This will occur if we have scrolled near
	***	to the end of the channel list or their are
	***	less channels connected than rows on the client.
	***
	***
	***	NB Do NOT display a partial channel as in AnalogIn
	***	This would cause the channel's button control to
	***	overwrite the status-bar, since we cannot hide the
	***	button control UNDER the status-bar.
	**/


	lNumChansVisible = min ( panDigitalOut.lNumRowsVisible,
		conninfo.nNumConnected - panDigitalOut.lFirstChanVisible );

	lLastInvalidRow = min ( lNumChansVisible - 1, lLastInvalidRow );




	/**	Set appropriate text and background
	***	colors and a small font for the text...
	**/

	dwOldTextColor =
		SetTextColor ( hdcClient, colors.crNormalText  );

	dwOldBkColor =
		SetBkColor ( hdcClient, colors.crNormalBk  );

	hfontOld =  SelectFont ( hdcClient, hSansSerif_8 );





	/**	Now draw each invalidate channel......
	***
	**/

	for ( lRow = lFirstInvalidRow; lRow <= lLastInvalidRow; lRow++ ) {

		/**	Which channel occupies this text row
		**/
		lChanIndex = lRow + panDigitalOut.lFirstChanVisible;

		/**	Calculate text row position
		**/
		lTextYPos = rectData.top + ( lRow * panDigitalOut.lRowHeight );





		/**
		***	C H A N N E L	V A L U E S
		***	(or Error code if channel in error)
		**/
		if ( ( bDrawValues ) &&
		    ( conninfo.pciConnections [ lChanIndex ].nError == 0 ) ) {



			/**	Channel Values,
			***
			***	Since the values will may be being updated
			***	at rates of upto 10 hertz, we will draw this
			***	text using the TextOut function which is
			***	considerably faster than DrawText. To do
			***	this we must calculate the coordinates
			***	which centre the vertically within the text
			***	rectangle.
			***
			***	To centre the text vertically subtract the
			***	text height from the row height. This gives
			***	the distance above and below the text. Add
			***	half this distance to the top of the text row
			***	to give the correct text position when the
			***	row.
			***
			**/
			lCentredYPos = lTextYPos +
			    ( ( panDigitalOut.lRowHeight -
					panDigitalOut.lTextHeight ) / 2 );





			/**	Inset the Exclaimation bitmap by an average
			***	character width to seperate it from
			***	the channel name push button
			**/
			lExclaimationXPos =
				rectValues.left + panDigitalOut.lAveTextWidth;


			/**	Inset the LED bitmap by  further average
			***	character widths ( 2.5 chars ) to seperate
			***	it from the Exclaimation bitmap if shown
			**/
			lLEDXPos = lExclaimationXPos +
					( 2 * panDigitalOut.lAveTextWidth );


			/**	Position the LED text; 'On' or 'Off'
			***	Slightly to the right of the LED bitmap
			***	Rather than calculate the width of bitmap
			***	just allow two a two character gap to
			***	prevent the text overwriting the bitmap
			***	and improve presentation.
			**/
			lTextXPos = lLEDXPos +
				( ( 5 * panDigitalOut.lAveTextWidth ) / 2 );



			/**	Display the corresponding bitmap for the
			***	state of the current vaule.
			***
			***	First draw the MASK this 'clears' the area
			***	which the opaque parts of the LED bit map
			***	will occupy but leaves the other pixels
			***	transparent.
			***
			**/

			if ( conninfo.lpodOutputData
						[ lChanIndex ].bWriteData ) {

				/** This channel has edited data.
				*** Display the edited state and
				*** indicate that it is not yet valid
				*** by displaying an addition 'Exclaimation'
				*** icon.
				**/
				nLEDState = conninfo.lpodOutputData
						[ lChanIndex ].nNewState;

				DrawBitmap ( hdcClient,
					lExclaimationXPos,
					lCentredYPos,
					hbmpExclaimationMask,
					hbmpExclaimation );



			} else {


				nLEDState =  atoi (
				     conninfo.pciConnections
						[ lChanIndex ].szValue );
			}

			
			/**	In both cases draw the LED bitmap
			***	in either its edited state or its
			***	current output state...
			**/
			DrawBitmap ( hdcClient,
					lLEDXPos,
					lCentredYPos,
					hbmpLEDMask,
					( ( nLEDState == 0 ) ?
					hbmpLEDOff : hbmpLEDOn ) );


			if ( nLEDState == 0 ) {

				/**	In version 4 we have extended this
				***	to write other types of digital states
				***	e.g. up/down, left/right, open/close etc.
				**/
				strcpy ( szValueText,
					  conninfo.pciConnections
						[ lChanIndex ].szOff );

			} else {

				strcpy ( szValueText,
					  conninfo.pciConnections
						[ lChanIndex ].szOn );
			}

			TextOut ( hdcClient, lTextXPos, lCentredYPos,
				(LPSTR) szValueText, strlen ( szValueText ) );


		} else if ( bDrawValues ) {

			/**	The channel is in error, display
			***	the channel's error code.
			**/
			sprintf ( szValueText, "Error %d",
					conninfo.pciConnections
						  [ lChanIndex ].nError );


			/**	Inset the Error text by three average
			***	character widths to seperate it from
			***	the channel name push button and to line
			***	it up with the LED bitmaps.
			**/
			lJustifiedXPos = rectValues.left +
					( 3 * panDigitalOut.lAveTextWidth );


			lCentredYPos = lTextYPos +
			    ( ( panDigitalOut.lRowHeight
					- panDigitalOut.lTextHeight ) / 2 );




			TextOut ( hdcClient, lJustifiedXPos, lCentredYPos,
				(LPSTR) szValueText, strlen ( szValueText ) );



		} /* End Values */







	} /* End Rows */

	SelectFont ( hdcClient, hfontOld );

	SetTextColor  ( hdcClient, dwOldTextColor );
	SetBkColor    ( hdcClient, dwOldBkColor );

	return ( OK );
}





/**	Function	UpdateDataArea
***
***	This function will update text/controls on the data area.
***	It will not repaint the area itself. It should be called
***	when data displayed on the client changes.
***
**/
short UpdateDataArea ( HWND hwndMain )
{

	long    lChanIndex;
	long    lRow;
	long    lButtonYPos;
	long    lButtonWidth;
	long    lButtonHeight;

	for ( lRow = 0; lRow < panDigitalOut.lNumRowsVisible; lRow++ ) {

		/**	Which channel occupies this text row
		**/
		lChanIndex = lRow + panDigitalOut.lFirstChanVisible;


		if ( lChanIndex >= conninfo.nNumConnected ) {

			/**	This row has no channel.
			***	i.e. the client area is being only
			***	partially used.
			**/
			ShowWindow ( abutChanButton [ lRow ].
						hButtonWnd, SW_HIDE );
		} else {


			/**
			***  Determine height and width of buttons
			***  Button height will be 1.75 the text height.
			**/
			lButtonHeight = ( panDigitalOut.lTextHeight * 7 ) / 4;
			lButtonWidth  = rectNames.right - rectNames.left;

			/**	Calclulate text row position...
			***	...then position the button centrally
			***	within the row.
			**/
			lButtonYPos   = rectData.top +
					( lRow * panDigitalOut.lRowHeight );

			lButtonYPos += ( panDigitalOut.lRowHeight -
							lButtonHeight ) / 2;


			/**	Put the channel's name into the button text
			**/
			Button_SetText ( abutChanButton [ lRow ].hButtonWnd,
					(LPSTR) conninfo.pciConnections
					       [ lChanIndex ].szChannel );


			MoveWindow ( abutChanButton [ lRow ].hButtonWnd,
				rectNames.left,
				lButtonYPos,
				lButtonWidth,
				lButtonHeight,
				TRUE );

			ShowWindow ( abutChanButton [ lRow ].
						hButtonWnd, SW_SHOW );
		}


	}


	for ( lRow = panDigitalOut.lNumRowsVisible;
			lRow < panDigitalOut.lMaxRowsVisible; lRow++ ) {

		/**	Hide any buttons 'outside' the data area
		***	This will prevent buttons overwritting the
		***	status bar.
		**/
		ShowWindow ( abutChanButton [ lRow ].
						hButtonWnd, SW_HIDE );
	}

	return ( OK );
}





short UpdateMenuBar ( HWND hwndParent )
{
	HMENU     hMenu;      /* Handle of Menu */
	DWORD     dwAction;   /* New State For Menu Items */


	hMenu 	= GetMenu ( hwndParent );


	/**     Disable menu items which we do
	***     not want to be used once cycling\dde has commenced
	***     PUT YOUR CONDITION IN HERE ....
	**/
	dwAction = ( ( panDigitalOut.nStatus == STATUS_AUTO ) ||
			     ( panDigitalOut.nNumDDELinks > 0 ) ) ?
				    MF_DISABLED | MF_GRAYED : MF_ENABLED;


	EnableMenuItem  ( hMenu, IDM_LOADIMS, 	    dwAction );
	EnableMenuItem  ( hMenu, IDM_RESTORE_SETUP, dwAction );



	/**     Disable menu items which we do
	***	not want to be used once cycling\dde has commenced
	***	OR if no vald setup is loaded....
	**/
	dwAction = ( ( panDigitalOut.nStatus == STATUS_AUTO ) ||
		    ( panDigitalOut.nNumDDELinks > 0 ) ||
		    ( conninfo.bIMSSetupLoaded == FALSE ) ) ?
					MF_DISABLED | MF_GRAYED : MF_ENABLED;

	EnableMenuItem	( hMenu, IDM_OUTPUTS,	 dwAction );
	EnableMenuItem	( hMenu, IDM_SAVE_SETUP, dwAction );




	DrawMenuBar ( hwndParent );

	return ( OK );
}






short CreateScrollBar ( HWND hWndParent )
{
	HINSTANCE	hModuleInst;


	/**     To create the Window we will first need to
	***     determine the modules instnace handle.
	**/
	hModuleInst = (HINSTANCE) GetWindowLong ( hWndParent, GWL_HINSTANCE );




	hScrollBar = CreateWindow ( "SCROLLBAR", "",
					WS_CHILD | WS_VISIBLE |
					SBS_VERT,
					0, 0,
					0, 0,
					hWndParent,
					(HMENU) IDC_VSCROLL,
					hModuleInst,
					NULL );



	ShowWindow   ( hScrollBar, SW_HIDE );
	UpdateWindow ( hScrollBar );

	return ( OK );
}




short UpdateScrollBar ( HWND hWndMain )
{

	MoveWindow ( hScrollBar,
	             rectScrollBar.left,
	             rectScrollBar.top,
	             rectScrollBar.right  - rectScrollBar.left,
	             rectScrollBar.bottom - rectScrollBar.top,
	             TRUE );

	return ( OK );

}


/**
***     APPLICATION UTILITIES
**/

/**
***
***     GetFontMetrics
***
***     Returns by Reference the tmHeight and tmAveCharWidth fields
***     of the TEXTMETRIC structure for the specified Font.
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
***     Button Bar & Status Bar Definitions
***     These specify the dimensions of buttons etc.
***     These are calculated using dimensions obtained
***     from the font size !
***
**/
VOID SizeControls ( VOID )
{
	controls.nPushButtonHeight   = ( panDigitalOut.lTextHeight * 7 ) / 4;
	controls.nPushButtonWidth    = panDigitalOut.lAveTextWidth * 8;
	controls.nEditBoxLength      = panDigitalOut.lAveTextWidth * 6;
	controls.nEditBoxHeight      = controls.nPushButtonHeight;
	controls.nEditBoxLabelLength = panDigitalOut.lAveTextWidth * 5;
	controls.nBarBorder          = 2;
	controls.nSingleBarHeight    = controls.nPushButtonHeight +
	                               ( 2 * controls.nBarBorder );

	controls.nDoubleBarHeight    = ( 2 * controls.nPushButtonHeight ) +
	                               ( 4 * controls.nBarBorder );

	controls.nMinSingleBarWidth  = ( controls.nPushButtonWidth *
	                                 NUM_BUTTONS ) +
	                                 controls.nEditBoxLength +
	                                 controls.nEditBoxLabelLength + 5;

	controls.nStatusBarHeight    =	panDigitalOut.lLargeTextHeight + 10;
	controls.nScrollBarHeight    =	( 3 * panDigitalOut.lTextHeight ) / 2;
}



/**
***	DrawBitmap			Draws the bitmap with handle hBitMap
***					at the position indicted by nXStart
***					nYStart.
***					This routine is copied from PETZOLD
***	Arguments :
***		hDC			The Device Context Handle ( from 
***					BeginPaint )
***		nXStart			X Position to Draw Bit Map
***		nYState			Y Position to Draw Bit Map
***		hbmMask			The handle to a bitmap MASK
***					This determnes which areas of the
***					bitmap are opaque and which are
***					transparent.
***					(White==Transparent, Black==Opaque)
***
***					This bitmap should have the same
***					dimensions as the bitmap to be
***					displayed.
***
***		hbmBitmap			The Handle to the 'LOADED' Bitmap
***
**/
void DrawBitmap ( HDC hDC, long lXStart, long lYStart,
					HBITMAP hbmMask, HBITMAP hbmBitmap )
{
	BITMAP		bm;
	HDC		hMemDC;
	POINT		pt;
	HBRUSH		hOldBrush, hWhiteBrush;

	hMemDC = CreateCompatibleDC ( hDC );




	/**	Create a white brush for the Screen DC
	**/
	hWhiteBrush = CreateSolidBrush	( RGB ( 255, 255, 255 ) );
	hOldBrush   = SelectObject ( hDC, hWhiteBrush );



	/**	Select the mask bit map first.
	***	This determnes which areas of the
	***	bitmap are opaque and which are
	***	transparent.
	***	(White==Transparent, Black==Opaque)
	***
	**/
	SelectObject ( hMemDC, hbmMask );


	/**
	***	Determine the size of the bit map
	**/
	SetMapMode   ( hMemDC, GetMapMode ( hDC ) );
	
	GetObject ( hbmMask, sizeof ( BITMAP ), (LPSTR) &bm );
	pt.x = bm.bmWidth;
	pt.y = bm.bmHeight;
	DPtoLP ( hDC, &pt, 1 );


	/**
	***	'AND' the mask with the destination
	**/
	BitBlt ( hDC, lXStart, lYStart, pt.x, pt.y,
						hMemDC, 0, 0, SRCAND );


	/**
	***	Select the colour bitmap and perform an XOR
	***	with the destination.
	**/
	SelectObject ( hMemDC, hbmBitmap );

	BitBlt ( hDC, lXStart, lYStart, pt.x, pt.y,
						hMemDC, 0, 0, SRCINVERT );




	/**	Delete the created brush selecting the old brush
	***	in the process...
	**/
	DeleteObject ( SelectObject ( hDC, hOldBrush ) );

	/**	Delete the compatible DC
	**/
	DeleteObject ( hMemDC );
	return;
}
