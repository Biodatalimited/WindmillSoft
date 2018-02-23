/**
***	WPAO_DRW.c		ATK-PANELS
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

#include "\Windmill 32\Compile Libs\IMLinter.h"	/* ...using the COMMSLIB Dynalink		 */
#include "\Windmill 32\Compile Libs\imslib.h"     /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATkit\ATKit.h"      /* Application Toolkit Header/Prototypes         */

#include "WPAO_def.h"	/* Manifest Constants and Structure Declarations */
			/* Functions Prototypes				 */



/**	Externally defined Globals Variables and structures
**/
extern HINSTANCE  hInst;		/* Program Instance Handle	    */
extern HWND	  hwndMain;		/* Program Window Handle	    */

/**	Fonts used by Application
**/
extern HFONT	hSansSerif_8;	/*  8 Point Sans Serif Font */
extern HFONT	hSansSerif_10;	/* 10 Point Sans Serif Font */
extern HBITMAP	hbmpLEDOn;	/* LED bitmap (on state)    */
extern HBITMAP	hbmpLEDOff;	/* LED bitmap (off state)   */
extern HBITMAP	hbmpLEDMask;	/* LED bitmaps' common mask */
extern HBITMAP	hbmpExclaimationMask;
extern HBITMAP	hbmpExclaimation;

extern RECT rectButtonBar;  /* Area occupied by Push-button Bar		   */
extern RECT rectStatusBar;  /* Area occupied by Status Bar		   */
extern RECT rectData;	    /* Area occupied by Channel Name, Data & Units */
extern RECT rectScrollBar;  /* Area occupied by Vertical Scroll Bar	   */

/**	Further divide the data area into fields
**/
extern RECT rectNames;	    /* Area occupied by Channel names		    */
extern RECT rectSymbols;    /* Area occupied by Exclaimation Bitmaps	    */
extern RECT rectValues;	    /* Area occupied by Data Values		    */
extern RECT rectUnits;	    /* Area occupied by Channel units		    */


/**	Individual rectangles for status bar text.
***	These allow long text to be clipped correctly by
***	using the ExtTextOut fn.
**/
extern RECT rectStatusBarItem [ NUM_STATUS_BAR_ITEMS  ];


/**	Application Details
**/
extern PANELINFO	panAnalogOut;
extern CONTROLS		controls;
extern ATKCOLOURINFO	colors;	      /* text colors for three alarm states */


/**	Channel Connection Information
**/
extern CONNECTIONS conninfo;




BUTTONBAR abutButtonInfo [ NUM_BUTTONS ] = {

	NULL, BS_PUSHBUTTON, "&Mode",	  0, 0,

};


/**
***     Handles of client area controls.....
**/
extern HWND	hScrollBar;
extern HWND	hModeText;

HWND aheditTextRow [ MAX_SUPPORTED_DATAROWS ];



/**	Text strings for Application Status
***	This array may be indexed by STATUS_
***	manifest constants.
**/
char *aszStatusText [ NUM_STATES ] = {

	"No Setup",
	"Ready",
	"Auto",
	"Busy"

};



/**	The following are pointers to default control handlers
***	These will be used when sub-classing the controls.
**/
WNDPROC	lpfnDefEditboxHandler;




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

	HPEN		hPen, hOldPen;
	HBRUSH      hOldBrush;
	long        lStatusBarLength;
	short		nItem;


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



	lStatusBarLength = STATUS_BAR_ITEM_LENGTH * panAnalogOut.lAveTextWidth;

	for ( nItem = 0; nItem < NUM_STATUS_BAR_ITEMS; nItem++ ) {


		rectStatusBarItem [ nItem ].top	   = rectStatusBar.top + 4;
		rectStatusBarItem [ nItem ].bottom = rectStatusBar.bottom - 4;
		rectStatusBarItem [ nItem ].left   = rectStatusBar.left
		                    + ( (lStatusBarLength + 4 ) * nItem ) + 4;
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
	strncpy ( aszItemText [ 0 ], aszStatusText [ panAnalogOut.nStatus ],
						   STATUS_BAR_ITEM_LENGTH );



	/**  Report number of DDE links currently connected.
	***
	**/
	switch ( panAnalogOut.nNumDDELinks ) {

	case 0 : strncpy ( aszItemText [ 1 ], "No DDE Links",
						   STATUS_BAR_ITEM_LENGTH );
		 break;

	case 1 : sprintf ( aszItemText [ 1 ], "1 DDE Link" );

		 break;

	default:

		/**	Report 'n' Number of links
		**/
		sprintf ( aszItemText [ 1 ], "%d DDE Links",
						panAnalogOut.nNumDDELinks );
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


        short   	nButton;
        HINSTANCE   	hModuleInst;

	/**	To create the Controls we will first need to
	***	determine the modules instance handle.
        **/
        hModuleInst = (HINSTANCE) GetWindowLong ( hWndParent, GWL_HINSTANCE );


	/**	Now Create the Push-Buttons.
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

	long    lXBorder;
	long    lYBorder;

	long    lYButtonPos;
	long    lFirstXButtonPos;



	short   nIndex;


	long    lButtonLength  = controls.nPushButtonWidth;
	long    lButtonHeight  = controls.nPushButtonHeight;


	long    lTextLength;   /* Mode description text control */
	long    lTextHeight;
	long    lTextXPos;
	long    lTextYPos;


	/**  Draw ButtonBar Background, Button Coloured Solid
	***  Interior with a black Border.
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



	/** The button bar is 'organised' as follows:
	***
	*** A single row of with a single push button
	*** and single text control
	***
	***
	***     The area assigned for the button bar is stored
	***     in the static rectangle rectButtonBar from this
	***     determine how to organise the bar.
	***
	***
	***     Allow for different fonts.
	**/






	/**     Calculate useful 'constants' **/
	lRectHeight    = rectButtonBar.bottom - rectButtonBar.top;
	lRectWidth     = rectButtonBar.right  - rectButtonBar.left;
	lXBorder       = controls.nBarBorder;



	/**	Use a single row Button Bar. The border should be the
	***	rectangle height - the total button height divided by
	***	two ( i.e. a border on both  sides )
	**/
	lYBorder   = ( lRectHeight - lButtonHeight ) / 2;
	lYButtonPos = rectButtonBar.top + lYBorder;

	lFirstXButtonPos = lXBorder;




	/**  Buttons are arranged in two rows
	***  The first row contains the AllOn and AllOff buttons
	***  The second row contains the Mode button. The
	***  Auto Edit box, Auto and Stop buttons will be implemented
	***  in Version 3
	***
	**/
	abutButtonInfo [ B_MODE ].lXPos = lFirstXButtonPos;
	abutButtonInfo [ B_MODE ].lYPos = lYButtonPos;



	for ( nIndex = 0; nIndex < NUM_BUTTONS; nIndex++ ) {

		MoveWindow ( abutButtonInfo [ nIndex ].hButtonWnd,
		             abutButtonInfo [ nIndex ].lXPos,
		             abutButtonInfo [ nIndex ].lYPos,
		             lButtonLength,
		             lButtonHeight,
		             TRUE );

	}

	/**	Position the Mode Text to the right of the 'Mode' button
	**/
	lTextXPos = abutButtonInfo [ B_MODE ].lXPos +
	                        lButtonLength + lXBorder;

	/**  Vertically centre the text to align with the mode button
	**/
	lTextYPos = abutButtonInfo [ B_MODE ].lYPos +
			( lButtonHeight - panAnalogOut.lTextHeight ) / 2;


	lTextLength = rectButtonBar.right - lTextXPos - lXBorder;
	lTextHeight = panAnalogOut.lTextHeight;


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



	/**	'Mode' button enabled if connections available
	**/
	Button_Enable ( abutButtonInfo [ B_MODE ].hButtonWnd,
					( conninfo.nNumConnected > 0 ) );




	/**	Update the static text detailing the mode of operation
	***	as being immediate or deffered....
	**/
	switch ( panAnalogOut.nMode ) {

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
	short		nTextRow;
	WNDPROC 	lpfnEditboxCallBackFn;
	HINSTANCE	hModuleInst;
	HCURSOR 	hcursorOld;


	/**	To create the Window we will first need to
	***	determine the modules instance handle.
	**/
	hModuleInst = (HINSTANCE) GetWindowLong ( hwndParent, GWL_HINSTANCE );

	/**	Create an Instance thunk for Editbox_WndProc
	***	Each Editbox created will be subclassed using
	***	this procedure.
	**/
	lpfnEditboxCallBackFn = (WNDPROC) MakeProcInstance ( (FARPROC)
					    EditBox_WndProc, hModuleInst );

	/**	Creating a screenfull of push-buttons may take
	***	several seconds, so display the hour glass cursor
	**/

	hcursorOld = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) );


	/**	Create a push-button control for each potential 'text row'
	***	visible.
	**/
	for ( nTextRow = 0; nTextRow < panAnalogOut.nMaxRowsVisible;
							   nTextRow++ ) {

		aheditTextRow [ nTextRow ] =
				CreateWindow ( "edit",
					NULL,
					WS_CHILD | WS_VISIBLE |
					WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
					0, 0,
					0, 0,
					hwndParent,
					(HMENU) (IDC_TEXTROW_EDIT + nTextRow),
					hModuleInst,
					NULL );

		/**	Subclass the editbox controls.
		***	This will allow us to intercept its
		***	keyboard messages and redirect them
		***	to AnalogOut_WndProc for processing.
		**/

		lpfnDefEditboxHandler = SubclassWindow (
				aheditTextRow [ nTextRow ],
				lpfnEditboxCallBackFn );



		/**	Select an appropriate font for the Editbox
		***	and restrict the length to MAX_VALUE
		**/
		FORWARD_WM_SETFONT ( aheditTextRow [ nTextRow ],
					hSansSerif_8, FALSE, PostMessage );

		Edit_LimitText ( aheditTextRow [ nTextRow ], MAX_VALUE );


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

	short	nChanIndex;		/* Channel Index		*/
	long	lTextYPos;		/* Y Position (all fields)	*/

	long	lExclaimationXPos;	/* X Position of bitmaps	*/


	BOOL	bDrawNames;		/* Names   Invalidated ?	*/
	BOOL	bDrawSymbols;		/* Symbols Invalidated ?	*/
	BOOL	bDrawUnits;		/* Units   Invalidated?	*/

	short	nNumChansVisible;
	short	nFirstInvalidRow;
	short	nLastInvalidRow;
	short	nRow;
	RECT	rectIntersect;

	HFONT	hfontOld;
	DWORD	dwOldTextColor;
	DWORD	dwOldBkColor;


	short	nCentredYPos;


	RECT	rectText;

	/**	Determine how much of the client is invalid.
	***	We need to know :
	***
	***	1) Which rows are invalidated
	***	2) Which fields are invalidated
	**/

	bDrawSymbols = ( IntersectRect ( (LPRECT) &rectIntersect,
			  &lpps->rcPaint, (LPRECT) &rectSymbols ) == 0 )
							       ? FALSE : TRUE;


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
	nFirstInvalidRow = (short) ( max ( 0, ( lpps->rcPaint.top - rectNames.top ) )
						/ panAnalogOut.lRowHeight  );

	nLastInvalidRow  = (short) ( max ( 0, ( lpps->rcPaint.bottom - rectNames.top ) )
						/ panAnalogOut.lRowHeight );




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


	nNumChansVisible = min ( panAnalogOut.nNumRowsVisible,
		conninfo.nNumConnected - panAnalogOut.nFirstChanVisible );

	nLastInvalidRow = min ( nNumChansVisible - 1, nLastInvalidRow );




	/**	Set appropriate text and background
	***	colors and a small font for the text...
	**/
	/**	Store the current pen colors, so we can
	***	restore them on exit of the function
	***
	***	And select a small font for the text...
	**/

	dwOldTextColor = SetTextColor ( hdcClient, colors.crNormalText );;
	dwOldBkColor   = SetBkColor   ( hdcClient, colors.crNormalBk );

	hfontOld =  SelectFont ( hdcClient, hSansSerif_8 );





	/**	Now draw each invalidate channel......
	***
	**/

	for ( nRow = nFirstInvalidRow; nRow <= nLastInvalidRow; nRow++ ) {

		/**	Which channel occupies this text row
		**/
		nChanIndex = nRow + panAnalogOut.nFirstChanVisible;

		/**	Calclulate text row position
		**/
		lTextYPos = rectData.top + ( nRow * panAnalogOut.lRowHeight );



		/**
		***	C H A N N E L	N A M E S
		***
		**/
		if ( bDrawNames ) {

			/**	Draw Channel Names, right justified.
			**/

			rectText.top	= lTextYPos;
			rectText.bottom = rectText.top
						+ panAnalogOut.lRowHeight;
			rectText.left	= rectNames.left;
			rectText.right	= rectNames.right;

			DrawText ( hdcClient, (LPSTR)
			   conninfo.pciConnections [ nChanIndex ].szChannel,
			   strlen ( conninfo.pciConnections [ nChanIndex ].szChannel ),
			   (LPRECT) &rectText,
			   DT_RIGHT | DT_SINGLELINE | DT_VCENTER );

			/**	If the client owns the focus and this
			***	is the focus channel, highlight it.
			**/
			if ( ( nChanIndex == panAnalogOut.nFocusChannel ) &&
					     ( GetFocus ( ) == hwndMain ) ) {

				/**	Highlight the selected channel
				**/
				DrawFocusRect ( hdcClient,
						(LPRECT) &rectText );
			}




		} /* End Channel Names */





		/**
		***	C H A N N E L	S Y M B O L S
		***
		**/
		if ( bDrawSymbols ) {



			/**	Channel Symbols, This sections draws the
			***	'Exclaimation' bitmap. The editboxes may be
			***	updated independently, by calling the
			***	UpdateData function.
			***
			***	To centre the bitmap vertically subtract the
			***	text height from the row height. This gives
			***	the distance above and below the text. Add
			***	half this distance to the top of the text row
			***	to give the correct text position when the
			***	row.
			***
			**/
			nCentredYPos = lTextYPos +
			    ( ( panAnalogOut.lRowHeight -
					panAnalogOut.lTextHeight ) / 2 );





			/**	Inset the Exclaimation bitmap by an average
			***	character width to seperate it from
			***	the channel name push button
			**/
			lExclaimationXPos = rectSymbols.left;



			/**	If the editbox value does NOT represent
			***	the current hardware output, display the
			***	exclaimation symbol...
			**/

			if ( conninfo.lpodOutputData
					 [ nChanIndex ].bWriteData == TRUE ) {

				/** This channel has edited data.
				*** Display the edited state and
				*** indicate that it is not yet valid
				*** by displaying an addition 'Exclaimation'
				*** icon.
				**/

				DrawBitmap ( hdcClient,
					lExclaimationXPos,
					nCentredYPos,
					hbmpExclaimationMask,
					hbmpExclaimation );



			}

		}
		/**
		***	C H A N N E L	V A L U E S
		***
		***	( see function: UpdateDataArea )
		***
		**/


		/**
		***	C H A N N E L	U N I T S
		***	It was intended only to draw the units if the
		***	channel was error free. However it is difficult
		***	to determine which channels are going in to/out off
		***	of error and therefore which channels should be
		***	invalidated ( without invalidating the whole units
		***	column ). For this reason we wont implement this
		***	feature in this version.
		**/
		if ( ( bDrawUnits ) && ( conninfo.lpodOutputData
					 [ nChanIndex ].nWriteError == 0 ) ) {


			/**	Channel Units, left justified
			**/
			rectText.top	= lTextYPos;
			rectText.bottom = rectText.top
						+ panAnalogOut.lRowHeight;
			rectText.left	= rectUnits.left
						+ panAnalogOut.lAveTextWidth;
			rectText.right	= rectUnits.right;

			DrawText ( hdcClient, (LPSTR)
			   conninfo.pciConnections [ nChanIndex ].szUnits,
			   strlen ( conninfo.pciConnections [ nChanIndex ].szUnits ),
			   (LPRECT) &rectText,
			   DT_LEFT | DT_SINGLELINE | DT_VCENTER );



		} /* End Units */






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

	short	nChanIndex;
	short	nRow;
	long	lEditboxYPos, lEditboxXPos;
	long	lEditboxWidth;
	long	lEditboxHeight;
	char	szErrorText [ MAX_VALUE + 1 ];


	for ( nRow = 0; nRow < panAnalogOut.nNumRowsVisible; nRow++ ) {

		/**	Which channel occupies this text row
		**/
		nChanIndex = nRow + panAnalogOut.nFirstChanVisible;


		if ( nChanIndex >= conninfo.nNumConnected ) {

			/**	This row has no channel.
			***	i.e. the client area is being only
			***	partially used.
			**/
			ShowWindow ( aheditTextRow [ nRow ], SW_HIDE );

		} else {




			/**
			***  Determine height and width of buttons
			***  Button height will be 1.75 the text height.
			**/
			lEditboxHeight = ( panAnalogOut.lTextHeight * 7 ) / 4;
			lEditboxWidth  = rectValues.right -
							rectValues.left;


			lEditboxXPos   =  rectValues.left;



			/**	Calclulate text row position...
			***	...then position the button centrally
			***	within the row.
			**/
			lEditboxYPos	= rectData.top +
					( nRow * panAnalogOut.lRowHeight );

			lEditboxYPos += ( panAnalogOut.lRowHeight -
							lEditboxHeight ) / 2;


			/**	Put the channel's current value into the
			***	editbox unless the value has been
			***	edited but not yet sent.
			**/


			if ( conninfo.lpodOutputData [ nChanIndex ].bWriteData
								  == TRUE ) {

				/** Value has or is being edited so
				*** DONT change the contents of the editbox
				**/

			} else if ( conninfo.lpodOutputData
				       [ nChanIndex ].nWriteError != 0 ) {


				/**	This channel is in error
				**/
				sprintf ( szErrorText, "Error %d",
					conninfo.lpodOutputData
						[ nChanIndex ].nWriteError );

				Edit_SetText ( aheditTextRow [ nRow ],
							(LPSTR) szErrorText );


			} else {

				/**	Show the current output value
				**/
				Edit_SetText ( aheditTextRow [ nRow ],
					(LPSTR) conninfo.pciConnections
					       [ nChanIndex ].szValue );
			}

			MoveWindow ( aheditTextRow [ nRow ],
				lEditboxXPos,
				lEditboxYPos,
				lEditboxWidth,
				lEditboxHeight,
				TRUE );

			ShowWindow ( aheditTextRow [ nRow ], SW_SHOW );
		}


	}


	for ( nRow = panAnalogOut.nNumRowsVisible;
			nRow < panAnalogOut.nMaxRowsVisible; nRow++ ) {

		/**	Hide any buttons 'outside' the data area
		***	This will prevent buttons overwritting the
		***	status bar.
		**/
		ShowWindow ( aheditTextRow [ nRow ], SW_HIDE );
	}

	return ( OK );
}







/**
***	UpdateChannelEdittext	Updates the text in the editbox
***				currently 'owned' by the requested
***				channel. The text will be either
***				the current hardware output value
***				or, if in error, an error code.
***
***				This function does NOT size or move
***				the editboxs.
***	Parameters
***
***	  hwnd			Handle of Parent owning the edit controls
***	  lChannel		The channel whose editbox needs updating.
***
***	Return			OK
**/
short UpdateChannelEdittext ( HWND hwnd, long lChannel )
{

	short	nTextRow;
	char	szErrorText [ MAX_VALUE + 1 ];



	if ( conninfo.lpodOutputData [ lChannel ].bWriteData ) {

		/**	DO NOT Update edit box IF USER is editing
		***	the value ready for output.
		**/
		return ( OK );
	}




	/**	Determine which 'text line' the requested channel
	***	is mapped onto. Using this value we can index the
	***	array of editbox handles; aheditTextRow, to locate
	***	editbox handle currently assigned to the channel.
	**/
	nTextRow = ChannelNumberToTextLine ( (short) lChannel );

	/**	Put the channel's current value into the
	***	editbox unless the channel is in error
	***	in which case display the error code...
	**/
	if ( conninfo.lpodOutputData [ lChannel ].nWriteError != 0 ) {

		/**	This channel is in error
		**/
		sprintf ( szErrorText, "Error %d",
					conninfo.lpodOutputData
						[ lChannel ].nWriteError );

		Edit_SetText ( aheditTextRow [ nTextRow ],
						(LPSTR) szErrorText );

	} else {

		Edit_SetText ( aheditTextRow [ nTextRow ],
					(LPSTR) conninfo.pciConnections
					       [ lChannel ].szValue );
	}

	return ( OK );


}







short UpdateMenuBar ( HWND hwndParent )
{
	HMENU     hMenu;		/* Handle of Menu		*/
	DWORD     dwAction;		/* New State For Menu Items	*/


	hMenu   = GetMenu ( hwndParent );


	/**   Disable menu items which we do
	***   not want to be used once cycling\dde has commenced
	***   PUT YOUR CONDITION IN HERE ....
	**/
	dwAction = ( ( panAnalogOut.nStatus == STATUS_AUTO ) ||
			     ( panAnalogOut.nNumDDELinks > 0 ) ) ?
				    MF_DISABLED | MF_GRAYED : MF_ENABLED;


	EnableMenuItem  ( hMenu, IDM_LOADIMS, 	    dwAction );
	EnableMenuItem  ( hMenu, IDM_RESTORE_SETUP, dwAction );



	/**     Disable menu items which we do
	***     not want to be used once cycling\dde has commenced
	***     OR if no vald setup is loaded....
	**/
	dwAction = ( ( panAnalogOut.nStatus == STATUS_AUTO ) ||
		    ( panAnalogOut.nNumDDELinks > 0 ) ||
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
***	APPLICATION UTILITIES
**/

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
	HDC     	hClientDC;
	HFONT   	hOldFont;
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
***	These are calculated using dimensions obtained
***	from the font size !
***
**/
VOID SizeControls ( VOID )
{
	controls.nPushButtonHeight   = ( panAnalogOut.lTextHeight * 7 ) / 4;
	controls.nPushButtonWidth    = panAnalogOut.lAveTextWidth * 8;
	controls.nEditBoxLength      = panAnalogOut.lAveTextWidth * 6;
	controls.nEditBoxHeight      = controls.nPushButtonHeight;
	controls.nEditBoxLabelLength = panAnalogOut.lAveTextWidth * 5;
	controls.nBarBorder          = 2;
	controls.nSingleBarHeight    = controls.nPushButtonHeight +
                                       ( 2 * controls.nBarBorder );

	controls.nDoubleBarHeight    = ( 2 * controls.nPushButtonHeight ) +
                                       ( 4 * controls.nBarBorder );

	controls.nMinSingleBarWidth  = ( controls.nPushButtonWidth *
                                         NUM_BUTTONS ) +
                                         controls.nEditBoxLength +
                                         controls.nEditBoxLabelLength + 5;

	controls.nStatusBarHeight    =	panAnalogOut.lLargeTextHeight + 10;
	controls.nScrollBarHeight    =	( 3 * panAnalogOut.lTextHeight ) / 2;
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
