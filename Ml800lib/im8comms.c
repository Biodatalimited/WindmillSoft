/** This DLL contains the command execution files for 800 series 
*** All these read and write files access functions of the 800
*** modules and usually return an IML error code which can be passed up to
*** the imml800 handler. 
**/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>		/* for strcpy etc. */
#include <time.h>

// Needed for error message values
#include "c:\windmill 32\handlers\kit\imerrors.h"
#include "c:\windmill 32\handlers\kit\imconst.h"

#include "im8proto.h"
#include "ml800lib.h"

/** Local prototypes **/
time_t	HexToTime ( char* szBuffer );
void	TimeToHex ( char* szHex, time_t lTime );
short	GetSerialAck (short nModule);

/** Storage for a list of ID code strings each 4 bytes long **/
char cHWType [30] [5];

extern HANDLE  hTimer;  /** Handle for semaphore **/
/***********************************************************/
/** This function returns the hardware type of the specified
*** module.
**/

short FNPREFIX ReadModuleID ( short nModule, char* cReply )

{
	char	szBuffer [MAX_BUFFER];
	char*	pcStart;
	int		nError;

	
	
	/** Prepare the command. It's of the form "DCO HT" **/
	strcpy (szBuffer, "DCO HT");

	/** Send Serial Command finishes the string formatting **/
	nError = SendSerialCommand ( nModule, szBuffer );

	if ( nError ) {
		strcpy ( cReply, "" );
		return ( nError );
	}

	/** Get the reply  **/
	nError = GetSerialReplyID ( nModule, szBuffer );

	/** Error means no reply ***/
	if ( nError ) {
		strcpy ( cReply, "" );		
		return ( nError );
	}

	/** Look for correct reply sequence **/
	pcStart = strstr ( szBuffer, "HT=" );

	if ( pcStart == NULL ) {
		strcpy ( cReply, "" );
	} else {
		strcpy (cReply, (pcStart + 3) );
	}

	return (OK);

}


/*************************************************************
*** Special routine for preparing a list of all available module ID Codes.
*** The list is only prepared when module 0 is requested. For all other 
*** modules the value stored in memory is returned.
*** To prepare the list we send out a hardware type request to each of the
*** 30 possible modules. These requests are separated by delete chars so each
*** module has a time to reply before the next module is addressed.
*** When all requests have been sent we start to read the replies which have
*** been collected in the serial input buffer. We extract the ID codes one
*** by one. When the first module is found we send off a second request for 
*** its ID. The reply to this second request will show that all the replies
*** have been processed. We should now only timeout in case of comms failure.
*** or no modules at all.
**/

short FNPREFIX ReadModuleIDList ( short nModule, char* cReply )

{
	char	szBuffer [MAX_BUFFER];
	char    szRepStart [10];
	char*	pcStart;
	int		nError;
	short   nIndex;
	int     nFirst = -1;


	/** Only module 0 causes us to actually read.
	*** Else get the data from memory.
	**/
	if ( nModule != 0 ) {
		strcpy ( cReply, cHWType [ nModule ] );
		return ( OK );
	}


	/** First clear the array of IDCodes **/
	for ( nIndex = 0; nIndex < 30; ++nIndex ) {
		cHWType [nIndex][0] = 0x00;
	}


	/** Prepare the command. It's of the form "DCO HT" **/
	strcpy (szBuffer, "DCO HT");
		
	/** Send Serial Command clears the receive buffer.
	*** Used for first request only
	**/
	nError = SendSerialCommand ( 0, szBuffer );

	if ( nError ) {
		strcpy ( cReply, "" );
		return ( OK );
	}

	for ( nIndex = 1; nIndex < 30; ++nIndex ) {
	
		/** Prepare the command. It's of the form "DCO HT" **/
		strcpy (szBuffer, "DCO HT");

		/** SendSerialCommandIDList finishes the string formatting.
		*** It also adds some delete chars for delay.
		*** It does not flush the receive buffer.
		**/
		nError = SendSerialCommandIDList ( nIndex, szBuffer );

		if ( nError ) {
			return ( OK );
		}
	}


	while ( nError == OK ) {

		/** Get the reply. Use special function.
		*** This returns full reply eg '12: HT=825A' 
		*** Will return 1 in case of error or Timeout
		**/
		nError = GetSerialReplyIDList ( szBuffer );

		/** Search downwards or we will confuse 8 and 18 etc **/
		for ( nIndex = 29; nIndex >= 0; --nIndex ) {

			itoa ( nIndex, szRepStart, 10);
			strcat	(szRepStart, ": HT=");

			/** Search the buffer **/
			pcStart = strstr  (szBuffer, szRepStart);


			/** We have found it **/
			if ( pcStart != NULL ) {
				
				/** This reply is the second getting of the
				*** first module. So all modules have been found.
				**/
				if ( nFirst == nIndex ) {
					/** The request must be for module 0 **/
					strcpy ( cReply, cHWType [0] );
					return (OK);
				}
				
				/** Point to the start of the actual type **/
				pcStart = pcStart + strlen (szRepStart);
				/** Check its not too big **/
				if ( strlen (pcStart) < 5 ) {
					strcpy ( cHWType [nIndex], pcStart );
				}

				/** If this is the first reply **/
				if ( nFirst == -1 ) {
				
					/** Delay for Radio modems **/
					//WaitForSingleObject ( hTimer, 5000 );

					/** Repeat the request for the first channel **/
					strcpy (szBuffer, "DCO HT");
					nError = SendSerialCommandIDList ( nIndex, szBuffer );
					nFirst = nIndex;

				}

				break;
			}			
		}
	}

	/** Should only get here in case of 0 modules **/
					
	/** The request must be for module 0 **/
	strcpy ( cReply, cHWType [0] );

	return (OK);

}


/***********************************************************/
/** This function returns the software version of  the specified
*** module.
**/

short FNPREFIX ReadModuleSoftVersion ( short nModule, char* cReply )

{
	char	szBuffer [MAX_BUFFER];
	char*	pcStart;
	int		nError;

	
	
	/** Prepare the command. It's of the form "DCO SV" **/
	strcpy (szBuffer, "DCO SV");

	/** Send Serial Command finishes the string formatting **/
	nError = SendSerialCommand ( nModule, szBuffer );

	if ( nError ) {
		strcpy ( cReply, "" );
		return ( nError );
	}

	/** Get the reply.**/
	nError = GetSerialReply ( nModule, szBuffer );

	/** Error means no reply ***/
	if ( nError ) {
		strcpy ( cReply, "" );		
		return ( nError );
	}

	/** Look for correct reply sequence **/
	pcStart = strstr ( szBuffer, "SV=" );

	/** This is different from the other messages since not all
	*** units are equipped with SV.
	**/

	if ( pcStart == NULL ) {
		strcpy ( cReply, "" );
	} else {
		strcpy ( cReply, (pcStart + 3) );
	}

	return (OK);

}



/*******************************************************************/
/** This function returns an integer reading from the specified module
*** and channel.
**/

short FNPREFIX ReadModuleChannel ( short nModule, short nChannel, int* ptnReading )

{
	char	szBuffer [MAX_BUFFER];
	char	szChannel [10];
	int		nError;

	/** Prepare the command. It's of the form "REA 99" **/
	strcpy (szBuffer, "REA ");

	if ( nChannel < 10 ) {
		strcat ( szBuffer, "0");
	}

	itoa ( nChannel, szChannel, 10 );

	strcat (szBuffer, szChannel );

	/** Send Serial Command finishes the string formatting **/
	nError = SendSerialCommand ( nModule, szBuffer );

	if ( nError ) {
		return ( nError );
	}

	/** Get the reply **/
	nError = GetSerialReply ( nModule, szBuffer );

	if ( nError ) {
		return ( nError );
	}

	/** Convert the reply to an integer **/
	*ptnReading = atoi ( szBuffer );

	return (OK);

}



/***************************************************/
/** Read the module time.
*** The module returns time as an 8 byte hex which we translate
*** to a long. In case of error we return -1.
**/

time_t	FNPREFIX ReadModuleTime ( short nModule )

{
	char	szBuffer [MAX_BUFFER];
	char*	pStart = NULL;

	strcpy (szBuffer, "DCO CK" );
	if ( SendSerialCommand ( nModule, szBuffer ) ) {
			return (-1);		
	}

	/** Look for the correct start sequence in the reply.
	*** If we don't find it try again. It may be an old
	*** delayed message that we have got. The correct one may follow.
	**/

	while ( pStart == NULL ) {

		/** Get the reply **/
		if ( GetSerialReply ( nModule, szBuffer ) ) {
			return (-1);
		}

		/** Reply must start with CK= **/
		pStart = strstr ( szBuffer, "CK=");
				
	}

	pStart = pStart + 3;

	/** Convert the hex to time **/
	return ( HexToTime ( pStart ) );


}

/*******************************************************/
/** Read the module epoch.
*** The module returns time as an ASCII string which we translate
*** to a long. In case of error we return -1.
**/

long	FNPREFIX ReadModuleEpoch ( short nModule )

{
	char	szBuffer [MAX_BUFFER];
	char*	pStart = NULL;
	
	/** Now read the module epoch **/
	strcpy (szBuffer, "DCO CE" );
	if ( SendSerialCommand ( nModule, szBuffer ) ) {
			return (-1);		
	}

	/** Look for the correct start sequence in the reply.
	*** If we don't find it try again. It may be an old
	*** delayed message that we have got. The correct one may follow.
	**/

	while ( pStart == NULL ) {

		/** Get the reply **/
		if ( GetSerialReply ( nModule, szBuffer ) ) {
			return (-1);
		}

		/** Reply must start with CE= **/
		pStart = strstr ( szBuffer, "CE=");
			
	}

	pStart = pStart + 3;

	return ( atoi ( pStart ) ) ;

}

/*******************************************************/
/** Read the module epoch.
*** The module returns time as an ASCII string which we translate
*** to a long. In case of error we return -1.
**/


long	FNPREFIX ReadModuleEpochNumber ( short nModule )

{
	char	szBuffer [MAX_BUFFER];
	char*	pStart = NULL;
	
	/** Now read the module epoch number**/
	strcpy (szBuffer, "DCO EP" );
	if ( SendSerialCommand ( nModule, szBuffer ) ) {
			return (-1);		
	}

	/** Look for the correct start sequence in the reply.
	*** If we don't find it try again. It may be an old
	*** delayed message that we have got. The correct one may follow.
	**/

	while ( pStart == NULL ) {

		/** Get the reply **/
		if ( GetSerialReply ( nModule, szBuffer ) ) {
			return (-1);
		}

		/** Reply must start with EP= **/
		pStart = strstr ( szBuffer, "EP=");
			
	}

	pStart = pStart + 3;

	return ( atoi ( pStart ) ) ;

}

/************************************************************/
/** Read the module next time parameter.
*** The module returns this as an ASCII string which we translate
*** to a long. In case of error we return -1.
**/

long	FNPREFIX ReadModuleNextTime ( short nModule )

{
	char	szBuffer [MAX_BUFFER];
	char*	pStart = NULL;
	
	/** Now read the module epoch **/
	strcpy (szBuffer, "DCO CN" );
	if ( SendSerialCommand ( nModule, szBuffer ) ) {
		return (-1);
	}

	/** Look for the correct start sequence in the reply.
	*** If we don't find it try again. It may be an old
	*** delayed message that we have got. The correct one may follow.
	**/

	while ( pStart == NULL ) {

		/** Get the reply **/
		if ( GetSerialReply ( nModule, szBuffer ) ) {
			return (-1);
		}

		/** Reply must start with CN= **/
		pStart = strstr ( szBuffer, "CN=");
			
	}

	pStart = pStart + 3;

	return ( atoi ( pStart ) ) ;

}
/*******************************************************/
/** Read the module ON time.
*** The module returns time as an ASCII string which we translate
*** to a long. In case of error we return -1.
**/

long	FNPREFIX ReadModuleTON ( short nModule )

{
	char	szBuffer [MAX_BUFFER];
	char*	pStart = NULL;
	
	/** Now read the on epoch **/
	strcpy (szBuffer, "DCO TO" );
	if ( SendSerialCommand ( nModule, szBuffer ) ) {
			return (-1);		
	}

	/** Look for the correct start sequence in the reply.
	*** If we don't find it try again. It may be an old
	*** delayed message that we have got. The correct one may follow.
	**/

	while ( pStart == NULL ) {

		/** Get the reply **/
		if ( GetSerialReply ( nModule, szBuffer ) ) {
			return (-1);
		}

		/** Reply must start with TO= **/
		pStart = strstr ( szBuffer, "TO=");
	}

	pStart = pStart + 3;

	return ( atoi ( pStart ) ) ;

}

/*******************************************************/
/** Read the module OFF Time.
*** The module returns time as an ASCII string which we translate
*** to a long. In case of error we return -1.
**/

long	FNPREFIX ReadModuleTOFF ( short nModule )

{
	char	szBuffer [MAX_BUFFER];
	char*	pStart = NULL;
	
	/** Now read the module off time **/
	strcpy (szBuffer, "DCO TF" );
	if ( SendSerialCommand ( nModule, szBuffer ) ) {
			return (-1);		
	}

	/** Look for the correct start sequence in the reply.
	*** If we don't find it try again. It may be an old
	*** delayed message that we have got. The correct one may follow.
	**/

	while ( pStart == NULL ) {

		/** Get the reply **/
		if ( GetSerialReply ( nModule, szBuffer ) ) {
			return (-1);
		}

	/** Reply must start with TF= **/
	pStart = strstr ( szBuffer, "TF=");
			
	}

	pStart = pStart + 3;

	return ( atoi ( pStart ) ) ;

}

/*****************************************************/
/** Reads nBytes of binary data for nEpoch into the dataArray.
*** Returns an IML style error.
**/

short	FNPREFIX ReadModuleEpochData ( short nModule, short nEpoch, short nBytes, char* pcDataArray )


{
	char	szBuffer [MAX_BUFFER];
	char	szTemp  [20];
	short	nError;
	
	/** Create the request string **/
	strcpy (szBuffer, "EPO " );
	itoa ( nEpoch, szTemp , 10 );
	strcat (szBuffer, szTemp);
	
	nError = SendSerialCommand ( nModule, szBuffer );
	if ( nError ) {
			return (nError);		
	}

	return ( GetSerialReplyBinary ( nModule, nBytes, pcDataArray ) );

}

/*******************************************************************/
/** This function writes an integer reading into the specified module
*** and channel.
**/

short FNPREFIX WriteModuleChannel ( short nModule, short nChannel, int nSetting )

{
	char	szBuffer [MAX_BUFFER];
	char	szTemp [10];
	int		nError;

	/** Prepare the command. It's of the form "WRI 99 xxxxx" **/
	strcpy (szBuffer, "WRI ");

	if ( nChannel < 10 ) {
		strcat ( szBuffer, "0");
	}

	itoa ( nChannel, szTemp, 10 );

	strcat (szBuffer, szTemp );

	strcat (szBuffer, " " );

	itoa (nSetting, szTemp, 10 );

	strcat ( szBuffer, szTemp );


	/** Send Serial Command finishes the string formatting **/
	nError = SendSerialCommand ( nModule, szBuffer );

	if ( nError ) {
		return ( nError );
	}

	return ( GetSerialAck ( nModule ) );

}

/*******************************************************************/
/** This function returns an integer reading from the specified module
*** and channel.
**/

short FNPREFIX WriteModuleChannelRange ( short nModule, short nChannel, int nSetting )

{
	char	szBuffer [MAX_BUFFER];
	char	szTemp [10];
	int		nError;

	/** Prepare the command. It's of the form "DEF 99 R=xx" **/
	strcpy (szBuffer, "DEF ");

	if ( nChannel < 10 ) {
		strcat ( szBuffer, "0");
	}

	itoa ( nChannel, szTemp, 10 );

	strcat (szBuffer, szTemp );

	strcat (szBuffer, " R=" );

	itoa (nSetting, szTemp, 10 );

	strcat ( szBuffer, szTemp );


	/** Send Serial Command finishes the string formatting **/
	nError = SendSerialCommand ( nModule, szBuffer );

	if ( nError ) {
		return ( nError );
	}

	return ( GetSerialAck ( nModule ) );

}

/*******************************************************/
/** This fuction writes a new time into the module clock.
*** The time is passed as a long which we convert to hex.
*** It returns an IML style error.
**/

short FNPREFIX WriteModuleTime ( short nModule, time_t lTime )

{
	char szBuffer[MAX_BUFFER];
	char szTemp [10];
	int  nError;

	/** Now write the command **/
	strcpy (szBuffer, "CCO CK=" );
	TimeToHex ( szTemp, lTime );

	strcat(szBuffer, szTemp);
	nError = SendSerialCommand ( nModule, szBuffer );
	if (nError ) {
		return (nError);
	}

	return ( GetSerialAck ( nModule ) );

}

/*******************************************************/
/** This fuction writes a new next time.
*** We pass the time and the epoch length. It calculates the setting.
*** It returns an IML style error.
**/
short FNPREFIX WriteModuleNextTime ( short nModule, time_t lTime, long lEpoch )

{
	char szBuffer[MAX_BUFFER];
	char szTemp [30];
	long lTemp;
	int nError;

	/** Now write the command **/
	strcpy (szBuffer, "CCO CN=" );
	
	/** Calculate the time to end of epoch **/
	lTemp = lEpoch - ( lTime % lEpoch );
	itoa ( lTemp, szTemp, 10 );

	strcat (szBuffer, szTemp);
	nError = SendSerialCommand ( nModule, szBuffer );
	if (nError) {
		return (nError);
	}

	return (GetSerialAck ( nModule ));

}

/*******************************************************/
/** This function writes a new epoch length.
*** We pass the epoch length as a long.
*** It returns an IML style error.
**/

short FNPREFIX WriteModuleEpoch ( short nModule, long lEpoch )

{
	char szBuffer[MAX_BUFFER];
	char szTemp [30];
	int nError;

	/** Now write the command **/
	strcpy (szBuffer, "CCO CE=" );
	
	itoa ( lEpoch, szTemp, 10 );

	strcat (szBuffer, szTemp);
	nError = SendSerialCommand ( nModule, szBuffer );
	if (nError) {
		return (nError);
	}

	return (GetSerialAck ( nModule ));

}
/*******************************************************/
/** This function writes a new ON time length.
*** We pass the ON time length as a long.
*** It returns an IML style error.
**/


short FNPREFIX WriteModuleTON ( short nModule, long lTON )

{
	char szBuffer[MAX_BUFFER];
	char szTemp [30];
	int nError;

	/** Now write the command **/
	strcpy (szBuffer, "CCO TO=" );
	
	itoa ( lTON, szTemp, 10 );

	strcat (szBuffer, szTemp);
	nError = SendSerialCommand ( nModule, szBuffer );
	if (nError) {
		return (nError);
	}

	return (GetSerialAck ( nModule ));

}
/*******************************************************/
/** This function writes a new OFF time length.
*** We pass the OFF time length as a long.
*** It returns an IML style error.
**/


short FNPREFIX WriteModuleTOFF ( short nModule, long lTOFF )

{
	char szBuffer[MAX_BUFFER];
	char szTemp [30];
	int nError;

	/** Now write the command **/
	strcpy (szBuffer, "CCO TF=" );
	
	itoa ( lTOFF, szTemp, 10 );

	strcat (szBuffer, szTemp);
	nError = SendSerialCommand ( nModule, szBuffer );
	if (nError) {
		return (nError);
	}

	return (GetSerialAck ( nModule ));

}

/*******************************************************/
/** This function changes the module number.
*** We pass the new number as a short.
*** It returns an IML style error.
**/

short FNPREFIX WriteModuleNumber ( short nModule, short nNumber  )

{
	char szBuffer[MAX_BUFFER];
	char szTemp [30];
	int nError;

	/** Now write the command **/
	strcpy (szBuffer, "CCO FI=" );
	
	itoa ( nNumber, szTemp, 10 );

	strcat (szBuffer, szTemp);
	nError = SendSerialCommand ( nModule, szBuffer );
	if (nError) {
		return (nError);
	}

	/* NB the module number has changed */
	return (GetSerialAck ( nNumber ));

}

/*******************************************************/
/** This function changes the baud rate.
*** We pass the rate as the number eg 9600.
*** The ack is returned at the old rate
*** It returns an IML style error.
**/

short FNPREFIX WriteModuleBaudRate ( short nModule, short nBaud  )

{
	char szBuffer[MAX_BUFFER];
	char szTemp [30];
	int nError;

	/** Now write the command **/
	strcpy (szBuffer, "CCO PB=" );
	
	itoa ( nBaud, szTemp, 10 );

	strcat (szBuffer, szTemp);
	nError = SendSerialCommand ( nModule, szBuffer );
	if (nError) {
		return (nError);
	}

	/* NB the module number has changed */
	return (GetSerialAck ( nModule ));

}


short	GetSerialAck (short nModule)

{
	char szBuffer[MAX_BUFFER];
	int nError;


	/** Get the reply **/
	nError = GetSerialReply ( nModule, szBuffer );
	if (nError) {
		return (nError);
	}

	if (szBuffer[0] != '*') {
		return (MOD_SETUP_FAILED);
	}

	return (OK);

}

/** Convert the 8 byte hex time into a long **/

long	HexToTime ( char* szBuffer ) {

	int		nIndex;
	time_t	lTime = 0;

	/** Must be 8 bytes long **/
	if ( strlen (szBuffer) != 8 ) {
		return (0);
	}

	/** 0 to 9 and A to F are the only possible values **/
	for ( nIndex = 0 ; nIndex < 8; nIndex ++ ) {

		lTime = lTime * 16;

		switch ( szBuffer[nIndex] ) {

		case '0' :
		case '1' :
		case '2' :
		case '3' :
		case '4' :
		case '5' :
		case '6' :
		case '7' :
		case '8' :
		case '9' :
			lTime = lTime + szBuffer [nIndex] - '0';
			break;
		case 'A' :
		case 'B' :
		case 'C' :
		case 'D' :
		case 'E' :
		case 'F' :
			lTime = lTime + szBuffer [nIndex] - 'A' + 10;
			break;
		default  :
			return ( 0 );
		}
		
		
	}

	return (lTime);
}

/** Convert a Time to an 8 byte hex string **/

void	TimeToHex ( char* szHex, time_t lTime )

{
	int		nIndex;
	long	lTemp;

	for ( nIndex = 0; nIndex < 8; ++nIndex ) {

		/** Mask off LSBs **/
		lTemp = lTime & 0x0000000f;

		/** Convert to Hex and store **/
		if (lTemp < 10 ) {
			szHex [7-nIndex] = (char) ( lTemp + '0' );
		} else {
			szHex [7-nIndex] = (char) ( lTemp - 10 + 'A' );
		}

		/** Shift to next nibble **/
		lTime = lTime >> 4;
	}

	/** Add 0 fence **/
	szHex [8] = 0;
}

/****************************************************/
/** This returns the current computer time in time_t format.
**/

long FNPREFIX  GetLongTime ( void ) 

{
	time_t	lTime;

	time ( &lTime );

	return ( lTime );
	
}

/****************************************************/
/** This converts the passed in lTime to a Time and Date String
**/

void FNPREFIX LongTimeToString ( long lTime, char* cBuffer )

{
	time_t lTemp;

	lTemp = lTime;

	strcpy ( cBuffer, ctime ( &lTemp ) );
}
