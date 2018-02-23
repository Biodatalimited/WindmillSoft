/**	CFMAIN.C	Nigel Roberts.		27 June, 1991
***
*** IML Configuration program CONFIML.
***
*** Main module.
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
*** P5.00 - June 1998  BGO
*** 32 bit version. - Changes made only to produce the conversion
***     except the addition of device number to the strings displayed
***     in the current devices dialog.
***     New copy protection arrangements added
***
*** P6.00 - Feb 2003   BGO
***     Changes made to allow IML.INI to be located in the Windmill dir
***     New installation mode does not use command line inf file install
***     method. Software computer code changed to registry entry.
**/







#define NOCOMM		/* To avoid bit packed fields producing warnings */


#include <windows.h>
#include <stdio.h>



#include "confiml.h"
#include "\Windmill 32\Compile Libs\imlinter.h"





static short InitialiseData ( HANDLE hInst );


BOOL WINAPI LoadingDlgFn ( HWND hDlg, WORD wMessage, WORD wParam,
							LONG lParam );




GENERAL_INFO giData;	/* our general information structure. */


/** Path to IML.INI **/
char cImlini [MAX_PATH + 1];



int WINAPI WinMain ( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine,
							      int nCmdShow )


{

HCURSOR hOldCursor;


	/** We can only have one instance so if prevInst is not null just
	*** return now.
	**/

	if ( hPrevInst != NULL ) {
		return ( FALSE );
	}

	/** Read the path to iml.ini **/
	if ( GetIMLINIPath ( cImlini, MAX_PATH ) != OK ) {
		HandleError ( FAIL_READ_REGISTRY );
		return ( FALSE );
	}


	/** Default our general info.
	**/
	if ( ! InitialiseData ( hInst ) ) {
		/** We have a problem so return now.
		**/
		HandleError ( FAIL_INITIALISE );
		return ( FALSE );
	}


	SetCapture ( GetActiveWindow () );
	hOldCursor = SetCursor ( giData.hHourGlass );


	/** Get the details of the hardware we can potentially support.
	*** This is what later appears in the Add dialog
	*** If we have a command line argument specifying a setup file with
	*** devices in then sort it out and append it to the IML.INI file.
	**/

	if ( ! GetHardWareDetails ( lpszCmdLine ) ) {
		/** We have an error so return now.
		**/

		SetCursor ( hOldCursor );
		ReleaseCapture ();

		HandleError ( FAIL_READ_HARDWARE );
		return ( FALSE );
	} 

	/** OK Now read the IML.INI to find the selected devices.
	*** These are the data for the initial dialog
	**/

	if ( ! ReadIMLDevices () ) {
		/** We have an error so return now.
		**/

		SetCursor ( hOldCursor );
		ReleaseCapture ();

		HandleError ( FAIL_READ_IML );
		return ( FALSE );
	}


	/** OK Now do the main device selection dialog. If the return is true
	*** the dialog OK'ed so if the config has changed we write IML.INI.
	*** If the return is false then the dialog was cancelled so just
	*** return.
	**/



	SetCursor ( hOldCursor );
	ReleaseCapture ();



	if ( DialogBox ( giData.hInst, "CONFIG_DEVICES", NULL, 
		                      (DLGPROC) SelectDeviceDlgFn ) ) {

		/** OK used to exit dialog so if the configuration has
		*** changed, we write the IML.INI file.
		**/

		if ( giData.bChangedConfig ) {
			/** It's changed so write IML.INI.
			**/

			SetCapture ( GetActiveWindow () );
			hOldCursor = SetCursor ( giData.hHourGlass );


			if ( ! WriteIMLINI () ) {

				SetCursor ( hOldCursor );
				ReleaseCapture ();

				HandleError ( FAIL_WRITE_IML );
			}

			SetCursor ( hOldCursor );
			ReleaseCapture ();

			HandleError ( IDS_RESTART_WINDOWS );
		}
	}


	return ( TRUE );

}










static short InitialiseData ( HANDLE hInst )

{

short nDev;
short nMod;

	giData.hahwHardware = NULL;
	giData.haudUserDev = NULL;
	giData.nIMLDevices = 0;
	giData.nHWDevices = 0;
	giData.nUserDevices = 0;
	giData.szSetupPath [ 0 ] = '\0';
	giData.szInstallDir [ 0 ] = '\0';
	giData.bChangedConfig = FALSE;
	giData.hInst = hInst;

	giData.bAutoAdd = FALSE;
	giData.bAddingDevice = FALSE;

	giData.hHourGlass = LoadCursor ( NULL, IDC_WAIT );


	for ( nDev = 0; nDev < MAX_DEVICES; nDev ++ ) {
		giData.aimlDev [ nDev ].szExeName [ 0 ] = '\0';
		giData.aimlDev [ nDev ].szHWName [ 0 ] = '\0';
		giData.aimlDev [ nDev ].nType = DEV_FREE;
		for ( nMod = 0; nMod < MAX_MODULES; nMod ++ ) {
			giData.aimlDev [ nDev ].acModule [ nMod ] = MOD_FREE;
		}
	}

	return ( TRUE );
}







void HandleError ( short nError )

{

char	szMessage [ 120 ];

	if ( LoadString ( giData.hInst, nError, szMessage, 119 ) == 0 ) {
		sprintf ( szMessage, "Error No %d" , nError );
	}

	MessageBox ( NULL, (LPSTR) szMessage, (LPSTR) "ConfIML", MB_OK |
						       MB_ICONEXCLAMATION );

	return;
}







void CenterDialog ( HWND hDlg )

{

RECT rc;

	GetWindowRect(hDlg,&rc);
	SetWindowPos(hDlg,NULL,
		(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3,
					0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	return;
}
