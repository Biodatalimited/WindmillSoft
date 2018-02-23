/**	CFDEVDLG.C	Nigel Roberts.		27 June, 1991
***
*** IML Configuration program CONFIML.
***
*** Device selection dialog.
***
*** Associated files:-
***	CFMIAN.C -	main module.
***	CFDEVDLG.C -	Main device selection dialog.
***	CFSETDLG.C -	Settings dialog.
***	CFINIINF.C -	INI and INF file handling.
***	CFUTIL.C -	Assorted utilities eg detokenising.
***	CONFIML.H -	Function prototypes structure defns #defines etc.
***	CFDLG.H -	#defines for dialogs.
***	CONFIML.DLG -	Dialog scripts.
***	CONFIML.RC -	Resource file.
***	CONFIML.DEF -	Definitions file.
***	CONFIML.ICO -	Program icon file.
***	CONFIML.MAK -	Make file.
***
***
*** Version History.
***
*** P1.00 - 27 June, 1991
***	Conception.
***
**/








#define NOCOMM		/* To avoid bit packed fields producing warnings */

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <htmlhelp.h>

#include "CONFIML.H"

#include "confdlg.h"
#include "resource.h"
#include "\Windmill 32\Compile libs\imlinter.h"






extern GENERAL_INFO giData;	/* our general information structure. */



static short UpdateDevDlg ( HWND hDlg );


static short DoXWinappSettings ( HWND hWnd, PUSER_DEV pudDev );
static short DoXWinappComms ( HWND hWnd, PUSER_DEV pudDev );








BOOL WINAPI SelectDeviceDlgFn ( HWND hDlg, WORD wMessage, WPARAM wParam,
							LONG lParam )

{

short		nDev;
short		nDelDev;
PUSER_DEV	paudDev;
HWND		hwndList;
char		szMess [ 256 ];
char		szFormat [ 80 ];
static short	bHelpStarted;




	switch ( wMessage ) {

		case WM_INITDIALOG :
			/** We need to fill the list box with the available hardware.
			*** We give each item in the list its hardware number i.e.
			*** the array index.
			**/

			CenterDialog ( hDlg );

			/** Set the version and serial numbers in the info box.
			**/
			
			LoadString ( giData.hInst, IDS_VERSION, szMess, 255 );
			
			SetDlgItemText ( hDlg, IDD_INFO, (LPSTR) szMess );


			/** Press the add button if auto add.
			**/

			if ( giData.bAutoAdd ) {
				PostMessage ( hDlg, WM_COMMAND, IDD_ADD, 0L );
			}


			if ( ! UpdateDevDlg ( hDlg ) ) {
				/** We have no devices so return.
				**/
				return ( TRUE );
			}


			paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev );
			if ( paudDev == NULL ) {
				HandleError ( NO_MEMORY );
				return ( TRUE );
			}

			hwndList = GetDlgItem ( hDlg, IDD_DEVICE );

			for ( nDev = 0; nDev < giData.nUserDevices; nDev ++ ) {
				sprintf ( szMess, "Device %d :-   ", paudDev [nDev].nDeviceNo );
				strcat  ( szMess, paudDev [ nDev ].szDesc );
				SendMessage ( hwndList, LB_ADDSTRING, 0, (DWORD)
						   szMess /*(LPSTR) paudDev [ nDev ].szDesc*/ );

			}

			SendMessage ( hwndList, LB_SETCURSEL, 0, 0L );
			LocalUnlock ( giData.haudUserDev );

			UpdateDevDlg ( hDlg );

			/** Let the list box get the focus.
			**/
			GetDlgItem ( hDlg, IDD_DEVICE );

			return ( TRUE );	/* No focus set */


		case WM_COMMAND :

			switch ( LOWORD (wParam) ) {

				case IDD_DEVICE :
					switch ( HIWORD ( wParam ) ) {

						case LBN_SELCHANGE :
							/** Update the dialog with the appropriate
							*** details for the current selection.
							**/
							UpdateDevDlg ( hDlg );
							break;

						case LBN_DBLCLK :
							/**	Same as set so send IDD_SET message
							**/
							SendMessage ( hDlg, WM_COMMAND, IDD_SET, 0L );
							return ( TRUE );
					}
					break;


				case IDD_HELP :
					bHelpStarted = InvokeWinHelp ( hDlg, HELP_INDEX );
					break;

				case IDOK :    /** This is SAVE **/

					/** Check we have a working dir **/
					strcpy ( szMess, "" );
					if ( GetIMLDirPath ( szMess, 255) == 1){
						strcpy ( szMess, "Please specify a default working directory. \r Use the Directory button.");
						MessageBox ( NULL, (LPSTR) szMess,
							(LPSTR) "ConfIML", MB_OK  );
						
						return (TRUE);
					}

					if ( bHelpStarted ) {
						InvokeWinHelp ( hDlg, HELP_QUIT );
					}

					EndDialog ( hDlg, TRUE );
					return ( TRUE );

				case IDCANCEL :
					
					strcpy ( szMess, "" );
					if ( GetIMLDirPath ( szMess, 255) == 1){
						strcpy ( szMess, "Please specify a default working directory. \r Use the Directory button.");
						MessageBox ( NULL, (LPSTR) szMess,
							(LPSTR) "ConfIML", MB_OK  );
						
						return (TRUE);
					}
					
					if ( giData.bChangedConfig ) {

						if ( LoadString ( giData.hInst, IDS_EXIT,
							szMess, sizeof (szMess)) == 0 ) {

							sprintf ( szMess,
							"IML Configuration has changed\n"
							"Save changes" );
						}

						switch ( MessageBox ( NULL, (LPSTR) szMess,
							(LPSTR) "ConfIML", MB_YESNOCANCEL |
							MB_ICONQUESTION ) ) {

							case IDYES :
								/** end dog true to save
								*** changes.
								**/
								EndDialog ( hDlg, TRUE );

								return ( TRUE );

							case IDNO :
								/** end dog false so changes
								*** not saved.
								**/
								EndDialog ( hDlg, FALSE );

								return ( TRUE );

							default :
								return ( TRUE );
						}
					}

					if ( bHelpStarted ) {
						InvokeWinHelp ( hDlg, HELP_QUIT );
					}


					EndDialog ( hDlg, FALSE );
					return ( TRUE );



				case IDD_ADD :

					/** Do the add dog box. If the return is true then
					*** we need to clear the list box and refill it with
					*** the new devices.
					**/

					if ( DialogBox ( giData.hInst, "ADD_DEVICE", hDlg,
								  (DLGPROC) AddDeviceDlgFn )){

						/** Clear the list box and refill it.
						**/

						paudDev = (PUSER_DEV) LocalLock ( giData.
										haudUserDev );

						if ( paudDev == NULL ) {
							HandleError ( NO_MEMORY );
							return ( TRUE );
						}

						hwndList = GetDlgItem ( hDlg, IDD_DEVICE );

						SendMessage ( hwndList, LB_RESETCONTENT,0,0L);

						for ( nDev = 0; nDev < giData.nUserDevices;
										nDev ++ ) {
							sprintf ( szMess, "Device %d :-   ", paudDev [nDev].nDeviceNo );
							strcat  ( szMess, paudDev [ nDev ].szDesc );

							SendMessage ( hwndList, LB_ADDSTRING,
									 0, (DWORD) (LPSTR)
									szMess /*paudDev [ nDev ].szDesc*/ );

						}

						SendMessage ( hwndList, LB_SETCURSEL, ( nDev
										  -1 ), 0L );

						LocalUnlock ( giData.haudUserDev );

						UpdateDevDlg ( hDlg );

						giData.bChangedConfig = TRUE;
					}

					break;


				case IDD_SET :
					/** We need to build the settings dialog then we
					*** do it.
					**/

					/** Get the current selection.
					**/
					hwndList = GetDlgItem ( hDlg, IDD_DEVICE );
					nDev = (short) SendMessage ( hwndList, LB_GETCURSEL,
										   0, 0L );

					/** Check if there are any questions for this device.
					*** If not then just break.
					**/


					paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev );
					if ( paudDev == NULL ) {
						HandleError ( NO_MEMORY );
						break;
					}


					if ( paudDev [ nDev ].nType == DEV_X_WINAPP ) {
						DoXWinappSettings ( hDlg, &paudDev [ nDev ] );
						LocalUnlock ( giData.haudUserDev );

						break;
					}


					if ( strcmp (paudDev [ nDev ].szExeName, "COMIML" ) == 0) {
						MessageBox ( NULL, "Edit the ComIML Device in ComDebug" , "ComIML Settings", MB_OK );
						LocalUnlock ( giData.haudUserDev );
						break;
					}



					if ( paudDev [ nDev ].nQns == 0 ) {
						LocalUnlock ( giData.haudUserDev );
						break;
					}

					LocalUnlock ( giData.haudUserDev );


					giData.nSetDevice = nDev;


					if ( DialogBox ( giData.hInst, "CONFIG_SETTINGS",
								 hDlg, (DLGPROC) SettingsDlgFn ) ) {

						giData.bChangedConfig = TRUE;
					}	

					break;



				case ID_COMMS :


					/** Get the current selection.
					**/
					hwndList = GetDlgItem ( hDlg, IDD_DEVICE );
					nDev = (short) SendMessage ( hwndList, LB_GETCURSEL,
										   0, 0L );

					/** Check if there are any questions for this device.
					*** If not then just break.
					**/


					paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev );
					if ( paudDev == NULL ) {
						HandleError ( NO_MEMORY );
						break;
					}


					if ( paudDev [ nDev ].nType == DEV_X_WINAPP ) {
						DoXWinappComms ( hDlg, &paudDev [ nDev ] );
						LocalUnlock ( giData.haudUserDev );

						break;
					}

					break;


				case IDD_REMOVE :
					/** Get the current selection and ask the user if
					*** we really want to remove this one.
					**/

					/** Get the current selection.
					**/
					hwndList = GetDlgItem ( hDlg, IDD_DEVICE );
					nDelDev = (short) SendMessage ( hwndList,LB_GETCURSEL,
											 0, 0L );


					paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev);
					if ( paudDev == NULL ) {
						HandleError ( NO_MEMORY );
						return ( TRUE );
					}


					if ( LoadString ( giData.hInst, IDS_REMOVE, szFormat,
								   sizeof ( szFormat ) ) == 0 ) {

						sprintf ( szMess,
								"Remove device '%s' from list",
								paudDev [ nDelDev ].szDesc );
					}

					sprintf ( szMess, szFormat, paudDev[nDelDev].szDesc);

					if ( MessageBox ( NULL, (LPSTR) szMess, (LPSTR)
							 "ConfIML", MB_YESNO | MB_ICONQUESTION )
										 == IDYES ) {


					/** OK Remove the selected device then refill
					*** the list box. We must unlock the list
					*** as the remove may need to free it.
					**/

					LocalUnlock ( giData.haudUserDev );

					if ( !RemoveUserDeviceFromList ( nDelDev ) ){
						break;
					}

					SendMessage ( hwndList, LB_RESETCONTENT,0,0L);


					paudDev = (PUSER_DEV) LocalLock ( giData.
									haudUserDev);
					if ( paudDev == NULL ) {
						HandleError ( NO_MEMORY );
						return ( TRUE );
					}

					for ( nDev = 0; nDev < giData.nUserDevices;
									nDev ++ ) {
						sprintf ( szMess, "Device %d :-   ", paudDev [nDev].nDeviceNo );
						strcat  ( szMess, paudDev [ nDev ].szDesc );
						SendMessage ( hwndList, LB_ADDSTRING,
								  0, (DWORD) (LPSTR)
							   szMess /*paudDev [ nDev ].szDesc*/ );

					}

					nDev = ( nDelDev > 0 ) ? ( nDelDev -1 ) : 0;
					if ( giData.nUserDevices == 0 ) {
						nDev = -1;
					}

					SendMessage (hwndList, LB_SETCURSEL, nDev,0L);

					UpdateDevDlg ( hDlg );

					giData.bChangedConfig = TRUE;
				}

				LocalUnlock ( giData.haudUserDev );
				break;
			case IDD_SOFTWARE :
		
				DialogBox ( giData.hInst, "SOFTWARE", hDlg,
									  (DLGPROC) SoftwareDlgFn );			
		
				break;		

			case IDD_DIRECTORY :

				DirectoryDlgFn ( giData.hInst, hDlg );

				break;

		}

		return ( TRUE );

	}

	return ( FALSE );
}










static short UpdateDevDlg ( HWND hDlg )

{

short		nDev;
PUSER_DEV	paudDev;
HWND		hwndList;
char		szStr [ 100 ];
HWDEV		hwDev;
USER_DEV	udDev;



	/** If there are no devices then disable the remove and settings
	*** buttons.
	**/

	if ( giData.nUserDevices == 0 ) {
		EnableWindow ( GetDlgItem ( hDlg, IDD_SET ), FALSE );
		EnableWindow ( GetDlgItem ( hDlg, ID_COMMS), FALSE);
		EnableWindow ( GetDlgItem ( hDlg, IDD_REMOVE ), FALSE );
		return ( FALSE );
	}

	EnableWindow ( GetDlgItem ( hDlg, IDD_REMOVE ), TRUE );

	paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev );
	if ( paudDev == NULL ) {
		HandleError ( NO_MEMORY );
		return ( FALSE);
	}

	/** Get the current selection.
	**/
	hwndList = GetDlgItem ( hDlg, IDD_DEVICE );
	nDev = (short) SendMessage ( hwndList, LB_GETCURSEL, 0, 0L );

	if ( ( nDev < 0 ) || ( nDev > giData.nUserDevices ) ) {
		/** We don't have a selection so just return.
		**/
		return ( TRUE );
	}

	/** Copy the user dev to our local one then unlock the list.
	**/
	udDev = paudDev [ nDev ];	/*|||| Structure assignment ||| */

	LocalUnlock ( giData.haudUserDev );

	if ( udDev.nType == DEV_330X ) {

		EnableWindow ( GetDlgItem ( hDlg, ID_COMMS ), FALSE );


		sprintf ( szStr, "IML device number %d", udDev.nDeviceNo );
	} else {
		sprintf ( szStr, "IML device number %d, module number %d",
				       udDev.nDeviceNo, udDev.nModuleNo );
	}

	SetDlgItemText ( hDlg, IDD_DEVICE_STRING, (LPSTR) szStr );

	/** If there are no questions so disable the settings button else
	*** enable it.
	**/

	if ( stricmp ( udDev.szExeName, "COMIML" ) == 0 ) {
			
		/** Enable ComIML despite no questions **/
		EnableWindow ( GetDlgItem ( hDlg, IDD_SET ), TRUE );

	} else {		
		EnableWindow ( GetDlgItem ( hDlg, IDD_SET ), ( ( udDev.nQns == 0 ) ?
							  FALSE : TRUE ) );
	}


	if ( udDev.nType == DEV_X_WINAPP ) {

		EnableWindow ( GetDlgItem ( hDlg, ID_COMMS ), TRUE );
		EnableWindow ( GetDlgItem ( hDlg, IDD_SET ), TRUE );

	}


	if ( udDev.nType == DEV_WINAPP ) {

		EnableWindow ( GetDlgItem ( hDlg, ID_COMMS ), FALSE );
	}


	/** Get the hardware description string for this device and put it
	*** in the description box.
	**/

	if ( ! GetHWStructForUserDev ((PUSER_DEV) &udDev, (PHWDEV) &hwDev )){

		SetDlgItemText ( hDlg, IDD_DESCRIPTION, (LPSTR) "" );
		return ( TRUE );
	}

	SetDlgItemText ( hDlg, IDD_DESCRIPTION, (LPSTR) hwDev.szDesc );

	return ( TRUE );
}













int InvokeWinHelp ( HWND hDlg, WORD wCommand )

{

char	szFullPath [ _MAX_PATH ];
char	szExeDrive [ _MAX_DRIVE ];
char	szExeDir   [ _MAX_DIR ];
char	szExeName  [ _MAX_FNAME ];
char	szExeExtn  [ _MAX_EXT ];
int 	nRet;


	switch ( wCommand ) {

	case HELP_INDEX:
		wCommand = HH_DISPLAY_TOPIC;
		nRet = TRUE;
		break;
	case HELP_QUIT:
		wCommand = HH_CLOSE_ALL;
		nRet = FALSE;
		break;
	}


	GetModuleFileName ( giData.hInst, (LPSTR) szFullPath, _MAX_PATH );
	
	_splitpath ( szFullPath, szExeDrive, szExeDir, szExeName, szExeExtn );
	
	_makepath ( szFullPath, szExeDrive, szExeDir, "CONFIML", ".CHM" );
	
//	return ( WinHelp ( hDlg, (LPSTR) szFullPath, wCommand, 0L ) );
	HtmlHelpA ( hDlg, (LPSTR) szFullPath, wCommand, 0L );

	return ( nRet );
}








static short DoXWinappSettings ( HWND hWnd, PUSER_DEV pudDev )

{

char		szDLLName [ _MAX_PATH ];
int ( WINAPI *lpfnChannel) ( HWND, char * );
HWDEV	hwDev;
short	nRet;


	if ( ! GetHWStructForUserDev ( pudDev, &hwDev ) ) {
		return ( FALSE );
	}

	strcpy ( szDLLName, hwDev.szInstallDir );

	if ( szDLLName [ strlen ( szDLLName ) - 1 ] != '\\' ) {
		strcat ( szDLLName, "\\" );
	}

	strcat ( szDLLName, hwDev.szExeName );
	strcat ( szDLLName, ".DLL" );


	hwDev.hInstDLL = LoadLibrary ( szDLLName );

	if ( hwDev.hInstDLL == NULL ) {
		return ( FALSE );
	}


	(void *) lpfnChannel = GetProcAddress ( hwDev.hInstDLL, "_DoSettingsDlg@8" );

	nRet = (*lpfnChannel) ( hWnd, pudDev->szName );


	if ( nRet ) {
		giData.bChangedConfig = TRUE;
	}


//	FreeLibrary ( hwDev.hInstDLL );

	return ( nRet );
}








static short DoXWinappComms ( HWND hWnd, PUSER_DEV pudDev )

{

char		szDLLName [ _MAX_PATH ];
int ( WINAPI *lpfnComms) ( HWND, char *, short, short );
HWDEV	hwDev;
short	nRet;


	if ( ! GetHWStructForUserDev ( pudDev, &hwDev ) ) {
		return ( FALSE );
	}

	strcpy ( szDLLName, hwDev.szInstallDir );

	if ( szDLLName [ strlen ( szDLLName ) - 1 ] != '\\' ) {
		strcat ( szDLLName, "\\" );
	}

	strcat ( szDLLName, hwDev.szExeName );
	strcat ( szDLLName, ".DLL" );


	hwDev.hInstDLL = LoadLibrary ( szDLLName );

	if ( hwDev.hInstDLL == NULL ) {
		return ( FALSE );
	}


	(void *) lpfnComms = GetProcAddress ( hwDev.hInstDLL, "_DoCommsDlg@16" );

	nRet = (*lpfnComms) ( hWnd, pudDev->szName, pudDev->nDeviceNo,
							pudDev->nModuleNo );

	if ( nRet ) {
		giData.bChangedConfig = TRUE;
	}


//	FreeLibrary ( hwDev.hInstDLL );

	return ( nRet );
}
