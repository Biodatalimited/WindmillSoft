/**	CFSETDLG.C	Nigel Roberts.		27 June, 1991
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
#include <memory.h>
#include <stdio.h>
#include <htmlhelp.h>

#include "CONFIML.H"

#include "confdlg.h"





static short InitialiseSettingDlg ( HWND hDlg );
static short SetupQuestions ( HWND hDlg, PUSER_DEV pudDev );
static short GetRepliesToQuestions ( HWND hDlg );
static short SetAnswerDefaults ( HWND hDlg );
static short InvokeHelp ( HWND hDlg, WORD wCommand );







extern GENERAL_INFO giData;	/* our general information structure. */









BOOL WINAPI SettingsDlgFn ( HWND hDlg, WORD wMessage, WORD wParam,
							LONG lParam )

{

static short    bHelpStarted;


	switch ( wMessage ) {

	case WM_INITDIALOG :
                InitialiseSettingDlg ( hDlg );

                CenterDialog ( hDlg );

                if ( giData.bAddingDevice ) {
                        /** Disable the cancel button.
                        **/

                        EnableWindow ( GetDlgItem ( hDlg, IDCANCEL ), FALSE );
                }

                return ( TRUE );

	case WM_COMMAND :

		switch ( wParam ) {

                case IDD_DEFAULT :
                        SetAnswerDefaults ( hDlg );
                        break;


                case IDD_HELP :

                        bHelpStarted = InvokeHelp ( hDlg, HELP_INDEX );
                        break;


		case IDOK :
                        /** Get the answers to the questions. If we have all
                        *** the answers then end the dialog otherwise beep.
                        **/
                        if ( GetRepliesToQuestions ( hDlg ) ) {

                                if ( bHelpStarted ) {
                                        InvokeHelp ( hDlg, HELP_QUIT );
                                }

                                EndDialog ( hDlg, TRUE );

                        } else {

                                MessageBeep ( 0 );

                        }

			return ( TRUE );

		case IDCANCEL :

                        if ( bHelpStarted ) {
                                InvokeHelp ( hDlg, HELP_QUIT );
                        }

                        if ( giData.bAddingDevice ) {

                                SetAnswerDefaults ( hDlg );
                        }

			EndDialog ( hDlg, FALSE );
			return ( TRUE );

		}

		return ( TRUE );
	}

	return ( FALSE );
}










static short InitialiseSettingDlg ( HWND hDlg )

{

PUSER_DEV       paudDev;
PUSER_DEV       pudDev;
short           nQuest;
char            szStr [ 120 ];
long			lBaseHeight;
long			lBaseWidth;
long            lBaseUnits;
short           nX;
short           nY;
long            lWidth;
short           nHeight;
short           nDlgBot;
short           nButtonBot;



HDC     	hDC;
TEXTMETRIC      tm;
short           nTextHeight;
short           nPixelsPerLogicalInch;
RECT            rectDlg;




        hDC = GetDC ( hDlg );

        /** 1 point = 1/72 of an inch so 8 point = 1/9 of an inch.
        *** so height in pixels is pixels per inch / 9.
        **/

        nPixelsPerLogicalInch = GetDeviceCaps ( hDC, LOGPIXELSY );

        nTextHeight = nPixelsPerLogicalInch / 9;


        GetTextMetrics ( hDC, &tm );


        ReleaseDC ( hDlg, hDC );


        lBaseUnits = GetDialogBaseUnits ();

        lBaseHeight = HIWORD ( lBaseUnits );
        lBaseWidth = LOWORD ( lBaseUnits );

        /** use font size for this not the bastard useless dialog base units
        **/

        lBaseHeight = tm.tmHeight + tm.tmExternalLeading;
        lBaseWidth = tm.tmAveCharWidth;


        paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev );
        if ( paudDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /** Copy the device we are interested in.
        **/
        pudDev = &(paudDev [ giData.nSetDevice ]);


        /** Set the dialog title etc.
        **/
        sprintf ( szStr, "Settings for Device %s", pudDev->szDesc );
        SetDlgItemText ( hDlg, IDD_DEVICE, (LPSTR) szStr );

        if ( pudDev->nType == DEV_330X ) {
                sprintf ( szStr, "Settings for IML device number %d",
                                                 pudDev->nDeviceNo );
        } else {
                sprintf ( szStr, "Settings for IML device number %d,"
                                                     " module number %d",
                                      pudDev->nDeviceNo, pudDev->nModuleNo );
        }

        SetWindowText ( hDlg, (LPSTR) szStr );

        /** Now sort out the questions and types etc.
        **/

        SetupQuestions ( hDlg, pudDev );

        LocalUnlock ( giData.haudUserDev );


        /** Now move all the other question stuff out of the way.
        **/

        for ( nQuest = pudDev->nQns; nQuest < MAX_QUESTIONS; nQuest ++ ) {

                MoveWindow ( GetDlgItem ( hDlg, (IDD_COMBO_0+nQuest)),
                                                       0, 0, 0, 0, FALSE );

                MoveWindow ( GetDlgItem ( hDlg, (IDD_EDIT_0+nQuest)),
                                                       0, 0, 0, 0, FALSE );

                MoveWindow ( GetDlgItem ( hDlg, (IDD_QUESTION_0+nQuest)),
                                                       0, 0, 0, 0, FALSE );

                ShowWindow ( GetDlgItem ( hDlg, (IDD_COMBO_0+nQuest)),
                                                                  SW_HIDE );

                ShowWindow ( GetDlgItem ( hDlg, (IDD_EDIT_0+nQuest)),
                                                                  SW_HIDE );

                ShowWindow ( GetDlgItem ( hDlg, (IDD_QUESTION_0+nQuest)),
                                                                  SW_HIDE );
        }

        /** Now move the buttons and resize the dialog.
        **/
        nDlgBot = ( ( 54 + ( pudDev->nQns * 14 ) ) * lBaseHeight ) / 8;

        nButtonBot = ( ( 20 + ( pudDev->nQns * 14 ) ) * lBaseHeight ) / 8;

        lWidth = ( 44 * lBaseWidth ) / 4;
        nHeight = ( 14 * lBaseHeight ) / 8;

        nX = ( 28 * lBaseWidth ) / 4;
        MoveWindow ( GetDlgItem ( hDlg, IDOK ), nX, nButtonBot, lWidth,
                                                        nHeight, FALSE );

        nX = ( 88 * lBaseWidth ) / 4;
        MoveWindow ( GetDlgItem ( hDlg, IDCANCEL ), nX, nButtonBot, lWidth,
                                                        nHeight, FALSE );

        nX = ( 152 * lBaseWidth ) / 4;
        MoveWindow ( GetDlgItem ( hDlg, IDD_DEFAULT ), nX, nButtonBot, lWidth,
                                                        nHeight, FALSE );

        nX = ( 212 * lBaseWidth ) / 4;
        MoveWindow ( GetDlgItem ( hDlg, IDD_HELP ), nX, nButtonBot, lWidth,
                                                        nHeight, FALSE );

        GetWindowRect ( hDlg, &rectDlg );

        lWidth = ( 294 * lBaseWidth ) / 4;


        lWidth = rectDlg.right - rectDlg.left;

        nX = ( 4 * lBaseWidth ) / 4;
        nY = ( 11 * lBaseHeight ) / 8;

        MoveWindow ( hDlg, nX, nY, lWidth, nDlgBot, FALSE );


        return ( TRUE );
}













static short    SetupQuestions ( HWND hDlg, PUSER_DEV pudDev )

{

LPQUEST          lpaqnQn;
short           nQuest;
short           nOpt;
HWND            hItem;



        lpaqnQn = (LPQUEST) GlobalLock ( pudDev->haqnQn );
        if ( lpaqnQn == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        for ( nQuest = 0; nQuest < pudDev->nQns; nQuest ++ ) {
                /** OK check the type of question this is. Move the control
                *** we don't need out of the way. Set the question text.
                *** Set the default.
                **/

                SetDlgItemText ( hDlg, (IDD_QUESTION_0 + nQuest),
                                                  lpaqnQn [ nQuest ].szQn );

                switch ( lpaqnQn [ nQuest ].nType ) {

                case QUESTION_ANSWER :
                        hItem = GetDlgItem ( hDlg, (IDD_COMBO_0 + nQuest) );
                        ShowWindow ( hItem, SW_HIDE );

                        if ( strlen ( lpaqnQn [ nQuest ].szAns ) != 0 ) {

                                SetDlgItemText ( hDlg, (IDD_EDIT_0 + nQuest),
                                             lpaqnQn [ nQuest ].szAns );
                        } else {
                                SetDlgItemText ( hDlg, (IDD_EDIT_0 + nQuest),
                                             lpaqnQn [ nQuest ].szDefault );
                        }

                        /** If we don't have an answer use the default.
                        **/

                        break;

                case QUESTION_OPTION :
                        hItem = GetDlgItem ( hDlg, (IDD_EDIT_0+nQuest) );

                        ShowWindow ( hItem, SW_HIDE );

                        hItem = GetDlgItem ( hDlg, (IDD_COMBO_0+nQuest) );

                        ShowWindow ( hItem, SW_SHOWNORMAL );

                        for ( nOpt = 0; nOpt < lpaqnQn [ nQuest ].nOptions;
                                                                nOpt ++ ) {

                                SendMessage ( hItem, CB_ADDSTRING, 0, (DWORD)
                                     (LPSTR) lpaqnQn [nQuest].aszOpt [nOpt] );
                        }

                        SendMessage ( hItem, CB_SETCURSEL, lpaqnQn [nQuest].
                                                          nCurrentOpt, 0L );
                        break;
                }
        }

        GlobalUnlock ( pudDev->haqnQn );

        return ( TRUE );
}









static short GetRepliesToQuestions ( HWND hDlg )

{

LPQUEST         lpaqnQn;
short           nQuest;
short           nOpt;
HWND            hItem;
char            szAns [ MAX_ANSWER_LENGTH ];
PUSER_DEV       paudDev;
PUSER_DEV       pudDev;



        paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev );
        if ( paudDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /** Copy the device we are interested in.
        **/
        pudDev = &(paudDev [ giData.nSetDevice ]);



        lpaqnQn = (LPQUEST) GlobalLock ( pudDev->haqnQn );
        if ( lpaqnQn == NULL ) {
                LocalUnlock ( giData.haudUserDev );
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        for ( nQuest = 0; nQuest < pudDev->nQns; nQuest ++ ) {
                /** OK check the type of question this is then get the
                *** answer.
                **/

                switch ( lpaqnQn [ nQuest ].nType ) {

                case QUESTION_ANSWER :

                        GetDlgItemText ( hDlg, (IDD_EDIT_0 + nQuest),
                                        (LPSTR) szAns, MAX_ANSWER_LENGTH );

                        strcpy ( lpaqnQn [ nQuest ].szAns, Despace (
                        					szAns )	);

                        if ( ( strlen ( Despace ( szAns ) ) ) == 0 ) {

                                /** Set the focus to this question and return
                                *** false as we have no answer.
                                **/
                                SetFocus ( GetDlgItem ( hDlg, (IDD_EDIT_0 +
                                                              nQuest) ) );


                                GlobalUnlock ( pudDev->haqnQn );
                                LocalUnlock ( giData.haudUserDev );
                                return ( FALSE );
                        }

                        break;


                case QUESTION_OPTION :
                        hItem = GetDlgItem ( hDlg, (IDD_COMBO_0+nQuest) );

                        nOpt = (short) SendMessage (hItem, CB_GETCURSEL,0,0L);

                        if ( nOpt == CB_ERR ) {
                                /** There is no selecion so set the focus
                                *** and return false.
                                **/

                                SetFocus ( hItem );
                                GlobalUnlock ( pudDev->haqnQn );
                                LocalUnlock ( giData.haudUserDev );

                                return ( FALSE );
                        }

                        lpaqnQn [nQuest].nCurrentOpt = nOpt;

                        strcpy ( lpaqnQn [ nQuest ].szAns,
                                      lpaqnQn [ nQuest ].aszOptAns [ nOpt ] );

                        break;
                }
        }

        GlobalUnlock ( pudDev->haqnQn );
        LocalUnlock ( giData.haudUserDev );

        return ( TRUE );
}








static short SetAnswerDefaults ( HWND hDlg )

{

LPQUEST         lpaqnQn;
short           nQuest;
HWND            hItem;
PUSER_DEV       paudDev;
PUSER_DEV       pudDev;




        paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev );
        if ( paudDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /** Copy the device we are interested in.
        **/
        pudDev = &(paudDev [ giData.nSetDevice ]);



        lpaqnQn = (LPQUEST) GlobalLock ( pudDev->haqnQn );
        if ( lpaqnQn == NULL ) {
                LocalUnlock ( giData.haudUserDev );
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        for ( nQuest = 0; nQuest < pudDev->nQns; nQuest ++ ) {
                /** OK check the type of question this is then get the
                *** answer.
                **/

                switch ( lpaqnQn [ nQuest ].nType ) {


                case QUESTION_ANSWER :
                        SetDlgItemText ( hDlg, (IDD_EDIT_0 + nQuest),
                                             lpaqnQn [ nQuest ].szDefault );

                        break;

                case QUESTION_OPTION :
                        hItem = GetDlgItem ( hDlg, (IDD_COMBO_0+nQuest) );

                        lpaqnQn [ nQuest ].nCurrentOpt = 0;

                        SendMessage ( hItem, CB_SETCURSEL, lpaqnQn [nQuest].
                                                          nCurrentOpt, 0L );
                        break;
                }
        }

        GlobalUnlock ( pudDev->haqnQn );
        LocalUnlock ( giData.haudUserDev );

        return ( TRUE );
}









static short InvokeHelp ( HWND hDlg, WORD wCommand )

{

char            szHelpPath [ 120 ];
PUSER_DEV       paudDev;
HWDEV           hwDev;
short			nRet;

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


        paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev );
        if ( paudDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /** Get the HW device for the user dev so we can get the install dir
        *** for the help path.
        **/

        GetHWStructForUserDev ( &(paudDev [ giData.nSetDevice ]), &hwDev );

        sprintf ( szHelpPath, "%s\\%s.chm", hwDev.szInstallDir,
                                                           hwDev.szExeName );

        LocalUnlock ( giData.haudUserDev );

		HtmlHelpA ( hDlg, (LPSTR) szHelpPath, wCommand, 0L );


        return ( nRet );
}
