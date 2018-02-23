/**
***	ATLodSet.c              GENAPPS - Applications Toolkit
***
***	ATK-P-1.01		Andrew Thomson
***
***	This file contains the source for the following toolkit
***	functions :
***
***	        ATKLoadIMSSetup
***	        ATKLoadIMSSetupHookProc
***
***
***	The functions in this module should be used by
***	Windmill applications to load an IMS file.
***
***	This module has one entry point from the application.
***	This is via a call to the ATKLoadIMSSetup function.
***
***	To  load a  IMS file, the Application  should call the
***	ATKLoadIMSSetup function. The function arguments allow
***	a Setup to be specified. If this is the case the Setup
***	will  be loaded. Otherwise the Load Setup  Dialog will
***	be displayed allowing the user to select the Setup.
***
***	Errors, if they occur, will be reported to the user. An
***	error code will also be returned from the function call
***	allowing the application to act upon the error.
***
***	The application MUST unlock the setup, using IMSUnlockSetup,
***	once it is no longer required. This allows the setup to be
***	edited by other applications.
***
***
***
***	To use this module :
***
***	- Include the function ATKLoadIMSSetupHookProc in the
***	  Export section of the application's DEF file.
***
***	- Include the line 'rcinclude ATLodSet.rc' in the
***	  application's Resource file.
***
***	- Specify the include file 'ATKit.h' in the module
***	  which will call ATKLoadSetup. This should be located
***	  after including <windows.h>
***
***	- Specify the include file 'IMSLIB.h' if you have not 
***	  already done so.
***
***	- Include the following toolkit files in the build list :
***
***	        ATLodSet.c, ATError.c
***
***
***
***
***	Associated Files
***
***	        ATKit.h		- Contains the Fn prototypes, structure
***	                          declarations and manifest constants.
***
***	        ATLodSet.dlg    - Resource script for the 'Load IMS Setup'
***	                          dialog box.
***
***	VERSION HISTORY :
***
***	17 September 91    ATK-P-1.00   First Version Written.
***
***	18 February 03   Traditional load set up replaced by standard
***                  Dialog Box with defaults.
**/

#define NOKANJI
#define NOCOMMS
#define IML_APPLICATION

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <commdlg.h>
#include "ATKit.h"
#include "\Windmill 32\Compile Libs\imslib.h"
#include "\Windmill 32\Compile Libs\imlinter.h"


#define NO_SETUPS  1

/**	Intrinsic Function Prototypes
**/

short GetTopSetupName ( char*  cFileName, char* cPath);


/**   Static Variables Local to this Module...
**/
static char	szSetupFileName [ ATK_MAX_PATH_LENGTH + 1 ];
static char	szFilter        [ ATK_MAX_FILTER_SIZE + 1 ];
static char szDirName   [ ATK_MAX_PATH_LENGTH + 1 ];

static BOOL    bDirSpecified = FALSE;


/**
***     ATKLoadIMSSetup
***
***
***     Loads an IMS setup. The setup may be loaded from disk or may
***     have been previously  loaded  by  another application. If no
***     setup  is  specified  in  the  IMSSETUP structure,  the Load
***     Setup Dialog  will be displayed  allowing the user to select
***     the setup to used. If successful the IMSSETUP structure will
***     contain the correct path, setup name and setup reference key.
***
***     The dialog will default to suggesting the ims file at the
***     top of the list within imslib. 
***     If there are no loaded files then it starts at the last 
***     specified directory.
***     If this is first use we default to the working directory.
***     Errors  will  be reported  using the toolkit  Error Handling
***     function.
***
***     Arguments :
***
***             hInst           Instance Handle of Application.
***             hWndParent      Window Handle of Application.
***             pimsIMS         Near Pointer to a IMSSETUP structure.
***
***     Return Value :
***
***             0 - No errors, setup loaded and details in the IMSSETUP
***                 structure.
***
***             IMS, COMMS, or GPIB errors - Load failed.
**/
short ATKLoadIMSSetup ( HANDLE hInst, HWND hWndParent,
					PIMSSETUP pimsIMS, WORD wFlags )
{


	short   nResult;                /* Return Value from Fn Calls */
	short   nMessageBoxReply;       /* Reply from MessageBox Call */
	static  OPENFILENAME  ofnSetupFile;   /*  GetOpenFileName Struct */


	/**     If no Setup file has been specified in the IMSSETUP
	***     structure, display the 'Load Hardware Setup' modal Dialog
	***		box to allow the user to make the selection.
	**/
	if ( strlen ( pimsIMS->szSetupPath ) == 0 ) {


		/**     Envoke the Customised Standard File Open Dialog
		***
		***     First initialise the OPENFILENAME structure
		***     members.
		**/

		sprintf ( szFilter, "%s%c%s%c%c",
				ATK_IMS_FILTER_DESC,
				'\0',
				ATK_IMS_FILTER,
				'\0',
				'\0' );

		

		/** If we have loaded set ups then get the top one to
		*** default the dialog to.
		**/
		if ( GetTopSetupName ( szSetupFileName, szDirName ) != OK ) {
			/** If the dialog has already been used then we have
			*** a specified directory
			**/
			if ( !bDirSpecified ) {
				GetIMLDirPath ( szDirName, ATK_MAX_PATH_LENGTH );
				strcat ( szDirName, "\\SetUps" );
			}
		}

		ofnSetupFile.lStructSize        = sizeof ( OPENFILENAME );
		ofnSetupFile.hwndOwner          = hWndParent;
		ofnSetupFile.hInstance          = hInst;
		ofnSetupFile.lpstrFilter        = (LPSTR) szFilter;
		ofnSetupFile.lpstrCustomFilter  = (LPSTR) NULL;
		ofnSetupFile.nMaxCustFilter     = ATK_MAX_FILTER_SIZE;
		ofnSetupFile.nFilterIndex       = 0L;
		ofnSetupFile.lpstrFile          = (LPSTR) szSetupFileName;
		ofnSetupFile.nMaxFile           = ATK_MAX_PATH_LENGTH;
		ofnSetupFile.lpstrFileTitle     = NULL;
		ofnSetupFile.nMaxFileTitle      = 0;
		ofnSetupFile.lpstrInitialDir    = szDirName;
		ofnSetupFile.lpstrTitle         = "Load Hardware Set Up";
		ofnSetupFile.Flags              = OFN_FILEMUSTEXIST |
										  OFN_HIDEREADONLY  |
		                                  OFN_PATHMUSTEXIST;
		ofnSetupFile.nFileOffset        = 0;
		ofnSetupFile.nFileExtension     = 0;
		ofnSetupFile.lpstrDefExt        = (LPSTR) ATK_DEF_IMS_EXT;
		ofnSetupFile.lCustData          = 0L;
		ofnSetupFile.lpfnHook           = NULL;
		ofnSetupFile.lpTemplateName		= NULL;

		nResult = GetOpenFileName ( (LPOPENFILENAME) &ofnSetupFile );

		if ( nResult == FALSE ) {

			/**     The Dialog was Cancelled or Failed
			***     So we are unable to continue.
			***
			***     CommDlgExtentedError may be used to
			***     determine if an error code or the dialog
			***     was merely canceled ( ret value == 0 )
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

		/** We now have a specified directory **/
		bDirSpecified = TRUE;
		/** Copy the full path result **/
		strcpy ( pimsIMS->szSetupPath, szSetupFileName );

	}


	/**     A setup file name has been now been specified.
	***     If wFlags == 0 load the setup using the
	***     IMSLoadAndLockSetup fn. This does not allow the
	***     setup to be configured. If wFlags == IMS_CONFIG
	***     use IMSLoadSetup which DOES allow the setup to
	***     be edited
	***
	***     NOTE : On exit from the dialog, szSetupFileName
	***       may contain either :
	***
	***            1) A filespec of a setup on disk.
	***            2) A loaded setup name appended onto the current
	***               directory selection. ( We cannot distinguish
	***               between loaded setups and setups on disk so
	***               a path name is always included )
	***
	***       If the setup is already loaded, the superflous
	***       pathname will be ignored by IMSLoadAndLockSetup.
	***       For this reason it is not necessary to attempt to
	***       discern between the two types of setup.
	**/

	do {
		switch ( wFlags ) {

			case IMS_CONFIG :

				/**     Allow setup to be edited...
				**/
				nResult = IMSLoadSetup (
					(LPSTR) pimsIMS->szSetupPath,
					(LPSTR) pimsIMS->szSetupName,
					TRUE );


				break;

			default :

				/**     Just allow the setup to be used...
				**/
				nResult = IMSLoadAndLockSetup (
					(LPSTR) pimsIMS->szSetupPath,
	 				(LPSTR) pimsIMS->szSetupName,
					(short *) &( pimsIMS->nSetupKey ) );

				break;
		}


		/**     Process the return value from the Call
		**/
		switch ( nResult ) {

			case IMS_SUCCESS :
			case IMS_ALREADY_LOADED    :
			case IMS_DUPLICATE_CHANNEL :

				/**     Setup loaded succesfully,
				***	no other action required.
				**/
				return ( OK );


			case IMS_PARTITIONING      :
			case IMS_CONFIGURING       :
			case IMS_OUT_OF_MEMORY     :
			case IMS_CHANNEL_IN_USE    :

				/**     Rectifiable Errors, requiring a RETRY option
				**/
				nMessageBoxReply = ATKMessageBox ( hInst, hWndParent,
							nResult, (LPSTR) NULL,
							MB_RETRYCANCEL | MB_ICONHAND );
				break;

			case IMS_FILE_NOT_FOUND    :
			case IMS_FILE_FORMAT_ERROR :
			case IMS_LOAD_ERRORS       :
			case IMS_LIB_NOT_LOADED    :
			case IMS_LIB_VERSION_ERROR :
			default 		   :

				/**     Unrectifiable Errors, indicating that
				***     this function has failed. Inform the
				***     user then exit the function with the
				***     error code.
				**/
				ATKMessageBox ( hInst, hWndParent,
						      nResult, (LPSTR) NULL,
						      MB_OK | MB_ICONHAND );

				return ( nResult );
		}

	} while ( nMessageBoxReply == IDRETRY );

	/**   A rectifiable error has occurred and the
	***   user informed but he has elected NOT to retry
	***   the operation. Exit the fn with the error code.
	**/
	return ( nResult );

}







/**
***     GetTopSetUpName
***
***     This function reads the array of setup summaries from
***     imslib and copies the top file name into the passed in
***     variable.
***
***     Arguments :
***
***          cFileName  fill with Null string or filename
***
***     Return Value :
***
***          OK     		-List Box successfully filled.
***          ATK_SETUPS_NOT_AVAILABLE   -List box cannot be filled.
***
***          Note if no setups are available, the return value is OK
***
**/
short GetTopSetupName ( char*  cFileName, char* cPath )
{


	HANDLE  	hSetupArray;    /* Array Handle to SETINFO structs.*/
	PSETUPINFO     lpSetupArray;   /* Ptr to the above array.     */
	short           nNumSetupsLoaded;
	char	szDrive [ _MAX_DRIVE + 1 ];
	char	szDir   [ _MAX_DIR   + 1 ];
	char	szFName [ _MAX_FNAME + 1 ];
	char	szExt   [ _MAX_EXT   + 1 ];


	/** Clear the filename **/
	*cFileName = 0;
	*cPath = 0;

	/**     Obtain an array of SETINFO structs detailing
	***     which setups, if any, are currently loaded.
	***     The IMSGetLoadedSetupList fn returns a handle
	***     to the array. The size of the array is passed
	***     by reference into 'nNumSetupsLoaded'
	**/
	hSetupArray = IMSGetLoadedSetupList ( &nNumSetupsLoaded );


	/** If the handle is hull we are not going to get an answer **/
	if ( hSetupArray == NULL ) {
		return ( NO_SETUPS );
	}

	/**	Lock the setup array ready to extract
	***	the setup name
	**/
	lpSetupArray = (PSETUPINFO) GlobalLock ( hSetupArray );

	if ( lpSetupArray == NULL ) {

		/**     No Memory... Exit the function
		***     with an appropriate error code.
		**/
		GlobalFree ( hSetupArray );
		return ( NO_SETUPS );
	}


	/** Split the path to get the file name **/
	strcpy ( cPath, lpSetupArray [ 0 ].szFileSpec );
	_splitpath ( cPath, szDrive, szDir, szFName, szExt );
	_makepath  ( cPath, szDrive, szDir, NULL, NULL );
	
	if ( cPath [ strlen ( cPath ) - 1 ] == '\\' ) {
		/**	Remove trailing '\' character
		**/
		cPath [ strlen ( cPath ) - 1 ] = '\0';
	}
	_makepath ( cFileName, NULL, NULL, szFName, szExt );


	/**  Unlock and Free the
	***  memory used by the array
	**/
	GlobalUnlock ( hSetupArray );
	GlobalFree   ( hSetupArray );

	return ( OK );

}
