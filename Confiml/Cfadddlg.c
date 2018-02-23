/**	CFADDDLG.C	Nigel Roberts.		27 June, 1991
***
*** IML Configuration program CONFIML.
***
*** Settings dialog.
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

#include <string.h>

#include <stdlib.h>
#include <stdio.h>



#include "CONFIML.H"

#include "confdlg.h"





extern GENERAL_INFO giData;	/* our general information structure. */


extern char cImlini [];




static short AddHardwareModulesToUserList ( HWND hDlg, PHWDEV phwDev );
static short AddXWinapHardwareToUserList ( HWND hDlg, PHWDEV phwDev );








BOOL WINAPI AddDeviceDlgFn ( HWND hDlg, WORD wMessage, WPARAM wParam,
							LONG lParam )

{

short           nHW;
PHWDEV          pahwDev;
HWND            hwndList;
short           nIMLDev = 0;
short           nDev = 0;
LRESULT		wIndex;
short           nRet;



	switch ( wMessage ) {

        case WM_INITDIALOG :
                /** We need to fill the list box with the available hardware.
                *** We give each item in the list its hardware number i.e.
                *** the array index.
                **/

                CenterDialog ( hDlg );

                pahwDev = (PHWDEV) LocalLock ( giData.hahwHardware );
                if ( pahwDev == NULL ) {
                        HandleError ( NO_MEMORY );
                        return ( TRUE );
                }

                hwndList = GetDlgItem ( hDlg, IDD_HARDWARE_LIST );

                for ( nHW = 0; nHW < giData.nHWDevices; nHW ++ ) {
                        wIndex = SendMessage ( hwndList, LB_ADDSTRING, 0,
                                (DWORD) (LPSTR) pahwDev [ nHW ].szDesc );

                        if (( wIndex == LB_ERR) || (wIndex == LB_ERRSPACE )){
                                HandleError ( NO_MEMORY );
                                LocalUnlock ( giData.hahwHardware );

                                return ( TRUE );
                        }
                }

                LocalUnlock ( giData.hahwHardware );

                SendMessage ( hwndList, LB_SETCURSEL, 0, 0L );

                /** Let focus go to list box. In 16 bit version
				*** wParam was set to the return value of this call. 
                **/
                GetDlgItem ( hDlg, IDD_HARDWARE_LIST );

                return ( TRUE );        /* No focus set */


	case WM_COMMAND :

		switch ( LOWORD (wParam) ) {

		case IDD_HARDWARE_LIST :
			switch ( HIWORD ( wParam ) ) {

			case LBN_DBLCLK :
				/**	Same as OK so send IDOK message
				**/
				SendMessage ( hDlg , WM_COMMAND , IDOK , 0L );
				return ( TRUE );
			}
			break;


		case IDOK :

                        /** Get the current selection.
                        **/
                        hwndList = GetDlgItem ( hDlg, IDD_HARDWARE_LIST );
                        nHW = (short) SendMessage ( hwndList, LB_GETCURSEL,
                                                                   0, 0L );


                        pahwDev = (PHWDEV) LocalLock ( giData.hahwHardware );
                        if ( pahwDev == NULL ) {
                                HandleError ( NO_MEMORY );
                                return ( TRUE );
                        }


			/**** need to switch between types here.
			***/

                        switch ( pahwDev [ nHW ].nDeviceType ) {

                        case DEV_X_WINAPP :
                                nRet = AddXWinapHardwareToUserList ( hDlg,
                        				&(pahwDev [ nHW ]) );
                                break;

                        default :

                                nRet = AddHardwareModulesToUserList ( hDlg,
		                               	&(pahwDev [ nHW ]) );
                                break;

                        }

                        LocalUnlock ( giData.hahwHardware );


			EndDialog ( hDlg, nRet );
			return ( TRUE );


		case IDCANCEL :
			EndDialog ( hDlg, FALSE );
			return ( TRUE );

		}

		return ( TRUE );
	}

	return ( FALSE );
}








static short AddHardwareModulesToUserList ( HWND hDlg, PHWDEV phwDev )

{

PUSER_DEV       paudDev;
short           nMod;
USER_DEV        udDev;
short           nIMLDev = 0;
short           nDev = 0;



	/** Get the modules for this HW then add then to the user list.
        *** We need to find the device and module numbers first.
        **/


        paudDev = (PUSER_DEV) LocalLock ( phwDev->hamodModule );
        if ( paudDev == NULL ) {
                HandleError ( NO_MEMORY );
	        return ( FALSE );
        }


        /** Check the unit for this hw. If it is device then we need to get
        *** a new iml device for it and use this for all the modules. If
        *** it is module then we need to get the device and module numbers
        *** for each module.
        **/

        if ( phwDev->nUnit == UNIT_DEVICE ) {
                /** Find a free device for the HW.
                **/

                for ( nDev = 0; nDev < MAX_DEVICES; nDev++ ) {
                        if ( giData.aimlDev [ nDev ].nType != DEV_FREE ) {
                                /** This one not free so continue.
                                **/
                                continue;
                        }

                        /** OK we can use this one.
                        **/
                        nIMLDev = nDev;
                        break;
                }

                if ( nDev == MAX_DEVICES ) {
                        /** No free device so return.
                        **/
                        return ( FALSE );
                }
        }



        for ( nMod = 0; nMod < phwDev->nModules; nMod ++ ) {

                /** Get the questions for this device and add them to our
                *** user dev we are going to add to the list.
                **/

                if ( ! CopyUserDevice ( &udDev, &(paudDev [ nMod ]) ) ) {
                        return ( FALSE );
                }


                /** Set the HW name in the user device.
                **/

                strcpy ( udDev.szHWName, phwDev->szName );


                if ( phwDev->nUnit == UNIT_DEVICE ) {

                        /** This one goes in the iml device we found above.
                        **/

                        udDev.nDeviceNo = nIMLDev;
                        udDev.nModuleNo = nMod;

                        strcpy ( giData.aimlDev [ nDev ]. szExeName,
                        				udDev.szExeName );

        		strcpy ( giData.aimlDev [ nDev ]. szHWName,
                        				udDev.szHWName );

        		giData.aimlDev [ nDev ].nType = udDev.nType;

        		giData.aimlDev [ nDev ].acModule [ nMod ] = MOD_INUSE;



                } else {

                        /** Get the device and module for this one.
                        **/

                	if ( ! GetDeviceAndModuleNumbers ( &udDev ) ) {
                        	return ( FALSE );
                	}
                }

                if ( ! AddUserDeviceToList ( &udDev ) ) {

                        return ( FALSE );
                }

                /** Do the settings dialog for this device. if there are any.
                **/

                if ( udDev.nQns == 0 ) {
                        /** There are no settings so just continue.
                        *** Set Question handle to NULL.
                        **/

                        udDev.haqnQn = NULL;
                        continue;
                }

                giData.nSetDevice = giData.nUserDevices -1;

                giData.bAddingDevice = TRUE;




				DialogBox ( giData.hInst, "CONFIG_SETTINGS", 
					               hDlg, (DLGPROC) SettingsDlgFn );

	     

                giData.bAddingDevice = FALSE;

	}

        LocalUnlock ( phwDev->hamodModule );

        return ( TRUE );
}






static short AddXWinapHardwareToUserList ( HWND hWnd, PHWDEV phwDev )

{

char    	szDLLName [ _MAX_PATH ];
USER_DEV 	udDev;
short           nRet;
char            szStr [ 20 ];

int ( WINAPI *lpfnAdd) ( HWND, char*, char*);
int ( WINAPI *lpfnComms) ( HWND, char*, short, short);
int ( WINAPI *lpfnChannel) ( HWND, char* );

short   nMod;
PUSER_DEV       paudDev;
short   bAddModule = TRUE;


	strcpy ( szDLLName, phwDev->szInstallDir );

        if ( szDLLName [ strlen ( szDLLName ) - 1 ] != '\\' ) {
        	strcat ( szDLLName, "\\" );
        }

        strcat ( szDLLName, phwDev->szExeName );
		strcat ( szDLLName, ".DLL" );
		

        phwDev->hInstDLL = LoadLibrary ( szDLLName );

        if ( phwDev->hInstDLL == NULL ) {
                return ( FALSE );
        }



        (void *)lpfnAdd = GetProcAddress ( phwDev->hInstDLL, "_DoNewModuleDlg@12" );

        nRet = (*lpfnAdd) ( hWnd, udDev.szName, udDev.szDesc );


		if ( nRet == FALSE ) {
			return ( nRet);
		}

        /** Set the HW name in the user device and default the other
        *** settings.
        **/

		strcpy ( udDev.szHWName, phwDev->szName );
		strcpy ( udDev.szExeName, phwDev->szExeName );
        udDev.nType = DEV_X_WINAPP;
        udDev.nQns = 0;
        udDev.haqnQn = NULL;



        /** Now add the new module to the hardware struct.
        **/


/*** ONLY do this if it is not already there.
*********/

        /** Look at the modules already in the hw dev to see if this one is
        *** already there. If not then we need to add it.
        **/


        if ( phwDev->hamodModule != NULL ) {

        	paudDev = (PUSER_DEV) LocalLock ( phwDev->hamodModule );

                for ( nMod = 0; nMod < phwDev->nModules; nMod ++ ) {

                        if ( stricmp ( paudDev [ nMod ].szName,
                                		udDev.szName ) == 0 ) {

                                /** We have a match.
                                **/

                                bAddModule = FALSE;
                                break;
                        }
                }

                LocalUnlock ( phwDev->hamodModule );
        }


        if ( bAddModule ) {

        	if ( ! AddModuleToHWDevice ( phwDev, &udDev ) ) {

                	return ( FALSE );
        	}
        }




        /** Get the device and module for this one.
        **/

       	if ( ! GetDeviceAndModuleNumbers ( &udDev ) ) {

               	return ( FALSE );
       	}


        if ( ! AddUserDeviceToList ( &udDev ) ) {

                return ( FALSE );
        }



        if ( nRet ) {

	        (void *) lpfnChannel = GetProcAddress ( phwDev->hInstDLL,
                					"_DoSettingsDlg@8" );

        	nRet = (*lpfnChannel) ( hWnd, udDev.szName );
        }



        if ( nRet ) {

	        (void *) lpfnComms = GetProcAddress ( phwDev->hInstDLL, "_DoCommsDlg@16" );

        	nRet = (*lpfnComms) ( hWnd, udDev.szName, udDev.nDeviceNo,
        						udDev.nModuleNo );
        }


//        FreeLibrary ( phwDev->hInstDLL );

        phwDev->hInstDLL = NULL;


        /** Add the module to the hardware in the IML.INI file.
        ***
        *** We must check if we already have one first.
        **/


        if ( bAddModule ) {

	        sprintf ( szStr, "mod%d", ( phwDev->nModules - 1 ) );

        	WritePrivateProfileString ( phwDev->szName, szStr,
                                                udDev.szName, cImlini );

        	sprintf ( szStr, "%s.%s", phwDev->szName, udDev.szName );

        	WritePrivateProfileString ( szStr, "Desc", udDev.szDesc,
        							cImlini );
        }

        return ( nRet );
}
