/**
***	ATError.C              	IMLAPPS-IML Applications Toolkit
***
***	IMLAPPS-ATK-P-1.02     Andrew Thomson
***
***	This file contains the source for the following toolkit
***	functions :
***
***	        ATKMessageBox
***
***	The function in this module may be used by
***	Windmill applications to display a message-box
***	using a text-string located in the Application's
***	Resource script.
***
***	This module has one entry point from the application.
***	This is via a call to the ATKMessageBox Function
***
***	To use this module :
***
***	- Include the line 'rcinclude ATError.rc' in the
***	  application's Resource file.
***
***	- Specify the include file 'ATKit.h' in the module
***	  which will call ATKMessageBox.
***
***	- Include the following toolkit files in the build list :
***
***	         ATError.c
***
***	- Strings unique to the application should use STRINGTABLE
***	  ID's in the range 1000 - 1999 only.
***
***	Assoiciated Files
***
***	        ATKit.h		- Contains the Fn prototypes, structure
***	                          declarations and manifest constants.
***
***	        ATError.rc      - Contains pre-defined Strings and
***	                          dialog control IDs
***
***
***	VERSION HISTORY :
***
***	17 September 91    ATK-P-1.00   First Version Written.
***
***
**/

#define NOKANJI
#define NOCOMMS

#include <windows.h>
#include <stdio.h>
#include "ATKit.h"



/**
***	ATKMessageBox
***
***	Loads a string from the application's resource script
***	and displays the string in a message-box.
***	The title, push-buttons and the icon which appear on
***	the message box are specified in the function parameters.
***
***	The return value indicates which button was pressed to exit
***	the dialog.
***
***
***	PARAMETERS :
***
***	hInstance       Instance
***
***	hWndParent	Window handle of caller. This may be NULL.
***	                This Window will recieve the focus when the
***	                function terminates.
***
***	nStringID	A STRINGTABLE ID value indicating which string
***			is to be displayed in the message-box. If no
***			string is found in the STRINGTABLE for nStringID,
***			the message will be 'Error n' where n is the ID.
***
***     szCaption	Points to a null-terminated character string
***			to be used for the dialog-box caption. If the
***			szCaption parameter is NULL, the Window title of
***			hWndParent will be used. If this is NULL, the
***			default caption "Error" is used.
***
***	wType		Specifies the contents of the dialog box.
***			The values for wType are identical to those used
***			by the MessageBox function and can be joined by
***			the bitwise OR operator. Common values are :
***
***			MB_OK,   MB_RETRYCANCEL, MB_OKCANCEL
***			MB_ICONSTOP, MB_ICONQUESTION,    MB_ICONEXCLAMATION
***
***			Refer to the IBM SAA Guide for correct usage.
***
***	
***	RETURN :	Indicates which Push-Button was pressed by the
***			operator. The buttons are defined as:
***				IDABORT
***				IDIGNORE
***				IDCANCEL
***				IDNO
***				IDOK
***				IDRETRY
***				IDYES
***
***			These are passed directly from the MessageBox function.
**/
short ATKMessageBox ( HANDLE hInstance, HWND hWndParent, WORD wStringID,
					        char *szCaption, WORD wType )
{
	char    szMessageText  [ 256 ]; /* 256 Character Message Buffer    */
	char    szCaptionText  [ 128 ]; /* 128 Character Title Buffer      */
	LPSTR   lpszCaption;            /* Pointer to Title Buffer         */



	/**     A String ID of zero, corresponds to no error
	***     in this case we can exit the function with no
	***     other action.
	**/
	if ( wStringID == OK ) {
		return ( IDOK );
	}


	/**     Obtain the Message Box, Message String
	**/
	if ( LoadString ( hInstance, wStringID, (LPSTR) szMessageText,
					 sizeof ( szMessageText ) ) == 0 ) {
					
		/**	No String Exists in resource file
		***	so just send the String ID Code numerically
		**/			
		sprintf ( szMessageText, "Error %d", (short) wStringID );
	}




	/**     Obtain the Message Box Caption
	**/
	if ( szCaption != NULL ) {

		/**     Use the specified Caption
		**/
		lpszCaption = (LPSTR) szCaption;

	} else {

		/**     Use the Caller Window's Caption, 127 chars maximum
		***	Store this in szCaptionText.
		**/
		if ( GetWindowText ( hWndParent, (LPSTR) szCaptionText, 127 )
						      == 0 ) {

			/**  No Caption, let MessageBox use its default
			**/
			lpszCaption = NULL;
		} else {
			/**  Application Caption stored in szCaptionText
			**/
			lpszCaption = (LPSTR) szCaptionText;
		}
	}



	/**	Show the message box, passing up the return value
	**/
	return ( MessageBox ( hWndParent, (LPSTR) szMessageText,
					          lpszCaption, wType ) );

}
