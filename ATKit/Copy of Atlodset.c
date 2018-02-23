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
***
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

/**	Intrinsic Function Prototypes
**/
short UpdateSetupListBox ( HWND hDlg );


UINT WINAPI ATKLoadIMSSetupHookProc ( HWND hDlg, UINT msg,
						WPARAM wParam, LPARAM lParam );





/**   Static Variables Local to this Module...
**/
char	szSetupFileName [ ATK_MAX_PATH_LENGTH + 1 ];
char	szPathName      [ ATK_MAX_PATH_LENGTH + 1 ];
char	szFilter        [ ATK_MAX_FILTER_SIZE + 1 ];

PROC		lpfnATKLoadIMSSetupHookProc = NULL; 





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
	static  char    szDirName   [ ATK_MAX_PATH_LENGTH + 1 ];
	static  char    szFileTitle [ ATK_MAX_PATH_LENGTH + 1 ];



	/**     If no Setup file has been specified in the IMSSETUP
	***     structure, display the 'Load Hardware Setup' modal Dialog
	***	box to allow the user to make the selection.
	**/
	if ( strlen ( pimsIMS->szSetupPath ) == 0 ) {

	/*	if ( lpfnATKLoadIMSSetupHookProc == NULL ) {

			lpfnATKLoadIMSSetupHookProc = 
				MakeProcInstance ( ATKLoadIMSSetupHookProc, hInst ); 

		} */

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

		szSetupFileName [ 0 ] 	= '\0';
		szDirName [ 0 ] 	= '\0';

		ofnSetupFile.lStructSize        = sizeof ( OPENFILENAME );
		ofnSetupFile.hwndOwner          = hWndParent;
		ofnSetupFile.hInstance          = hInst;
		ofnSetupFile.lpstrFilter        = (LPSTR) szFilter;
		ofnSetupFile.lpstrCustomFilter  = (LPSTR) NULL;
		ofnSetupFile.nMaxCustFilter     = ATK_MAX_FILTER_SIZE;
		ofnSetupFile.nFilterIndex       = 0L;
		ofnSetupFile.lpstrFile          = (LPSTR) szSetupFileName;
		ofnSetupFile.nMaxFile           = ATK_MAX_PATH_LENGTH;
		ofnSetupFile.lpstrFileTitle     = szFileTitle;
		ofnSetupFile.nMaxFileTitle      = sizeof ( szFileTitle );
		ofnSetupFile.lpstrInitialDir    = szDirName;
		ofnSetupFile.lpstrTitle         = (LPSTR) NULL;
		ofnSetupFile.Flags              =  OFN_ENABLETEMPLATE |
		                                   OFN_HIDEREADONLY |
		                                   OFN_PATHMUSTEXIST |
										   OFN_ENABLEHOOK;
		ofnSetupFile.nFileOffset        = 0;
		ofnSetupFile.nFileExtension     = 0;
		ofnSetupFile.lpstrDefExt        = (LPSTR) ATK_DEF_IMS_EXT;
		ofnSetupFile.lCustData          = 0L;
		ofnSetupFile.lpfnHook           = ATKLoadIMSSetupHookProc;
		ofnSetupFile.lpTemplateName	= "ATLODSET";

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
***	ATKLoadIMSSetupHookProc
***
***
***	This is the Hook Procedure for the Load Hardware Setup Dialog.
***
***	The 'Load IMS Setup Dialog' is envoked using the Window's 3.1
***	GetOpenFileName Function. This function is used to process
***	the custom controls which have been added to the standard controls
***	implemented by GetOpenFileName.
***
***
***	Messages sent the dialog may be intercepted here prior to being
***	sent to the normal message processing function. The exception is
***	the WM_INITDIALOG message which is recieved AFTER the common dialog
***	function has recieved it.
***
***	More specifically this function is responsibe for filling the
***	'Loaded Setups' combobox and copying any selections made using
***	this control to the 'Filename' edit box.
***
***	Arguments
***	  hDlg            Identifies the 'Load Hardware Setup' dialog box.
***	  wMsg		  Specifies the message number.
***	  wParam          Specifies 16 bits of additional message-dependent
***	                  information.
***	  lParam          Specifies 32 bits of additional message-dependent
***	                  information.
***
***
***
***	Return Value
***	         FALSE  - If message IS ALSO to be processed by Common Dlg Fn
***	         TRUE   - If message to to be IGNORED by the Common Dlg Fn
***
***
***
**/
UINT WINAPI ATKLoadIMSSetupHookProc ( HWND hDlg, UINT msg,
						WPARAM wParam, LPARAM lParam )
{
	WORD    wIndex;         /* Combo box Selection Index */
	short   nReturnValue;
	char    szSetupName [ ATK_MAX_SETUP_NAME_LEN ];

	switch ( msg ) {

		case WM_INITDIALOG :

			/**     Initialise the controls
			***
			**/
			if ( ( nReturnValue = UpdateSetupListBox (  hDlg ) )
							!= OK ) {

				ATKMessageBox ( (HINSTANCE) GetWindowLong ( hDlg, GWL_HINSTANCE ),
						hDlg,
						nReturnValue,
						(LPSTR) NULL,
						MB_OK | MB_ICONHAND );

				/**   Abort the dialog, informing the caller
				***   that the dialog failed...
				***
				***   NOTE we cannot use EndDialog in a Hook fn.
				**/
				SendMessage ( hDlg, WM_COMMAND, IDABORT, (LONG) 42 );



			}
			/**     Select the first item if any items exists
			**/
			if ( SendMessage ( GetDlgItem ( hDlg, ID_SETUPCOMBO ),
							 CB_GETCOUNT, 0, 0L ) > 0 ) {

				SendMessage ( GetDlgItem ( hDlg, ID_SETUPCOMBO ),
					   CB_SETCURSEL, 0, 0L );

				/**     Select this as the default dialog selection
				**/
				SendMessage ( hDlg, WM_COMMAND,
					MAKEWPARAM ( ID_SETUPCOMBO, CBN_SELCHANGE ),
				   (LPARAM)GetDlgItem ( hDlg, ID_SETUPCOMBO ) );

			}
			return ( TRUE );


		case WM_COMMAND    :

			switch ( LOWORD (wParam) ) {

				case ID_SETUPCOMBO :

					if ( HIWORD ( wParam ) == CBN_SELCHANGE ) {

						/**     New 'Loaded Setup' selected.
						***     Get the new selection and copy
						***     to the standard 'Filename' Edit box.
						**/
						wIndex = (WORD) SendMessage
							( GetDlgItem ( hDlg, ID_SETUPCOMBO ),
							CB_GETCURSEL, 0, 0L );

						SendMessage ( GetDlgItem ( hDlg,
							ID_SETUPCOMBO ),
							CB_GETLBTEXT, wIndex,
							(LONG) (LPSTR) szSetupName );


						SetDlgItemText ( hDlg, edt1, szSetupName );

					}
					break;


					default :
						/**     Allow standard Processing
						**/
						return ( FALSE );

			}
	}
	/**     Allow Standard Processing
	**/
	return ( FALSE );

}






/**
***     UpdateSetupListBox
***
***     This function fills the 'Loaded Setup' list box
***     with the setup names of any loaded setups. If
***     no setups are loaded the list box remains empty.
***
***     Arguments :
***
***          hDlg       Handle to Dialog Window.
***
***     Return Value :
***
***          OK     		-List Box successfully filled.
***          ATK_SETUPS_NOT_AVAILABLE   -List box cannot be filled.
***
***          Note if no setups are available, the return value is OK
***
**/
short UpdateSetupListBox ( HWND hDlg )
{

	HANDLE  	hComboBox;	/* Setup Combo Box Window Handle.  */
	HANDLE  	hSetupArray;    /* Array Handle to SETINFO structs.*/
	PSETUPINFO     lpSetupArray;   /* Ptr to the above array.     */
	short           nSetup;         /* Index into Array                */

	short           nNumSetupsLoaded;


	/**     Get the handle of the list box, we will need this
	***     to send the list box messages.
	**/
	hComboBox = GetDlgItem ( hDlg, ID_SETUPCOMBO );

	/**     Obtain an array of SETINFO structs detailing
	***     which setups, if any, are currently loaded.
	***     The IMSGetLoadedSetupList fn returns a handle
	***     to the array. The size of the array is passed
	***     by reference into 'nNumSetupsLoaded'
	**/
	hSetupArray = IMSGetLoadedSetupList ( &nNumSetupsLoaded );


	if ( hSetupArray == NULL ) {

		/**     No Array available, this may be because
		***     no setups are currently loaded or because
		***     insufficient memory was available to
		***     allocate the array. We can check this by
		***     examining the nNumSetupsLoaded variable.
		**/
		if ( nNumSetupsLoaded != 0 ) {

			/**     No Memory... Exit the function
			***     with an appropriate error code.
			**/
			return ( ATK_ERR_NO_MEMORY_IN_DLG );

		} else {
			/**     No setups are loaded. This is not
			***     an error but we should
			***     disable the List box used to
			***     select loaded setups.
			**/
			EnableWindow ( hComboBox, FALSE );
			return ( OK );
		}

	}

	/**	Lock the setup array ready to extract
	***	the setup names...
	**/
	lpSetupArray = (PSETUPINFO) GlobalLock ( hSetupArray );
	if ( lpSetupArray == NULL ) {

		/**     No Memory... Exit the function
		***     with an appropriate error code.
		**/
		GlobalFree ( hSetupArray );
		return ( ATK_ERR_NO_MEMORY_IN_DLG );
	}


	/**	For each array element, extract the Setup
	***	name and add it to the listbox list.
	**/
	for ( nSetup = 0; nSetup < nNumSetupsLoaded; nSetup++ ) {

		/**     Send the Setup name to the list box
		**/
		SendMessage ( hComboBox, CB_ADDSTRING, (WPARAM) NULL,
		         (LONG) (LPSTR)  lpSetupArray [ nSetup ].szSetupName );
	}

	if ( nNumSetupsLoaded != 0 ) {

		/**     Select the first entry in the list-box ???
		***/
		SendMessage ( hComboBox,  CB_SETCURSEL, 0, 0L );
		
	}
	/**  Unlock and Free the
	***  memory used by the array
	**/
	GlobalUnlock ( hSetupArray );
	GlobalFree   ( hSetupArray );

	return ( OK );

}
