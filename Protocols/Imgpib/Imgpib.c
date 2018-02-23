/**	IMGPIB.C	Low level routines for GPIB protocol for IML
***
***	(C) Copyright Windmill Software Ltd, 1991-93. All rights reserved.
***
***  This file is to be used with IMMLDRIV.C. Please read the comments
***  at the top of IMMLDRIV.C for a full explanation.  The routines in
***  this file send the IML commands over GPIB to a Microlink 3302 module
***  and retrieve the replies via GPIB.
***
***  Other GPIB-specific files (not including IMML... common files):
***	IMGPIB.RC	resources
***	IMGPIB.ICO	icon
***	IMGPIB.DLG	dialog (About box)
***	IMGPIB.DEF	linker def file
***	IMGPIB.MAK	build file
***
***  The finished .EXE file is called IMGPIB.EXE.
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
#include "\Windmill 32\Compile Libs\gpiblib.h"

#define IML_INTERPRETER 	/* activate correct sections of imlinter.h */
#define DLL_CALLING

#include "\Windmill 32\Compile Libs\imlinter.h"		/* for BDP structure */
#include "\Windmill 32\Compile Libs\gpiblib.h"

#include "\Windmill 32\Protocols\immldriv.h"




/**  Prototype of local function
**/

short	LogAndTranslateError ( short nDevice, short nProtocolError );





/**  Export the Windows class name and Window title to IMMLDRIV module:
**/

char *szDriverName = "IMGPIB";	       /* Our prog name: export to IMMLDRIV */

char *szTitle	   = "Microlink Driver";   /* Our window title: export also */

char *szIdent	   = "GPIB 5.00";	   /* Response to ~ID command	    */




/**  Import the table of Microlink devices and the path of the error logging
***  file in from the IMMLDRIV module.
**/

extern IMLDEVICE   Microlink [ NUM_DEVICES ];
extern char	   szLogFile [ _MAX_PATH ];




short	nGPIBHandle;		/* Our GPIBLIB user handle		   */
short	bBusy = FALSE;		/* TRUE when we are holding GPIB semaphore */






/**  This open function simply tries to open the GPIBLIB.DLL and get a user
***  handle for it. If this fails, the error will be reported in the log
***  file.
***
***  Returns OK or generic translation of fatal error.
**/

short  OpenDriverInterface ( short *pnHandle )
{

	nGPIBHandle = GPIBOpen ();

	if ( nGPIBHandle == GPIB_NOT_OPEN ) {

		return ( LogAndTranslateError ( -1, GPIB_NOT_OPEN ) );
	}

	return ( OK );
}








/**  This is notification from IMMLDRIV section that we will be dealing with
***  the IML device specified. This is our chance to initialise and prepare
***  for future commands. In the case of GPIB Microlinks, we set a very short
***  timeout, then try to read any reply out of the frame. This should fail
***  since there should not be any reply left hanging around, but if some
***  freak accident left a half-digested reply stuck in the buffer (such as
***  rebooting the PC while Windmill is mid-cycle), this will clear it out
***  to ensure a reliable clean start.
***
***  If the GPIB is busy, then we just skip the process and return
***  IML_NOT_READY. It simply means that this extra safety precaution was
***  not possible when requested. The caller should not worry about this.
**/

short  PrepareLinkToDevice ( short nDevice )
{
	short		nRead;
	unsigned char	byBuf [ 25 ];



	if ( bBusy ) {
		return ( IML_NOT_READY );
	}


	/** Request the GPIB bus. If fail then return busy.
	**/

	if ( ! GPIBRequest ( nGPIBHandle ) ) {

		return ( IML_NOT_READY );
	}

	/** Set up the command bytes to configure the GPIB bus with a very
	*** short timeout, and address the appropriate Microlink to talk.
	**/

	byBuf [ 0 ] = TOSET;	/* set timeout to ...		  */
	byBuf [ 1 ] = 5;	/*   5 * 5 millisec		  */
	byBuf [ 2 ] = EXTOEN;	/* extended timeout of 25 ms.	  */
	byBuf [ 3 ] = UNL;	/* ensure nobody else is listening   */

    byBuf [ 4 ] = (char)( TAD + Microlink [ nDevice ].nHardwareAddress );


	GPIBWriteCom ( byBuf, 5 );

	/** Now try to read any old reply from the frame. This will either fail
	*** immediately if the reply is not ready, or we will read some data.
	*** If we get some data then we must continue reading until we get an
	*** EOI and/or Term Char which indicates the end of the reply.
	**/
	
	do {

		nRead = GPIBReadDat ( byBuf, 20 );

	} while ( nRead > 0 );


	GPIBSendCom ( UNT );

	GPIBRelease ();

	return ( OK );
}






/**
***
**/

short  ProcessTildeCommand ( short nDevice, char *szCommand,
							  char *szReply )
{

	if ( strnicmp ( szCommand, "~ID", 3 ) == 0 ) {

		strcpy ( szReply, szIdent );	/* see top of file */
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
	unsigned char byBuf [ 10 ];
	short	nCompCode;
	short	nCommandLength;


	if ( bBusy ) {
		return ( IML_NOT_READY );
	}


	/** Request the GPIB bus. If fail then return busy.
	**/

	if ( ! GPIBRequest ( nGPIBHandle ) ) {

		return ( IML_NOT_READY );
	}

	bBusy = TRUE;


	/** Set up the command bytes we need to configure the GPIB bus as we
	*** need it. I.e. timeout 0.6 sec, EOI on last byte, interrupts on.
	*** Also address the appropriate Microlink to listen.
	**/

	byBuf [ 0 ] = TOSET;	/* set timeout to ...		*/
	byBuf [ 1 ] = 25;	/*   25 * 25 millisec		*/
	byBuf [ 2 ] = EXTOEN;	/* extended timeout of 625 ms.	*/
	byBuf [ 3 ] = EOIEN;	/* send EOI signal at end of message */
	byBuf [ 4 ] = UNT;	/* ensure nobody else is talking     */

	byBuf [ 5 ] = (char)( LAD + Microlink [ nDevice ].nHardwareAddress );
	

	if ( GPIBWriteCom ( byBuf, 6 ) != 6 ) {

		/** failed to write commands so find error code & return
		**/

		nCompCode = GPIBCompCode ();

		GPIBRelease ();
		bBusy = FALSE;

		return ( LogAndTranslateError ( nDevice, nCompCode ) );
	}

	/** OK we can now send the IML command message.
	**/

	nCommandLength = strlen ( lpszCommand );

	if ( GPIBWriteDat ( lpszCommand, nCommandLength ) != nCommandLength ){
			
		/** We failed to write the whole message so return GPIB hardware
		*** error as either the frame will not accept the bytes or there
		*** is no frame.
		**/

		nCompCode = GPIBCompCode ();

		GPIBSendCom ( UNL );		/* unlisten the frame */

		GPIBRelease ();
		bBusy = FALSE;

		return ( LogAndTranslateError ( nDevice, nCompCode ) );
	}

	/** We have sent the command so unaddress the frame.
	**/

	GPIBSendCom ( UNL );


	/** If there is no reply then we can release the GPIB bus.
	**/


	if ( unFlags == IML_NO_REPLY ) {

		bBusy = FALSE;
		GPIBRelease ();
	}

	return ( 0 );
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

short	GetDriverReply ( short nHandle, short *nDevice,
			     char *lpszBuffer, short nSize, WORD unFlags )
{
	unsigned char  byBuf [ 10 ];
	short	nRead;
	short	nCompCode;
	short	nReadSoFar = 0;
	char * lpszTerm;




	/** Set up the command bytes we need to configure the GPIB bus as we
	*** need it. I.e. timeout 1 sec, enable Term char.
	*** Also address the appropriate Microlink to talk.
	**/

	byBuf [ 0 ] = TOSET;	/* set timeout to ...		  */
	byBuf [ 1 ] = 32;	/*   32 * 32 millisec		  */
	byBuf [ 2 ] = EXTOEN;	/* extended timeout of 1024 ms.   */
	byBuf [ 3 ] = TCSET;	/* set terminating char detection */
	byBuf [ 4 ] = 13;	/* stop on Carriage return.	  */
	byBuf [ 5 ] = TCEN;
	byBuf [ 6 ] = UNL;	/* ensure nobody else is listening   */

	byBuf [ 7 ] = (char)( TAD + Microlink [ *nDevice ].nHardwareAddress );


	if ( GPIBWriteCom ( byBuf, 8 ) != 8 ) {

		/** failed to write commands so get error code & return.
		**/

		nCompCode = GPIBCompCode ();

		GPIBRelease ();
		bBusy = FALSE;

		return ( LogAndTranslateError ( *nDevice, nCompCode ) );
	}


	/** OK Now try to read the reply from the frame. This will either fail
	*** immediately if the reply is not ready, or we will read some data.
	*** If we get some data then we must continue reading until we get an
	*** EOI and/or Term Char which indicates the end of the reply.
	**/
	
	while ( TRUE ) {

		nRead = GPIBReadDat ( &(lpszBuffer [ nReadSoFar ]),
						(short)( nSize - nReadSoFar ) );

		nCompCode = GPIBCompCode ();	/* find out what happended */

		switch ( nCompCode ) {


			  /** Terminating character or EOI received so
			  *** this is the end of the reply.
			  **/
		case 5 :
		case 6 :
			  GPIBSendCom ( UNT );	  /* UNTALK microlink */

			  /** Replace the \r on the end of the reply with \0
			  **/

			  lpszTerm = strchr ( lpszBuffer, '\r' );

			  *lpszTerm = '\0';

			  GPIBRelease ();
			  bBusy = FALSE;

			  return ( OK );


		case 0 :  /** No error but we havn't reached the end yet
			  *** so carry on.
			  **/

			  nReadSoFar += nRead;
			  break;


		default : /** We have a GPIB error so return it.
			  **/

			  GPIBSendCom ( UNT );

			  GPIBRelease ();
			  bBusy = FALSE;

			  return ( LogAndTranslateError ( *nDevice,
							  nCompCode ) );
		}
	}

	return ( 0 );
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

	if ( bBusy ) {			/* Are we holding GPIB semaphore? */

		GPIBRelease (); 		/* release GPIB */

		bBusy = FALSE;
	}

	PrepareLinkToDevice ( nDevice );	/* Flush 3302 reply buffer */

	return ( OK );
}






/**
***
**/

short  AbortDriverBDPReply ( short nDevice )
{

	return ( OK );
}









/**  Incoming BDP Retriever - NOT IMPLEMENTED.
***
**/

short	GetBDPFromDriver ( WPARAM wParam, long lParam, LPIMLBDP *plpbdpData )
{
	return ( 0 );
}






/**  CloseDriverInterface - just release semaphore if necessary and
***  close GPIBLIB.DLL.
**/

short	CloseDriverInterface ( short nHandle )
{

	if ( bBusy ) {			/* Are we holding GPIB semaphore? */

		GPIBRelease ();
		bBusy = FALSE;
	}

	GPIBClose ( nGPIBHandle );

	return ( 0 );
}







/**  BackgroundProcess - NOT NEEDED in this case.
***
**/

void	BackgroundProcess ( void )

{

	return;
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

short  LogAndTranslateError ( short nDevice, short nProtocolError )
{
	short	nGenericError, nLogLevel;
	time_t	tTimeNow;
	char	szTimeDate [ 30 ];
	FILE	*pfLog;



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

		if ( nProtocolError == GPIB_NOT_OPEN ) {

			fprintf ( pfLog,
				  "%s - failed to open GPIB hardware driver\n",
				  szTimeDate );

		} else {	/* Error from GPIB drivers */

			fprintf ( pfLog,
				  "%s - GPIB error %d on IML device %d\n",
				  szTimeDate, nProtocolError, nDevice );
		}

		fclose ( pfLog );
	}


	switch ( nProtocolError ) {


	 case GPIB_NOT_OPEN :	/* Failed to find GPIB drivers */
	 case 11 :		/* GPIB drivers report hardware error */

		nGenericError = IML_CONFIG_ERROR;
		break;


	 case 4 :	/* GPIB error: no Microlink present on bus */

		nGenericError = IML_FATAL_ERROR;
		break;


	 case 7 :	/* GPIB error: Microlink not talking to us   */
	 case 9 :	/* GPIB error: Microlink not listening to us */

		nGenericError = IML_LINK_ERROR;
		break;


	 default :	/* Some miscellaneous unexpected error */

		nGenericError = nProtocolError;
		break;
	}

	return ( nGenericError );
}
