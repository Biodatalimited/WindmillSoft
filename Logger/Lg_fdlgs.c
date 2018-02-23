/**
***	LG_FDLGS.c	       ATK-LOGGER
***
***     Andrew Thomson
***
***     This module is responsible for processing messages sent to
***	the Logger 'File' dialog boxes...
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

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <direct.h>
#include <math.h>

#include "\Windmill 32\Compile Libs\imslib.h"

#include "\Windmill 32\ATKit\ATkit.h"      /* Tollkit Defns and Prototypes                  */
#include "lg_def.h"     /* Manifest Constants and Structure Declarations */
#include "lg_prot.h"    /* Logger modules Function Prototypes            */
#include "lg_dlgs.h"    /* Dialog Control ID Codes                       */


extern  SETTINGS        settings;      /* Current Logger Settings        */
extern  HWND		hwndMain;      /* Application Window Handle      */





/**
***	PeriodicFiles_DlgProc
***
***	Message handling for 'Advanced File Settings' dialog box
***
**/
BOOL PASCAL PeriodicFiles_DlgProc ( HWND hdlg, WORD msg, WPARAM wParam,
                                                             LONG lParam )
{
	static WORD	wNewPeriodType;
	HINSTANCE	hinst;

    /**     Get the instance handle
    **/
    hinst  = GetWindowInstance ( hdlg );



	switch ( msg ) {

	case WM_INITDIALOG :

		/** Initialise the file options group **/
		if (  !settings.bLogDataToDisk ) {
			Button_SetCheck ( GetDlgItem ( hdlg, IDB_NOT_LOG_TO_FILE ), 1 );
		} else if ( settings.bPeriodicFiles ) {
			Button_SetCheck ( GetDlgItem ( hdlg, IDB_LOG_PERIODIC ), 1 );			
		} else {
			Button_SetCheck ( GetDlgItem ( hdlg, IDB_LOG_SINGLE ), 1 );			
		}


		if ( settings.bPeriodicFiles ) {
		
			/**
			***	What type of period are we using ? :
			***
			***	- An hourly period
			***	- A  daily period
			***	- A Monthly period
			***
			**/
			switch ( settings.wPeriodType ) {


			case PERIOD_EVERY_HOUR:

				Button_SetCheck ( GetDlgItem ( hdlg, IDB_EACH_HOUR ), 1 );
				break;

			case PERIOD_EVERY_DAY:

				Button_SetCheck ( GetDlgItem ( hdlg, IDB_EACH_DAY ), 1 );
				Edit_Enable ( GetDlgItem ( hdlg, IDB_HH ), 0 );
				break;
			
			case PERIOD_EVERY_MONTH:
				
				Button_SetCheck ( GetDlgItem ( hdlg, IDB_EACH_MONTH ), 1 );
				Edit_Enable ( GetDlgItem ( hdlg, IDB_HH ), 0 );
				Edit_Enable ( GetDlgItem ( hdlg, IDB_DDHH ),  0 );

				break;
			}

			/** The postfix group **/
			switch ( settings.wPostFixType ) {

			case POSTFIX_HH:
				Button_SetCheck ( GetDlgItem ( hdlg, IDB_HH ), 1 );
				break;

			case POSTFIX_DDHH:
				Button_SetCheck ( GetDlgItem ( hdlg, IDB_DDHH ), 1 );
				break;
			
			case POSTFIX_MMDDHH:				
				Button_SetCheck ( GetDlgItem ( hdlg, IDB_MMDDHH ), 1 );
				break;

			default :
				Button_SetCheck ( GetDlgItem ( hdlg, IDB_YYMMDDHH ), 1 );
				break;

			}


		} else {

			/** Grey the periodic options **/
			Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_HOUR ),  0 );
			Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_DAY ), 0 );
			Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_MONTH ),  0 );
			Edit_Enable ( GetDlgItem ( hdlg,IDB_HH ),  0 );
			Edit_Enable ( GetDlgItem ( hdlg,IDB_DDHH ), 0 );
			Edit_Enable ( GetDlgItem ( hdlg,IDB_MMDDHH ),  0 );
			Edit_Enable ( GetDlgItem ( hdlg,IDB_YYMMDDHH ), 0 );			
		}

		switch ( settings.wFileFormat ) {

		case SPACE_SEPARATED :
			Button_SetCheck ( GetDlgItem ( hdlg, IDB_SPACES ), 1 );			
			break;
		case COMMA_SEPARATED :
			Button_SetCheck ( GetDlgItem ( hdlg, IDB_SEPARATE ), 1 );						
			break;
		default :      /* Default to tab separated */
			Button_SetCheck ( GetDlgItem ( hdlg, IDB_TABS ), 1 );			
			break;

		}


		/**	   Return TRUE since we havn't used SetFocus
        **/
        return ( TRUE );

	case WM_COMMAND :

		switch ( LOWORD (wParam ) ) {

		
		case IDB_NOT_LOG_TO_FILE :
			
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_NOT_LOG_TO_FILE ) ) ) {

				Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_HOUR ),  0 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_DAY ), 0 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_MONTH ),  0 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_HH ),  0 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_DDHH ), 0 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_MMDDHH ),  0 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_YYMMDDHH ), 0 );
				wNewPeriodType = PERIOD_EVERY_HOUR;
			}
			break;
		
		case IDB_LOG_SINGLE :
			
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_LOG_SINGLE ) ) ) {
				Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_HOUR ),  0 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_DAY ), 0 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_MONTH ),  0 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_HH ),  0 );
				Edit_Enable ( GetDlgItem ( hdlg, IDB_DDHH ), 0 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_MMDDHH ),  0 );
				Edit_Enable ( GetDlgItem ( hdlg, IDB_YYMMDDHH ), 0 );
				wNewPeriodType = PERIOD_EVERY_HOUR;
			}
			break;

		case IDB_LOG_PERIODIC :
			
			/** Enable the periodic files buttons and set default values **/
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_LOG_PERIODIC ) ) ) {
				Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_HOUR ),  1 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_DAY ), 1 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_EACH_MONTH ),  1 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_HH ),  1 );
				Edit_Enable ( GetDlgItem ( hdlg, IDB_DDHH ), 1 );
				Edit_Enable ( GetDlgItem ( hdlg,IDB_MMDDHH ),  1 );
				Edit_Enable ( GetDlgItem ( hdlg, IDB_YYMMDDHH ), 1 );
				Button_SetCheck ( GetDlgItem ( hdlg,IDB_EACH_HOUR ),  1 );				
				Button_SetCheck ( GetDlgItem ( hdlg,IDB_HH ),  1 );				
				wNewPeriodType = PERIOD_EVERY_HOUR;

			}
			break;
		
		case IDB_EACH_HOUR:

			/** Enable all file extension possibilities **/
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_EACH_HOUR ) ) ) {
				Edit_Enable ( GetDlgItem ( hdlg,IDB_HH ),  1 );
				Edit_Enable ( GetDlgItem ( hdlg, IDB_DDHH ), 1 );
				wNewPeriodType = PERIOD_EVERY_HOUR;
			}
			break;

		case IDB_EACH_DAY:

			/** Some file extensions are not allowed now so disable
			*** and uncheck them.
			**/
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_EACH_DAY ) ) ) {
				Edit_Enable ( GetDlgItem ( hdlg,IDB_HH ),  0 );
				Edit_Enable ( GetDlgItem ( hdlg, IDB_DDHH ), 1 );
				if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_HH ) ) ) {
					Button_SetCheck ( GetDlgItem ( hdlg, IDB_HH ), 0 );
					Button_SetCheck ( GetDlgItem ( hdlg, IDB_DDHH ), 1 );	
				}
				wNewPeriodType = PERIOD_EVERY_DAY;
			}
			break;

		case IDB_EACH_MONTH:

			/** Some file extensions are not allowed now so disable
			*** and uncheck them.
			**/
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_EACH_MONTH ) ) ) {
				Edit_Enable ( GetDlgItem ( hdlg, IDB_HH ),  0 );
				Edit_Enable ( GetDlgItem ( hdlg, IDB_DDHH ), 0 );
				if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_HH ) ) ) {
					Button_SetCheck ( GetDlgItem ( hdlg, IDB_HH ), 0 );
					Button_SetCheck ( GetDlgItem ( hdlg, IDB_MMDDHH ), 1 );	
				}
				if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_DDHH ) ) ) {
					Button_SetCheck ( GetDlgItem ( hdlg, IDB_DDHH ), 0 );
					Button_SetCheck ( GetDlgItem ( hdlg, IDB_MMDDHH ), 1 );	
				}
				
				wNewPeriodType = PERIOD_EVERY_MONTH;
			}
			break;

		case IDD_HELP :
		
			
            /**     Invoke the help for the periodic dialog...
            ***     use the main window handle ( instance ) so
            ***     that the Help is properly 'released' on exit.
            **/
			InvokeWinHelp ( hwndMain, HELP_CONTEXT, HC_PERIODICFILES );
            break;

		case IDOK :

			/**	What type of Period, Interval, day, week etc
			***	was last selected...
			**/
			
			settings.bLogDataToDisk = !Button_GetCheck ( GetDlgItem ( hdlg, IDB_NOT_LOG_TO_FILE ) );
			settings.bPeriodicFiles = Button_GetCheck ( GetDlgItem ( hdlg, IDB_LOG_PERIODIC ) );
			
			/** The Period Types **/
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_EACH_HOUR ))) {
				settings.wPeriodType = PERIOD_EVERY_HOUR;
			}
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_EACH_DAY ))) {
				settings.wPeriodType = PERIOD_EVERY_DAY;
			}
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_EACH_MONTH ))) {
				settings.wPeriodType = PERIOD_EVERY_MONTH;
			}
			
			/** The postfix types **/
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_HH ))) {
				settings.wPostFixType = POSTFIX_HH;
			}
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_DDHH ))) {
				settings.wPostFixType = POSTFIX_DDHH;
			}
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_MMDDHH ))) {
				settings.wPostFixType = POSTFIX_MMDDHH;
			}
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_YYMMDDHH ))) {
				settings.wPostFixType = POSTFIX_YYMMDDHH;
			}

			/** The file types **/
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_SPACES ))) {
				settings.wFileFormat = SPACE_SEPARATED;
			}
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_SEPARATE ))) {
				settings.wFileFormat = COMMA_SEPARATED;
			}
			if ( Button_GetCheck ( GetDlgItem ( hdlg, IDB_TABS ))) {
				settings.wFileFormat = TAB_SEPARATED;
			}
			

			EndDialog ( hdlg, TRUE );
            break;


        case IDCANCEL :

			EndDialog ( hdlg, FALSE );
            break;


        default :
            return ( FALSE );
        }

	}
	return ( FALSE );
}




/**
***	FileExists_DlgProc
***
***	Message handling for 'File Exists' dialog box
***
**/
BOOL PASCAL FileExists_DlgProc ( HWND hdlg, WORD msg, WPARAM wParam,
                                                            LONG lParam )
{
	char    szMessage [ 128 ];


    switch ( msg ) {

		case WM_INITDIALOG :


			/**     Include the filename in the dialog text. This
			***     name has periodic extensions but no path
			**/
            sprintf ( szMessage, "File %s already exists!",
                					settings.szDataFileCurrent );

            Edit_SetText ( GetDlgItem ( hdlg, IDD_EXISTS_TEXT ),
                						szMessage );

			/** If the periodic file is not current we cannot append.
			*** Needs the full path to the actual file for this call.
			**/
			if ( settings.bPeriodicFiles ) {							
				if ( !IsPeriodicFileCurrent ( settings.szDataFileFull ) ) {
					Button_Enable ( GetDlgItem ( hdlg, IDD_APPEND ), TRUE );				
				}
			}
            return ( TRUE );

		case WM_COMMAND :


			/**     One of the three push button has been selected
            ***     merely return the ID code of the button as the
            ***     return value of the DialogBox call.
            **/
            switch ( LOWORD (wParam) ) {

                case IDCANCEL :
                case IDD_OVERWRITE :
                case IDD_APPEND :

	                EndDialog ( hdlg, (int) LOWORD (wParam) );
        	        break;
            }
            break;

       	default :
        	return ( FALSE );
	}

	return ( TRUE );
}
