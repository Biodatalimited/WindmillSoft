/**
***	LG_FILE.c              ATK-LOGGER
***
***     Andrew Thomson
***
***     This module is responsible for data file creation and
***     service.
***
***
***	Associated Files : 	
***
***	Version History :
***	
**/
#define  NOMINMAX
#define  NOKANJI
#define  NOCOMM
#define  STRICT         /* Use Windows 3.1 Strict Style Coding  */
#define  CTLAPIS        /* Use Control API Macros 		*/

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <time.h>

#include "\Windmill 32\Compile Libs\IMLinter.h"   /* ...using the COMMSLIB Dynalink                */
#include "\Windmill 32\Compile Libs\imslib.h"     /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATKit\ATKit.h"      /* Application Toolkit Header/Prototypes         */
#include "lg_def.h"     /* Manifest Constants and Structure Declarations */
#include "lg_prot.h"    /* Logger modules Function Prototypes            */
#include "lg_dlgs.h"    /* Dialogs Identification Codes                  */


extern  LOGINFO         logInfo;
extern  SETTINGS        settings;
extern  CONNECTIONS     conninfo;
extern	CLIENTINFO	clntInfo;
extern	INTLINFO	intl;
extern CLOCKINFO	clkRealTime;   /* 'Real Time Clock' data	*/

extern	HINSTANCE	hInst;		/** Program Instance Handle **/
extern	HWND		hwndMain;	/** Program Window Handle   **/

HFILE		hDataFile   = 0;     /* Initialise to NULL, no open file  */
HFILE		hHeaderFile = 0;	/* Initialize to NULL, no open File  */
OFSTRUCT	ofDataFile;		/* Data File Details 		     */	
OFSTRUCT	ofHeaderFile;		/* Hdr File details if serperate     */
static 	LONG	lLoggerStoppedStringPos;   /* Ptr into Header File           */

/**     Static Function Prototypes
**/
static VOID  PadWithSpaces ( char *acBuffer, long lNumberOfSpaces );
static short BuildLogFilename ( char *szPathName );
static short BuildHdrFilename ( char *szPathName );
void Int2Bytes ( char* szString, int nValue );

/**
***	CreateDataFile
***
***
***	Opens a new log data file.
***
***	Parameters :
***
***		bOpenFirstFile	If TRUE, the file being opened is
***				assumed to be the first ( or only )
***				data file of a sequence. This file
***				will have a file existance check.
***
***				If FALSE, the file is not checked
***				for existance.
***
***
**/
short	CreateDataFile	( BOOL bOpenFirstFile )

{

	WORD	wFlags;
	short	nReturn;
	short	nDlgControlID;
	char	szPathName 	[ _MAX_PATH  + 1 ];
	BOOL    bFileExists = FALSE;
	BOOL    bCurrentPeriodic = FALSE;
	BOOL    bAppend = FALSE;


	/**     Create the full path name from the details
	***     stored in the 'settings' structure.
	**/
	nReturn = BuildLogFilename ( szPathName );

	if ( nReturn != OK ) {
		return ( nReturn );
	}

	/** Make these checks for the first file **/
	if ( bOpenFirstFile ) {
		 /** Test for existance. **/
		if ( OpenFile ( szPathName, &ofDataFile, OF_READ | OF_EXIST ) != -1 ) {
			bFileExists = TRUE;
			/** Is this a current periodic file **/
			if ( settings.bPeriodicFiles) {
				bCurrentPeriodic = 	IsPeriodicFileCurrent ( szPathName );
			}
		}
	}

	/**	Now decide if to append or not.
	*** We start from the assumtion that we do not append.
	***	In order to append :
	***		The file must exist
	***		The first file flag must be set
	***
	***     If we are autostarting append to existing simgle
	***     files or current periodic files.
	***
	***     If we are not autostarting let the user decide.
	**/


	/** We are autostarting a single existing file just append **/
	if ( (bOpenFirstFile) && (bFileExists) &&
		(!settings.bPeriodicFiles) && (logInfo.bAutoStarted) ) {
		bAppend = TRUE;
	}

	/** We are autostarting an existing period file. Append if the
	*** file is current otherwise create a new file.
	**/
	if ( (bOpenFirstFile) && (bFileExists) &&
		 (settings.bPeriodicFiles) && (logInfo.bAutoStarted) ) {
		if ( bCurrentPeriodic ) {
			bAppend = TRUE;
		}
	}

    /** We are manually opening an existing file. Ask the user what
	*** to do.
	**/
	if ( (bOpenFirstFile) && (bFileExists) && (!logInfo.bAutoStarted) ) {


		/**	Envoke the dialog.
		***	This returns the control
		***	ID of the button pressed...
		**/
		nDlgControlID = DialogBox ( hInst,
									"FILEEXISTS", hwndMain,
									(DLGPROC) FileExists_DlgProc );


		switch ( nDlgControlID	) {

		case IDCANCEL:

			/**	User has Canceled the dialog
			**/
			return ( ERR_USER_ABORTED );
			break;

		case IDD_OVERWRITE :
			break;

		case IDD_APPEND :

			/** Append to file **/
			bAppend = TRUE;
			
		}
	}


	/** Flags to append or not **/
	if ( bAppend ) {
		wFlags = OF_WRITE | OF_SHARE_DENY_WRITE;
	} else {
		wFlags = OF_CREATE | OF_WRITE | OF_SHARE_DENY_WRITE;
	}

	/**
	***     Now open the file properly....
	**/
	hDataFile = OpenFile ( szPathName, &ofDataFile, wFlags );


	/**	Obtain a handle to the header file. Unless opening
	***	a seperate header file ( first comma-seperated file )
	***	this handle will be the same as the data file since
	***	the header information is integral to the data file.
	**/
	if ( ( settings.wFileFormat == COMMA_SEPARATED ) &&
						( bOpenFirstFile ) ) {

		BuildHdrFilename ( szPathName );

		hHeaderFile = OpenFile ( szPathName, &ofHeaderFile, wFlags );

	} else {

		hHeaderFile = hDataFile;   /* Same File for Both */
	}



	/**	Write the header unless this is a 'subsequent' comma
	***	seperated file which already has a header file common
	***	between all data files.
	**/
	if ( ( settings.wFileFormat == COMMA_SEPARATED ) &&
						( !bOpenFirstFile ) ) {

		/**	Only first comma seperated data file has
		***	associated header information. Do nothing...
		**/

	} else {

		if ( ( hDataFile == (HFILE) -1 ) || ( hHeaderFile == (HFILE) -1 ) ) {

			/**	One or both files could not be created, more detailed error
			***	information available from 'ofDataFile.nErrCode'
			***	
			***  Close any file which did open OK **** SRG 12 July 1997
			**/

			if ( hHeaderFile != (HFILE) -1 ) {	/*** SRG 12/7/97 */
				_lclose ( hHeaderFile );	/*** SRG 12/7/97 */
			}

			if ( hDataFile != (HFILE) -1 ) {	/*** SRG 12/7/97 */
				_lclose ( hDataFile );		/*** SRG 12/7/97 */
			}

			hHeaderFile = 0;			/*** SRG 12/7/97 */
			hDataFile = 0;              		/*** SRG 12/7/97 */
			return ( ERR_FILE_CREATION );
		}



		/**	Store the header Information in the data file
		***	unless Comma seperated type selected. In this
		***	case create a seperate header file.
		***
		**/

		/**	Move to end of the file incase we are appending data
		***	to an existing file...
		**/
		_llseek ( hHeaderFile, 0L, 2 );

		nReturn = WriteHeader ( hHeaderFile );
		if ( nReturn != OK ) {
	
			_lclose ( hHeaderFile );
			return ( nReturn );
		}
	
		/**	Close the header if stored seperately
		***	and at any rate destroy the redundant handle.
		**/
		if ( hHeaderFile != hDataFile ) {
			_lclose ( hHeaderFile );
		}
		hHeaderFile = 0;
	

	} /* End of Writing Header Info */



	/**	If time allows, close the data file
	***
	**/
	if ( settings.dwScanInterval < 1000L ) {

		/**     Leave File Open till logging Stops
		***     hDataFile will remain a valid DOS
		***     file handle.
		**/

	} else {
		/**     Close the file, which was left open
		***     and set the handle to NULL to indicate
		***     no open file.
		**/
		_lclose ( hDataFile );

		hDataFile = 0;
	}

	/** Set the target time to start the next file **/
	if ( settings.bPeriodicFiles ) {
		logInfo.tmCalenderTimeNextFile = GetPeriodicFileTime ( TRUE );
	}

	return ( OK );

}





/**
***  CloseDataFile
***
***
***	Closes a log data file.
***
***	Parameters :
***
***		bCloseLastFile	If TRUE, the file being closed is
***				assumed to be the last ( or only )
***				data file of a sequence. If a seperate
***				header is included, this should be
***				updated and closed now.
***
***
***
***
***
***
***
**/
short	CloseDataFile  ( BOOL bCloseLastFile )
{
	char		acBuffer [ DATA_FILE_BUFFER_SIZE + 1 ];
	struct	tm	*ptmGMT;
	HFILE		hHeaderFile;

	if ( hDataFile != 0 ) {

		/**     Close the data file, which was left open
		***     and set the handle to NULL to indicate
		***     no open file.
		**/
		_lclose ( hDataFile );
		hDataFile = 0;

	}



	/**	The rest of this function places the
	***	logging stopped time in the header
	***	Dont bother doing this if we are using
	***	a seperate header file and this is NOT
	***	the very last file. ( Since we only
	***	have one header for all of the log files )
	**/
	if ( ( settings.wFileFormat == COMMA_SEPARATED ) &&
					( !bCloseLastFile ) ) {

		/**	We are using a single, seperate header
		***	file. We will only update this when we close
		***	the very last log data file.
		**/
		return ( OK );
	}







	/**	Update the Header File. ( This may have been the file
	***	we just closed but its easier just to reopen it. )
	**/
	if ( settings.wFileFormat == COMMA_SEPARATED ) {

		hHeaderFile = OpenFile ( ofHeaderFile.szPathName,
				   &ofHeaderFile, OF_WRITE | OF_REOPEN );

	} else {
		/**	Integral Data/Header File.
		**/
		hHeaderFile = OpenFile ( ofDataFile.szPathName, &ofDataFile,
	 					   OF_WRITE | OF_REOPEN );
	}
	
	 					    		 
	_llseek ( hHeaderFile, lLoggerStoppedStringPos, 0 );

	ptmGMT = gmtime ( (time_t *) &logInfo.tmCalenderTimeStopped );
	
	if ( settings.wFileFormat == COMMA_SEPARATED ) {

		/**	Enclose string in with quotation marks 
		**/
		strftime ( acBuffer, DATA_FILE_BUFFER_SIZE,
		  "\"File closed at %H:%M:%S %a %d %b %Y\"",
		  ptmGMT );

	} else { 		
		/**	No quotes so include two space chars instead.
		**/
		strftime ( acBuffer, DATA_FILE_BUFFER_SIZE,
		  "File closed at %H:%M:%S %a %d %b %Y  ", ptmGMT );
	}
	_lwrite ( hHeaderFile, acBuffer, strlen ( acBuffer ) );
	_lclose ( hHeaderFile );
	hHeaderFile = 0; 	



	return ( OK );

}





/**
***  AppendDataToFile
***
***
***
***
***
***
***
**/
short   AppendDataToFile ( VOID )
{
	char    acBuffer [ DATA_FILE_BUFFER_SIZE + 1 ];
	char    cTerminator;
	short   nConnection;
	short   bCloseAfterAppend;
	WORD    wBytesSent;

	if ( hDataFile == 0  ) {

		hDataFile = OpenFile ( ofDataFile.szPathName, &ofDataFile,
	 					    OF_WRITE | OF_REOPEN  );
		/**     Move to end of the file..
		**/
		_llseek( hDataFile, 0L, 2);

		bCloseAfterAppend = TRUE;
	} else {
		/**     File is already open, leave open after use...
		**/
		bCloseAfterAppend = FALSE;
	}

	/**     Determine type of filed terminator...
	**/
	switch ( settings.wFileFormat ) {
		default :
		case ( TAB_SEPARATED ) :
			cTerminator = '\t';
			break;
		case ( SPACE_SEPARATED ) :
			cTerminator = ' ';
			break;
		case ( COMMA_SEPARATED ) :
			cTerminator = intl.cListSeparator;
			break;
	}

	lstrcpy ( acBuffer, "" );	/* Init buffer with empty string */

	/**	Add the date stamp + terminator if required
	**/
	if ( settings.wTimeStampFormat == DATE_TIME ) {

		lstrcat ( acBuffer, clntInfo.alpHistIndx [ 0 ]->szDate );


		/**	Terminate Field, add the new null terminator
		***	onto the existing null terminator before
		***	overwriting with the field terminator...
		**/
		if ( settings.wFileFormat == SPACE_SEPARATED ) {

			PadWithSpaces ( acBuffer, max ( 0, 15 - lstrlen
				( clntInfo.alpHistIndx [ 0 ]->szTime ) ) );

		} else {

			acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
			acBuffer [ strlen ( acBuffer )	   ] = cTerminator;
		}

	} /* End if Date */



	/**     Initialise the buffer adding the Time stamp + terminator
	**/
	lstrcat ( acBuffer, clntInfo.alpHistIndx [ 0 ]->szTime );


	/**     Terminate Field, add the new null terminator
	***     onto the existing null terminator before
	***	overwriting with the field terminator...
	**/
	if ( settings.wFileFormat == SPACE_SEPARATED ) {

		PadWithSpaces ( acBuffer, max ( 0, 15 - lstrlen
			( clntInfo.alpHistIndx [ 0 ]->szTime ) ) );

	} else {

		acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
		acBuffer [ strlen ( acBuffer )     ] = cTerminator;
	}



	/**     Now append the data from each connected channel
	**/
	for ( nConnection = 0;
		nConnection < conninfo.nNumConnected; nConnection++ ) {

		lstrcat ( acBuffer,
			clntInfo.alpHistIndx [ 0 ]->aszData [ nConnection ] );

		/**	Terminate Field, add the new null terminator
		***	onto the existing null terminator before
		***	overwriting with the field terminator...
		**/
		if ( settings.wFileFormat == SPACE_SEPARATED ) {

			PadWithSpaces ( acBuffer, max ( 0, 15 - lstrlen
				( clntInfo.alpHistIndx [ 0 ]->
					aszData [ nConnection ] ) ) );

		} else if ( nConnection != ( conninfo.nNumConnected - 1 ) ) {

			acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
			acBuffer [ strlen ( acBuffer )     ] = cTerminator;
		}
	}

	/**     Append EOLN terminator - carriage return\line feed
	**/
	lstrcat ( acBuffer, "\r\n" );

	wBytesSent = _lwrite ( hDataFile, acBuffer, strlen ( acBuffer ) );

	if ( bCloseAfterAppend ) {

		_lclose ( hDataFile );
		hDataFile = 0;
	}


	if ( wBytesSent != strlen ( acBuffer ) ) {

		/**     Write operation failed...
		**/
		return ( ERR_FILE_WRITE );

	} else {
		return ( OK );
	}
}





/**
***	WriteHeader
***
***
***
***
**/
short WriteHeader ( HFILE hHeaderFile )
{

	char	acBuffer [ DATA_FILE_BUFFER_SIZE + 1 ];
	char	cTerminator;
	short	nConnection;
	WORD	wBytesSent;
	struct	tm      *ptmGMT;


	/**     Determine type of filed terminator...
	**/
	switch ( settings.wFileFormat ) {
		default :
		case ( TAB_SEPARATED ) :
			cTerminator = '\t';
			break;
		case ( SPACE_SEPARATED ) :
			cTerminator = ' ';
			break;
		case ( COMMA_SEPARATED ) :
			cTerminator = ',';
			break;
		}


	/**	If we already have information in the file
	***	throw a couple of carriage return line feeds
	***	to seperate the new header info from existing
	***	data.
	**/
	if ( _llseek ( hHeaderFile, 0L, 1 ) != 0L ) {
		_lwrite ( hHeaderFile, "\r\n\r\n", 4 );		
	} 


	/**	Include the user assigned Logger title in data file header
	**/        
	lstrcpy ( acBuffer, settings.szTitle ); 
	lstrcat ( acBuffer, "\r\n" );
	_lwrite ( hHeaderFile, acBuffer, strlen ( acBuffer ) );

        
	ptmGMT = gmtime ( (time_t *) &logInfo.tmCalenderTimeStarted );
        
	if ( settings.wFileFormat == COMMA_SEPARATED ) {
		/**	Enclose string in with quotation marks 
		**/
		strftime ( acBuffer, DATA_FILE_BUFFER_SIZE,
		  "\"File opened at %H:%M:%S %a %d %b %Y\"\r\n",
		  ptmGMT );
	} else { 		
		strftime ( acBuffer, DATA_FILE_BUFFER_SIZE,
		  "File opened at %H:%M:%S %a %d %b %Y\r\n", ptmGMT );
	}
	_lwrite ( hHeaderFile, acBuffer, strlen ( acBuffer ) );
	
	/**	Leave Room for 'Logging Stopped' string, noting the seek
	***	position (using _llseek with nOrigin set to the current pos )
	***	We we will fill this string in after logging has terminated.
	***	The duumy string must be large enough to hold the final
	***	string.
	**/
	lLoggerStoppedStringPos = _llseek  ( hHeaderFile, 0L, 1 );

	strcpy	( acBuffer,
		"\"File closed at ........................\"\r\n\r\n" );



	_lwrite ( hHeaderFile,  acBuffer, strlen ( acBuffer ) );


	/**     Now append the channel name for each connected channel
	***	The first field is normally 'Time' unless we are using
	***	the DATE_TIME format when we have both Data and Time.
	**/
	lstrcpy ( acBuffer, "" );  /* Empty the buffer */
	if ( settings.wTimeStampFormat == DATE_TIME ) {

		if ( settings.wFileFormat == COMMA_SEPARATED ) {
			/**	Enclose string in with quotation marks
			**/
			lstrcat ( acBuffer, "\"Date\"" );
		} else {
			lstrcat ( acBuffer, "Date" );
		}


		if ( settings.wFileFormat == SPACE_SEPARATED ) {

			PadWithSpaces ( acBuffer, ( 15 - 4 ) );

		} else {

			acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
			acBuffer [ strlen ( acBuffer )	   ] = cTerminator;
		}
	}

	/**
	***	Always have time.
	**/
	if ( settings.wFileFormat == COMMA_SEPARATED ) {
		/**	Enclose string in with quotation marks 
		**/
		lstrcat ( acBuffer, "\"Time\"" );
	} else {
		lstrcat ( acBuffer, "Time" );
	}


	if ( settings.wFileFormat == SPACE_SEPARATED ) {

		PadWithSpaces ( acBuffer, ( 15 - 4 ) );

	} else {

		acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
		acBuffer [ strlen ( acBuffer )     ] = cTerminator;
	}

	for ( nConnection = 0;
		nConnection < conninfo.nNumConnected; nConnection++ ) {

		if ( settings.wFileFormat == COMMA_SEPARATED ) {
			acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
			acBuffer [ strlen ( acBuffer ) ]     = '"';
		}				


		lstrcat ( acBuffer,
			conninfo.pciConnections [ nConnection ].szChannel );



		/**     Terminate Field, add the new null terminator
		***     onto the existing null terminator before overwriting
		***     with the field terminator...
		**/
		switch ( settings.wFileFormat ) {
			case ( SPACE_SEPARATED ) :
		
				/**	Pad with spaces but no terminator
				**/
				PadWithSpaces ( acBuffer, max ( 0, 15 - lstrlen (
				 conninfo.pciConnections [ nConnection ].szChannel ) ) );
				break;
			
			case ( COMMA_SEPARATED ) :
				/**	Append quotation mark...
				***	before adding field terminator
				**/
				acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
				acBuffer [ strlen ( acBuffer )     ] = '"';

				/**	Drop thru .. to add field terminator
				**/
			
			default :			
				/**	Add field terminator to all but last field.
				**/
				if ( nConnection != ( conninfo.nNumConnected - 1 ) ) {

					acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
					acBuffer [ strlen ( acBuffer )     ] 
						=  cTerminator;
				}
				break;
		}
	}

	/**     Append EOLN terminator - carriage return\line feed
	**/
	lstrcat ( acBuffer, "\r\n" );

	wBytesSent = _lwrite ( hHeaderFile, acBuffer, strlen ( acBuffer ) );






	/**	Now append the Channel Units for each connected channel
	***	The first field is normally 'Secs' for time but if
	***	we are using DATE_TIME we should provide an empty field
	***	in the first column keeping the remaining columns in order
	***	OR provide Units string of Days
	**/
	lstrcpy ( acBuffer, "" );  /* Empty the buffer */
	if ( settings.wTimeStampFormat == DATE_TIME ) {

		if ( settings.wFileFormat == COMMA_SEPARATED ) {
			/**	Enclose string in with quotation marks
			**/
			lstrcat ( acBuffer, "\"Days\"" );
		} else {
			lstrcat ( acBuffer, "Days" );
		}


		if ( settings.wFileFormat == SPACE_SEPARATED ) {

			PadWithSpaces ( acBuffer, ( 15 ) );

		} else {

			acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
			acBuffer [ strlen ( acBuffer )	   ] = cTerminator;
		}
	}



	/**	Always have a units for the time field
	**/ 
	switch ( settings.wTimeStampFormat ) {
	case MINUTES_AND_SECS : 
	 
		if ( settings.wFileFormat == COMMA_SEPARATED ) {
			/**	Enclose string in with quotation marks 
			**/
			lstrcat ( acBuffer, "\"Mins\"" );
		} else {
			lstrcat ( acBuffer, "Mins" );
		}		        
		break;
		
	default :		        
		if ( settings.wFileFormat == COMMA_SEPARATED ) {
			/**	Enclose string in with quotation marks 
			**/
			lstrcat ( acBuffer, "\"Secs\"" );
		} else {
			lstrcat ( acBuffer, "Secs" );
		}		        
		break;
		
	}
	  
	  
	  
	if ( settings.wFileFormat == SPACE_SEPARATED ) {

		PadWithSpaces ( acBuffer, ( 15 - 4 ) );

	} else {

		acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
		acBuffer [ strlen ( acBuffer )     ] = cTerminator;
	}

	for ( nConnection = 0;
		nConnection < conninfo.nNumConnected; nConnection++ ) {


		if ( settings.wFileFormat == COMMA_SEPARATED ) {
			acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
			acBuffer [ strlen ( acBuffer ) ]     = '"';
		}				



		lstrcat ( acBuffer,
			conninfo.pciConnections [ nConnection ].szUnits );

		/**     Terminate Field, add the new null terminator
		***     onto the existing null terminator before overwriting
		***     with the field terminator...
		**/
		switch ( settings.wFileFormat ) {
			case ( SPACE_SEPARATED ) :
		
				/**	Pad with spaces but no terminator
				**/
				PadWithSpaces ( acBuffer, max ( 0, 15 - lstrlen (
				 conninfo.pciConnections [ nConnection ].szUnits ) ) );
				break;
			
			case ( COMMA_SEPARATED ) :
				/**	Append quotation mark...
				***	before adding field terminator
				**/
				acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
				acBuffer [ strlen ( acBuffer )     ] = '"';

				/**	Drop thru .. to add field terminator
				**/
			
			default :			
				/**	Add field terminator to all but last field.
				**/
				if ( nConnection != ( conninfo.nNumConnected - 1 ) ) {

					acBuffer [ strlen ( acBuffer ) + 1 ] = '\0';
					acBuffer [ strlen ( acBuffer )     ] 
						=  cTerminator;
				}
				break;
		}        

	}

	/**     Append EOLN terminator - carriage return\line feed
	**/
	lstrcat ( acBuffer, "\r\n" );

	wBytesSent = _lwrite ( hHeaderFile, acBuffer, strlen ( acBuffer ) );



	/**
	***     Check Last Write Operation was sucessfull
	**/
	if ( wBytesSent != strlen ( acBuffer ) ) {

		/**     Write operation failed...
		**/
		return ( ERR_FILE_WRITE );

	} else {
		return ( OK );
	}

}



VOID PadWithSpaces ( char *acBuffer, long lNumberOfSpaces )
{
	long	lIndexToCurrentTerminator;
	long    lIndexToNewTerminator;
	long    lIndex;


	lIndexToCurrentTerminator = strlen ( acBuffer );
	lIndexToNewTerminator     = lIndexToCurrentTerminator +
	                                        lNumberOfSpaces;

	/**     Add 'nNumberOfSpaces' spaces...
	**/
	for ( lIndex = lIndexToCurrentTerminator;
	                lIndex < lIndexToNewTerminator; lIndex++ ) {

		acBuffer [ lIndex ] = ' ';
	}



	/**     Terminate the padded string...
	**/
	acBuffer [ lIndexToNewTerminator ] = '\0';
}



/**	BuildLogFilename
***
***	Returns, by reference, the full file specification 
***	for the data file. This includes the _yymmddhh postfix
***
**/
static short BuildLogFilename ( char* szPathName )

{
	char	szName		[ _MAX_FNAME + 1 ];
	char    szTime   [10];

	time_t  lTime;
	struct tm*  TimeNow;

	/** ???  **/
	if ( !strcmpi ( settings.szDataFile, ".wl") ) {
		return ( ERR_INVALID_FILENAME );
	}

	/** Split the file name from the extension **/
	_splitpath ( settings.szDataFile, NULL, NULL, szName, NULL );


	/**	Create the full path name from the details
	*** stored in the 'settings' structure.
	**/
	if ( settings.bPeriodicFiles ) {

		/** Read the time **/
		time ( &lTime );
		TimeNow = localtime ( &lTime );

		/** Prepare the full string to postfix. The switch
		*** leaves 0s where required.
		**/
		strcpy ( szTime, "00000000" );

		switch ( settings.wPeriodType ) {

		case PERIOD_EVERY_HOUR :
			Int2Bytes ( szTime + 6, TimeNow->tm_hour) ;
		case PERIOD_EVERY_DAY :
			Int2Bytes ( szTime + 4, TimeNow->tm_mday) ;
		case PERIOD_EVERY_MONTH :
			Int2Bytes ( szTime + 2, (TimeNow->tm_mon + 1) );
			Int2Bytes ( szTime, TimeNow->tm_year );
		}

		/** add the _ **/
		strcat ( szName, "_" );

		/** Postfix the needed bit **/
		switch ( settings.wPostFixType ) {

		case POSTFIX_HH :
			strcat ( szName, szTime + 6 );
			break;
		case POSTFIX_DDHH :
			strcat ( szName, szTime + 4);
			break;
		case POSTFIX_MMDDHH :
			strcat ( szName, szTime + 2);
			break;
		case POSTFIX_YYMMDDHH :
			strcat ( szName, szTime );
			break;
		}

	}		
	/** Add the extension **/		
	switch (  settings.wFileFormat ) {
			
	case COMMA_SEPARATED :	
		strcat ( szName, ".WLD" );
		break;
					
	default :	/** All other current formats 
				*** use a combined data/hdr
				*** file with extension .WL
				**/
		strcat ( szName, ".WL" );	
		break;				
	}

	/** Save it in the settings.
	*** This is the name with postfixes and extension.
	**/	
	strcpy ( settings.szDataFileCurrent, szName );

	/** Add the path **/	
	sprintf ( szPathName, "%s\\%s", settings.szDataDirectory, szName );

	/** Save the full path to the file in the settings **/
	strcpy ( settings.szDataFileFull, szPathName );

	return ( OK );


}


/***************************************************/
/** This function converts the passed in nValue to a string
*** and then replaces the first 2 chars of szString with
*** the 2 lsbs of the converted value.
**/

void Int2Bytes ( char* szString, int nValue )

{
	char szTemp [10];

	itoa (nValue, szTemp, 10);

	*(szString + 1) = szTemp [ strlen(szTemp) - 1];

	if ( strlen (szTemp) > 1 ) {
		*(szString) = szTemp [ strlen(szTemp) - 2];
	}

}


/**	BuildHdrFilename
***
***	Returns, by reference, the full file specification of
***	for the Header data file.
***
**/
static short BuildHdrFilename ( char *szPathName )
{
	char	szDrive 	[ _MAX_DRIVE + 1 ];
	char	szDir   	[ _MAX_DIR   + 1 ];
	char	szExt   	[ _MAX_EXT   + 1 ];
	char	szName		[ _MAX_FNAME + 1 ];



	/**	Create the full path name from the details
	***	stored in the 'settings' structure. ????
	**/
	
	sprintf ( szPathName, "%s\\%s",
			settings.szDataDirectory, settings.szDataFile );

	

	/**	Append an appropriate file extension
	***
	**/
	_splitpath ( szPathName, szDrive, szDir, szName, szExt );
	_makepath  ( szPathName, szDrive, szDir, szName, ".WLH" );

	return ( OK );


}




void GetFileExtension ( char* szExt )

{

	switch (  settings.wFileFormat ) {
			
		case COMMA_SEPARATED :
			strcpy ( szExt, "WLD" );
			break;
					
		default :	
			/** All other current formats
			*** use a combined data/hdr
			*** file with extension .WL
			**/
				strcpy ( szExt, "WL" );
				break;
	}

}


/******************************************************/
/** This routine reads the current time and works out :
*** if bEnd is true the time at which the next periodic file should start.
*** .......... false the time at which this file started.
*** It returns the value as a time_t variable.
**/


time_t GetPeriodicFileTime ( BOOL bEnd )

{

	struct	tm	*ptmGMT;
	time_t  tTime;

	/** Convert the time to a structure **/
	ptmGMT = gmtime ( &clkRealTime.timeNow );

	/** There will be a difference between these 2 if daylight
	*** saving is in effect. Allow for it in the return value
	**/
	tTime = clkRealTime.timeNow - mktime ( ptmGMT );

	
	/** Convert the time to a structure.
	*** Must repeat mktime messes it up.
	**/
	ptmGMT = gmtime ( &clkRealTime.timeNow );

	/** These are always 0 at file change time**/
	ptmGMT->tm_sec = 0;
	ptmGMT->tm_min = 0;

	/** We can just change the hour, day, month elements
	*** The mktime function handles 25 hours etc.
	**/
	switch ( settings.wPeriodType ) {

	case PERIOD_EVERY_HOUR :
		if ( bEnd ) {
			++ptmGMT->tm_hour;
		}
		break;
	case PERIOD_EVERY_DAY :
		ptmGMT->tm_hour = 0;
		if ( bEnd ) {
			++ptmGMT->tm_mday;
		}
		break;
	case PERIOD_EVERY_MONTH :
		ptmGMT->tm_mday = 1;
		if ( bEnd ) {
			++ptmGMT->tm_mon;
		}
		break;

	}


	return ( tTime + mktime ( ptmGMT ) );

}

/******************************************************/
/** Read the last write time of the passed in file.
*** If it is greater than the start time of the about to be
*** opened periodic file then return true. Otherwise return false.
*** This is used to determine if file append is sensible.
***
*** NB We cannot simply use the _mmhhdd etc since the file may be an
*** old one which is being overwritten. This is likely to be the 
*** normal case
***
*** The cFilename must be the full path in order to find the file.
**/


BOOL IsPeriodicFileCurrent ( char* cFileName )

{
	time_t StartTime;
	struct	_finddata_t myFile;
	long   hFile;

	/** The Start of File Time **/
	StartTime = GetPeriodicFileTime ( FALSE );

	hFile = _findfirst ( cFileName, &myFile );

	if ( hFile == -1 ) {
		return (FALSE);
	}

	if ( myFile.time_write > StartTime ) {
		_findclose ( hFile );
		return ( TRUE );
	}

	_findclose ( hFile );
	return (FALSE);
	
}


/** This routine examines the string in settings.szDataFile. If it
*** contains a periodic postfix it is stripped off.
*** This function is used when the user selects a datafile which
*** contains such a postfix.
**/

void StripPeriodicPostFix ( void )

{

	int nIndex;
	char	cTemp [ _MAX_FNAME + 1 ];

	strcpy ( cTemp, settings.szDataFile );

	/** Find the period **/
	strtok ( cTemp, "." );

	/** Look for the _ . If we find a non numeric char then it
	*** can't be a periodic extension so return.
	**/

	for ( nIndex = strlen (cTemp) - 1; nIndex > 0; --nIndex ) {
		
		if ( cTemp [nIndex ] == '_' ) {
			break;
		}
		
		if (  ( cTemp [nIndex] < '0' ) || ( cTemp [nIndex] > '9' ) ) {
			return;
		}

	}

	/** This would be an all numeric filename **/
	if ( nIndex == 0 ) {
		return;
	}

	/** Replace the _ with a 0 fence **/
	settings.szDataFile [nIndex] = 0;

	/** Add the extension **/
	strcat ( settings.szDataFile, ".WL" );

}
