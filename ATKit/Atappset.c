/**
*** ATappset.c              GENAPPS - Applications Toolkit
***
*** ATK-P-2.00		Andrew Thomson
***
*** This file contains the source for the following toolkit
*** functions :
***
***             ATKSaveApplicationSetup
***             ATKLoadApplicationSetup
***
***
*** The functions in this module should be used by
*** Windmill applications to save and restore setup details
*** for the application. ( Not hardware setup details ) but rather
*** the total chart or logger set ups.
***
*** This module has two entry points from the application.
*** These are via calls to the ATKSaveApplicationSetup and
*** ATKLoadApplicationSetup functions.
***
***
*** To use these functions the application writer MUST write
*** the following functions :
***
***             short SaveAppSetup ( char *szAppSetup )
***     and     short LoadAppSetup ( char *szAppSetup )
***
***
*** These functions are responsible for actually reading or writing
*** the setup details and will be called by the toolkit functions
*** once they have established the setup filename.
***
***
*** Errors, if they occur, will be reported to the user. An
*** error code will also be returned from the function call
*** allowing the application to act upon the error.
***
***
***
***
*** To use this module :
***
***     - Write ( and test ) the SaveAppSetup and LoadAppSetup fns.
***
***     - Include the line 'rcinclude ATKit.rc' in the
***       application's Resource file.
***
***     - Specify the include file 'ATKit.h' in the module
***       which will call the toolkit functions. This should be
***       located after including <windows.h>
***
*** - Include the following toolkit files in the build list :
***
***             ATAppSet.c, ATError.c
***
***     - Include 'COMMDLG.lib' in the link library list.
***
***
***     Associated Files
***
***     ATKit.h    - Contains the Fn prototypes, structure
***                  declarations and manifest constants.
***
***     ATError.c  - Resource script for the 'Load IMS Setup'
***                  dialog box.
***
***     ATKit.rc  - Error Message Text resource script.
***
***   VERSION HISTORY :
***
***   17 September 91    ATK-P-1.00   First Version Written.
***
***   July 98            ATK-P-2.00   Converted to 32 bit
**/

#define NOKANJI
#define NOCOMMS

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <commdlg.h>
#include "\Windmill 32\atkit\atkit.h"
#include "\Windmill 32\Compile Libs\imslib.h"
//#include "\Windmill 32\Compile Libs\commslib.h"
#include "\Windmill 32\Compile Libs\imlinter.h"



static BOOL    bDirSpecified = FALSE;
static char     szAppSetFileName [ ATK_MAX_PATH_LENGTH + 1 ];
static char     szAppSetDirName  [ ATK_MAX_PATH_LENGTH + 1 ];
static char     szAppSetTitle    [ ATK_MAX_PATH_LENGTH + 1 ];
static char     szAppSetFilter   [ ATK_MAX_FILTER_SIZE + 1 ];


/**
*** ATKSaveApplicationSetup  This function is used to save
***                          application setup details. A
***                          common Save dialog is used
***                          to establish the filespec.
***                          This is then passed to the
***                          application's SaveAppSetup fn.
***				  At first use the default working dir is
***               got from the registry. Subsequently the 
***               last used Dir and file name is used.
***
*** Arguments :
***
*** hInst         Instance Handle of Application.
*** hWndParent    Window Handle of Application.
*** szAppSet      The 'default application' filename.
*** szDefExtn     String specifying extension to use
***               if none specified. Do Not include
***               a proceding '.' character.
*** szFilterDesc  String describing Filter e.g.
***               "Logger Setup (*.wlg)"
*** szFilter      String containing Filter e.g.
***               "*.wlg"
***
***     Return Value :
***
***             ATK_ERR_USER_ABORTED  - The operation was canceled
***
***             ATK_ERR_COMMON_DIALOG - GetSaveFileName failed.
***
***             The return value from the application's SaveAppSetup
***             function. This should be OK if successful.
**/

short ATKSaveApplicationSetup ( HANDLE hInst, HWND hWndParent,
        char *szAppSet, char *szDefExtn, char *szFilterDesc, char *szFilter )

{
	static OPENFILENAME	ofnAppSet;      /* for GetSaveFileName Func */

	
	char    szDrive [ _MAX_DRIVE + 1 ];
	char    szDir   [ _MAX_DIR   + 1 ];
	char    szFName [ _MAX_FNAME + 1 ];
	char    szExt	[ _MAX_EXT   + 1 ];
 

	short   nResult;

	/** Envoke the Common File Save Dialog
	*** First initialise the OPENFILENAME structure
	*** members.
	**/

	/** Form the 'list' of filter descs/filters
	*** seperating with \0 chars and terminating with
	*** a additional \0 character. We of course will
	*** only use a single filter...
	**/
	sprintf ( szAppSetFilter, "%s%c%s%c%c",
	              szFilterDesc, '\0', szFilter,'\0','\0' );

	/**	At first use we get the Working dir from the registry and
	*** use the passed in default file name. Otherwise the last used 
	*** values are in the variables.
	**/
	
	if ( bDirSpecified == FALSE ) {
		strcpy  ( szAppSetFileName, szAppSet );

		/**	Split the supplied path into file name and file path
		***		filename : szAppSet
		***		path	 : szAppSetDirName
		**/
		_splitpath ( szAppSet, szDrive, szDir, szFName, szExt );
		_makepath ( szAppSetFileName, NULL, NULL, szFName, szExt );


		GetIMLDirPath ( szAppSetDirName, ATK_MAX_PATH_LENGTH );
		strcat ( szAppSetDirName, "\\SetUps");
	}


	ofnAppSet.lStructSize         = sizeof ( OPENFILENAME );
	ofnAppSet.hwndOwner           = hWndParent;
	ofnAppSet.hInstance           = hInst;
	ofnAppSet.lpstrFilter         = (LPSTR) szAppSetFilter;
	ofnAppSet.lpstrCustomFilter   = (LPSTR) NULL;
	ofnAppSet.nMaxCustFilter      = ATK_MAX_FILTER_SIZE;
	ofnAppSet.nFilterIndex        = 0L;
	ofnAppSet.lpstrFile           = (LPSTR) szAppSetFileName;
	ofnAppSet.nMaxFile            = ATK_MAX_PATH_LENGTH;
	ofnAppSet.lpstrFileTitle      = szAppSetTitle;
	ofnAppSet.nMaxFileTitle       = sizeof ( szAppSetTitle );
	ofnAppSet.lpstrInitialDir     = (LPSTR) szAppSetDirName;
	ofnAppSet.lpstrTitle          = (LPSTR) NULL;
	ofnAppSet.Flags               = OFN_OVERWRITEPROMPT |
	                                OFN_HIDEREADONLY |
	                                OFN_PATHMUSTEXIST;
	ofnAppSet.nFileOffset         = 0;
	ofnAppSet.nFileExtension      = 0;
	ofnAppSet.lpstrDefExt         = (LPSTR) szDefExtn;
	ofnAppSet.lCustData           = 0L;
	ofnAppSet.lpfnHook            = NULL;
	ofnAppSet.lpTemplateName      = NULL;

	nResult = GetSaveFileName ( &ofnAppSet );

	if ( nResult == FALSE ) {

		/** The Dialog was Cancelled or Failed
		*** So we are unable to continue.
		***
		*** CommDlgExtentedError may be used to
		*** determine if an error code or the dialog
		*** was merely canceled ( ret value == 0 )
		***
		**/
		nResult = (short) CommDlgExtendedError ( );

		if ( nResult != OK ) {

			ATKMessageBox ( hInst, hWndParent,
				ATK_ERR_COMMON_DIALOG,
				(LPSTR) NULL,
				MB_OK | MB_ICONHAND );

 

			return ( ATK_ERR_COMMON_DIALOG );

		} else {

			/**     Dialog Canceled...
			**/
			return ( ATK_ERR_USER_ABORTED );

		}
	}
	/**   Call SaveAppSetup with the selected filename
	***   returning this fns outcome to the caller.
	**/
	return ( SaveAppSetup ( szAppSetFileName ) );

}




/**
*** ATKLoadApplicationSetup     This function is used to restore
***                             application setup details. A
***                             common 3.1 Open dialog is used
***                             to establish the the filespec.
***                             This is then passed to the
***                             application's LoadAppSetup fn.
*** Arguments :
***
*** hInst                       Instance Handle of Application.
*** hWndParent                  Window Handle of Application.
*** szAppSet                    The 'default application' filename.
***                             This may be NULL.
*** szDefExtn                   String specifying extension to use
***                             if none specified. Do Not include
***                             a proceding '.' character.
***     szFilterDesc            String containing Filter description
***                             e.g. "Logger Setup (*.wlg)"
***     szFilter                String containing Filter e.g.
***                             "*.wlg"
***
***	Return Value :
***
***		ATK_ERR_USER_ABORTED  - The operation was canceled
***
***		ATK_ERR_COMMON_DIALOG - GetOpenFileName failed.
***
***		The return value from the application's LoadAppSetup
***		function. This should be OK if successful.
**/
short ATKLoadApplicationSetup ( HANDLE hInst, HWND hWndParent,
        char *szAppSet, char *szDefExtn, char *szFilterDesc, char *szFilter )

{
	static OPENFILENAME    	ofnAppSet;  /* for GetSaveFileName Func */

	
	char	szDrive [ _MAX_DRIVE + 1 ];
	char	szDir   [ _MAX_DIR   + 1 ];
	char	szFName [ _MAX_FNAME + 1 ];
	char	szExt   [ _MAX_EXT   + 1 ];

	short   nResult;


	/**     Envoke the Common File Open Dialog
	***     First initialise the OPENFILENAME structure
	***     members.
	**/

	/**     Form the 'list' of filter descs/filters
	***     seperating with \0 chars and terminating with
	***     a additional \0 character. We of course will
	***     only use a single filter...
	**/
	sprintf ( szAppSetFilter, "%s%c%s%c%c",
		szFilterDesc, '\0', szFilter,'\0','\0' );

	/**     If this is the first use then get the 
	***     specified 'Default' file name.
	***     Get the working directory from the registry.
	***     Otherwise the variables will contain the last used 
	***     paths and names.
	**/
	if ( bDirSpecified == FALSE ) {
		strcpy  ( szAppSetFileName, szAppSet );

		/**	Split the supplied path into file name and file path
		***		filename : szAppSet
		***		path	 : szAppSetDirName
		**/
		_splitpath ( szAppSet, szDrive, szDir, szFName, szExt );
		_makepath ( szAppSetFileName, NULL, NULL, szFName, szExt );


		GetIMLDirPath ( szAppSetDirName, ATK_MAX_PATH_LENGTH );
		strcat ( szAppSetDirName, "\\SetUps");
	}

	ofnAppSet.lStructSize       = sizeof ( OPENFILENAME );
	ofnAppSet.hwndOwner         = hWndParent;
	ofnAppSet.hInstance         = hInst;
	ofnAppSet.lpstrFilter       = (LPSTR) szAppSetFilter;
	ofnAppSet.lpstrCustomFilter = (LPSTR) NULL;
	ofnAppSet.nMaxCustFilter    = ATK_MAX_FILTER_SIZE;
	ofnAppSet.nFilterIndex      = 0L;
	ofnAppSet.lpstrFile         = (LPSTR) szAppSetFileName;
	ofnAppSet.nMaxFile          = ATK_MAX_PATH_LENGTH;
	ofnAppSet.lpstrFileTitle    = szAppSetTitle;
	ofnAppSet.nMaxFileTitle     = sizeof ( szAppSetTitle );
	ofnAppSet.lpstrInitialDir   = (LPSTR) szAppSetDirName;
	ofnAppSet.lpstrTitle        = (LPSTR) NULL;
	ofnAppSet.Flags             = OFN_PATHMUSTEXIST |
	                              OFN_FILEMUSTEXIST |
	                              OFN_HIDEREADONLY;
	ofnAppSet.nFileOffset       = 0;
	ofnAppSet.nFileExtension    = 0;
	ofnAppSet.lpstrDefExt       = (LPSTR) szDefExtn;
	ofnAppSet.lCustData         = 0L;
	ofnAppSet.lpfnHook          = NULL;
	ofnAppSet.lpTemplateName    = NULL;

	nResult = GetOpenFileName ( &ofnAppSet );

	if ( nResult == FALSE ) {

		/** The Dialog was Cancelled or Failed
		*** So we are unable to continue.
		***
		*** CommDlgExtentedError may be used to
		*** determine if an error code or the dialog
		*** was merely canceled ( ret value == 0 )
		***
		**/
		nResult = (short) CommDlgExtendedError ( );

		if ( nResult != OK ) {

			ATKMessageBox ( hInst, hWndParent,
				ATK_ERR_COMMON_DIALOG,
				(LPSTR) NULL,
				MB_OK | MB_ICONHAND );

			return ( ATK_ERR_COMMON_DIALOG );

		} else {

			/**     Dialog Canceled...
			**/
			return ( ATK_ERR_USER_ABORTED );

		}
	}
	
	
	bDirSpecified = TRUE;
	
	/**   Call LoadAppSetup with the selected filename
	***   returning this fns outcome to the caller.
	**/
	return ( LoadAppSetup ( szAppSetFileName ) );

}
