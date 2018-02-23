/**
***	LG_LOG.c              ATK-LOGGER
***
***     Andrew Thomson
***
***     This module is responsible for acquiring channel data.
***
***
***	Associated Files : 	
***
***	Version History :
***
***	4.04		Code added to support date stamping to data file.
***			(i.e. wTimeStampFormat == DATE_TIME )
**/

#define  NOMINMAX
#define  NOKANJI
#define  NOCOMM
#define  STRICT
#define  IML_APPLICATION

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <string.h>

#include "\Windmill 32\Compile Libs\IMLinter.h"   /* ...using the COMMSLIB Dynalink                */
#include "\Windmill 32\Compile Libs\imslib.h"     /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATKit\ATKit.h"      /* Application Toolkit Header/Prototypes         */
#include "lg_def.h"     /* Manifest Constants and Structure Declarations */
#include "lg_prot.h"    /* Functions Prototypes                          */


extern LOGINFO		logInfo;       /* Logging Information		*/
extern CLIENTINFO	clntInfo;      /* Client Area  Details		*/
extern CLOCKINFO	clkRealTime;   /* 'Real Time Clock' data	*/
extern CONNECTIONS	conninfo;       /* Channel Connections		*/
extern SETTINGS         settings;      /* Logger Settings               */
extern INTLINFO		intl;	       /* International Settings	*/

extern RECT     	rectScroll;     /* Area to Scroll up      	*/
extern RECT             rectHighlight;  /* Current Data Readings        */
extern RECT             rectLoggedData;
extern RECT             rectLoggedTime;







/**
***	LogData
***
***
***
***	Determines if new data is required, and whether any data
***	has been missed. New data, if collected, is transfered to
***	HISTORICDATA strutures ready for painting onto the client area.
***
***	If required, data is logged to a log disk file.
***
***
**/
short   LogData ( HWND hwnd )
{

	short           nScansElapsed;	 /* Num Scans which need Processing */
	long            lVertScrollDist; /* Num Pixels to Scroll up         */
	RECT            rectInvalidated; /* Area left blank by scroll       */
	HDC             hdcClient;       /* Client Area device context      */
	short           nConnection;
	short           nMissed;         /* No Scans which we have missed   */
	short           nReturn;         /* Misc fn return code             */
	short           nFailedToRead;
	char            szLoggedTime [ MAX_TIMESTAMP_CHARS + 1 ];
	char            szLoggedDate [ MAX_DATESTAMP_CHARS + 1 ];
	char            szErrorText  [ MAX_DATA_CHARS + 1 ];
	DWORD           dwTimeNow;   /* Time on entry to this fn 	    */
	DWORD           dwDelay;
	short           nChar;

	static  BOOL    bInLogData = FALSE;  /* Prevent reentrance	    */
	static  DWORD	dwLogTime = 0;
    BOOL            bAlarmSet;


	/**   Stop this function being reenterant
	**/
	if ( bInLogData ) {
		
		/**	Already 'logging' data
		**/
		return ( OK );
	} 
	bInLogData = TRUE;	/* Now processing this fn-set the interlock */
	

	if ( settings.dwScanInterval == 0L ) {

		/**     Abort, invalid scan interval
		**/
		bInLogData = FALSE;
		return ( ERR_INVALID_INTERVAL );
	}


	dwTimeNow = GetIMLMillisecondCount ( );

	
	/** Wait until time is up. We sometimes arrive a little early **/
	while (  (dwTimeNow - logInfo.dwTimeLastScan) < settings.dwScanInterval  ) {
		dwTimeNow = GetIMLMillisecondCount ( );
		
		/** if we have more than 100 msec to wait the settings must
		*** have been changed. Return to stop the app freezing.
		*** This was incorrect dwTimeNow and logInfo.dwTimeLastScan
		*** were the wrong way round. This meant that the difference
		*** between them was added to the scan interval. So we exited
		*** every time
		*** 
		**/
		if ( ( settings.dwScanInterval - 
			(dwTimeNow - logInfo.dwTimeLastScan ) ) > 100 ) {
			bInLogData = FALSE;
			return (OK);
		}
	}




	/**	Calculate how many scan periods have elapsed.
	**/
	nScansElapsed = (short) ( ( dwTimeNow - logInfo.dwTimeLastScan )
					/ settings.dwScanInterval );


	/** If we are not logging simply on time **/
	if ( settings.bLogOnAlarm || settings.bLogOnKeyPress ||
		 settings.bLogOnChange ) {

		/** If we are waiting to start logging then missed scans
		*** are not important. So adjust the last scan time to
		*** leave just one scan.
		**/
		if ( (dwLogTime == 0) && (nScansElapsed > 1) ) {			
			logInfo.dwTimeLastScan += ( ( nScansElapsed - 1 ) * settings.dwScanInterval ) ;
			nScansElapsed = 1;		
		}
	}



	lVertScrollDist = -( nScansElapsed * clntInfo.lTextHeight );

	/** Check for change return if there is none **/
	if ( settings.bLogOnChange ) {
		if ( ReadFirstChannelChange ( hwnd ) == FALSE ) {
			
			/** If we are responding to a previous change
			*** then still log.
			**/
			if ( dwLogTime == 0 ) {
				/** Not logging **/
				bInLogData = FALSE;
				return ( OK );
			}
		} else {
			/** We will now log for this time minimum **/
			dwLogTime = settings.dwLogDuration + 1;
		}
	}


	switch ( nScansElapsed ) {

		case 0 :
			/**     No data required
			**/
			bInLogData = FALSE;
			return ( OK );
			break;

		default:
			/**     Drop in a Dummy line to indicate
			***     that we have missed one or more scans
			***     then drop thru to read latest scan
			**/
			if ( settings.bIndicateEmptyScans == TRUE ) {

				for ( nMissed = 0; nMissed < ( nScansElapsed - 1 );
						      nMissed++ ) {
					ReindexHistoricBuffer ( );

					lstrcpy ( clntInfo.alpHistIndx [ 0 ]
						->szTime, "--------" );

					for ( nConnection = 0;
					  nConnection < conninfo.nNumConnected;
						 nConnection++ ) {

						/**  Replace value with
						*** 'missed scan' indicator
						***  string
						**/
						lstrcpy ( clntInfo.alpHistIndx [ 0 ]
						    ->aszData [ nConnection ],
								 "--------" );


						/**	Default 'nAlarmState'
						**/
						clntInfo.alpHistIndx [ 0 ]
						->anAlarmState [ nConnection ]
								= ALARM_NORMAL;

					}

				}
				/**	Missed scans should NOT be
				***	recorded in the data file.
				***	Drop thru and collect some
				***	valid data.
				**/
			}




			/**     Drop Thru.............
			**/

		case 1 :
			/**     Take new Reading, first adjust
			***     the 'Time Last Scan' value. This stores
			***     the time of the current scan.
			**/
			logInfo.dwTimeLastScan
				+= ( nScansElapsed * settings.dwScanInterval ) ;

			nFailedToRead = ATKReadList ( hwnd,
						conninfo.nIMLInterface,
						conninfo.pciConnections,
						TRUE );

			/** how long to next reading **/
			dwDelay = settings.dwScanInterval - 
				  ( GetIMLMillisecondCount() - logInfo.dwTimeLastScan );
			
			
			/** If we are overdue for next reading set a very
			*** short delay
			**/
			if ( dwDelay < settings.dwScanInterval ) {
				SetTimer ( hwnd, SAMPLE_TIMER, dwDelay, NULL );
			} else {
				SetTimer ( hwnd, SAMPLE_TIMER, 10, NULL );
			}
			
			if ( nFailedToRead == conninfo.nNumConnected ) {

				/**	Failed to read data on ALL channels.
				***     Since all channels have 
				***	failed it may be necessary
				***	to abort logging. Check the error
				***	on the first channel. If its not
				***	an IML error (or its error 100, 
				***	demo violated) call it a day and 
				***	abort logging...
				**/
				if ( conninfo.pciConnections [ 0 ].nError
								 == 100 ) {
							
					/**	Demo Version
					***	Violation. Report
					***	this IML error to
					***	the user..
					**/
					bInLogData = FALSE;
					return ( ERR_DEMO_VIOLATED );



					/**	Non-IML errors which are
					***	usually fatal...
					**/

				} else {

					/** IML errors,
					*** We do NOT want to abort
					*** logging here as the errors
					*** are probably recoverable.
					***
					*** We can therefore continue, in
					*** which case the error number will
					*** be logged instead of the data OR
					*** simply exit now neither logging
					*** the data or the data. We do this
					*** at the user's request.
					**/
					if ( !settings.bIndicateEmptyScans ) {

						/**  User's not interested
						***  in displaying the error
						***  He is probably using an
						***  LabIML device which is
						***  expected to fail when
						***  data is not ready.
						***  Dont log the errors
						***  or update the client
						**/
						bInLogData = FALSE;
						return ( OK );

					} else {
						/** Log the Errors
						**/
					}
				}
			}

			/** Adjust the log time until it reaches 0 **/
			if ( dwLogTime > (nScansElapsed * settings.dwScanInterval ) ) {
				dwLogTime -= (nScansElapsed * settings.dwScanInterval );	
			} else {
				dwLogTime = 0;
			}


			/** Key press logging **/
			if ( settings.bLogOnKeyPress ) {
				
				if ( logInfo.bLogPressed ) {
					dwLogTime = settings.dwLogDuration + 1;
					logInfo.bLogPressed = FALSE;
				}

				/** No Alarms so give up **/
				if ( dwLogTime == 0  ) {
					bInLogData = FALSE;
					return (OK);
				}
			
			}


			/** Do the log on alarms code **/
			if ( settings.bLogOnAlarm ) {

				bAlarmSet = FALSE; 
				/** Are any alarms set **/
				for ( nConnection = 0; nConnection < conninfo.nNumConnected;
								nConnection++ ) {
					if ( conninfo.pciConnections [ nConnection ].nAlarmState != ALARM_NORMAL ) {
						/** + 1 rounds up so that at 1 sec rate you will
						*** get 2 readings after end of alarm for 2 sec
						*** setting.
						**/
						dwLogTime = settings.dwLogDuration + 1;
						bAlarmSet = TRUE;
					}
				}

				/** No Alarms so give up **/
				if ( dwLogTime == 0  ) {
					bInLogData = FALSE;
					return (OK);
				}

			}


			/**     Reorder the store of historic data
			***     so we may place the new data in the
			***     first HISTORICDATAROW structure
			**/
			ReindexHistoricBuffer ( );


			/**	Store the new data in the Historic Buffer
			***	If a channel was in error, replace the data
			***	string with a new string containing the
			***	error number.
			**/
			for ( nConnection = 0;
				nConnection < conninfo.nNumConnected;
							nConnection++ ) {

				if ( conninfo.pciConnections
						[ nConnection ].nError != OK ) {

					sprintf ( szErrorText, "Error %d",
						conninfo.pciConnections
						[ nConnection ].nError );

					lstrcpy ( clntInfo.alpHistIndx [ 0 ]
						->aszData [ nConnection ],
						szErrorText );


				} else {
					/**	Valid Logged data
					***
					***
					***	If digital we may need
					***	to replace the IML value
					***	with a user defined string
					***	Check for this case first...
					**/

					if ( ( settings.bDigitalAsText ) &&
					     ( conninfo.pciConnections
						[ nConnection ].bDigital ) ) {

						/**	replace 0 or 1
						***	with text string
						**/
						if ( atoi ( conninfo.pciConnections
						 [ nConnection ].szValue )
								    == 1 ) {

						  lstrcpy (
						   clntInfo.alpHistIndx [ 0 ]
						   ->aszData [ nConnection ],
						   conninfo.pciConnections
						   [ nConnection ].szOn );

						} else {

						  lstrcpy (
						   clntInfo.alpHistIndx [ 0 ]
						   ->aszData [ nConnection ],
						   conninfo.pciConnections
						   [ nConnection ].szOff );
						}

					} else {


						/** Copy IML data string
						**/

						lstrcpy (
						  clntInfo.alpHistIndx [ 0 ]
						  ->aszData [ nConnection ],
						  conninfo.pciConnections
						  [ nConnection ].szValue );

					}

					/** Also extract and store the
					*** alarm state.
					**/
					clntInfo.alpHistIndx [ 0 ]
					   ->anAlarmState [ nConnection ] =
					       conninfo.pciConnections
						[ nConnection ].nAlarmState;

				}


			}



			/**	Check we have stored the data using
			***	the correct decimal separator
			***
			**/
			if ( intl.cDecimalSeparator != '.' ) {

				/**    The International Settings
				***    have specified a new decimal
				***    separator which should be used
				***    in-place of the normal '.' char
				**/
				for ( nConnection = 0;
				  nConnection < conninfo.nNumConnected;
							    nConnection++ ) {

					nChar = 0;
					while ( clntInfo.alpHistIndx [ 0 ]
					  ->aszData [ nConnection ] [ nChar ]
								!= '\0' ) {

						if ( clntInfo.alpHistIndx [ 0 ]
							->aszData [ nConnection ]
							[ nChar ] == '.' ) {


							/**	Swap the separator
							***
							**/
							clntInfo.alpHistIndx [ 0 ]
							  ->aszData [ nConnection ]
							   [ nChar ] =
							      intl.cDecimalSeparator;


							/**	Assume only one
							***	decimal point per
							**	value.
							**/
							break;
						}
						nChar++;
					}
				}
			}





			/**     Derive the time Stamp from the first
			***     Connections Structure
			**/
			if (( settings.wTimeStampFormat == HOURS_MINS_SECS )||
			    ( settings.wTimeStampFormat == DATE_TIME ) ) {

				FormatTime ( szLoggedTime,
						conninfo.pciConnections [ 0 ].ulTime,
						  settings.wTimeStampFormat );

			} else {
				/**	At the moment, FormatTime
				***	cannot format the time stored
				***	in the Channel structures into
				***	seconds and tenth's. This is
				***	mainly because we do not know the
				***	absolute time when logging
				***	started to sufficient resolution
				***	(or in a convienient form)
				***	Instead use the time derived
				***	from GetTickCount at the start
				***	of this function. FormatTime
				***	compares this value with the
				***	GetTickCount value taken when 
				***	logging started.
				**/
			
				FormatTime ( szLoggedTime, dwTimeNow,
					settings.wTimeStampFormat );

			}

			/**	Derive the Date Stamp (if needed)
			***	from the first Connections Structure
			**/
			FormatDate ( szLoggedDate,
				conninfo.pciConnections [ 0 ].ulTime,
					  settings.wTimeStampFormat );



			if ( intl.cTimeSeparator != ':' ) {

				/**    The International Settings
				***    have specified a new time
				***    separator which should be used
				***    in-place of the normal ':' char
				**/

				nChar = 0;
				while ( szLoggedTime [ nChar ] != '\0' ) {

					if ( szLoggedTime [ nChar ] == ':' ) {


						/**	Swap the separator
						***
						**/
						szLoggedTime [ nChar ] =
						      intl.cTimeSeparator;


					}
					nChar++;
				}
			}




			/**	Copy into our client Info structure
			**/
			lstrcpy ( clntInfo.alpHistIndx [ 0 ]->szTime,
							       szLoggedTime );

			lstrcpy ( clntInfo.alpHistIndx [ 0 ]->szDate,
							       szLoggedDate );



			/**	if displaying the data, scroll the old data
			***	up to make way for the new data.
			***
			**/
			if ( settings.bScreenDisplayOn == TRUE ) {



				/**  Scroll the client and invalidated the
				***  uncovered area as well as the highlighted
				***  bar containing the new data.....
				**/
				hdcClient = GetDC ( hwnd );

				ScrollDC ( hdcClient,
				   0,                      /* Scroll Horz */
				   lVertScrollDist,  	   /* Scroll Vert */
				   &rectScroll,
				   &rectScroll,
				   NULL,
				   &rectInvalidated );



				ReleaseDC ( hwnd, hdcClient );
			}
			/**	Store the new data in the data file...
			***     remembering the return value which we
			***     will pass upto the caller.
			**/
			if ( settings.bLogDataToDisk == TRUE ) {

				/** If we are doing periodic files ensure the
				*** Clock is right up to date.
				**/
				if ( settings.bPeriodicFiles ) {
					ServiceRealTimeClock ( hwnd );
				}

				/**	If requested to open periodic
				***	files, check if a new file needs
				***	opening...
				**/
				if ( ( settings.bPeriodicFiles ) &&
				     ( logInfo.tmCalenderTimeNextFile
						< clkRealTime.timeNow )  ){

					/**	Time to open a new file
					***
					***	Store the termination time and close the
					***	existing file.
					**/
					logInfo.tmCalenderTimeStopped
							= clkRealTime.timeNow;

					/** Close existing file
					**/
					CloseDataFile ( FALSE );


					logInfo.tmCalenderTimeStarted
							= clkRealTime.timeNow;

					/**	Open Next File
					**/
					nReturn = CreateDataFile ( FALSE );

					if ( nReturn != OK ) {

						/** Failed to create
						*** new file.
						**/
						return ( nReturn );
					}

					/**	Inform User of new file number
					**/
					UpdateStatusBar ( hwnd );
				}
				nReturn = AppendDataToFile ( );

			} else {
				nReturn = OK;

			}

			/** Invalidate the area uncovered by the scroll
			*** and the highlighted area displaying the
			*** newly acquired data.
			**/
			if ( settings.bScreenDisplayOn == TRUE ) {

			  InvalidateRect ( hwnd, &rectInvalidated, FALSE );
			  InvalidateRect ( hwnd, &rectHighlight,   FALSE );

			}


			/**	A DDE Client may be interested in
			***	this channel.
			**/

			PTKAdviseDDE ( (short)conninfo.nNumConnected );


			break;


	}
	bInLogData = FALSE;
	return ( nReturn );
}


/***************************************************/
/** This routine reads the first channel in the logging scan
*** and compares it with the previous stored value. It returns TRUE
*** if it has changed. FALSE else.
**/

BOOL ReadFirstChannelChange ( HWND hwnd )

{
	

	ATKReadChannel ( hwnd, conninfo.nIMLInterface,
					conninfo.pciConnections , TRUE );

	if ( strcmp ( conninfo.pciConnections [0].szValue, logInfo.szFirstChannel ) == 0 ) {
		return ( FALSE );
	}

	strcpy ( logInfo.szFirstChannel, conninfo.pciConnections [0].szValue );
	return (TRUE);

}