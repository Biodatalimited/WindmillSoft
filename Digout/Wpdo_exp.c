/**
***	WPDO_EXP.c		ATK-PANELS
***				AnalogIn Export Functions
***
***	Andrew Thomson
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



#include "WPDO_def.h"	/* Manifest Constants and Structure Declarations */
			/* Functions Prototypes				 */

#include "WPDO_dlg.h"	/* Dialog control IDs				 */






/**	Externally declared variable used by this module
***
**/

/**	The following are pointers to default control handlers
***	These will be used when sub-classing the controls.
**/
extern WNDPROC lpfnDefButtonHandler;






/**
***	Function	About_DlgProc
***
***	Description	Message handler for the About Box dialog.
***
***	Returns 	TRUE if message processed.
***
**/
BOOL FAR PASCAL About_DlgProc ( HWND hdlg, WORD msg, WPARAM wParam,
                                                            LPARAM lParam )
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




















/**	Function		PushButton_WndProc
***
***                             This is a callback function which
***                             may be used to intercept messages
***				destined for the Push button controls
***				located on the client area ( and on the
***				button bar if desired ). We will
***                             use this function to intercept
***                             messages destined for scroll bar
***				or for the client ( F1 Help, ESC, Return ).
***
***				NOTE
***                             Accelerator Messages for the buttons
***                             are processed using WM_GETMENUCHAR
***                             messages.
***
**/
LRESULT CALLBACK PushButton_WndProc ( HWND hWnd, WORD iMessage,
					   WPARAM wParam, LPARAM lParam )
{

        /**
	***	Intercept all keyboard input and redirect
	***	to the main application message handler.
	***	Typically these will be to move the focus from
	***	one button to the other, move the scrollbar thumb
	***	or invoke the help, escape from deferred mode etc.
	***
	**/

        switch ( iMessage ) {

		

	case ( WM_CHAR	)   :
	case ( WM_KEYUP )   :
	case ( WM_KEYDOWN ) :


		SendMessage ( GetParent ( hWnd ), iMessage, wParam, lParam );


		return ( 0L );

        default : 


		/**	All other messages, i.e mouse click notification
		***	etc should be handled in the normal way by the
		***	default button handler.
		**/
		return ( CallWindowProc ( lpfnDefButtonHandler,
	                                   hWnd, iMessage, wParam, lParam ) );
	}
	return 0L;
	
}
