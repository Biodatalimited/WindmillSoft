/**
***	LG_DLG.c              ATK-LOGGER
***
***     Andrew Thomson
***
***     This module is responsible for processing messages sent to
***     the Logger dialog boxes...
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


extern  SETTINGS	settings;      /* Current Logger Settings        */
extern  HWND		hwndMain;      /* Application Window Handle      */


/**
***	Settings_DlgProc
***
***	Message handling for 'Settings' dialog box
***
**/
BOOL PASCAL Settings_DlgProc ( HWND hdlg, WORD msg, WPARAM wParam,
                                                             LONG lParam )
{
	WORD    	wID;
	char    	szEditText     [ MAX_EDITBOX_STRLEN  + 1 ];
	char		szComboBoxText [ MAX_COMBOBOX_STRLEN + 1 ];
	HINSTANCE	hinst;
	double		dIntervalInUnits;
	char		szMessageText	[ 128 ];
	WORD		wWidth;
	static BOOL	bIntervalRepresentedAsMinutes;   
        
        
	/**     Get the instance handle
	**/
	hinst  = GetWindowInstance ( hdlg );



	switch ( msg ) {

	case WM_INITDIALOG :

		/**     Load the descriptions for the combo boxes
		***     from the resource script string table.
                **/
		for ( wID = TIMESTAMP_BASE; wID <
			TIMESTAMP_BASE + NUM_TIMESTAMP_FORMATS; wID++ ) {

			LoadString ( hinst, wID, szComboBoxText,
					   MAX_COMBOBOX_STRLEN );

			ComboBox_AddString (
				GetDlgItem ( hdlg, IDD_TIMESTAMPS ),
						szComboBoxText );

			if ( wID == settings.wTimeStampFormat ) {

				/**     Select the current selection...
				**/
	 		ComboBox_SelectString ( GetDlgItem ( hdlg,
				    IDD_TIMESTAMPS ),  -1, szComboBoxText );
			}


		}



		/**	Scan Interval Units (seconds or minutes)
		**/
		
		if ( ( settings.dwScanInterval % 60000 ) == 0 ) {		
			/**	Whole number of minutes**/
			bIntervalRepresentedAsMinutes = TRUE; 			
		}  else {		
			bIntervalRepresentedAsMinutes = FALSE;
		}
			
		
		for ( wID = SCANINTERVAL_UNITS_BASE; wID <
			SCANINTERVAL_UNITS_BASE + NUM_SCANINTERVAL_UNIT_TYPES; wID++ ) {

			LoadString ( hinst, wID, szComboBoxText,
					   MAX_COMBOBOX_STRLEN );

			ComboBox_AddString (
				GetDlgItem ( hdlg, IDD_INTERVAL_UNITS ),
						szComboBoxText );



			if ( ( bIntervalRepresentedAsMinutes ) && 
			      ( wID == SCANINTERVAL_UNITS_MINS ) ) {

				/**     Select the current selection...**/
				ComboBox_SelectString ( GetDlgItem ( hdlg,
					IDD_INTERVAL_UNITS ),  -1, szComboBoxText );

			} else if ( ( !bIntervalRepresentedAsMinutes ) && 
				( wID == SCANINTERVAL_UNITS_SECS ) ) {                                 

				/**     Select the current selection...**/
				ComboBox_SelectString ( GetDlgItem ( hdlg,
					IDD_INTERVAL_UNITS ),  -1, szComboBoxText );

			}
		}



		/**	Scan Interval
		**/ 
		if ( bIntervalRepresentedAsMinutes ) { 

			/**	Millisecs to minutes **/
			sprintf ( szEditText, "%.0f",
				(double) settings.dwScanInterval / 60000.0 );
		} else {
		
			/**	Millisecs to seconds **/
			sprintf ( szEditText, "%.2f",
				(double) settings.dwScanInterval / 1000.0 );		
		
		}
                
                

		Edit_SetText ( GetDlgItem ( hdlg, IDD_INTERVAL ),
						 szEditText );


		/**	The Options check-boxes
		**/
		CheckDlgButton ( hdlg, IDD_SCREEN_DISPLAY,
					(UINT) settings.bScreenDisplayOn );

		CheckDlgButton ( hdlg, IDD_DIGITAL_AS_TEXT,
					(UINT) settings.bDigitalAsText );

		CheckDlgButton ( hdlg, IDD_SHOW_EMPTY_SCANS,
					(UINT) settings.bIndicateEmptyScans );



		for ( wWidth = MIN_COLUMN_WIDTH; wWidth <= MAX_COLUMN_WIDTH;
								wWidth++ ) {

			sprintf ( szComboBoxText, "%2d", wWidth );

			ComboBox_AddString (
				GetDlgItem ( hdlg, IDD_COLUMN_WIDTH ),
							szComboBoxText );

			if ( wWidth == settings.wCharsPerColumn ) {

				/**	Select the current selection...
				**/
				ComboBox_SelectString ( GetDlgItem ( hdlg,
				    IDD_COLUMN_WIDTH ),	-1, szComboBoxText );
			}


		}
                
                
                
        /**	Logger's title
        **/
		Edit_LimitText ( GetDlgItem ( hdlg, IDD_TITLE ), MAX_TITLE );
		Edit_SetText   ( GetDlgItem ( hdlg, IDD_TITLE ), settings.szTitle );


		if ( settings.bLogOnAlarm ) {
			Button_SetCheck ( GetDlgItem ( hdlg, IDB_LOG_ON_ALARM ), 1 );
		} else if ( settings.bLogOnKeyPress ) {
			Button_SetCheck ( GetDlgItem ( hdlg, IDB_LOG_ON_KEYPRESS ), 1 );
		} else if ( settings.bLogOnChange ) {
			Button_SetCheck ( GetDlgItem ( hdlg, IDB_LOG_ON_CHANGE ), 1 );
		} else {
			Button_SetCheck ( GetDlgItem ( hdlg, IDB_LOG_ON_TIME ), 1 );
			Edit_Enable ( GetDlgItem ( hdlg, IDC_LOG_DURATION ),  0 );

		}


		
		/**	Millisecs to seconds
		**/
		sprintf ( szEditText, "%.2f",
				(double) settings.dwLogDuration / 1000.0 );		

		Edit_SetText ( GetDlgItem ( hdlg, IDC_LOG_DURATION ),
						 szEditText );

		


		/**     Return TRUE since we havn't used SetFocus
		**/
		return ( TRUE );

	case WM_COMMAND :

		switch ( LOWORD (wParam) ) {

		case IDD_HELP :

			/**     Invoke the help for the setting dialog...
			***     use the main window handle ( instance ) so
			***     that the Help is properly 'released' on exit.
			**/
			InvokeWinHelp ( hwndMain, HELP_CONTEXT,
							HC_SETTINGSHELP );
			break;

		case IDD_SCREEN_DISPLAY :
		case IDD_DIGITAL_AS_TEXT :
		case IDD_SHOW_EMPTY_SCANS :
		case IDD_INTERVAL :
		case IDD_COLUMN_WIDTH :
		case IDD_TIMESTAMPS :

			break;

		case IDB_LOG_ON_TIME :
			Edit_Enable ( GetDlgItem ( hdlg, IDC_LOG_DURATION ),  0 );
			break;

		case IDB_LOG_ON_ALARM :
		case IDB_LOG_ON_KEYPRESS :
		case IDB_LOG_ON_CHANGE :
			Edit_Enable ( GetDlgItem ( hdlg, IDC_LOG_DURATION ),  1 );
			break;


		case IDOK :
			/**     What type of unit is the interval specified in
			**/
			ComboBox_GetText ( GetDlgItem ( hdlg, IDD_INTERVAL_UNITS ),
				szComboBoxText, MAX_COMBOBOX_STRLEN );

			if ( ComboBox_FindString (
				GetDlgItem ( hdlg, IDD_INTERVAL_UNITS ),
				-1, szComboBoxText ) == 1 ) {         

				bIntervalRepresentedAsMinutes = FALSE;

			} else {
			
				bIntervalRepresentedAsMinutes = TRUE;
			}



			/**	Get and check the new sample interval
			**/
			Edit_GetText ( GetDlgItem ( hdlg, IDD_INTERVAL ),
					szEditText, MAX_EDITBOX_STRLEN );

			dIntervalInUnits = atof ( szEditText );

			/**     Convert the interval to millisecs.
			**/ 
			if ( bIntervalRepresentedAsMinutes ) {
			
				/**  Minutes to millisecs
				**/
				settings.dwScanInterval =
					(DWORD) ( ( dIntervalInUnits * 60000.0 ) + 0.5 );
			} else {
				
				/**  Seconds to millisecs	
				**/
				settings.dwScanInterval =
			   		(DWORD) ( ( dIntervalInUnits * 1000.0 ) + 0.5 );
			
			}  
			
			/**
			***	Enforce a minimum sampling interval
			***	of 20 millisecs.
			**/
			if ( settings.dwScanInterval < MIN_INTERVAL ) {

				settings.dwScanInterval = MIN_INTERVAL;

				/**	Interval too small. Warn the user
				**/
				sprintf ( szMessageText,
					"Smallest available target interval is "
					"%.2lf seconds",
					(double) settings.dwScanInterval / 1000.0 );


				if ( MessageBox ( hdlg, szMessageText,
					"Logger - Settings",
					MB_ICONQUESTION | MB_OKCANCEL )
					    == IDCANCEL ) {

					/**     User doesn't like the
					***     new interval.
					**/
					break;
				}

			}


			/**
			***	Enforce a maximum sampling interval
			***	of 24 hours
			**/
			if ( settings.dwScanInterval > MAX_INTERVAL ) {

				settings.dwScanInterval = MAX_INTERVAL;

				/**	Interval too small. Warn the user
				**/
				sprintf ( szMessageText,
				 "Maximum available target interval is 1440 Minutes (24 hours)" );


				if ( MessageBox ( hdlg, szMessageText,
					"Logger - Settings",
					MB_ICONQUESTION | MB_OKCANCEL )
						    == IDCANCEL ) {

					/**     User doesn't like the
					***     new interval.
					**/
					break;
					}

			}

                        
                        
                        
                        
			/**     Get the new TimeStamp Format
			**/

			ComboBox_GetText ( GetDlgItem ( hdlg, IDD_TIMESTAMPS ),
				szComboBoxText, MAX_COMBOBOX_STRLEN );

			settings.wTimeStampFormat = ComboBox_FindString (
				GetDlgItem ( hdlg, IDD_TIMESTAMPS ),
				-1, szComboBoxText ) + TIMESTAMP_BASE;



			/**	Get the new column width
			**/
			ComboBox_GetText ( GetDlgItem ( hdlg, IDD_COLUMN_WIDTH ),
					szComboBoxText, MAX_COMBOBOX_STRLEN );


			settings.wCharsPerColumn =
					(WORD) atoi ( szComboBoxText );





			/**	Finally the Checkboxes
			**/
			settings.bScreenDisplayOn = IsDlgButtonChecked ( hdlg,
							IDD_SCREEN_DISPLAY );


			settings.bIndicateEmptyScans =
					IsDlgButtonChecked ( hdlg,
							IDD_SHOW_EMPTY_SCANS );


			settings.bDigitalAsText = IsDlgButtonChecked ( hdlg,
							IDD_DIGITAL_AS_TEXT );
                        
			/**	The title
			**/
			Edit_GetText   ( GetDlgItem ( hdlg, IDD_TITLE ), 
					settings.szTitle, MAX_TITLE );



			settings.bLogOnAlarm = Button_GetCheck ( 
				                GetDlgItem ( hdlg, IDB_LOG_ON_ALARM ) );
			settings.bLogOnKeyPress = Button_GetCheck ( 
				                GetDlgItem ( hdlg, IDB_LOG_ON_KEYPRESS ) );
			settings.bLogOnChange = Button_GetCheck ( 
				                GetDlgItem ( hdlg, IDB_LOG_ON_CHANGE ) );

			if ( settings.bLogOnAlarm || settings.bLogOnKeyPress ||
				 settings.bLogOnChange ) {
				/**	Get and check the new log duration
				**/
				Edit_GetText ( GetDlgItem ( hdlg, IDC_LOG_DURATION ),
					szEditText, MAX_EDITBOX_STRLEN );

				dIntervalInUnits = atof ( szEditText );
				/**  Seconds to millisecs	**/
				settings.dwLogDuration =
			   		(DWORD) ( dIntervalInUnits * 1000.0 );

			} else {
				settings.dwLogDuration = 0;
			}
			

			EndDialog ( hdlg, TRUE );
			break;


		case IDCANCEL :

			EndDialog ( hdlg, FALSE );
			break;


		default :
			return ( FALSE );
		}


	default :
		return ( FALSE );
	}


	return ( TRUE );
}








/**
***	About_DlgProc
***
***	Message handling for 'About' dialog box
***
**/
BOOL PASCAL About_DlgProc ( HWND hdlg, WORD msg, WPARAM wParam,
                                                            LONG lParam )
{


	switch ( msg ) {

		case WM_INITDIALOG :


			return ( TRUE );

		case WM_COMMAND :

			switch ( LOWORD (wParam) ) {

				case IDOK :

					EndDialog ( hdlg, TRUE );
					break;

				default :
					return ( FALSE );
			}

		default :
			return ( FALSE );
	}

	return ( TRUE );
}
