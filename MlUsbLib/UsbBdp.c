#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "c:\windmill 32\handlers\kit\imconst.h"
#include "c:\Windmill 32\handlers\usbkit\imusb.h"

/**  Shared statics which record the current state of the BDP streaming engine
**/
short	nMaxBufferSize;	/* Max data bytes we can put into one packet */
short	nNotifyCount;	/* Min data bytes in a packet */

HWND    hwndNotify;		/* Window to post "packet ready" message to */
WORD    wNotifyMsg;		/* Message value to be posted */


char	bdpBuffer [ MAX_DATA_PACKET + 50 ];	/* Local Data Packet */

BOOL	bMessageSent = FALSE;	/* TRUE when msge posted to window  */

unsigned long	ulTotalBytes;		/* cumulative total data bytes 	    */



/********************************************************/
/** This routine is called to read data from the mlusb driver
*** and put it into a BDP. If it reads any data then it will have
*** enough to send the packet to Streamer. It is called from the idle
*** loop which is started by a timer 20 times a second.
**/ 


void  BDP_ReadDataFromUSB ( void )
{
	PIMLBDP	  pbdpBuf;
	DWORD     dwReturnedBytes;


	/** If the last message is under way then we cannot
	*** read any more data.
	**/
	if ( bMessageSent ) {
		return;
	}

	pbdpBuf = (PIMLBDP) bdpBuffer;	   /* treat buffer as BDP header */

	

	/** Read the data. Return in case of error 
	*** We always ask for a full BDP. We only get any data 
	*** back if we have more than notify level
	**/
	if ( UsbReadIsoch ( (pbdpBuf->aucBinaryData + pbdpBuf->nDataLength), 
					nMaxBufferSize,  &dwReturnedBytes ) )  {
		pbdpBuf->unBDPFlags |= IML_BDP_OVERFLOW;

	} else if ( dwReturnedBytes == 0 ) {
		return;
	}

	/** We either have an error or we have some data. 
	*** In both cases we need to send a message
	**/

	/** The bytes read are always greater than notify level **/
	pbdpBuf->nDataLength = (short) dwReturnedBytes;		
	
	bMessageSent = TRUE;
	PostMessage ( hwndNotify, wNotifyMsg, 0, 0L );					
			
	return;	
}


/*******************************************************/
/** This is called from AUX_TRIGGER to initialise the
*** static variables used for the BDP.
**/

short  BDP_Initialise ( short nMinPac, short nMaxPac,
					 WORD hWnd, WORD wMsg )
{

	hwndNotify   = (HWND) hWnd;	/* who we send message to   */
//	hwndNotify = hWndMain;
	wNotifyMsg   = wMsg;		/* the message to send      */
	bMessageSent = FALSE;		/* not yet sent it 	    */

	nNotifyCount   = nMinPac;	/* when we send the message     */
	nMaxBufferSize = nMaxPac;	/* when the buffer overflows    */

	ulTotalBytes = 0L;			/* total bytes so far = 0 */

	( (PIMLBDP) bdpBuffer )->nDataLength   = 0;   /* no data in packet yet*/
	( (PIMLBDP) bdpBuffer )->nBDPFormat    = 1;
	( (PIMLBDP) bdpBuffer )->nBDPErrorCode = 0;
	( (PIMLBDP) bdpBuffer )->unBDPFlags    = 0;

	return ( OK );
}




/**  Called when the WinApp's main window procedure is ready to post a BDP
***  off to COMMSLIB. This occurs in response to us posting the message to
***  that window (see ExtractDataFromBuffer func). Gets here via a AUX_GETBDP
***  call to Aux570Clk function.
**/

short  BDP_DespatchDataPacket ( LPIMLBDP fpBDP )
{
	PIMLBDP	  pbdpBuf;

	
		
	
	pbdpBuf = (PIMLBDP) bdpBuffer;	   /* treat buffer as BDP header */


	/** Keep running total of all data bytes sent (this must be included
	*** in the BDP headers):
	***/

	ulTotalBytes += (unsigned long) pbdpBuf->nDataLength;
	
	pbdpBuf->ulByteCount = ulTotalBytes;


	/** Send off the full packet to the application, by copying contents
	*** into the transit buffer provided.
	**/

	pbdpBuf->nTotalPacketLength = ( sizeof ( IMLBDP ) +
							pbdpBuf->nDataLength );
	
	memcpy ( (void *) fpBDP, (void *) pbdpBuf,
						 pbdpBuf->nTotalPacketLength );

/****
***** {char sz[80];
***** sprintf (sz,"Sent BDP: size %d flags %u \n", pbdpBuf->nDataLength,
*****					           pbdpBuf->unBDPFlags );
***** OutputDebugString (sz);
***** }
****/

	/** Re-Initialise the local packet, ready for re-use
	**/

	pbdpBuf->nDataLength   = 0;		/* no data in packet yet */
	pbdpBuf->nBDPFormat    = 1;
	pbdpBuf->nBDPErrorCode = 0;
	pbdpBuf->unBDPFlags    = 0;

	
	
	bMessageSent = FALSE;		/* can send notify message again */


	return ( OK );
}

