/**  ATimlcom.C         Source for function ATKProcessIMLCommand
***
***
***  This is a simple synchronous function which sends a command to an
***  IML device, and if there is a text reply, waits in a PeekMessage loop
***  for the reply to arrive. It displays error message boxes if necessary.
***
***  It can be used to start commands with binary replies or binary data, but
***  the direct BDP functions must then be used to transfer the data and to
***  terminate the data stream.
***
***  Version:   GENAPPS-ATK-P-3.00
***
***  History:
***     May 1991   Stuart Grace    First written for SetupIML ver 2.00
***     Sep 1991   Stuart Grace    Adapted for general use in IML App Tool Kit
***     Oct 1992   ATK-P-1.01      Timeout added for command reply.
***     Nov 1992   ATK-P-1.02      Improved timeouts to handle long periods
***                                spent in peek message.
***                                Aborts outstanding replys if in error.
***
***     May 1993   ATK-P-2.00      Longer timeout while waiting for commslib
***                                Does not AbortReply if timedout sending
***                                a command ( no reply will come back
***                                anyway )
***
***     Jan 1994   ATK-P-3.00      If no reply received reissues Command
***                                Does not report errors using Message Boxes
**/

#define _NOKANJI
#define IML_APPLICATION

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "\Windmill 32\Compile Libs\imlinter.h"          /* for generic IML interface */
#include "atkit.h"

/** If this is set then we are destroying the window. Get back to 
*** the main app ASAP.
**/
extern BOOL bDestroy;

/**     Local function prototypes ( for United Glass )
***
**/
short   ProcessIMLCommand (  HWND    hWndParent,
				short   nIMLHandle,
				char    * szCommand,
				short   nIMLDev,
				char    * szReply,
				short   nReplySize,
				WORD    wFlags  );
				
/**     Manifest Constants ( for United Glass )
***
**/                     
#define MAX_REISSUES    3



/**  ATKProcessIMLCommand - 
***  
***  This is the new entry-point.  It handles the reissuing of commands
***  in advent of failure. It is basically a new loop around the existing
***  ATKProcessIMLCommand code ( which has been moved into a static function )
***
***  Args:      hWndParent      window handle of caller
***             nIMLHandle      handle returned from OpenIMLInterface
***             szCommand       pointer to command string (eg. "READ 00204" )
***             nIMLDev         IML Device number to send command to
***             szReply         pointer to buffer to hold reply (if any)
***             nReplySize      size of reply buffer
***             wFlags          flags for SendIMLCommand
***
***  Return value is the error code: if not OK, a message box will have been
***  displayed BEFORE returning.
**/
short   ATKProcessIMLCommand (  HWND    hWndParent,
				short   nIMLHandle,
				char    * szCommand,
				short   nIMLDev,
				char    * szReply,
				short   nReplySize,
				WORD    wFlags  )

{
	short   nAttempt = 0;
	short   nRetCode;
	
	
	do {
		
		nRetCode = ProcessIMLCommand ( hWndParent, nIMLHandle,
			szCommand, nIMLDev, szReply, nReplySize, wFlags );
		if ( bDestroy) {
			return ( nRetCode );
		}
	

	} while ( ( ++nAttempt != MAX_REISSUES ) && ( nRetCode != OK ) );


	return ( nRetCode );
}





 
/**  ProcessIMLCommand - the function which actually sends out IML
***  commands and gets ASCII replies. It uses the Generic IML Interface defined
***  in IMLINTER.H. It includes the essential "PeekMessage" loop while waiting
***  for a reply, which allows interpreters in other Windows Apps to run,
***  since they may be generating the answer we are waiting for.
***
***  Args:      hWndParent      window handle of caller
***             nIMLHandle      handle returned from OpenIMLInterface
***             szCommand       pointer to command string (eg. "READ 00204" )
***             nIMLDev         IML Device number to send command to
***             szReply         pointer to buffer to hold reply (if any)
***             nReplySize      size of reply buffer
***             wFlags          flags for SendIMLCommand
***
***  Return value is the error code: if not OK, a message box will have been
***  displayed BEFORE returning.
**/
short ProcessIMLCommand ( HWND hWndParent,
				short   nIMLHandle,
				char    * szCommand,
				short   nIMLDev,
				char    * szReply,
				short   nReplySize,
				WORD    wFlags  )

{
	short           nCode;
	DWORD           dwStartWait;   /* elapsed mSecs when starting wait */
	BOOL            bTimedOut;
	DWORD			stime, ftime, mtime;
	MSG		msg;


	if ( bDestroy ) {
		return ( IML_NOT_READY );
	}


	stime = GetIMLMillisecondCount ();

	dwStartWait = GetIMLMillisecondCount ();   /* 'Start' timeout */

	do {

		
		nCode = SendIMLCommand ( nIMLHandle, nIMLDev,
					  (LPSTR) szCommand,  wFlags );

		/** Service messages until the queue is empty **/
		while ( PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE )) {	
				TranslateMessage ( &msg );
				DispatchMessage  ( &msg );			
		}
		
				
		mtime = GetIMLMillisecondCount ();

		/**     Have we timed-out ?  Remember always subtract
		***     a previous time from the current time. This
		***     correctly handles the DWORD wrapping round.
		**/
		bTimedOut = ( ( GetIMLMillisecondCount ( ) - dwStartWait ) >
				   ATK_SEND_TIMEOUT ) ? TRUE : FALSE;                              


	} while ( ( nCode == IML_NOT_READY ) && !bTimedOut);



	/**   If we were unable to send the command or if we
	***   the command was sent and expects no reply, we
	***   should exit the function now. If we are exiting
	***   because of an error, report it and abort the reply
	***   to be on the safe side.
	**/
	if ( ( nCode != OK ) || (( wFlags & IML_TEXT_REPLY ) == 0 ) ) {
		return ( nCode );   /* Return if no text reply, or error */
	}

	/** for debugging **/
	mtime = GetIMLMillisecondCount ();



	/**     Get the Reply to the command.
	***     Allow upto five seconds for reply ( It may be a slow
	***     responding ASCII device ) before terminating.
	**/
	dwStartWait = GetIMLMillisecondCount ( );   /* 'Start' timeout          */

	do {    /* if no answer - keep trying */

		nCode = GetIMLReply ( nIMLHandle, (short *) &nIMLDev,
					       (LPSTR) szReply, nReplySize, 0 );

		/** Service messages until the queue is empty **/
		while ( PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE )) {			

			TranslateMessage ( &msg );
			DispatchMessage  ( &msg );
		}



		/**     Have we timed-out ?  Remember always subtract
		***     a previous time from the current time. This
		***     correctly handles the DWORD wrapping round.
		***
		**/
		bTimedOut = ( ( GetIMLMillisecondCount ( ) - dwStartWait ) >
				   ATK_REPLY_TIMEOUT ) ? TRUE : FALSE;


	} while ( ( nCode == IML_NOT_READY ) && !bTimedOut  );


	/**     Report any errors and return the error code to caller.
	***     If reply received successfully, nCode will equal OK.
	***     We have a special error code for timed-out replys to
	***     differentiate it from the IML_NOT_READY error received
	***     if the bus was not available when attempting to send
	***     the IML command.
	**/
	if ( ( bTimedOut ) && ( nCode == IML_NOT_READY ) ) {


		/**     Report and return Timeout error
		***
		***     As the reply was not received successfully,
		***     abort it to keep things tidy in the Commslib...
		**/
		AbortIMLReply ( nIMLHandle, nIMLDev );

		return ( ATK_ERR_REPLY_TIMEOUT );

	} 


	/**     Report and return any IML/commslib errors or
	***     OK if reply received successfully.
	**/
	if ( nCode != OK ) {

		/**     As the reply was not recieved successfully,
		***     abort it to keep things tidy in the Commslib...
		**/
		AbortIMLReply ( nIMLHandle, nIMLDev );
	}
	
	ftime = GetIMLMillisecondCount ();
	
	return ( nCode );
	


}
