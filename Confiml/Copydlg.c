
/** This file contains the code used to implement the software
*** security system. 
**/

#include <windows.h>

#include <stdio.h>                                                           
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Resource.h"
#include "Confiml.h"
#include "Copycomm.h"



extern GENERAL_INFO giData;	/* our general information structure. */
extern SOFTWARE SoftwareEntry [32];
extern char cImlini [];


char szReleaseKey [8];
char szTargetKey [8];



void InitialiseSoftwareDialog ( HWND hDlg )

{

	HWND	hwndList;
	int		nPrograms = 0;
	int		nWindmillDate, nComputerCode, nReleases;
	int		nInvalidKeys = 0;
	char	cTemp [80];
	char	cBuffer [18];
	long	lResult;
	char	cDate [11];
	int		nYear;
	int     nMonth;
	int		nCurrentDate;

	hwndList = GetDlgItem ( hDlg, IDD_SOFTWARE_LIST );

	SendMessage ( hwndList, LB_RESETCONTENT, 0, 0 );

	GetSoftwareDetailsFromINI ( cImlini );

	// for each program in turn
	while ( strlen ( SoftwareEntry[nPrograms].szName ) != 0 ) {

		/** Search for the Windmill entry We need its
		*** Date for later.
		**/
		if (stricmp ( "Windmill", SoftwareEntry[nPrograms].szName) == 0) {

			nWindmillDate = SoftwareEntry[nPrograms].nDate;
		}

	
		/** Prepare the list string.
		*** First the software name
		**/
		strcpy ( cTemp, SoftwareEntry[nPrograms].szName );
		
		
		/** Add Tab **/
		strcat ( cTemp,"\t" );
		cTemp[15] = 0;

							
		/** Add spaces**/
		strcat ( cTemp, "  " );


		/** Read the current date as "mm/dd/yy" and work out the
		*** current date in our months since JAN 99 format
		**/
		_strdate ( cDate );
		
		/** Work out months and years **/	
		nYear = atoi ( cDate + 6 );
		cDate[2] = 0;
		nMonth = atoi ( cDate ); 
	
		/** Convert the date to months since JAN 99 **/	
		if ( nYear > 40) {
			nCurrentDate = (nYear - 99) * 12 + nMonth - 1 ;
		} else {
			nCurrentDate = ( nYear + 1 ) * 12 + nMonth - 1;
		}

		nCurrentDate &= 0x7F;

		/** We have prepared the current date as months since JAN 1999 and
		*** reduced it to 0 to 127 format. If the install date is less than or equal to this
		*** it could be either one of 2 possibilties since we do not know the state
		*** of the 128 bit. We assume its the more recent date
		**/


		if ( SoftwareEntry [nPrograms].nDate <= nCurrentDate ) {
			/** Prepare the year **/
			itoa ( ( ( SoftwareEntry[nPrograms].nDate + 128 ) / 12) + 1999, 
					cBuffer, 10 );

			/** Add it **/
			strcat ( cTemp, cBuffer );

			/** Separator **/
			strcat ( cTemp, ":");
			
			/** Prepare the month **/
			itoa ( (( SoftwareEntry[nPrograms].nDate + 128)  % 12) + 1, 
					cBuffer, 10 );

		} else {
			/** Prepare the year **/
			itoa ( (SoftwareEntry[nPrograms].nDate / 12) + 1999, 
					cBuffer, 10 );
			/** Add it **/
			strcat ( cTemp, cBuffer );

			/** Separator **/
			strcat ( cTemp, ":");
			
			/** Prepare the month **/
			itoa ( (SoftwareEntry[nPrograms].nDate % 12) + 1, 
					cBuffer, 10 );
		}
		
		
		/** Add it **/
		strcat ( cTemp, cBuffer );
			

		/** add tab**/
		strcat ( cTemp, "\t     " );

		/** Check the code **/
		if ( CalculateSoftwareCode ( SoftwareEntry[nPrograms].szName )
			== SoftwareEntry[nPrograms].nCode ) {
			strcat ( cTemp, "yes");
		} else {
			strcat ( cTemp, "no");
			/** Count the keys **/
			++nInvalidKeys;
		}
        
		lResult = SendMessage ( hwndList, LB_ADDSTRING, 0,
                                            (LPARAM)  cTemp);
        if (( lResult == LB_ERR) || (lResult == LB_ERRSPACE )){
             HandleError ( NO_MEMORY );
			 return;
        }
							
		// Move to next program
		++ nPrograms;
	}


	/** Get the computer code **/
	nComputerCode = GetComputerCode ();

	/** Get the previous releases stored in HWX **/
	nReleases = GetNumberofReleases ();

	CalculateKey ( cTemp, nPrograms, nInvalidKeys,
			nWindmillDate, nComputerCode, nReleases );
	
	SetDlgItemText ( hDlg, IDC_REQUEST_KEY, cTemp);

	
			
	/** Now calculate the key required for a release **/
	
	++nReleases; // Increment the releases

	if ( nReleases > 7 ) {
		nReleases = 0;
	}
	CalculateKey ( szTargetKey, nPrograms, 0,
			nWindmillDate, nComputerCode, nReleases );

			

}

/****************************************************************
**/

BOOL WINAPI SoftwareDlgFn ( HWND hDlg, WORD wMessage, WPARAM wParam,
							LONG lParam )

{

static int bHelpStarted ;

	switch ( wMessage ) {

        case WM_INITDIALOG :
                /** We need to fill the list box with the installed software.              
                **/

                CenterDialog ( hDlg );
				
				InitialiseSoftwareDialog ( hDlg );
			
			return (TRUE);        /* No focus set */


		case WM_COMMAND :

			switch ( wParam ) {

			case IDD_SOFTWARE_LIST :
				break;

			case IDC_ENTER_RELEASE :
				
				DialogBox (giData.hInst, "RELEASE", hDlg, (DLGPROC) ReleaseDlgFn );
				SetDlgItemText ( hDlg, IDC_RELEASE_KEY, szReleaseKey);

				if ( stricmp ( szReleaseKey, szTargetKey) == 0 ) {
					AuthoriseAllPrograms ( GetNumberofReleases () + 1);
					InitialiseSoftwareDialog ( hDlg );
				}

				break;
				
				case IDDHELP :
					bHelpStarted = InvokeWinHelp ( hDlg, HELP_INDEX );
					break;

				case IDOK :
				case IDCANCEL:

					if ( bHelpStarted ) {
						InvokeWinHelp ( hDlg, HELP_QUIT );
					}

				
				EndDialog ( hDlg, FALSE );
				return ( TRUE );

			}

			return ( TRUE );
	}

	return ( FALSE );
}



/****************************************************************
**/

BOOL WINAPI ReleaseDlgFn ( HWND hDlg, WORD wMessage, WPARAM wParam,
							LONG lParam )

{	
	
	
	
	switch ( wMessage ) {

        case WM_INITDIALOG :
                /** We need to fill the list box with the installed software.              
                **/

                CenterDialog ( hDlg );

			return (TRUE);        /* No focus set */


		case WM_COMMAND :

			switch ( wParam ) {


			case IDOK : 
			case IDCANCEL :
				
                GetDlgItemText ( hDlg, IDC_RELEASE_BOX,
                                   szReleaseKey, 8 );
				
				EndDialog ( hDlg, FALSE );
				return ( TRUE );

			}

			return ( TRUE );
	}

	return ( FALSE );
}
