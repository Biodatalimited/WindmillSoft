
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Needed for IML error codes
#include "c:\windmill 32\handlers\kit\imconst.h"
#include "c:\windmill 32\handlers\kit\imerrors.h"

#include	"im8proto.h"
#include	"ml800lib.h"

extern HANDLE hComPort;
extern char	szLogFile [MAX_PATH];


/**  SendATCommand. 
***  The command string passed in is completed by the addition of
***  <CR>. We send the bytes and 
***  wait for the transmit buffer to empty before returning.
***  Any error is passed to LogAndTranslateError and the resulting
***  generic error code is passed back to the caller.
**/

short  FNPREFIX SendATCommand ( char *lpszCommand )

{

	DWORD	dwBytesWritten;
	DWORD   dwBytesToWrite;
	static char  szBuffer [ MAX_BUFFER ];
	COMSTAT csCommStat;		/* will hold queue lengths   */
	DWORD   dwStartTime;
	DWORD   dwError;


	/** Flush any rubbish out of the receive queue before we start a
	*** new command. This could be just noise, or the remains of some
	*** previous reply which was aborted. Either way, lets start with
	*** a clean slate.
	**/

	PurgeComm    ( hComPort, PURGE_RXCLEAR | PURGE_TXCLEAR );	 /* Clear RX and TX queue */
	ClearCommError ( hComPort, &dwError, NULL );	 /* Clear any error */

	/** Add <CR>
	**/

	
	strcpy ( szBuffer, lpszCommand );
	strcat	 ( szBuffer, "\r" );

	/** Now transfer the message text to the transmit queue, and check
	*** if there was any problem.
	**/

	dwBytesToWrite = strlen(szBuffer);

	WriteFile ( hComPort, szBuffer, dwBytesToWrite , &dwBytesWritten, NULL );
				
	if ( dwBytesWritten != dwBytesToWrite ) {

		ClearCommError ( hComPort, &dwError, NULL );

		return ( LogAndTranslateError ( GENERAL_ERROR, dwError, ERROR_COMM_TYPE ) );
	}

	/** Wait for the message to go. There can be no reply before
	*** this. There is no flow control so the message must go.
	*** The wait here makes it easier to set a timeout in 
	*** GetSerialReply. We will set a generous timeout as good practice
	*** although it should never be needed.
	**/

	/** Remember the start time **/
	dwStartTime = MillisecondCount ();

	do {
		ClearCommError ( hComPort, &dwError, &csCommStat );
		if ( (MillisecondCount () - dwStartTime) > 1000 ) {
			return ( LogAndTranslateError ( GENERAL_ERROR, ERR_TX_TIMEOUT, ERROR_INTERNAL_TYPE ) );
		}
	} while ( csCommStat.cbOutQue ) ;

	
	return ( OK  );
}

/**  GetModemReply.
***  Having sent a command to the modem we call this function to
***  get the reply. It returns a set of values that define the reply.
***  The timeout is set in seconds.
**/

short FNPREFIX	GetModemReply ( short nTimeout )
{

	char	*pcStart;
	DWORD	dwInBuffer;
	BOOL    bRead;
	DWORD   dwBytesRead, dwError;
	int		nBytesToRead;
	char	RXBuffer [81];
	DWORD	dwStartTime ;
	BOOL	bTimeLeft = 1;
	

	/** Nothing in buffer yet **/
	dwInBuffer = 0;


	dwStartTime = MillisecondCount();


	do  {

		if (  ( MillisecondCount() - dwStartTime )  > (1000 * (DWORD) nTimeout) ) {
			bTimeLeft = 0;
		}

        nBytesToRead = 80 - dwInBuffer;

		if ( nBytesToRead == 0 ) {
			return ( LogAndTranslateError ( GENERAL_ERROR, ERR_RX_OVERFLOW, ERROR_INTERNAL_TYPE ) );
		}

		bRead = ReadFile ( hComPort,
					RXBuffer + dwInBuffer ,	/** Buffer address **/
					nBytesToRead,		    /** Bytes to Read  **/
					&dwBytesRead, NULL);

		dwInBuffer = dwInBuffer + dwBytesRead;
		RXBuffer [ dwInBuffer ] = 0;	 /* end of string */

		if ( !bRead ) {


			/** Windows serial driver has detected an error.
			*** We assume that it is just line noise and try again.
			*** If we don't get a valid reply we will just time out.
			**/	
			ClearCommError ( hComPort, &dwError, NULL );	 /* Clear any error */
		}
	
		/** Convert to upper case **/
		strupr ( RXBuffer );

		/** Try and parse the string
		**/

		pcStart = strstr ( RXBuffer, "OK" );

		if ( pcStart != NULL ) {     /* OK found */
			return (REPLY_OK);
		}

		pcStart = strstr ( RXBuffer, "CONNECT" );

		if ( pcStart != NULL ) {     /* OK found */
			return (REPLY_CONNECT);
		}

		pcStart = strstr ( RXBuffer, "BUSY" );

		if ( pcStart != NULL ) {     /* OK found */
			return (REPLY_BUSY);
		}

		pcStart = strstr ( RXBuffer, "NO CARRIER" );

		if ( pcStart != NULL ) {     /* OK found */
			return (REPLY_NO_CARRIER);
		}
		
		pcStart = strstr ( RXBuffer, "NO DIALTONE" );

		if ( pcStart != NULL ) {     /* OK found */
			return (REPLY_NO_DIALTONE);
		}

		pcStart = strstr ( RXBuffer, "NO ANSWER" );

		if ( pcStart != NULL ) {     /* OK found */
			return (REPLY_NO_ANSWER);
		}

	} while ( bTimeLeft );

	

	LogAndTranslateError (GENERAL_ERROR, ERR_RX_TIMEOUT, ERROR_INTERNAL_TYPE);  //No reply

	return ( 0 );	    
}


/******************************************
*** This function causes the modem to hang up.
*** First we send delay '+' delay '+' delay '+' 
*** This returns the modem to command mode.
*** Then we send 'ATH0' which tells it to hang up
**/

short FNPREFIX HangUp ( WORD nDelay )

{
	DWORD	dwBytesWritten;
	DWORD   dwBytesToWrite;
	DWORD   dwStartTime;
	DWORD   dwError;
	char	szBuffer [10];


	/** Flush any rubbish out of the receive queue before we start a
	*** new command. This could be just noise, or the remains of some
	*** previous reply which was aborted. Either way, lets start with
	*** a clean slate.
	**/

	PurgeComm    ( hComPort, PURGE_RXCLEAR | PURGE_TXCLEAR );	 /* Clear RX and TX queue */
	ClearCommError ( hComPort, &dwError, NULL );	 /* Clear any error */


	
	strcpy ( szBuffer, "+" );

	/** We send quiet + quiet + quiet +
	*** 
	**/

	/** Wait **/
	dwStartTime = MillisecondCount ();
	while ( (MillisecondCount () - dwStartTime) < nDelay ) { }

	/** Send + **/
	dwBytesToWrite = 1;
	WriteFile ( hComPort, szBuffer, dwBytesToWrite , &dwBytesWritten, NULL );
				

	/** Wait **/
	dwStartTime = MillisecondCount ();
	while ( (MillisecondCount () - dwStartTime) < nDelay ) { }

	/** Send + **/
	dwBytesToWrite = 1;
	WriteFile ( hComPort, szBuffer, dwBytesToWrite , &dwBytesWritten, NULL );
				

	/** Wait **/
	dwStartTime = MillisecondCount ();
	while ( (MillisecondCount () - dwStartTime) < nDelay ) { }

	/** Send + **/
	dwBytesToWrite = 1;
	WriteFile ( hComPort, szBuffer, dwBytesToWrite , &dwBytesWritten, NULL );

	/** Wait **/
	dwStartTime = MillisecondCount ();
	while ( (MillisecondCount () - dwStartTime) < nDelay ) { }
	

	strcpy (szBuffer, "ATH0\r");

	/** Send + **/
	dwBytesToWrite = 5;
	WriteFile ( hComPort, szBuffer, dwBytesToWrite , &dwBytesWritten, NULL );

	/** Wait **/
	dwStartTime = MillisecondCount ();
	while ( (MillisecondCount () - dwStartTime) < 250 ) { }

	return ( OK  );	
}

