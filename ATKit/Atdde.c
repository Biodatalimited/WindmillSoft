/**
***	ATDDE.c		  IMLAPPS-IML Applications Toolkit
***
***	IMLPAPPS-ATK-P-1.01	Andrew Thomson
***
***	This file contains the source for the following toolkit
***	functions :
***
***		ATKGetDDEServiceName
***		ATKGetDDEServiceNameDialogFn
***
***
***     The functions in this module should be used by
***	acquire a DDE Service name for the Windmill application.
***	The name may then be registered by the application if
***	it is to support DDE conversations.
***	Ideally each application should be assigned a unique
***	service name.
***
***
***	This module has one entry point from the application.
***	This is via a call to the ATKGetDDEServiceName function.
***
***
***	To use this module :
***
***	- Include the function ATKGetDDEServiceNameDlgFn in the
***	 Export section of the application's DEF file.
***
***	- Include the line 'rcinclude ATDDE.dlg' in the
***	 application's Resource file.
***
***     - Specify the include file 'ATKit.h' in the module
***	  which will call ATKGetDDEServiceName.
***	  This should be located after including <windows.h>
***
***	- Include the following toolkit files in the build list :
***
***		ATDDE.c, ATError.c
***
***
***
***
***	Associated Files
***
***     ATKIT.h       - Contains the Fn prototypes, structure
***                   		declarations and manifest constants.
***
***		ATDDE.dlg     - Resource script for the 'Channel Connections'
***				dialog box.
***
***	VERSION HISTORY :
***
***	17 September 91    ATK-P-1.00   First Version Written.
***	 1 October   92    ATK-P-1.01	Error Message if no suitable
***	                                channels available.
***
**/

#define NOKANJI
#define NOCOMMS
#define IML_APPLICATION

#include <windows.h>
#include <windowsx.h>	/* Using Control API's */
#include <string.h>
#include "ATKit.h"
#include "\Windmill 32\Compile Libs\imslib.h"




/**	Static variables; used to pass information in to and out off
***	the dialog box message handling function.
**/
static	BOOL bDDEEnabled;
static	char szEditText [ ATK_MAX_DDE_SERVICE_NAME + 1 ];

static	DLGPROC lpfnATKGetDDEServiceNameDlgFn; /* dlg proc-instance addr */

/**     Intrinsic Function Prototypes
**/
short  PASCAL ATKGetDDEServiceNameDlgFn ( HWND hDlg, unsigned iMessage,
						   WORD wParam, LONG lParam );







/**
***	ATKGetDDEServiceName
***
***	This function is used to prompt the user for a string
***	suitable for registering as a DDE Service name.
***
***	The default string is "Application Name_Setup Name"
***
***
***	Arguments
***
***	hInst           The Instance handle of the calling Application.
***	hWndParent      The Window Handle of the calling Application.
***			(usually the application's Window handle)
***
***	szApplicationName   Application name e.g "AnalogIn"
***	szSetupName	    Application Setup name ( not including
***			    the directory or extension names ) e.g.
***			    "mysetup". By convention do not use the
***			    hardware setup file name.
***	szDDEServiceName    Pointer to string to receive service name
***
***	Return Value :	0		- No Error, a service name has been
***					  succesfully entered.
***
***			non-zero        - A predefined error code from the
***                                       IMSLIB DLL or a from the toolkit
***                                       (see ATKit.h ). No Connections have
***                                       been made.
**/
short ATKGetDDEServiceName ( HANDLE hInst, HWND hWndParent,
	char *szApplicationName, char *szSetupName, char *szDDEServiceName )
{
	short   nResult;


	if ( lpfnATKGetDDEServiceNameDlgFn == NULL ) {

	/**  Create a procedure-instance address to bind
	***  the Dialog call-back function to the instance's
	***  data segment
	**/

		lpfnATKGetDDEServiceNameDlgFn =
		    (DLGPROC)  MakeProcInstance ( ATKGetDDEServiceNameDlgFn, hInst );


	}


	/**	Store current service name/ enable flag in
	***	globals, accessible by dialog message handler
	**/
	if ( strlen ( szDDEServiceName ) == 0 ) {

		/**	No existing service name has been passed
		***	in so create a default using the application
		***	and setup name ( if it exists )
		**/
		strcpy ( szEditText, szApplicationName );
		if ( strlen ( szSetupName ) != 0 ) {

			strcat ( szEditText, "_" );
			strcat ( szEditText, szSetupName );
		}

	} else {

		/**	Use the current service name entry
		**/
		strcpy ( szEditText, szDDEServiceName );

	}



		
	/**     Set local variables which will be used by to pass
	***	information between this fn and the 'Select Channels'
	***	Dialog Handling function.
	**/



	/**	Invoke the modal-dialog box to obtain the servicename
	**/
	nResult = DialogBox ( hInst, (LPSTR) "ATK_DDE_SERVICE_NAME",
			       hWndParent,  lpfnATKGetDDEServiceNameDlgFn );



	if ( nResult == FALSE ) {

		/**	The Dialog failed or was canceled
		**/

		return ( OK );

	} else {


		/**     Set parameters values to be returned by reference.
		***     from the local static variables.
		**/
		strcpy ( szDDEServiceName, szEditText );

		return ( OK );
	}
}





/**
***	ATKGetDDEServiceNameDlgFn
***
***
***	This is the Dialog callback function for the
***	'DDE Service Name' Dialog.
***
***	Current control settings are stored in static variables
***	declared at the beginning of this module,.
***
***	Arguments
***	  hDlg            Identifies the dialog box that receives the message.
***	  wMsg		  Specifies the message number.
***	  wParam          Specifies 16 bits of additional message-dependent
***	                  information.
***	  lParam          Specifies 32 bits of additional message-dependent
***	                  information.
***
***
***
***	Return Value
***	         FALSE  - If message not processed
***	         TRUE   - If message Processed
***
***
***
**/
short PASCAL ATKGetDDEServiceNameDlgFn ( HWND hDlg, unsigned iMessage,
						   WORD wParam, LONG lParam )
{



	switch ( iMessage ) {

		case ( WM_INITDIALOG ) :        /* Request to Initialise */

			Edit_LimitText ( GetDlgItem ( hDlg, IDC_DDE_SERVICE_NAME ),
							  ATK_MAX_DDE_SERVICE_NAME );

			Edit_SetText   ( GetDlgItem ( hDlg, IDC_DDE_SERVICE_NAME ),
								(LPSTR) szEditText );


			/**	Leave the summary open, we will need it when
			***	the user OK's the dialog to find required
			***	information on the 'connected' channels.
			***
			***	Return TRUE since we havn't used SetFocus
			**/
			return ( TRUE );



		case ( WM_COMMAND ) :

			switch ( wParam ) {

				case IDOK :

					Edit_GetText (
						GetDlgItem ( hDlg, IDC_DDE_SERVICE_NAME ),
						(LPSTR) szEditText,
						ATK_MAX_DDE_SERVICE_NAME );



					EndDialog ( hDlg, TRUE );
					break;

				case IDCANCEL :


					EndDialog ( hDlg, FALSE );
					break;

			}
		break;
	}

	return ( FALSE );
}
