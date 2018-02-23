/**	IMSERL800.C	Low level routines for IML Serial Protocol
***
***	(C) Copyright Windmill Software Ltd, 2000
***
***
***  Version History:
***
***	2000	1.0 BGO	  First version.
*** 2001    1.1		  Increase ID timeouts to cope with slow USB serial ports
*** 2002    1.2       Add Software Version Function
*** 2003    1.3       Rearranged Timeouts to be adaptive. Long normally but
***                   shorter after error.
***                   Added new ID code process. See Text file.
***                   Normal ID method should not now be needed.
***                   Added semaphore to put app to sleep whilst waiting
***                   for data ( needed for Ethernet ports ).
***                   Arranged so that we check that the comm port has data
***                   before we ask to read. Shouldn't need to but it crashes
***                   one of the ethernet ports if we don't.
**/	


#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Needed for IML error codes
#include "\windmill 32\handlers\kit\imconst.h"
#include "\windmill 32\handlers\kit\imerrors.h"

#include	"im8proto.h"
#include	"ml800lib.h"

//Local prototype
DWORD MillisecondCount ( void );

int	DataOK ( char cTest, int* nStage );


/**  This is our com port handle**/
HANDLE	hComPort = NULL;

/**	Error Log file path and enable flag **/
char	szLogFile [MAX_PATH];
int		nLogFileEnable = 0;

#define NORMAL_TIMEOUT 5000
#define SHORT_TIMEOUT  100 


/** Read time out settings and storage **/
DWORD	dwTimeOut = NORMAL_TIMEOUT;
DWORD	dwTimeOutID;	//Initialised when coms opened
BOOL    bTimeOutExt = FALSE;
BOOL    bIDSearch = FALSE;

/** Handle for semaphore **/
HANDLE  hTimer = INVALID_HANDLE_VALUE;

/** Parsing states **/
#define RXS_WAIT_START      1           /* searching for start sequence     */
#define RXS_IN_TEXT         2           /* accumulating text - look for end */


/**  This routine opens the Com port at the passed in baud rate.
***  Errors are returned in generic IML codes. More error detail 
***  can be recorded in the Log File. 
**/

short  FNPREFIX OpenSerialLink ( int nPort, int nBaud )
{
	char	szComPort [10];
	DCB	    dcbComm;
	COMMTIMEOUTS  Timeout;

	/** Request the COM port, unless we already
	*** control this port.
	**/

	if ( hComPort != NULL ) {    /* We already have it */

		return ( OK );
	}

	/** Ask windows if we can have exclusive access to this COM port:
	*** First set up a string to be "COMn" - where n is the port no,
	*** then open the comms device. Check the return value which is the
	*** comms channel id. If it is INVALID then we have an error so get
	*** this logged and translated into the most appropriate generic
	*** error code. Needed to add the \\\\.\\ to make it work at 10 plus. 
	**/

	sprintf ( szComPort, "\\\\.\\COM%d", nPort );

	hComPort = CreateFile ( szComPort, GENERIC_READ | GENERIC_WRITE,
	                                  0, NULL, OPEN_EXISTING, 
									  FILE_ATTRIBUTE_NORMAL,
									  NULL );
	
	if ( hComPort == INVALID_HANDLE_VALUE ) {

		/** Error prevented the port being opened - must fail
		**/
		return ( LogAndTranslateError ( GENERAL_ERROR, ERR_COM_PORT_NUMBER, ERROR_INTERNAL_TYPE ) );
	}

	/** We have a handle to our commport. Create data buffers in the drivers **/
	SetupComm ( hComPort, RX_QUEUE_SIZE, TX_QUEUE_SIZE );

	/** Clear the buffers **/
	PurgeComm( hComPort , PURGE_TXABORT | PURGE_RXABORT |
                                      PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

      
	/** Set the read timeouts to return the data already received.
	*** If no data time out after 1 msec.
	**/
	Timeout.ReadIntervalTimeout = MAXDWORD ;
	Timeout.ReadTotalTimeoutMultiplier = 0;
	Timeout.ReadTotalTimeoutConstant = 1 ;
    
    /** Don't use write timeouts **/
	Timeout.WriteTotalTimeoutMultiplier = 0 ;
	Timeout.WriteTotalTimeoutConstant = 0 ;
    SetCommTimeouts( hComPort, &Timeout ) ;

	/** Get the current state of the comms port. We can then modify this
	*** DCB to get the configuration we need.
	**/	
	if ( GetCommState ( hComPort, &dcbComm ) == 0 ) {		
		/** failed to get the state **/
		CloseHandle ( hComPort );

		hComPort = NULL;	    /* port closed */

		return ( LogAndTranslateError ( GENERAL_ERROR, ERR_GET_STATE, ERROR_INTERNAL_TYPE ) );
	}


	/** The serial hardware protocol we use for IML is standardised
	*** as follows:
	***
	***	8 data bits, 1 stop bit, no parity, no hardware handshake
	***	no software flow control, no end-of-text char,
	***	baud rate selected by user (1200-19200)
	***
	*** This has NO FLOW CONTROL WHATSOEVER Thus it is the applications
	*** responsibility to ensure it never sends a block of commands
	*** one after another without any breaks or replies.
	**/
	
	dcbComm.fBinary      = TRUE;	/* set to binary mode	      */
	dcbComm.fNull	     = FALSE;	/* don't discard null chars   */
	dcbComm.fParity      = FALSE;	/* disable parity checking    */
	dcbComm.fInX	     = FALSE;	/* disable Xon/Xoff on input  */
	dcbComm.fOutX	     = FALSE;	/* disable Xon/Xoff on output */
	dcbComm.fOutxCtsFlow = FALSE;	/* disable CTS handshake      */
	dcbComm.fOutxDsrFlow = FALSE;	/* disable RTS handshake      */
	dcbComm.fAbortOnError= FALSE;	/* carry on after error		  */
	dcbComm.ByteSize     = 8;
	dcbComm.Parity	     = NOPARITY;
	dcbComm.StopBits     = ONESTOPBIT;

	switch ( nBaud ) {

	  case MLINK_BAUD_1200 :  dcbComm.BaudRate = 1200;
			dwTimeOutID = 320;	  
		    break;

	  case MLINK_BAUD_2400 :  dcbComm.BaudRate = 2400;
			dwTimeOutID = 160;	  
		    break;

	  case MLINK_BAUD_4800 :  dcbComm.BaudRate = 4800;
			dwTimeOutID = 80;
		    break;

	  default :
	  case MLINK_BAUD_9600 :  dcbComm.BaudRate = 9600;   /* default */
			dwTimeOutID = 80;	  
			break;

	  case MLINK_BAUD_19200 : dcbComm.BaudRate = 19200;
			dwTimeOutID = 80;	  
		    break;

	}


	/** Set the new comms state**/
	if ( ! SetCommState ( hComPort, &dcbComm ) ) {	       /* did it fail? */

		CloseHandle ( hComPort );

		hComPort = NULL;	  /* port closed */

		return ( LogAndTranslateError ( GENERAL_ERROR, ERR_SET_STATE, ERROR_INTERNAL_TYPE ) );
	}

	/** Create the semaphore that we use to put the app to sleep **/
    hTimer = CreateSemaphore (NULL, 0, 1, NULL );


	return ( OK );
}



/**  SendSerialCommand. 
***  The command string passed in is completed by the addition of
***  start chars eg "*29 " and a final <CR>. We send the bytes and 
***  wait for the transmit buffer to empty before returning.
***  Any error is passed to LogAndTranslateError and the resulting
***  generic error code is passed back to the caller.
**/

short  SendSerialCommand ( short nModule, char *lpszCommand )

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

	/** Add start and terminating sequences
	***
	***	  * N command <CR>	where N is device ID char
	**/

	sprintf  ( szBuffer, "*%d ", nModule );
	strcat ( szBuffer, lpszCommand );
	strcat	 ( szBuffer, "\r" );

	/** Now transfer the message text to the transmit queue, and check
	*** if there was any problem.
	**/

	dwBytesToWrite = strlen(szBuffer);

	WriteFile ( hComPort, szBuffer, dwBytesToWrite , &dwBytesWritten, NULL );
				
	if ( dwBytesWritten != dwBytesToWrite ) {

		ClearCommError ( hComPort, &dwError, NULL );

		return ( LogAndTranslateError ( nModule, dwError, ERROR_COMM_TYPE ) );
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
		if ( dwError ) {
			return ( LogAndTranslateError ( nModule, dwError, ERROR_COMM_TYPE ) );
		}		
		if ( (MillisecondCount () - dwStartTime) > 1000 ) {
			return ( LogAndTranslateError ( nModule, ERR_TX_TIMEOUT, ERROR_INTERNAL_TYPE ) );
		}
		

		/** Just wait for 5 msec. This is needed to allow Ethernet
		*** virtual comm ports to operate quickly.
		**/
		WaitForSingleObject ( hTimer, 5 );
	
	} while ( csCommStat.cbOutQue ) ;

	
	return ( OK  );
}


/**  SendSerialCommandIDList. 
***  The command string passed in is completed by the addition of
***  start chars eg "*29 " and a final <CR>. We send the bytes and 
***  wait for the transmit buffer to empty before returning.
***  Any error is passed to LogAndTranslateError and the resulting
***  generic error code is passed back to the caller.
***  In this routine unlike SendSerialCommand we :
***       do not clear the receive buffer
***       we prefix delay chars
**/

short  SendSerialCommandIDList ( short nModule, char *lpszCommand )

{

	DWORD	dwBytesWritten;
	DWORD   dwBytesToWrite;
	static char  szBuffer [ MAX_BUFFER ];
	COMSTAT csCommStat;		/* will hold queue lengths   */
	DWORD   dwStartTime;
	DWORD   dwError;
	unsigned char cDelay [] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };


	/** Don't flush the buffer we will send several of these commands
	*** before we look for a reply
	**/

	/** Add start and terminating sequences
	***
	***	  * N command <CR>	where N is device ID char
	**/

	/** Prefix some delay chars **/
	strcpy ( szBuffer, cDelay );
	strcat ( szBuffer, cDelay );
	strcat ( szBuffer, cDelay );

	sprintf  ( ( szBuffer + strlen(szBuffer) ), "*%d ", nModule );
	strcat ( szBuffer, lpszCommand );
	strcat	 ( szBuffer, "\r" );


	/** Now transfer the message text to the transmit queue, and check
	*** if there was any problem.
	**/

	dwBytesToWrite = strlen(szBuffer);

	WriteFile ( hComPort, szBuffer, dwBytesToWrite , &dwBytesWritten, NULL );
				
	if ( dwBytesWritten != dwBytesToWrite ) {

		ClearCommError ( hComPort, &dwError, NULL );

		return ( LogAndTranslateError ( nModule, dwError, ERROR_COMM_TYPE ) );
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
		if ( dwError ) {
	//		return ( LogAndTranslateError ( nModule, dwError, ERROR_COMM_TYPE ) );
		}		
		if ( (MillisecondCount () - dwStartTime) > 1000 ) {
			return ( LogAndTranslateError ( nModule, ERR_TX_TIMEOUT, ERROR_INTERNAL_TYPE ) );
		}
		

		/** Just wait for 5 msec. This is needed to allow Ethernet
		*** virtual comm ports to operate quickly.
		**/
		WaitForSingleObject ( hTimer, 5 );
	
	} while ( csCommStat.cbOutQue ) ;

	
	return ( OK  );
}


/** This routine gets a serial reply with a short timeout. This
*** is needed since non existent modules obviously don't reply
*** and real ones reply quickly. dwTimeout is also used as a flag
*** in the error log so that we don't record failed ID code searches
*** which are not an error
**/

short   GetSerialReplyID ( short nModule, char *lpszBuffer )

{
	short nError;

	if ( !bTimeOutExt ) {
		dwTimeOut = dwTimeOutID;
	}

	bIDSearch = TRUE;
	nError = GetSerialReply ( nModule, lpszBuffer );
	bIDSearch = FALSE;

	if ( !bTimeOutExt ) {
		dwTimeOut = NORMAL_TIMEOUT;
	}

	return ( nError );

}


/**  GetSerialReply.
***  This reads data from the serial port. It is placed in an 80 byte
***  buffer.We search the buffer for a correct start sequence and then 
***  a final <CR>. We extract the intervening message string and 
***  return it in the passed in buffer.
***  Any error is passed to LogAndTranslateError and the resulting
***  generic error code is passed back to the caller.
**/

short	GetSerialReply ( short nModule, char *lpszBuffer )
{

	char	szRepStart [ 8 ];
	char	*pcStart, *pcTerm;
	DWORD	dwInBuffer;
	BOOL    bRead;
	DWORD   dwBytesRead, dwError;
	int		nBytesToRead;
	char	RXBuffer [81];
	DWORD	dwStartTime ;
	int		nRxState;
	BOOL	bTimeLeft = 1;
	COMSTAT csCommStat;		/* will hold queue lengths   */
	DWORD   nIndex;

	/** Nothing in buffer yet **/
	dwInBuffer = 0;
	RXBuffer [0] = 0;


	nRxState = RXS_WAIT_START;
	dwStartTime = MillisecondCount();


	do  {

		if (  ( MillisecondCount() - dwStartTime )  > dwTimeOut ) {
			bTimeLeft = 0;
		}

		/** Wait for 5 msec. Ethernet virtual comm ports need this  **/
		WaitForSingleObject ( hTimer, 5 );


		if ( dwInBuffer == 810 ) {
			return ( LogAndTranslateError (nModule, ERR_RX_OVERFLOW, ERROR_INTERNAL_TYPE ) );
		}

		/** Call this to get the number of bytes received **/
		ClearCommError ( hComPort, &dwError, &csCommStat );

		/** Don't check for errors we might get some from an o/c line **/

		/** If we have some bytes to read */
		if ( csCommStat.cbInQue > 0 ){
			
			/** Either read to fill the buffer or just what
			*** is available.
			**/
			if ( (dwInBuffer + csCommStat.cbInQue ) > 80 ) {
				nBytesToRead = 80 - dwInBuffer;
			} else {
				nBytesToRead = csCommStat.cbInQue;
			}
			

			bRead = ReadFile ( hComPort,
						RXBuffer + dwInBuffer ,	/** Buffer address **/
						nBytesToRead,		    /** Bytes to Read  **/
						&dwBytesRead, NULL);
			
		

			dwInBuffer = dwInBuffer + dwBytesRead;
			RXBuffer [ dwInBuffer ] = 0;	 /* end of string */

			/** Reset the start time if any data comes back **/
			dwStartTime = MillisecondCount();

		}

		/** Remove any 0s caused by noise **/

		for ( nIndex = 0; nIndex < dwInBuffer; ++ nIndex ) {

			if ( RXBuffer [ nIndex ] == 0 ) {
				RXBuffer [nIndex] = (char) 255;
			}
		}
		



		/** Try and parse the string
		**/

		switch ( nRxState ) {

			case RXS_WAIT_START:	/* Look for start sequence */

				itoa ( nModule, szRepStart, 10);
				strcat	(szRepStart, ": ");

				pcStart = strstr ( RXBuffer, szRepStart );

				if ( pcStart == NULL ) {     /* start seq not found */

					break;
				}

				/** Point to start of data message **/
				pcStart = pcStart + strlen (szRepStart );

				nRxState = RXS_IN_TEXT;


			case RXS_IN_TEXT:

				pcTerm = strchr ( pcStart, (int) '\r' );

				if ( pcTerm != NULL ) {    /* terminator found */

					*pcTerm = 0;	/* replace CR with zero byte */

					strcpy ( lpszBuffer, pcStart );


					/** If the system works ok then 
					*** always restore the normal timeout.
					**/
					if ( !bIDSearch ) {
						dwTimeOut = NORMAL_TIMEOUT;
					}

					return (OK);
				}
		}

	} while ( bTimeLeft );

	/** If the system times out then drop it to a short 
	*** timeout to avoid everything locking up.
	**/
	if ( !bIDSearch ) {
		dwTimeOut = SHORT_TIMEOUT;
	}


	return ( LogAndTranslateError (nModule, ERR_RX_TIMEOUT, ERROR_INTERNAL_TYPE) );	    
}



/**  GetSerialReplyIDList.
***  This is called after we have sent out a whole string of ID requests.
***  It reads data byte by byte from the serial port until it finds a
***  carriage return. The CR is replaced by 0 and the string is returned.
***  It returns OK if it succeeds 1 if Timeout occurs. 
**/

short	GetSerialReplyIDList ( char *lpszBuffer )
{


	DWORD	dwInBuffer;
	DWORD   dwBytesRead, dwError;
	char	RXBuffer [81];
	DWORD	dwStartTime ;
	BOOL	bTimeLeft = 1;
	COMSTAT csCommStat;		/* will hold queue lengths   */
	int     nStage = 0;


	/** Nothing in buffer yet **/
	dwInBuffer = 0;
	RXBuffer [0] = 0;

	dwStartTime = MillisecondCount();


	do  {

		if (  ( MillisecondCount() - dwStartTime )  > dwTimeOut ) {
			bTimeLeft = 0;
		}

		if ( dwInBuffer == 80 ) {
			return ( 1 );
		}

		/** Call this to get the number of bytes received **/
		ClearCommError ( hComPort, &dwError, &csCommStat );

		/** Don't check for errors we might easily get some from noise when
		*** no unit is driving the lines.
		**/

		/** If we have some bytes to read */
		if ( csCommStat.cbInQue > 0 ){
						
			/** Read byte by Byte **/
			ReadFile ( hComPort,
						RXBuffer + dwInBuffer ,	/** Buffer address **/
						1,		                /** Bytes to Read  **/
						&dwBytesRead, NULL);

	
			/** At this point we reject rubbish data **/ 
			
			switch ( DataOK ( RXBuffer [dwInBuffer], &nStage ) ) {

			case 2 :
				/** Got a CR so send the message back **/
				RXBuffer [ dwInBuffer ] = 0;	 /* end of string */
				strcpy ( lpszBuffer, RXBuffer );				
				return (OK);
			case 1 :						
				/** A valid byte was received **/
				++dwInBuffer;
				/** Reset the start time **/
				dwStartTime = MillisecondCount();
				break;
			case 0 :
				/** Rubbish so throw it away **/
				dwInBuffer = 0;

			}

		} else {
			/** Wait for 5 msec. Ethernet virtual comm ports need this **/
			WaitForSingleObject ( hTimer, 5 );
		}

	} while ( bTimeLeft ) ;

	return ( 1 );	    
}

/*********************************************************
***	Reply sequences start either "x: " or "xx: " where x is the ID number of the module.
*** These may be preceeded by noise from o/c lines. We look for this sequence throwing 
*** away any unsuitable byte and starting the sequence again. nStage has 3 values
*** 0  Looking fo a number
*** 1  Looking for a number or a :
*** 2  Looking for a space
*** 3  Looking for a CR.
***    In stage 3 any byte will be accepted until CR is encountered.
**/ 

int	DataOK ( char  cTest, int* nStage )

{

	switch ( *nStage ) {


	/** Looking for a number. Goto Stage 1 if we find one **/
	case	0:
		if ( (cTest >= '0') && (cTest <= '9' ) ) {
			*nStage = 1;				
			return ( 1 );
		}
		break;
		
	/** Looking for a number or a : . Accept numbers but remain in Stage 1.
	*** Move to Stage 2 when a CR is found.
	**/
	case	1:
		if ( (cTest >= '0') && (cTest <= '9' ) ) {		
			return ( 1 );
		} else if ( cTest == ':') {
			*nStage = 2	;				
			return ( 1 );
		}
		break;

	/** Looking for a space. Goto stage 3 if one is found **/
	case    2:
		if ( cTest == ' ' ) {
			*nStage = 3;		
			return  ( 1 );
		}
		break;
	/** Looking for a CR any other data is OK **/
	case	3:
		
		if ( cTest == '\r') {
			return (2);
		}

		return (1);
	}

	/** If we get here the character is not suitable start again **/			
	*nStage = 0;		

	return (0);
}


/**  GetSerialReplyBinary.
***  This reads data from the serial port. It is placed in an 80 byte
***  buffer.We search the buffer for a correct start sequence and then 
***  a final <CR>. We extract the intervening message string and 
***  return it in the passed in buffer.
***  Any error is passed to LogAndTranslateError and the resulting
***  generic error code is passed back to the caller.
**/

short	GetSerialReplyBinary ( short nModule, int nBytes, char *lpszBuffer )
{

	char	szRepStart [ 8 ];
	char	*pcStart;
	DWORD	dwInBuffer;
	BOOL    bRead;
	DWORD   dwBytesRead, dwError;
	int		nBytesToRead;
	char	RXBuffer [81];
	DWORD	dwStartTime ;
	int		nRxState;
	int		nTotalBytes;
	int		nIndex;
	BOOL	bTimeLeft = 1;
	COMSTAT csCommStat;		/* will hold queue lengths   */


	/** Nothing in buffer yet **/
	dwInBuffer = 0;

	nRxState = RXS_WAIT_START;
	dwStartTime = MillisecondCount();

	do {
		if (  ( MillisecondCount() - dwStartTime )  > dwTimeOut ) {
			bTimeLeft = 0;
		}

		/** Wait for 5 msec. Ethernet virtual comm ports need this  **/
		WaitForSingleObject ( hTimer, 5 );

		if ( dwInBuffer == 80 ) {
			return ( LogAndTranslateError (nModule, ERR_RX_OVERFLOW, ERROR_INTERNAL_TYPE ) );
		}

		/** Call this to get the number of bytes received **/
		ClearCommError ( hComPort, &dwError, &csCommStat );

		if ( dwError ) {
			return ( LogAndTranslateError ( nModule, dwError, ERROR_COMM_TYPE ) );
		}		


		/** If we have some bytes to read */
		if ( csCommStat.cbInQue > 0 ){
			
			/** Either read to fill the buffer or just what
			*** is available.
			**/
			if ( (dwInBuffer + csCommStat.cbInQue ) > 80 ) {
				nBytesToRead = 80 - dwInBuffer;
			} else {
				nBytesToRead = csCommStat.cbInQue;
			}
			

			bRead = ReadFile ( hComPort,
						RXBuffer + dwInBuffer ,	/** Buffer address **/
						nBytesToRead,		    /** Bytes to Read  **/
						&dwBytesRead, NULL);
			

			dwInBuffer = dwInBuffer + dwBytesRead;
			RXBuffer [ dwInBuffer ] = 0;	 /* end of string */

			dwStartTime = MillisecondCount();

		}	
 
		/** Try and parse the string
		**/

		switch ( nRxState ) {

			case RXS_WAIT_START:	/* Look for start sequence */

				itoa ( nModule, szRepStart, 10);
				strcat	(szRepStart, ": ");

				pcStart = strstr ( RXBuffer, szRepStart );

				if ( pcStart == NULL ) {     /* start seq not found */

					break;
				}

				/** Put in a null fence **/
				/** Strlen RXBuffer now gies bytes to start of header **/
				pcStart [0] = 0;
				
				/** The total number of bytes needed in the RXBuffer
				*** for a valid message is the number of binary bytes
				*** plus those up to the start of the binary.
				**/
				nTotalBytes = nBytes + strlen (RXBuffer) + strlen (szRepStart);

				/** Point to start of data message **/
				pcStart = pcStart + strlen (szRepStart );

				nRxState = RXS_IN_TEXT;

			case RXS_IN_TEXT:

				if ( dwInBuffer < ( DWORD) nTotalBytes ) {
					break;
				}

				for ( nIndex = 0; nIndex < nBytes ; ++nIndex ) {

					lpszBuffer [nIndex] = pcStart [nIndex];
				}

				return (OK);
				break;
		}

	} while ( bTimeLeft );


	return ( LogAndTranslateError (nModule, ERR_RX_TIMEOUT, ERROR_INTERNAL_TYPE) );	    
}


/**  CloseDriverInterface -
***
**/

short	FNPREFIX CloseSerialLink ()
{

	CloseHandle ( hComPort );
	CloseHandle ( hTimer );
	hComPort = NULL;

	return ( OK );
}







/**  LogAndTranslateError. This takes two parameters:
***
***	nDevice 	IML device number being accessed, or -1 for general
***			 errors not specific to any one device.
***
***	nProtocolError	a GPIB error code obtained from GPIBLIB.DLL.
***
***  General errors (nDevice = -1) always get recorded in the log file with
***  the time, date & description. Errors on a specific device only get logged
***  if the Microlink.ErrorLogLevel field for that device is above zero.
***
***  The function translates the GPIB error code into the most appropriate
***  generic IML error such as IML_LINK_ERROR or IML_FATAL_ERROR. This is
***  returned, so it can be passed back to COMMSLIB and the application.
**/

short  LogAndTranslateError ( short nModule, DWORD dwProtocolError, short nType  )
{
	short	nGenericError;
	time_t	tTimeNow;
	char	szTimeDate [ 30 ];
	char	*szMessage;
	FILE	*pfLog;



	if ( dwProtocolError == 0 ) {	/* Error code 0 means no problem */

		return ( OK );		 /* just return OK and exit */
	}


	szMessage = "";


	if ( nType == ERROR_INTERNAL_TYPE ) {

		switch ( dwProtocolError ) {


		 case ERR_COM_PORT_NUMBER :	/* port not supported by our code */

			szMessage = "Cannot open COM port";
			nGenericError = CANNOT_OPEN_DEVICE;
			break;


		 case ERR_GET_STATE :		/* GetCommState func failed */

			szMessage = "Failed to get comm state";
			nGenericError = CANNOT_OPEN_DEVICE;
			break;

		 case ERR_SET_STATE :		/* SetCommState func failed */

			szMessage = "Failed to configure serial port";
			nGenericError = CANNOT_OPEN_DEVICE;
			break;

		 case ERR_CCO_FAILED :		  /* CCO PPC etc failed to work */

			szMessage = "Failed to configure Module at";
			nGenericError = MOD_SETUP_FAILED;
			break;


		 case ERR_RX_OVERFLOW :

			szMessage = "Receive queue soft overflow on";
			nGenericError = MOD_READ_FAILED;
			break;

		 case ERR_RX_TIMEOUT :

			szMessage = "Receive Time Out";
			nGenericError = MOD_READ_FAILED;
			break;

		 case ERR_TX_TIMEOUT :

			szMessage = "Transmit Time Out";
			nGenericError = MOD_WRITE_FAILED;
			break;


		 default :	/* Some miscellaneous unexpected error */

			szMessage = "Unspecified internal error";
			nGenericError = UNSPEC_HW_FAILURE;
			break;
		}

	} else {

		switch(dwProtocolError) {

		 case CE_FRAME :		 /* UART found framing error */

			szMessage = "Corrupted byte received from";
			nGenericError = MOD_READ_FAILED;
			break;

		 case CE_OVERRUN :		 /* UART receive char overrun */

			szMessage = "Baud rate too high (over-run) on";
			nGenericError = MOD_READ_FAILED;
			break;

		 case CE_RXOVER :		 /* Receive queue overflow */

			szMessage = "Receive queue overflow on";
			nGenericError = MOD_READ_FAILED;
			break;

		 default :

			szMessage = "Unspecified comm error";
			nGenericError = UNSPEC_HW_FAILURE;


		}
	}



	/** Do we need to log errors to file 
	*** We don't if this is an ID search.
	**/

	if ( nLogFileEnable &&  !bIDSearch  ) {

		time ( &tTimeNow );

		sprintf ( szTimeDate, "%s", ctime ( &tTimeNow ) );

		szTimeDate [ 24 ] = 0;	 /* remove trailing return */

		pfLog = fopen ( szLogFile, "at" );  /* Append Text mode */

		if ( nModule == -1 ) {
			
			fprintf ( pfLog,
				  "%s - %s \n",
				  szTimeDate, szMessage);

		} else {
			fprintf ( pfLog,
				  "%s - %s 800 Module %d\n",
				  szTimeDate, szMessage, nModule );
		}

		fclose ( pfLog );
	}


	return ( nGenericError );
}

/*****************************************************************
/** This function is called to pass in the details of the error log
*** file. It can be enabled by any non zero enable value.
**/

void	FNPREFIX	EnableErrorLog ( char* cFilePath, int nEnable ) 

{
	strcpy ( szLogFile, cFilePath );
	nLogFileEnable = nEnable;

}




/**************************************************
*** Routine Copied from commslib
**/


DWORD MillisecondCount ( void )

{
	LARGE_INTEGER Frequency, CurrentCount;
	double fCurrentCount, fFrequency;
	double f32bit = 0xFFFFFFFF;

	QueryPerformanceFrequency ( &Frequency );
	QueryPerformanceCounter ( &CurrentCount );
	
	/** Convert the 64 bit integers to float **/
	fFrequency = Frequency.LowPart + Frequency.HighPart * f32bit;
	fCurrentCount = CurrentCount.LowPart + CurrentCount.HighPart * f32bit;
 
	/** Convert to milliseconds **/
	fCurrentCount = (fCurrentCount * 1000) / fFrequency;

	/** Subtract any overflows. Division may give rounding errors **/
	while ( fCurrentCount > f32bit ) {
		fCurrentCount = fCurrentCount - f32bit;
	}

	/** Return a 32 bit value **/
	return ( (DWORD) fCurrentCount );

}

/*********************************************
*** Set the timeout in Milliseconds
*** 0 restores the normal settings.
*** Using this function allows us to control the timeouts
*** With this disabled. The timeout is shortened for IDs
**/

void FNPREFIX SetTimeOut ( long lTimeOut )

{

	if ( lTimeOut == 0 ) {
		dwTimeOut = NORMAL_TIMEOUT ;
		bTimeOutExt = FALSE;
	} else {
		dwTimeOut = (DWORD) lTimeOut;
		bTimeOutExt = TRUE;
	}

	return;
}

