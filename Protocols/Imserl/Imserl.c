/**	IMSERL.C	Low level routines for IML Serial Protocol
***
***	(C) Copyright Windmill Software Ltd, 1994. All rights reserved.
***
***  This file is to be used with IMMLDRIV.C. Please read the comments
***  at the top of IMMLDRIV.C for a full explanation.  The routines in
***  this file send the IML commands over serial links to IML Microlinks
***  and retrieve the replies.
***
***  Other serial comms specific files (not including IMML... common files):
***	IMSERL.RC	resources
***	IMSERL.ICO	icon
***	IMSERL.DLG	dialog (About box)
***	IMSERL.DEF	linker def file
***	IMSERL.MAK	build file
***
***  The finished .EXE file is called IMSERL.EXE.
***
***  Version History:
***
***	Dec 93	  Stuart Grace	  First version.
**/


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define IML_INTERPRETER 	/* activate correct sections of imlinter.h */

#include "\Windmill 32\Compile Libs\imlinter.h"		/* COMMSLIB include file */

#include "\Windmill 32\Protocols\immldriv.h"
#include "imserl.h"





/**  Local prototypes
**/

static short ConfigureMicrolinkProtocol ( short nDevice );
static short ReceiveCRCMessage ( short nDevice, char *szBuffer );
static short SendCRCMessage ( short nDevice, char *szCommand,
							    WORD unFlags );
static short PerformCRCAbort ( short nDevice );

static BOOL  GetCompletePacket ( short nDevice, BYTE *szPacket );
static void  SupplyThisPacketAgain ( BYTE *szPacket, short nPort );
WORD	     GenerateCRC ( WORD unStartValue, char *szData, WORD unLength );

static short GetNextCommChar ( short nPort, char *pcChar, DWORD *pdwError );
static void  FlushReceivedChars ( short nPort );
static short TransmitBlockOfBytes ( short nDevice, char *acBuffer,
							    short nCount );




/**  Export the Windows class name and Window title to IMMLDRIV module:
**/

char *szDriverName = "IMSERL";	       /* Our prog name: export to IMMLDRIV */

char *szTitle	   = "Microlink Driver";	/* Our window title: export */

char *szIdent	   = "SERIAL 1.03";		/* Response to ~ID command  */






/**  Import the table of Microlink devices and the path of the error logging
***  file in from the IMMLDRIV module.
**/

extern IMLDEVICE   Microlink [ NUM_DEVICES ];
extern char	   szLogFile [ _MAX_PATH ];

extern HWND	   hWndMain;		/* handle of our main Window  */



/**  These globals are used by both "N" and "C" protocols:
**/

static short  nPendingDevice;			  /* Which device replying  */
static char   acRecdText [ MAX_BDP_SIZE + 20 ];   /* Reply text rcvd so far */



/**  Globals used by "N" protocol receiver:
**/

static char   *szCompBlocks;	/* ptr to end of complete blocks so far     */
static short  nRxState; 	/* What "N" recvr is doing: RXS_SLEEPING etc*/



/**  Globals used by "C" protocol receiver, including BDPs:
**/

static BYTE   aucInPkt [ MAX_PKT_SIZE ];	/* "C" packet being receivd */
static short  nInPktIndex = 0;			/* num bytes in aucInPkt    */
static short  nRecdSize;			/* num bytes in acRecdText  */

static char   acBDP [ MAX_BDP_SIZE + 20 ];	/* BDP ready for collection */
static short  nBDPErrorCode;			/* Error code for this BDP  */

#define NO_BDP_WAITING	    -998
#define PACKET_IN_BUFFER    -999




/**  This array holds the handles for the open comm ports. 
***  hComPort [ 1 ] is for COM1 etc.
**/

#define MAX_COM_PORT	8      /* Allow serial ports COM1 to COM8 max */

HANDLE	hComPort [ MAX_COM_PORT + 1 ];





/**  The next three arrays implement a mini-buffer for each COM port. The
***  packet building routine (GetCompletePacket) reads chars one at a time
***  but performing a Windows API call for every byte is inefficient
***  (especially for big BDPs). Instead they call the routine GetNextCommChar
***  in this file - this prefetches bunches of chars from the Windows driver
***  (if available) and supplies them one at a time. These arrays keep track
***  of the number of bytes obtained from Windows, number of these passed back
***  to caller, and the actual chars, for each possible COM port.
***
***  NB: The function FlushReceivedChars clears both the Windows COMM queue
***	 and the mini-buffer managed by GetNextCommChar.
**/

#define MINIBUF_SIZE	8	/* pre-fetch chars in bunches of 8 */

static short	nObtained [ MAX_COM_PORT + 1 ];
static short	nSupplied [ MAX_COM_PORT + 1 ];
static char	acFound   [ MAX_COM_PORT + 1 ][ MINIBUF_SIZE ];









/**  This open function simply initialises the COM port handles to NULL because
***  none of the ports have been opened by us yet.
***
***  Returns OK.
**/

short  OpenDriverInterface ( short  *pnHandle )
{
	short	nIndex;



	for ( nIndex = 0; nIndex < ( MAX_COM_PORT + 1 ); nIndex++ ) {

		hComPort [ nIndex ] = NULL;	/* port not yet open	   */
		nObtained   [ nIndex ] = 0;	/* no chars in mini-buffer */
	}

	*pnHandle = 0;			/* not used */

	nRxState = RXS_SLEEPING;	/* Not waiting for "N" reply */

	strcpy ( acRecdText, "" );	/* reset buffer - empty */

	szCompBlocks = acRecdText;	/* no text blocks from N protocol */

	nRecdSize = 0;			/* no bytes from C protocol */

	nBDPErrorCode = NO_BDP_WAITING;     /* no BDP found yet */

	return ( OK );
}








/**  This is notification from IMMLDRIV section that we will be dealing with
***  the IML device specified. This is our chance to initialise and prepare
***  for future commands. In the case of Serial Microlinks, if the COM port
***  specified in the Microlink structure (read from IML.INI by IMMLDRIV.C)
***  is not already opened by us, we try to open it and configure it to the
***  required baud rate, and set serial protocol to 8 data/no parity/etc.
***  If any of these steps fail, an appropriate message will be recorded
***  in the IMSERL.LOG file (if logging enabled by user) and a suitable
***  IML error code is returned.
**/

short  PrepareLinkToDevice ( short nDevice )
{
	short	nPort ;
	char	szComPort [ 10 ];
	DWORD   dwError;
	DCB	    dcbComm;
	COMMTIMEOUTS  Timeout;


	/** Check where the user has said the Microlink is connected -
	*** because you know what users can be like sometimes.
	**/

	nPort = Microlink [ nDevice ].nHardwareAddress;

	if ( ( nPort < 1 ) || ( nPort > MAX_COM_PORT ) ) {

		return ( LogAndTranslateError ( nDevice,
						ERR_COM_PORT_NUMBER ) );
	}


	/** Request the COM port used by this Microlink, unless we already
	*** control this port.
	**/

	if ( hComPort [ nPort ] != NULL ) {    /* this port is already open */

		/** Just ensure the Microlink is using the right protocol
		**/

		ConfigureMicrolinkProtocol ( nDevice );

		return ( OK );
	}


	/** Ask windows if we can have exclusive access to this COM port:
	*** First set up a string to be "COMn" - where n is the port no,
	*** then open the comms device. Check the return value which is the
	*** comms channel id. If it is INVALID then we have an error so get
	*** this logged and translated into the most appropriate generic
	*** error code. \\ needed for com port over 10
	**/


	sprintf ( szComPort, "\\\\.\\COM%d", nPort );


	hComPort [ nPort ] = CreateFile ( szComPort, GENERIC_READ | GENERIC_WRITE,
	                                  0, NULL, OPEN_EXISTING, 
									  FILE_ATTRIBUTE_NORMAL,
									  NULL );
	
	if ( hComPort[ nPort ] == INVALID_HANDLE_VALUE ) {

		/** Error prevented the port being opened - must fail
		**/
		dwError = GetLastError ();
		return ( LogAndTranslateError ( nDevice, dwError ) );
	}

	/** We have a handle to our commport. Create data buffers in the drivers **/
	SetupComm ( hComPort [nPort], RX_QUEUE_SIZE, TX_QUEUE_SIZE );

	/** Clear the buffers **/
	PurgeComm( hComPort [nPort], PURGE_TXABORT | PURGE_RXABORT |
                                      PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

      
	/** Set the read timeouts to return the data already received.
	*** If no data time out after 1 msec.
	**/
	Timeout.ReadIntervalTimeout = MAXDWORD ;
	Timeout.ReadTotalTimeoutMultiplier = MAXDWORD ;
	Timeout.ReadTotalTimeoutConstant = 1000 ;
    
    /** Don't use write timeouts **/
	Timeout.WriteTotalTimeoutMultiplier = 0 ;
	Timeout.WriteTotalTimeoutConstant = 0 ;
    SetCommTimeouts( hComPort [nPort], &Timeout ) ;

	/** Get the current state of the comms port. We can then modify this
	*** DCB to get the configuration we need.
	**/	
	if ( GetCommState ( hComPort[ nPort ], &dcbComm ) == 0 ) {		
		/** failed to get the state **/
		CloseHandle ( hComPort [ nPort ] );

		hComPort [ nPort ] = NULL;	    /* port closed */

		return ( LogAndTranslateError ( nDevice, ERR_GET_STATE ) );
	}


	/** The serial hardware protocol we use for IML is standardised
	*** as follows:
	***
	***	8 data bits, 1 stop bit, no parity, no hardware handshake
	***	no software flow control, no end-of-text char,
	***	baud rate selected by user (1200-38400)
	***
	*** This has NO FLOW CONTROL WHATSOEVER (because half-duplex RS485
	*** does not support any flow control). Thus it is the applications
	*** responsibility to ensure it never sends a block of commands
	*** one after another without any breaks or replies. The recommended
	*** solution (as used by IMSLIB and WM App Toolkit) is to insert
	*** REP commands at regular intervals and wait for the replies. This
	*** forces the app to allow time for the Microlink to catch up.
	**/
	
	dcbComm.fBinary      = TRUE;	/* set to binary mode	      */
	dcbComm.fNull	     = FALSE;	/* don't discard null chars   */
	dcbComm.fParity      = FALSE;	/* disable parity checking    */
	dcbComm.fInX	     = FALSE;	/* disable Xon/Xoff on input  */
	dcbComm.fOutX	     = FALSE;	/* disable Xon/Xoff on output */
	dcbComm.fOutxCtsFlow = FALSE;	/* disable CTS handshake      */
	dcbComm.fOutxDsrFlow = FALSE;	/* disable RTS handshake      */

	dcbComm.ByteSize     = 8;
	dcbComm.Parity	     = NOPARITY;
	dcbComm.StopBits     = ONESTOPBIT;

	switch ( Microlink [ nDevice ].nRate ) {

	  case MLINK_BAUD_1200 :  dcbComm.BaudRate = 1200;
				  break;

	  case MLINK_BAUD_2400 :  dcbComm.BaudRate = 2400;
				  break;

	  case MLINK_BAUD_4800 :  dcbComm.BaudRate = 4800;
				  break;

	  default :
	  case MLINK_BAUD_9600 :  dcbComm.BaudRate = 9600;   /* default */
				  break;

	  case MLINK_BAUD_19200 : dcbComm.BaudRate = 19200;
				  break;

	  case MLINK_BAUD_38400 : dcbComm.BaudRate = 38400;
				  break;
	}


	/** Set the new comms state**/
	if ( ! SetCommState ( hComPort [ nPort ], &dcbComm ) ) {	       /* did it fail? */

		CloseHandle ( hComPort [ nPort ] );

		hComPort [ nPort ] = NULL;	  /* port closed */

		return ( LogAndTranslateError ( nDevice, ERR_SET_STATE ) );
	}


	/** Now ensure the Microlink is using the same protocol we are...
	**/

	ConfigureMicrolinkProtocol ( nDevice );

	return ( OK );
}






/**
***
***
**/


static short  ConfigureMicrolinkProtocol ( short nDevice )
{
	short	nCode;
	DWORD	ulStartTime;
	char	szReply [ 50 ];



	if ( Microlink [ nDevice ].nExtraInfo != MLINK_CRC ) {

		return ( OK );		/* nothing needs to be done */
	}


	/** Select "C" protocol on this frame. Until this has been
	*** done, the frame will be using "N" protocol by default,
	*** so temporarily override the user's choice.
	**/

	Microlink [ nDevice ].nExtraInfo = MLINK_DEFAULT;


	/** ############### USE CORRECT CCO COMMAND HERE #####
	**/

	SendDriverCommand ( 0, nDevice, "CCO PPC PX0 QPC QX0", IML_NO_REPLY );

	Microlink [ nDevice ].nExtraInfo = MLINK_CRC;


	/** Verify that C protocol has been selected (or was already in use).
	*** We do this by sending a simple REP command using C protocol and
	*** look for a reply. If this fails or times out after half a second,
	*** the C protocol is not available.
	**/

	SendDriverCommand ( 0, nDevice, "REP", IML_TEXT_REPLY );

	ulStartTime = GetTickCount ();

	do {
		nCode = ReceiveCRCMessage ( nDevice, szReply );

		if ( ( GetTickCount () - ulStartTime ) > 500L ) {

			nCode = IML_LINK_ERROR; 	/* Timed out */
		}

	} while ( nCode == IML_NOT_READY );

	if ( nCode != OK ) {

		/** We got no recognisable response within half a second.
		*** Could be for several reasons:
		***	- Microlink not connected or powered up
		***	- Microlink using different baud rate
		***	- Microlink has an old ROM without C protocol
		***
		*** Only hope for recovery is the last of these. To allow
		*** this to work, we will use this Microlink with "N" protocol
		*** in future (since C was useless).
		**/

		/**** ######## Microlink [ nDevice ].nExtraInfo = MLINK_DEFAULT;
		****/

		return ( LogAndTranslateError ( nDevice, ERR_CCO_FAILED ) );
	}


	return ( OK );	/* C protocol working */
}






/**
***
**/

short  ProcessTildeCommand ( short nDevice, char *szCommand,
							  char *szReply )
{

	if ( strnicmp ( szCommand, "~ID", 3 ) == 0 ) {

		strcpy ( szReply, szIdent );		/* see top of file */
	} else {
		strcpy ( szReply, "Unrecognised command" );
	}

	return ( OK );
}






/**  SendDriverCommand. This goes through the following simple steps:
***	- request GPIB semaphore if free
***	- configure GPIB drivers with timeout, etc.
***	- address appropriate 3302 to LISTEN
***	- send command text, terminating with EOI
***	- UNLISTEN the 3302
***	- if no reply is expected, release GPIB semaphore; else keep it.
***
***  Returns IML_NOT_READY
***  Any GPIB error is passed to LogAndTranslateError and the resulting
***  generic error code is passed back to the caller.
**/

short  SendDriverCommand ( short nHandle, short nDevice,
					char *lpszCommand, WORD unFlags )

{
	short	nCode, nPort;

	static char  szBuffer [ MAX_COMMAND ];



	/** Which port are we going to use. If it failed to open during
	*** PrepareLinkToDevice function, we can retry now in case the
	*** problem has been cleared (eg. port was in use but now free).
	**/

	nPort = Microlink [ nDevice ].nHardwareAddress;

	if ( hComPort [ nPort ] == NULL ) {

		/** Error prevented the port being opened - have another go.
		**/

		nCode = PrepareLinkToDevice ( nDevice );

		if ( nCode != OK ) {
			return ( nCode );      /* Was logged in Prepare...*/
		}
	}


	/** Flush any rubbish out of the receive queue before we start a
	*** new command. This could be just noise, or the remains of some
	*** previous reply which was aborted. Either way, lets start with
	*** a clean slate.
	**/

	FlushReceivedChars ( nPort );


	/** If this Microlink is using "C" protocol, just pass the request
	*** through to the function which deals with this.
	**/

	if ( Microlink [ nDevice ].nExtraInfo == MLINK_CRC ) {

		nCode = SendCRCMessage ( nDevice, lpszCommand, unFlags );

	} else {

		/** The device is using the conventional "N" protocol. If
		*** this command has a binary reply, we cannot handle it
		*** unless everything is switched to the "C" protocol.
		**/

		if ( unFlags == IML_BIN_REPLY ) {

			return ( LogAndTranslateError ( nDevice,
						   ERR_WRONG_PROTOCOL ) );
		}


		/** Add start and terminating sequences for "N" protocol:
		***
		***	  * N command <CR>	where N is device ID char
		**/

		sprintf  ( szBuffer, "*%c", GetFrameIDChar ( nDevice ) );
		strcat ( szBuffer, lpszCommand );
		strcat	 ( szBuffer, "\r" );

		nCode = TransmitBlockOfBytes ( nDevice, szBuffer,
						      (short) strlen ( szBuffer ) );


		/** Is a reply going to be sent back? if so, prepare for it:
		**/

		if ( ( nCode == OK ) && ( unFlags == IML_TEXT_REPLY ) ) {

			nRxState = RXS_WAIT_START;    /* Looking for start */
		}
	}


	nPendingDevice = nDevice;     /* This device will reply (if any) */

	return ( nCode );
}






/**  SendCRCMessage
***
***
**/

static short SendCRCMessage ( short nDevice, char *szCommand,
							    WORD unFlags )
{
	BYTE		szPacket [ MAX_PKT_SIZE ];
	short		nCRC, nPort, nError, nTries;
	unsigned long	ulStartTime;



	nTries = 0;

	// Had to add this ! 
	nPort = Microlink [ nDevice ].nHardwareAddress;
	
	SendPacket:	/* GOTO LABEL */


	nTries++;

	if ( nTries > MAX_SEND_ATTEMPTS ) {

		return ( LogAndTranslateError ( nDevice, ERR_MAX_RETRIES ) );
	}


	/** Construct a complete text packet:
	***
	***   <Type> <Dest> <Size> <text...> <CRC1> <CRC2>
	***
	**/

	szPacket [ 0 ] = PKT_COMP_MESSAGE;		  /* complete msg   */
	szPacket [ 1 ] = GetFrameIDChar ( nDevice );	  /* destination    */
	szPacket [ 2 ] = (BYTE) strlen ( szCommand );   /* length of text */
	
	strcpy ( &( szPacket [ 3 ] ), szCommand );
	
	nCRC = GenerateCRC ( 0, szPacket, (short) ( szPacket [ 2 ] + 3 ) );
	
	szPacket [ ( szPacket [ 2 ] + 3 ) ] = (BYTE) ( nCRC & 0xFF );
	szPacket [ ( szPacket [ 2 ] + 4 ) ] = (BYTE) ( nCRC >> 8 );


	nError = TransmitBlockOfBytes ( nDevice, szPacket,
						    (short) ( szPacket [ 2 ] + 5 ) );

	if ( nError != OK ) {
		return ( nError );	/* failed to send bytes */
	}


	/** T2 - We now sit and wait for the ACK to come back. This has a
	***	 timeout.
	**/

	ulStartTime = GetTickCount ();

	while ( GetCompletePacket ( nDevice, szPacket ) == FALSE ) {

		if ( ( GetTickCount () - ulStartTime ) > ACK_TIMEOUT ) {

			if ( nTries == 1 ) {	/* First timeout? */

				LogAndTranslateError ( nDevice,
						       ERR_ACK_TIMEOUT );
			}

			goto SendPacket;	/* ACK timeout */
		}
	}

	/** Packet received. If it is a NAK, we must repeat the transmission
	*** because the Microlink found the command corrupted.
	**/

	if ( ( szPacket [ 0 ] == PKT_NAK ) &&
	     ( szPacket [ 1 ] == COMP_ID_CHAR ) ) {

		LogAndTranslateError ( nDevice, ERR_NAK_RECEIVED );

		goto SendPacket;	/* Microlink sent NAK to us */
	}


	/** Any other packet - treat as if ACK received. If the packet
	*** really was an ACK, then fine. If it was a text packet, then the
	*** ACK we were expecting must have got lost, because the Microlink
	*** has gone straight into sending a reply. We must keep this reply
	*** safe, until the next time the ReceiveCRCMessage function calls
	*** GetCompletePacket, when this is the packet it is looking for.
	**/

	if ( ( szPacket [ 0 ] == PKT_COMP_MESSAGE ) ||
	     ( ( szPacket [ 0 ] & 0x00F8 ) == PKT_PART_MESSAGE ) ||
	     ( ( szPacket [ 0 ] & 0x00F8 ) == PKT_END_MESSAGE ) ) {


		SupplyThisPacketAgain ( szPacket, nPort );
	}


	return ( OK );
}







/**  GetDriverReply. For GPIB this is a synchronous process - it waits in
***  this function until the complete reply is received. The method is
***  simple:
***	1 - configure GPIB drivers (which are still requested by us)
***	2 - address Microlink to TALK
***	3 - try reading reply bytes from frame and check result
***	4 - repeat step 3 as necessary.
***
***  Any GPIB error is passed to LogAndTranslateError and the resulting
***  generic error code is passed back to the caller.
**/

short	GetDriverReply ( short nHandle, short *pnDevice,
			     char *lpszBuffer, short nSize, WORD unFlags )
{

	char	szRepStart [ 8 ];
	char	*pcStart, *pcTerm;
	short	nPort, nInBuffer;
	BOOL    bRead;
	DWORD   dwBytesRead, dwError;
	COMSTAT csCommStat;

	nPort = Microlink [ *pnDevice ].nHardwareAddress;


	
	/** Clear any errors. Added this to allow for an Ethernet driver that locks
	*** if you ask for data with 0 available.
	**/
	ClearCommError ( hComPort [ nPort ], &dwError, &csCommStat );
	
	/** Give up if no bytes **/
	if ( csCommStat.cbInQue == 0 ) {
		return (0);
	}



	if ( Microlink [ *pnDevice ].nExtraInfo == MLINK_CRC ) {
		return ( ReceiveCRCMessage ( *pnDevice, lpszBuffer ) );
	}



	nInBuffer = strlen ( acRecdText );



	bRead = ReadFile ( hComPort [ nPort ],
			   &( acRecdText [ nInBuffer ] ), /** Buffer address **/
			   ( MAX_REPLY - nInBuffer ),     /** Bytes to Read  **/
			    &dwBytesRead, NULL);

	if ( bRead ) {

		dwError = 0;

		acRecdText [ nInBuffer + dwBytesRead ] = 0;	 /* end of string */

	} else {
		dwError = GetLastError ();

		if ( dwError != 0 ) {

			/** Windows serial driver has detected a real error.
			*** We must abort this reply and record the error.
			**/

			nRxState = RXS_SLEEPING;      /* Put recvr to sleep */

			strcpy ( acRecdText, "" );    /* reset buffer */

			szCompBlocks = acRecdText;

			return ( LogAndTranslateError ( *pnDevice, dwError ) );
		}
	}


	if ( dwBytesRead == 0 ) {			/* no new chars arrived */
		return ( IML_NOT_READY );	/* try again later */
	}


	/** One or more chars have just been appended to the string in
	*** acRecdText. See if this now allows us to move into the next
	*** phase of reply building.
	**/

	while ( nRxState != RXS_SLEEPING ) {	/* until we are finished */

		switch ( nRxState ) {

		  case RXS_WAIT_START:

			szRepStart [ 0 ] = GetFrameIDChar ( *pnDevice );
			szRepStart [ 1 ] = ':';
			szRepStart [ 2 ] = 0;

			pcStart = strstr ( szCompBlocks, szRepStart );

			if ( pcStart == NULL ) {     /* start seq not found */

				return ( IML_NOT_READY );
			}

			memmove ( szCompBlocks, ( pcStart + 2 ),
							strlen ( pcStart ) );

			nRxState = RXS_IN_TEXT;

			break;


		  case RXS_IN_TEXT:

			pcTerm = strchr ( acRecdText, (int) '\r' );

			if ( pcTerm == NULL ) {    /* terminator not found */

				return ( IML_NOT_READY );
			}


			if ( *( pcTerm - 1 ) == '&' ) {

				*( pcTerm - 1 ) = ' ';

				szCompBlocks = pcTerm;

				memmove ( szCompBlocks, ( pcTerm + 1 ),
							strlen ( pcTerm ) );

				nRxState = RXS_WAIT_START;

				break;
			}

			*pcTerm = 0;	/* replace CR with zero byte */

			strcpy ( lpszBuffer, acRecdText );

			nRxState = RXS_SLEEPING;      /* Put recvr to sleep */

			strcpy ( acRecdText, "" );    /* reset buffer */

			szCompBlocks = acRecdText;

			break;
		}

	} /* repeat */


	return ( OK );	     /* complete reply obtained */
}





/**
***  Returns:
***	OK		if complete message received and supplied,
***	IML_NOT_READY	if full message not yet ready,
***	other generic error codes (IML_LINK_ERROR etc) as appropriate.
**/

static short ReceiveCRCMessage ( short nDevice, char *lpszBuffer )
{
	BYTE		acResponse [ 6 ];
	short		nPreviousSeq;

	static BYTE	szPacket [ 300 ];
	static short	nSeqNumber = 0;

	

	if ( GetCompletePacket ( nDevice, szPacket ) == FALSE ) {

		return ( IML_NOT_READY );	  /* no packet ready yet */
	}

	if ( szPacket [ 1 ] != COMP_ID_CHAR ) {   /* addressed elsewhere? */
	
		return ( LogAndTranslateError ( nDevice, ERR_PKT_ADDRESS ) );
	}


	switch ( (BYTE) ( szPacket [ 0 ] & 0x00F8 ) ) {


	  case PKT_COMP_MESSAGE:	/* complete self-contained reply */
	  case PKT_PART_MESSAGE:	/* incomplete section of reply	 */
	  case PKT_END_MESSAGE: 	/* last section of reply	 */


		/** R2 - (abort) not implemented.
		*** R3 - check the CRC residue.
		**/

		if ( GenerateCRC ( 0, szPacket, (short) ( szPacket [ 2 ] + 5 ) )
								     != 0 ) {

			/** Checksum error - send NAK back to Microlink
			**/

			acResponse [ 0 ] = PKT_NAK;
			acResponse [ 1 ] = GetFrameIDChar ( nDevice );

			TransmitBlockOfBytes ( nDevice, acResponse, 2 );

			LogAndTranslateError ( nDevice, ERR_PKT_CRC );

			break;
		}


		/** Checksum is correct - send ACK back to Microlink
		**/

		acResponse [ 0 ] = PKT_ACK;
		acResponse [ 1 ] = GetFrameIDChar ( nDevice );

		TransmitBlockOfBytes ( nDevice, acResponse, 2 );



		/** R4 - check sequence number if used
		**/

		if ( ( szPacket [ 0 ] == PKT_COMP_MESSAGE ) ||
		     ( ( szPacket [ 0 ] & 0x0007 ) == nSeqNumber ) ) {


			/** R5 - This is text we must keep. Append just the
			***	 text portion onto anything already in the
			***	 received buffer.
			**/

			memcpy ( &( acRecdText [ nRecdSize ] ),
					&( szPacket [ 3 ] ), szPacket [ 2 ] );

			nRecdSize += szPacket [ 2 ];	/* num chars added */


			/** Is this the end of the message?
			**/

			if ( ( szPacket [ 0 ] == PKT_COMP_MESSAGE ) ||
			     ( ( szPacket [ 0 ] & 0x00F8 ) ==
							 PKT_END_MESSAGE ) ) {

				/** R9 - complete message received
				**/

				memcpy ( lpszBuffer, acRecdText, nRecdSize);

				lpszBuffer [ nRecdSize ] = 0;

				strcpy ( acRecdText, "" );     /* Clear buf */

				nRecdSize  = 0;     /* reset buffer */
				nSeqNumber = 0;     /* reset block seq num */

				return ( OK );
			}

			nSeqNumber++;	/* advance blk seq number */

			if ( nSeqNumber > 7 ) {
				nSeqNumber = 0; 	/* wrap from 7 to 0 */
			}

		} else {

			/** Wrong sequence number on this block. There are
			*** two distinct cases:
			***
			***  Seq num received is same as previous block
			***
			*** Our ACK for the last block must have got lost,
			*** causing the Microlink to resend the block. We
			*** just need to ACK again and continue looking
			*** for the next block. This can only apply if we
			*** have already received a block of this message.
			***
			***  Seq num received is totally out of place
			***
			*** This means a complete breakdown of the protocol
			*** with the two ends somehow getting out of step.
			*** This transfer must be abandoned.
			**/

			if ( nRecdSize > 0 ) {	    /* block been received */

				nPreviousSeq = nSeqNumber - 1;

				if ( nPreviousSeq < 0 ) {

					nPreviousSeq = 7;
				}

				if ( ( szPacket [ 0 ] & 0x0007 ) ==
							     nPreviousSeq ) {

					LogAndTranslateError ( nDevice,
							     ERR_PKT_REPEAT );

					break;

				}
			}

			/** Sequence disaster - scrap this whole transfer
			**/

			strcpy ( acRecdText, "" );     /* Clear buf */

			nRecdSize  = 0; 	/* reset buffer */
			nSeqNumber = 0; 	/* reset block seq num */

			return ( LogAndTranslateError ( nDevice,
							ERR_PKT_SEQ_NUM ) );
		}

		break;


	  default:		/* Any other type of packet - Error! */

		return ( LogAndTranslateError ( nDevice, ERR_PKT_TYPE ) );

	}

	return ( IML_NOT_READY );	/* no usable message received */
}







/**  This function will cancel a pending reply. After SendDriverCommand has
***  been called with IML_TEXT_REPLY, the normal course would be to call
***  GetDriverReply one or more times until a reply was successfully
***  obtained. This process can be aborted by calling AbortDriverReply, which
***  will reset the internal status ready to start sending a new command.
***  It also makes a call to PrepareLinkToDevice (above) which will try to
***  flush and discard any reply that is left in the Microlink 3302 buffer.
***  This function allows recovery from a failed command.
**/

short  AbortDriverTextReply ( short nDevice )
{
	short	nPort;



	nPort = Microlink [ nDevice ].nHardwareAddress;


	FlushReceivedChars ( nPort );	      /* Clear RX queue */


	if ( Microlink [ nDevice ].nExtraInfo == MLINK_CRC ) {

		PerformCRCAbort ( nDevice );
	}


	/** This is an abnormal event - log it
	**/

	LogAndTranslateError ( nDevice, ERR_REPLY_ABORT );

	return ( OK );
}







/**
***
**/

short  AbortDriverBDPReply ( short nDevice )
{
	BYTE		szPacket [ MAX_PKT_SIZE ];
	unsigned long	ulStartTime;



	ulStartTime = GetTickCount ();


	while ( GetCompletePacket ( nDevice, szPacket ) == FALSE ) {

		if ( ( GetTickCount () - ulStartTime ) > MAX_ABORT_WAIT ) {

			return ( LogAndTranslateError ( nDevice,
							ERR_CANNOT_ABORT ) );
		}
	}


	PerformCRCAbort ( nDevice );

	return ( OK );
}






/**
***  This abort function is called from IMMLDRIV.C under two circumstances:
***
***   A - the app has either encountered an error or timed out waiting
***	  for a reply (which got lost?), and has called the abort function
***	  in COMMSLIB to reset everything to known state.
***
***   B - the user has aborted the reception of a stream of BDPs
***	  (eg. Streamer's stop button).
***
***  We are not sure what exactly the Microlink is doing just now. There are
***  three main cases:
***
***   1 - Microlink is idle and listening for a command. It will respond
***	  with ABAK immediately it sees an ABORT packet. (This is likely
***	  result of case A above).
***
***   2 - Microlink is waiting for an ACK from us. Again, it is listening
***	  and will respond with ABAK immediately it sees an ABORT. (This is
***	  possible in case A above).
***
***   3 - Microlink is processing a command and constructing a reply packet.
***	  This is probably true in case B above, where it will be gathering
***	  a BDP ready for transmission. It will not notice our ABORT packet
***	  until this reply packet has been sent and it then moves into
***	  case 2: listening for ACK.
***
***
***
**/

static short PerformCRCAbort ( short nDevice )
{
	static BYTE	szPacket [ MAX_PKT_SIZE ];
	short		nError, nTries;
	unsigned long	ulStartTime;




	nTries = 0;


  SendAbort:	/* GOTO LABEL */


	nTries++;

	if ( nTries > MAX_ABORT_ATTEMPTS ) {

		return ( LogAndTranslateError ( nDevice, ERR_MAX_RETRIES ) );
	}


	/** Construct an ABORT packet:
	**/

	szPacket [ 0 ] = PKT_ABORT;
	szPacket [ 1 ] = GetFrameIDChar ( nDevice );	  /* destination */

	nError = TransmitBlockOfBytes ( nDevice, szPacket, 2 );

	if ( nError != OK ) {
		return ( nError );	/* failed to send bytes */
	}


	/** We now sit and wait for the ABAK to come back. This has a
	*** timeout.
	**/

	ulStartTime = GetTickCount ();

	while ( GetCompletePacket ( nDevice, szPacket ) == FALSE ) {

		if ( ( GetTickCount () - ulStartTime ) > ABAK_TIMEOUT ) {

			goto SendAbort; 	/* ABAK timeout */
		}
	}

	/** Packet received. If it is anything other than ABAK, we must
	*** repeat the transmission of the ABORT packet.
	**/

	if ( ( szPacket [ 0 ] != PKT_ABAK ) ||
	     ( szPacket [ 1 ] != COMP_ID_CHAR ) ) {

		goto SendAbort; 	/* acknowledgement not received */
	}

	return ( OK );		/* Abort has been acknowledged */
}






/**  GetCompletePacket (Computer Version).
***
***
***
***
**/

static BOOL  GetCompletePacket ( short nDevice, BYTE *szPacket )
{
	static unsigned long	ulLastCharArrived;	/* static */
	BYTE		cChar;
	short		nCode, nPort;
	DWORD       dwError;


	nPort = Microlink [ nDevice ].nHardwareAddress;


	/** First check if a complete packet has been pushed back into the
	*** buffer, by a previous call to SupplyThisPacketAgain. This can be
	*** detected by nInPktIndex having the special value PACKET_IN_BUFFER.
	**/

	if ( nInPktIndex == PACKET_IN_BUFFER ) {

		memcpy ( szPacket, aucInPkt, ( aucInPkt [ 2 ] + 5 ) );

		nInPktIndex = 0;	/* reset ready for next packet */

		return ( TRUE );	/* complete pkt available */
	}


	/** Look for characters coming from the serial driver...
	**/

	while ( TRUE ) {

		nCode = GetNextCommChar ( nPort, &cChar, &dwError );

		if ( nCode == GETNEXT_NONE ) {

			break;		/* No chars received */
		}

		if ( nCode == GETNEXT_ERROR ) {

			/** Windows serial driver has detected a hard error.
			*** We must abort this packet and record the error.
			**/

			LogAndTranslateError ( nDevice, dwError );

			nInPktIndex = 0;     /* Cancel current pckt */

			continue;
		}


		if ( nInPktIndex == 0 ) {      /* looking for pkt type byte */
			
			if ( strchr ( VALID_TYPE_BYTES, cChar ) != NULL ) {

				aucInPkt [ 0 ] = cChar;

				nInPktIndex = 1;
			}
			continue;
		}

		ulLastCharArrived = GetTickCount ();

		if ( nInPktIndex == 1 ) {	/* looking for dest ID byte */

			/** Incoming packets to the computer are only valid
			*** if the destination is marked as the computer.
			*** Only the computer is allowed to send packets with
			*** Microlink codes as destinations.
			**/

			if ( cChar == COMP_ID_CHAR ) {

				if ( ( aucInPkt [ 0 ] == PKT_ACK   ) ||
				     ( aucInPkt [ 0 ] == PKT_NAK   ) ||
				     ( aucInPkt [ 0 ] == PKT_ABORT ) ||
				     ( aucInPkt [ 0 ] == PKT_ABAK  ) ) {

					*szPacket	  = aucInPkt [ 0 ];
					*( szPacket + 1 ) = cChar;

					nInPktIndex = 0;    /* pkt complete */

					return ( TRUE );    /* ctrl pkt */
				}

				/** Must be some sort of text packet...
				**/

				aucInPkt [ 1 ] = cChar;
				nInPktIndex = 2;	/* look for size */

			} else {
				nInPktIndex = 0;     /* NOT a valid packet */
			}
			continue;
		}

		aucInPkt [ nInPktIndex ] = cChar;	/* append to text */
		nInPktIndex++;
		
		if ( nInPktIndex > ( aucInPkt [ 2 ] + 4 ) ) {

			memcpy ( szPacket, aucInPkt, ( aucInPkt [ 2 ] + 5 ) );

			nInPktIndex = 0;

			return ( TRUE );	/* text pkt found */
		}

	} /* look for another char */


	if ( nInPktIndex > 1 ) {     /* are we in the middle of a packet? */

		if ( ( GetTickCount () - ulLastCharArrived ) >
							  PACKET_TIMEOUT ) {
			
			/** It has been more than 400 milliseconds since we last
			*** received a character. Cancel this packet - it seems
			*** to have dried up before being completed.
			**/
			
			LogAndTranslateError ( nDevice, ERR_PKT_UNFINISHED );
			nInPktIndex = 0;
		}
	}

	return ( FALSE );	/* no complete message ready yet */
}







/**
***
**/

static void  SupplyThisPacketAgain ( BYTE *szPacket, short nPort )
{

	memcpy ( aucInPkt, szPacket, MAX_PKT_SIZE );	 /* replace into buffer */

	nInPktIndex = PACKET_IN_BUFFER; 	/* set special flag value */

	return;
}






/**
***
***
**/

static short  TransmitBlockOfBytes ( short nDevice, char *acBuffer,
							     short nCount )
{
	short	nSpaceNeeded, nPort;
	short	nFreeSpace;
	unsigned char	acPadChars [ 4 ];
	COMSTAT csCommStat;		/* will hold queue lengths   */
	DWORD   dwError, dwBytesWritten;



	nPort = Microlink [ nDevice ].nHardwareAddress;


	/** Check the type of serial link. If it is RSI (half-duplex RS485)
	*** we need to put a 0xFF byte before the message bytes.
	*** (This flushes and synchonises the autoswitching hardware of the
	*** RSI adapter). For example:
	***
	***  RS232 :	    <msg-text>
	***  RS485 :   <FF> <msg-text>
	**/

	if ( Microlink [ nDevice ].nMicrolinkType == MLINK_RSI ) {

		nSpaceNeeded = nCount + 2;

		acPadChars [ 0 ] = 0xFF;
		acPadChars [ 1 ] = 0xFF;

	} else {
		nSpaceNeeded = nCount;
	}


	/** We must check that there is room in the transmit buffer for the
	*** bytes we are about to write to the comm port. If there is
	*** insufficient space we must wait a while. Fill the COMMSTAT struct
	*** so we can see how much space there is in the transmit queue.
	**/

	do {
		ClearCommError ( hComPort [ nPort ], &dwError, &csCommStat );

		nFreeSpace = TX_QUEUE_SIZE - (short) csCommStat.cbOutQue;

	} while ( nFreeSpace < nSpaceNeeded );


	/** If RS485, start with a padding byte...
	**/

	if ( Microlink [ nDevice ].nMicrolinkType == MLINK_RSI ) {

		WriteFile ( hComPort [ nPort ], acPadChars, 2, &dwBytesWritten, NULL );
	}


	/** Now transfer the message text to the transmit queue, and check
	*** if there was any problem.
	**/

	WriteFile ( hComPort [ nPort ], acBuffer, nCount, &dwBytesWritten, NULL );
				
	if ( ((short) dwBytesWritten) != nCount ) {

		ClearCommError ( hComPort [ nPort ], &dwError, NULL );

		return ( LogAndTranslateError ( nDevice, dwError ) );
	}


	return ( OK );
}






/**
***
**/

static short GetNextCommChar ( short nPort, char *pcChar, DWORD *pdwError )
{

	DWORD dwBytesRead;


	/** Any chars left in the buffer for this port from a previous call?
	**/

	if ( nSupplied [ nPort ] < nObtained [ nPort ] ) {

		*pcChar = acFound [ nPort ][ nSupplied [ nPort ] ];

		( nSupplied [ nPort ] )++;

		return ( GETNEXT_OK );
	}


	nSupplied [ nPort ] = 0;

	ReadFile ( hComPort [ nPort ],
					 &( acFound [ nPort ][ 0 ] ),
					 MINIBUF_SIZE,
					 &dwBytesRead, NULL);

	nObtained [nPort] = (short) dwBytesRead;

	if ( nObtained [ nPort ] < 1 ) {

		/** When ReadComm returns zero or negative values
		*** you have to call GetCommError to find if there
		*** was a serial error and to clear it.
		**/

		ClearCommError ( hComPort[ nPort ], pdwError, NULL );

		if ( *pdwError == 0 ) {

		       return ( GETNEXT_NONE );
		}

		return ( GETNEXT_ERROR );
	}

	*pcChar = acFound [ nPort ][ 0 ];

	nSupplied [ nPort ] = 1;

	return ( GETNEXT_OK );
}






/**
***
**/

static void  FlushReceivedChars ( short nPort )
{
	DWORD  dwError;

	PurgeComm    ( hComPort [ nPort ], PURGE_RXCLEAR );	 /* Clear RX queue */
	ClearCommError ( hComPort [ nPort ], &dwError, NULL );	 /* Clear any error */

	nObtained [ nPort ] = 0;	/* flush our own mini-buffer */

	nRxState = RXS_SLEEPING;	      /* No reply pending */

	strcpy ( acRecdText, "" );	      /* Clear buffer	*/

	szCompBlocks = acRecdText;	/* no text blocks from N protocol */

	nRecdSize = 0;			/* no bytes from C protocol */

	return;
}





/**	High Performance CRC-16 Computation Routine
***
***  This is based on the example given in chapter 16 of the book:
***
***	C Programmer's Guide to NetBIOS, IPX and SPX
***		    by W. David Schwaderer
***	    Sams Publishing 1992.   ISBN 0672300508  
***
***  It has been modified to allow multiple calls to be chained together
***  by passing in the result from one call as the start value for the next
***  call. This allows you to calculate the overall CRC for a long message
***  which is split into several blocks. This start value MUST be zero when
***  starting a new calculation.
**/

WORD GenerateCRC ( WORD unStartValue, char *szData, WORD unLength )
{
	WORD	crc, i, index;

	static WORD crc_table [ 256 ] = {

    0x0000,  0xC0C1,  0xC181,  0x0140,	0xC301,  0x03C0,  0x0280,  0xC241,
    0xC601,  0x06C0,  0x0780,  0xC741,	0x0500,  0xC5C1,  0xC481,  0x0440,
    0xCC01,  0x0CC0,  0x0D80,  0xCD41,	0x0F00,  0xCFC1,  0xCE81,  0x0E40,
    0x0A00,  0xCAC1,  0xCB81,  0x0B40,	0xC901,  0x09C0,  0x0880,  0xC841,
    0xD801,  0x18C0,  0x1980,  0xD941,	0x1B00,  0xDBC1,  0xDA81,  0x1A40,
    0x1E00,  0xDEC1,  0xDF81,  0x1F40,	0xDD01,  0x1DC0,  0x1C80,  0xDC41,
    0x1400,  0xD4C1,  0xD581,  0x1540,	0xD701,  0x17C0,  0x1680,  0xD641,
    0xD201,  0x12C0,  0x1380,  0xD341,	0x1100,  0xD1C1,  0xD081,  0x1040,
    0xF001,  0x30C0,  0x3180,  0xF141,	0x3300,  0xF3C1,  0xF281,  0x3240,
    0x3600,  0xF6C1,  0xF781,  0x3740,	0xF501,  0x35C0,  0x3480,  0xF441,
    0x3C00,  0xFCC1,  0xFD81,  0x3D40,	0xFF01,  0x3FC0,  0x3E80,  0xFE41,
    0xFA01,  0x3AC0,  0x3B80,  0xFB41,	0x3900,  0xF9C1,  0xF881,  0x3840,
    0x2800,  0xE8C1,  0xE981,  0x2940,	0xEB01,  0x2BC0,  0x2A80,  0xEA41,
    0xEE01,  0x2EC0,  0x2F80,  0xEF41,	0x2D00,  0xEDC1,  0xEC81,  0x2C40,
    0xE401,  0x24C0,  0x2580,  0xE541,	0x2700,  0xE7C1,  0xE681,  0x2640,
    0x2200,  0xE2C1,  0xE381,  0x2340,	0xE101,  0x21C0,  0x2080,  0xE041,
    0xA001,  0x60C0,  0x6180,  0xA141,	0x6300,  0xA3C1,  0xA281,  0x6240,
    0x6600,  0xA6C1,  0xA781,  0x6740,	0xA501,  0x65C0,  0x6480,  0xA441,
    0x6C00,  0xACC1,  0xAD81,  0x6D40,	0xAF01,  0x6FC0,  0x6E80,  0xAE41,
    0xAA01,  0x6AC0,  0x6B80,  0xAB41,	0x6900,  0xA9C1,  0xA881,  0x6840,
    0x7800,  0xB8C1,  0xB981,  0x7940,	0xBB01,  0x7BC0,  0x7A80,  0xBA41,
    0xBE01,  0x7EC0,  0x7F80,  0xBF41,	0x7D00,  0xBDC1,  0xBC81,  0x7C40,
    0xB401,  0x74C0,  0x7580,  0xB541,	0x7700,  0xB7C1,  0xB681,  0x7640,
    0x7200,  0xB2C1,  0xB381,  0x7340,	0xB101,  0x71C0,  0x7080,  0xB041,
    0x5000,  0x90C1,  0x9181,  0x5140,	0x9301,  0x53C0,  0x5280,  0x9241,
    0x9601,  0x56C0,  0x5780,  0x9741,	0x5500,  0x95C1,  0x9481,  0x5440,
    0x9C01,  0x5CC0,  0x5D80,  0x9D41,	0x5F00,  0x9FC1,  0x9E81,  0x5E40,
    0x5A00,  0x9AC1,  0x9B81,  0x5B40,	0x9901,  0x59C0,  0x5880,  0x9841,
    0x8801,  0x48C0,  0x4980,  0x8941,	0x4B00,  0x8BC1,  0x8A81,  0x4A40,
    0x4E00,  0x8EC1,  0x8F81,  0x4F40,	0x8D01,  0x4DC0,  0x4C80,  0x8C41,
    0x4400,  0x84C1,  0x8581,  0x4540,	0x8701,  0x47C0,  0x4680,  0x8641,
    0x8201,  0x42C0,  0x4380,  0x8341,	0x4100,  0x81C1,  0x8081,  0x4040
  };

	crc = unStartValue;	 /* crc starts at zero for a new message */

	for (i = 0; i < unLength; i++, szData++) {

		index = ( (crc ^ *szData) & 0x00FF);
		crc = ( (crc >> 8) & 0x00FF) ^ crc_table[index];
	}

	return ( crc );
}






/**  IMMLDRIV calls this function to retrieve a BDP we have we received, or
***  at least, get the error code if it went wrong. It calls here when it
***  finds the MSG_BDP_READY message sent from BackgroundProcess.
**/

short  GetBDPFromDriver ( WPARAM wParam, long lParam, LPIMLBDP *plpbdpData )
{
	short	 nCode;
	PIMLBDP  pbdpRecd;


	*plpbdpData = (LPIMLBDP) acBDP;    /* This is the received BDP */

	nCode = nBDPErrorCode;		   /* This is associated error code */

	nBDPErrorCode = NO_BDP_WAITING;    /* This shows we have taken it */


	/** Fill in field showing where we got it from:
	**/

	pbdpRecd = (PIMLBDP) acBDP;		/* treat buffer as BDP */

	pbdpRecd->nIMLDeviceNumber = nPendingDevice;


	if ( nCode != OK ) {		/* if genuine problem with BDP */

		PerformCRCAbort ( nPendingDevice );	/* stop transfer */

		return ( nCode );		     /* report the problem */
	}


	/** Check for flags set within BDP which tell us this is the last one.
	**/

	if ( ( pbdpRecd->unBDPFlags & IML_BDP_END_DATA ) != 0 ) {
		
		return ( END_OF_DATA ); 	/* all data has been sent */
	}

	if ( ( pbdpRecd->unBDPFlags & IML_BDP_OVERFLOW ) != 0 ) {

		return ( BUFFER_OVERFLOW );	/* buffer overflowed */
	}


	return ( OK );
}





/**  BackgroundProcess.  Only called while BDPs are expected.
***
**/

void	BackgroundProcess ( void )
{
	short	 nCode;
	PIMLBDP  pbdpFound;


	if ( nBDPErrorCode != NO_BDP_WAITING ) {	/* BDP in buffer? */

		return;    /* Cannot get another BDP - one already waiting */
	}


	nCode = ReceiveCRCMessage ( nPendingDevice, acBDP );


	if ( nCode == IML_NOT_READY ) {     /* BDP not ready yet */
		return;
	}

	pbdpFound = (PIMLBDP) acBDP;

	nBDPErrorCode = nCode;		  /* code is OK or IML_LINK_ERROR */

	PostMessage ( hWndMain, MSG_BDP_READY, 0, 0L );

	return;
}







/**  CloseDriverInterface -
***
**/

short	CloseDriverInterface ( short nHandle )
{
	short	nIndex;



	for ( nIndex = 0; nIndex < ( MAX_COM_PORT + 1 ); nIndex++ ) {

		if ( hComPort [ nIndex ] != NULL ) {

			CloseHandle ( hComPort [ nIndex ] );

			hComPort [ nIndex ] = NULL;
		}
	}

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

short  LogAndTranslateError ( short nDevice, DWORD dwProtocolError )
{
	short	nGenericError, nLogLevel;
	time_t	tTimeNow;
	char	szTimeDate [ 30 ];
	char	*szMessage;
	FILE	*pfLog;



	if ( dwProtocolError == 0 ) {	/* Error code 0 means no problem */

		return ( OK );		 /* just return OK and exit */
	}


	szMessage = "";


	switch ( dwProtocolError ) {


	 case IE_BADID :		/* port not supported by Windows  */
	 case ERR_COM_PORT_NUMBER :	/* port not supported by our code */

		ErrorMessageBox ( ERR_COM_PORT_NUMBER );    /* Message Box */

		szMessage = "Non-existant COM port for";
		nGenericError = IML_CONFIG_ERROR;
		break;

	 case IE_DEFAULT :		/* default port config error(?) */

		szMessage = "Default port config error for";
		nGenericError = IML_CONFIG_ERROR;
		break;

	 case IE_MEMORY :

		szMessage = "Serial driver out of memory for";
		nGenericError = IML_FATAL_ERROR;
		break;

	 case IE_HARDWARE :		 /* port already open */
	 case IE_OPEN :

		szMessage = "Serial port is busy for";
		nGenericError = IML_FATAL_ERROR;
		break;

	 case ERR_GET_STATE :		/* GetCommState func failed */

		szMessage = "Failed to get comm state for";
		nGenericError = IML_FATAL_ERROR;
		break;

	 case ERR_SET_STATE :		/* SetCommState func failed */

		szMessage = "Failed to configure serial port for";
		nGenericError = IML_FATAL_ERROR;
		break;

	 case CE_FRAME :		 /* UART found framing error */

		szMessage = "Corrupted byte received from";
		nGenericError = IML_LINK_ERROR;
		break;

	 case CE_OVERRUN :		 /* UART receive char overrun */

		szMessage = "Baud rate too high (over-run) on";
		nGenericError = IML_LINK_ERROR;
		break;

	 case CE_RXOVER :		 /* Receive queue overflow */

		szMessage = "Receive queue overflow on";
		nGenericError = IML_LINK_ERROR;
		break;

	 case ERR_REPLY_ABORT : 	 /* App aborted a failed command */

		szMessage = "App aborted command to";
		nGenericError = OK;
		break;

	 case ERR_PKT_ADDRESS : 	 /* Packet addressed elsewhere */

		szMessage = "Packet address error from";
		nGenericError = IML_LINK_ERROR;
		break;

	 case ERR_PKT_CRC :		 /* Packet failed CRC calc */

		szMessage = "Corrupt reply received from";
		nGenericError = OK;			/* recoverable */
		break;

	 case ERR_PKT_SEQ_NUM : 	 /* Packet sequence not as expectd */

		szMessage = "Packets out of sequence from";
		nGenericError = IML_LINK_ERROR;
		break;

	 case ERR_PKT_TYPE :		 /* Recvd packet has bad type */

		szMessage = "Unexpected packet type from";
		nGenericError = IML_LINK_ERROR;
		break;

	 case ERR_MAX_RETRIES : 	 /* Repeated failure to send cmnd */

		szMessage = "Getting no response from";
		nGenericError = IML_LINK_ERROR;
		break;

	 case ERR_NAK_RECEIVED :	  /* Packet was NAKed by Microlink */

		szMessage = "Command corrupted sending to";
		nGenericError = OK;			/* recoverable */
		break;

	 case ERR_PKT_UNFINISHED :	  /* Timout during packet building */

		szMessage = "Incomplete reply packet from";
		nGenericError = OK;			/* recoverable */
		break;

	 case ERR_ACK_TIMEOUT : 	  /* Timout waiting for ACK */

		szMessage = "Command had to be repeated to";
		nGenericError = OK;			/* recoverable */
		break;

	 case ERR_WRONG_PROTOCOL :	  /* Trying to get BDPs on N protcl */

		ErrorMessageBox ( ERR_WRONG_PROTOCOL );    /* Message Box */

		szMessage = "Attempted binary transfer with N protocol from";
		nGenericError = IML_CONFIG_ERROR;
		break;

	 case ERR_CCO_FAILED :		  /* CCO PPC etc failed to work */

		szMessage = "Failed to configure Microlink at";
		nGenericError = IML_CONFIG_ERROR;
		break;

	 case ERR_CANNOT_ABORT :	   /* Timout on abort BDP link */

		szMessage = "No binary data packets arriving from";
		nGenericError = OK;			/* recoverable ?? */
		break;

	 case ERR_PKT_REPEAT :		   /* Same packet received twice */

		szMessage = "Duplicate packet received from";
		nGenericError = OK;			/* recoverable */
		break;


	 default :	/* Some miscellaneous unexpected error */

		nGenericError = IML_LINK_ERROR;
		break;
	}


	if ( nDevice < 0 ) {		/* is error logging required? */

		nLogLevel = 1;		/* general error - always log */
	} else {
		nLogLevel = Microlink [ nDevice ].nErrorLogLevel;
	}


	if ( nLogLevel > 0 ) {

		time ( &tTimeNow );

		sprintf ( szTimeDate, "%s", ctime ( &tTimeNow ) );

		szTimeDate [ 24 ] = 0;	 /* remove trailing return */

		pfLog = fopen ( szLogFile, "at" );  /* Append Text mode */

		if ( strlen ( szMessage ) > 0 ) {

			fprintf ( pfLog,
				  "%s - %s IML device %d\n",
				  szTimeDate, szMessage, nDevice );

		} else {	/* Misc error without a message */

			fprintf ( pfLog,
				  "%s - error %d on IML device %d\n",
				  szTimeDate, dwProtocolError, nDevice );
		}

		fclose ( pfLog );
	}


	return ( nGenericError );
}





/**  This utility function maps a frame or device number onto the Frame ID
***  character for that device:
***	Frames	0 to  9 	char "0" to "9"
***	Frames 10 to 35 	char "A" to "Z" (upper case)
***	Frames 36 to 61 	char "a" to "z" (lower case)
***
***  Returns the correct char.
**/

char  GetFrameIDChar ( short nFrame )
{

	if ( nFrame < 10 ) {
		return ( (char) ( '0' + nFrame ) );
	}

	if ( nFrame < 36 ) {
		return ( (char) ( 'A' + nFrame - 10 ) );
	}

	return ( (char) ( 'a' + nFrame - 36 ) );
}
