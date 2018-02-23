/**	IMLCOML.C		IML App-to-COMMSLIB Interface Module
***
***  Implements the standard IML functions and return codes specified in
***  GENIML - WP 2.0, for use by Windows IML Apps.  Uses the COMMSLIB DLL
***  to do most of the work.
***
***	History
***
***  26 June 1991	Stuart Grace	First Version - uses IML.INI only.
**/


#define IML_APPLICATION


#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "\Windmill 32\Compile Libs\commslib.h"
#include "\Windmill 32\Compile Libs\imlinter.h"


#define	MAX_DEVICES	31		/* only supports devices 0 to 30 */


static BOOL	bReplyPending [ MAX_DEVICES ];




/**  OpenIMLInterface. This opens a channel in COMMSLIB.DLL. If this fails,
***  it maps the error codes from Commslib onto the Generic IML error codes.
***  Argument is pointer to short which will hold handle. Returns error code.
**/

short	OpenIMLInterface ( short *pnHandle )

{
	
	*pnHandle = OpenMicrolinkComms ();		/* open Commslib DLL */

	if ( *pnHandle != COMMS_NOT_OPEN ) {

		return ( OK );		/* successfully opened COMMSLIB */
	}

	switch ( GetMicrolinkCommsError ( COMMS_NOT_OPEN ) ) {

	  case COMMS_HARDWARE_NOT_PRESENT :
	  case COMMS_GPIB_VERSION_ERROR :
	  case COMMS_GPIB_INSTALL_ERROR :  return ( IML_CONFIG_ERROR );

	  case COMMS_GPIB_HARDWARE_ERROR :
	  case COMMS_BAD_BAUD_RATE :	   return ( IML_FATAL_ERROR );
	  
	  case COMMS_OUT_OF_MEMORY :	   return ( IML_NO_HANDLES );
	  
	  case COMMS_NO_MICROLINKS :	   return ( IML_NO_DEVICES );
	  
	  default :
	  case COMMS_DEVICE_BUSY :	   return ( IML_LINK_ERROR );
	}
}




/**  GetIMLDeviceMap. This examines the IML.INI file to determine which device
***  numbers are in use. It returns a bitmap in two longs. Entries it detects
***  are of the form:
***
***	[DeviceN]
***	Type = xxxx
***
***    NOTE: This version only detects up to 30 devices maximum.
**/

short	GetIMLDeviceMap ( short nHandle, long *plLowMap,
							 long  *plHighMap )
{
	short	nDevice;
	char	szKeyName [ 20 ];
	char 	szProfileString [ 80 ];


	*plLowMap  = 0x00000000L;
	*plHighMap = 0x00000000L;	/* high map NOT USED in this version */
	

	/** For each possible device address X we look for:
	*** 
	*** File: IML.INI	Section: [DeviceX]	Entry: type=...
	**/
	
	for ( nDevice = 0 ; nDevice < MAX_DEVICES ; nDevice++ ) {


		sprintf ( szKeyName , "DEVICE%d", nDevice );
		
		GetPrivateProfileString ( (LPSTR) szKeyName,
					  (LPSTR) "type",
					  (LPSTR) "",
					  (LPSTR) szProfileString,
					  79,
					  (LPSTR) "IML.INI" );

		if ( strlen ( szProfileString ) > 0 ) {

			*plLowMap |= (long) ( 0x00000001L << nDevice );
			continue;
		}
	}

	return ( OK );		
}



/**  SendIMLCommand. This requests exclusive use of the device, and if granted,
***  it sends the command to it. If the command has no reply, it releases the
***  device and returns the error code (probably OK). If there is a reply, it
***  keeps the semaphore set and returns OK to the caller. The caller should
***  run a PeekMessage loop to allow the command to be executed, then call
***  GetIMLReply to retrieve the answer and release the semaphore.
**/

short	SendIMLCommand ( short nHandle, short nDevice, char *szCommand,
								WORD unFlags )
{
	short	nCode;
	

	nCode = RequestMicrolinkComms ( nHandle, nDevice );
	
	if ( nCode != 0 ) {
		return ( nCode );	/* abort - request failed */
	}
	
	nCode = SendMicrolinkCommand ( nHandle, nDevice, szCommand,
			      ( ( unFlags & IML_REPLY ) == 0 ? FALSE : TRUE ) );

	if (( nCode != 0 ) || ( ( unFlags & IML_REPLY ) == 0 )) {

		ReleaseMicrolinkComms ( nHandle, nDevice );

		bReplyPending [ nDevice ] = FALSE;

		if (( nCode > 0 ) && ( nCode < 16 )) {    /* map GPIB errors */
			return ( IML_LINK_ERROR );        /* to "link errors" */
		}
		return ( nCode );
	}

	bReplyPending [ nDevice ] = TRUE;
	return ( OK );
}



/**  GetIMLReply. This should be called to get the reply after a call to
***  SendIMLCommand with the IML_REPLY flag set.
***
***   NOTE: This version DOES NOT IMPLEMENT nDevice value -1 for "Any Device"!
***
***  *nDevice must point to the device number from which the reply is expected.
***  It will return one of the following:
***
***	OK			complete reply received correctly
***	IML_NOT_READY		no reply available yet
***	IML_NO_REPLY_EXPECTED	specified device has no pending reply
***	IML_LINK_ERROR		out of memory
***	IML_BUFFER_TOO_SMALL	reply does not fit into buffer - it was LOST!
***
**/

short	GetIMLReply ( short nHandle, short *nDevice, char *szBuffer,
						   short nSize, WORD unFlags )
{
	short	nCode, nSizeLeft;
	BOOL	bMoreComing;
	HANDLE	hReply;
	LPSTR	lpszReply;
	

	/**
	*** !! nDevice value -1 is NOT YET SUPPORTED !!
	**/


	bMoreComing    = FALSE;		/* true when in middle of receiving */
	szBuffer [ 0 ] = 0x00;		/* initialise buffer to "" */
	nSizeLeft      = nSize;		/* space left in partially filled buf */
	
	if ( !bReplyPending [ *nDevice ] ) {
		return ( IML_NO_REPLY_EXPECTED );
	}

	while ( TRUE ) {

		hReply = GetMicrolinkReply ( nHandle );
		
		nCode = GetMicrolinkCommsError ( nHandle );
	
		if ( hReply != NULL ) {		/* if we got a valid handle */

			lpszReply = (LPSTR) GlobalLock ( hReply );
		
			if ( lpszReply == NULL ) {	      /* fail to lock */
				GlobalFree ( hReply );
				ReleaseMicrolinkComms ( nHandle, *nDevice );
				return ( IML_LINK_ERROR );	/* out of mem */
			}

			if ( lstrlen ( lpszReply ) > ( nSizeLeft - 1 ) ) {
				return ( IML_BUFFER_TOO_SMALL );
			}
	
			lstrcat ( szBuffer, lpszReply );	/* add reply */
			nSizeLeft -= lstrlen ( lpszReply );	/*  into buf */

			GlobalUnlock ( hReply );
			GlobalFree ( hReply );			/* free mem */

			if ( nCode == OK ) {		/* was that the end? */

				ReleaseMicrolinkComms ( nHandle, *nDevice );
				bReplyPending [ *nDevice ] = FALSE;

				return ( OK );		/* OK - all finished */

			} else if ( nCode == COMMS_MORE_REPLY_TO_COME ) {

				bMoreComing = TRUE;	/* note: Mid reply */
				continue;		/* go try again */
			}
		}

		/** Comes here if we did not get a valid handle: find out why
		**/

		switch ( nCode ) {

		  case COMMS_NO_REPLY_RECEIVED :	/* nothing arrived */

			if ( bMoreComing ) {

				/** Flag says we are in the middle of getting
				*** a reply, so just keep on trying...
				**/

				continue; 
			}
			
			/** Nothing has arrived yet - return to caller
			**/

			return ( IML_NOT_READY );

		  case COMMS_BAD_CHANNEL_NUMBER : 	/* bad handle */

		  	nCode = IML_BAD_HANDLE;		/* programmer error! */
		  	break;

		  case COMMS_NO_REPLY_EXPECTED :	/* We disagree!! */

		  	nCode = IML_NO_REPLY_EXPECTED;  /* Er..library error? */
			break;

		  default :
		  	nCode = IML_LINK_ERROR;		/* Some other cock-up */
		  	break;
		}

		ReleaseMicrolinkComms ( nHandle, *nDevice );
		return ( nCode );
	}
}




/**
***  NOT FULLY IMPLEMENTED YET!
**/

short	GetIMLInterfaceVersion ( short nHandle, char * szBuffer,
								 short nSize )
{
	return ( 1 );
}



/**  Just closes our channel in COMMSLIB DLL.
***
**/

short	CloseIMLInterface ( short nHandle )
{
	return ( CloseMicrolinkComms ( nHandle ) );
}
