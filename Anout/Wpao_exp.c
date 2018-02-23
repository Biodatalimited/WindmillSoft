/**
***	WPAO_EXP.c		ATK-PANELS
***				AnalogIn Export Functions
***
***     Andrew Thomson
***
***	This module holds control and dialog callback functions.
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

#include "WPAO_dlg.h"	/* Dialog control IDs				 */






/**	Externally declared variables used by this module
***
**/


/**	The following are pointers to default control handlers
***	These will be used when sub-classing the controls.
**/
extern WNDPROC lpfnDefEditboxHandler;

/**	Channel Connection Information, General AnalogOut Info and
***	the array of text-line editbox handles.
**/
extern CONNECTIONS	conninfo;
extern PANELINFO	panAnalogOut;
extern HWND		aheditTextRow [ MAX_SUPPORTED_DATAROWS ];







/**
***	Function	About_DlgProc
***
***	Description	Message handler for the About Box dialog.
***
***	Returns 	TRUE if message processed.
***
**/
BOOL WINAPI About_DlgProc ( HWND hdlg, WORD msg, WORD wParam,
                                                            LONG lParam )
{
     

        switch ( msg ) {

	case WM_INITDIALOG :


                return ( TRUE );

	case WM_COMMAND :

                switch ( LOWORD (wParam) ) {


                case IDOK :

                	EndDialog ( hdlg, TRUE );
                	break;




                default :
                	return ( FALSE );
                }


       	default :
        	return ( FALSE );
        }


	return ( TRUE );
}



















/**	Function		EditBox_WndProc
***
***             This is a callback function which
***             may be used to intercept messages
***				destined for the Auto edit box
***             located on the button bar. We will
***             use this function to intercept
***             messages destined for scroll bar
***             or for the client ( F1 Help ).
***             Accelerator Messages for the buttons
***             are processed using WM_GETMENUCHAR
***             messages.
***
**/
LRESULT CALLBACK EditBox_WndProc ( HWND hWnd, WORD iMessage,
					   WPARAM wParam, LPARAM lParam )
{
	short	nTextRow;
	short	nThisChannel;

    /**
	***	Intercept all key stroke combinations
	***	and pass up those not 'belonging' to the edit box
	***	up to main application message handler, these may
	***	be for the push-buttons, menus, scrollbar whatever
	***
    **/

	/**	First we need to know what channel 'owns' the edit control
	***	receiving this message. To do this :
	***
	***	 - Determine which text line the control
	***	   occupies. To locate the row index we must
	***	   search the array of editbox handles for
	***	   the handle supplied with this message.
	***
	***	 - Determine which channel is associated with
	***	   that row using the TextLinetoChannelNumber
	***	   mapping function.
	***
	***
	**/
	nThisChannel = 0;	/* Default Value */
	for ( nTextRow = 0; nTextRow < panAnalogOut.nMaxRowsVisible;
								nTextRow++ ) {

		if ( hWnd == aheditTextRow [ nTextRow ] ) {

			nThisChannel = TextLineToChannelNumber ( nTextRow );
			break;


		}

	}





        switch ( iMessage ) {


	case ( WM_SETFOCUS ) :

	       /**	This control has received the focus..
	       ***	Make this the 'focus channel'.
	       **/
	       panAnalogOut.nFocusChannel = nThisChannel;


	       /**	Pass the message onto the default handler
	       ***	It will need to further process the focus
	       ***	message.
	       **/
	       return ( CallWindowProc ( lpfnDefEditboxHandler,
					   hWnd, iMessage, wParam, lParam ) );



	case ( WM_KEYDOWN ) :


                switch ( LOWORD (wParam) ) {


		case VK_RETURN :
		case VK_ESCAPE :
		case VK_TAB    :
		case VK_PRIOR  :
		case VK_NEXT   :
		case VK_HOME   :
		case VK_END    :
		case VK_F1     :
		case VK_UP     :
		case VK_DOWN   :

			/**	Forward these keys to the main
			***	application handler...
			**/
			SendMessage ( GetParent ( hWnd ), iMessage,
							wParam, lParam );

			return ( 0L );

		case VK_DELETE :

			/**	Flag the channel as having been edited
			***	then forward the character to the default
			***	handler.
			**/
			if ( conninfo.lpodOutputData
				[ nThisChannel ].bWriteData == FALSE ) {

				conninfo.lpodOutputData
					[ nThisChannel ].bWriteData = TRUE;

				InvalidateChannel ( GetParent ( hWnd ),
					nThisChannel, CHANFIELD_BITMAP );


			}

			/**	Fall Thru...
			**/

		default :
			/**	All other keys to the edit box.
			**/
			return ( CallWindowProc ( lpfnDefEditboxHandler,
					  hWnd, iMessage, wParam, lParam ) );
					  
		} /* End WM_KEYDOWN */



	case ( WM_CHAR	) :

		/**	WM_CHAR recieves character codes, these are
		***	the virtual key codes after being translated by
		***	TranslateMessage. i.e The 'P' virtual will have been
		***	translated to a lower case 'p' if CAPSLOCK is off.
		**/
		switch ( LOWORD (wParam) ) {

		case VK_RETURN :
		case VK_ESCAPE :
		case VK_TAB    :
		case VK_HOME   :
		case VK_END    :


			/**	Intercept keys which we have been processed
			***	on the WM_KEYDOWN message and which we DONT
			***	want to be processed by the default handler.
			***	VK_RETURN and VK_ESCAPE etc are not understood
			***	by the default handler and if passed thru.
			***	will result in a message beep.
			***
			***	NOTE Dont filter out VK_F1, VK_UP etc.
			***	     Translate message may have translated
			***	     lower case letters to these values.
			**/
			return ( 0L );

		default :



			/**	All other keys to the edit box.
			***	Flag the channel as having been edited
			***	then forward the character to the default
			***	handler.
			**/
			if ( conninfo.lpodOutputData
				[ nThisChannel ].bWriteData == FALSE ) {

				conninfo.lpodOutputData
					[ nThisChannel ].bWriteData = TRUE;

				InvalidateChannel ( GetParent ( hWnd ),
					nThisChannel, CHANFIELD_BITMAP );


			}
			/**	Forward the message to the default handler
			***	for further processing....
			**/
			return ( CallWindowProc ( lpfnDefEditboxHandler,
					  hWnd, iMessage, wParam, lParam ) );

		}
		break;

	case ( WM_KEYUP ) :

		switch ( LOWORD (wParam) ) {

		case VK_RETURN :
		case VK_ESCAPE :
		case VK_TAB    :
		case VK_PRIOR  :
		case VK_NEXT   :
		case VK_HOME   :
		case VK_END    :
		case VK_F1     :
		case VK_UP     :
		case VK_DOWN   :


			/**	Intercept keys which we have been processed
			***	on the WM_KEYDOWN message.
			**/
			return ( 0L );

		default :
			/**	All other keys to the edit box
			**/
			return ( CallWindowProc ( lpfnDefEditboxHandler,
					  hWnd, iMessage, wParam, lParam ) );
					  
		}
		break;



        default : 
       
		return ( CallWindowProc ( lpfnDefEditboxHandler,
	                                   hWnd, iMessage, wParam, lParam ) );
	}
	return 0L;
	
}
