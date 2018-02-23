/** COMMBDP.C	Nigel Roberts,	15 Feb, 1994.
***
*** Commslib BDP routines.
**/
#define COMMSLIB





#define NOMINMAX	1	/** to prevent min max redefinition error **/


/** Include Files 
**/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>




/** include files which are specific to this code
**/

#include "comdef.h"	/**library specific #defined constants and structures**/
#include "\Windmill 32\Compile Libs\imlinter.h"	/** error codes and exported function prototypes **/
#include "comproto.h"	/** library specific function prototypes - non ststic**/





/** externally declared variables and structures which are required by this 
*** module.
**/


/** Array of DeviceInfo structs indexed on device number. These hold
*** information on the Devices with which the library can communicate.
**/

extern IMLDEVICE adiDevice [ MAX_DEVICES ];

extern BDPLINK aBDP [ MAX_BDP_LINKS ];

extern short	nCurrentMode;

/** Prototype **/
static short FreeBDPLink ( short nDevice );
short VerifyPacket ( short nDevice, short nLink );



/************************************************************/
/** This routine sends a binary data packet to COMMSLIB.
*** It may originate with an App or an IML Device. 
*** The packet is contained in lpbdpSend. unFlags contains
*** a set of flags. These include :
*** IML_OVERWRITE_IF_FULL
**/



short _declspec (dllexport)  WINAPI SendIMLBinaryDataPacket ( LPIMLBDP lpbdpSend, WORD
								unFlags )

{

short	nTermCode;
short   nRet;
short	nDevice;
short	nLink;



	/** Get the device number and the link number **/
	nDevice = lpbdpSend->nIMLDeviceNumber;
	nLink = adiDevice [ nDevice ].nBDPLink;


	/** Check that everthing is OK with the request **/
	nRet = VerifyPacket ( nDevice, nLink );
	if (nRet != OK ) {
		return (nRet);
	}
	
	
	/** Check if the link has been terminated by the packet
	*** destination. If it has then we free the link and return
	*** the termination code set by the destination to the
	*** calling app. This allows one end of the link to tell
	*** the other end why it has terminated.
	**/
	if ( ( aBDP [ nLink ].wFlags & BDP_LINK_TERMINATED ) != 0 ) {

		nTermCode = aBDP [ nLink ].nTerminateCode;
		FreeBDPLink ( nDevice );
		return ( nTermCode );
	}



	/** check if the buffer is empty.
	**/
	if ( ( aBDP [ nLink ].wFlags & BDP_DATA_IN_BUFFER ) != 0 ) {

		/** Are we going to overwrite it.
		**/

		if ( ( unFlags & IML_OVERWRITE_IF_FULL ) == 0 ) {
			/** We fail cos its full.
			**/

			return ( IML_NO_FREE_BUFFER );
		}
	}

	/** OK copy the packet in.
	**/

	memcpy ( (LPSTR) aBDP [ nLink ].cBuffer, (LPSTR) lpbdpSend,
					lpbdpSend->nTotalPacketLength );

	aBDP [ nLink ].wFlags |= BDP_DATA_IN_BUFFER;

	return ( 0 );
}


/************************************************************/
/** This routine gets a binary data packet from COMMSLIB.
*** It may got to  an App or an IML Device. 
*** nDevice specifies the IML device that the packet is from or to.
*** The packet is put into lpbdpGet. 
*** nSize is the max size of the receiving buffer.
*** unFlags contains a set of flags. These include :
*** IML_PRESERVE_BUFFER
**/



short _declspec (dllexport)  WINAPI GetIMLBinaryDataPacket ( short nDevice, LPIMLBDP
				      lpbdpGet, short nSize, WORD unFlags )

{

short	nTermCode;
short   nRet;
short	nLink;



	/** Get the device number and the link number **/
	nDevice = lpbdpGet->nIMLDeviceNumber;
	nLink = adiDevice [ nDevice ].nBDPLink;


	/** Check that everthing is OK with the request **/
	nRet = VerifyPacket ( nDevice, nLink );
	if (nRet != OK ) {
		return (nRet);
	}



	/** If there is data in the buffer then copy it through.
	**/
	if ( ( aBDP [ nLink ].wFlags & BDP_DATA_IN_BUFFER ) != 0 ) {
		/** Check the size is OK.
		**/
		if ( nSize < ( ( (PIMLBDP) (aBDP [ nLink ].cBuffer) )->
						nTotalPacketLength ) ) {

			return ( IML_PACKET_TOO_BIG );
		}

		/** OK copy the packet across.
		**/
		memcpy ( (LPSTR) lpbdpGet, (LPSTR) aBDP [ nLink ].cBuffer,
			((PIMLBDP)(aBDP [ nLink ].cBuffer))->nTotalPacketLength );


		/** Look at the flags to see if we are to flush the buffer.**/
		if ( ( unFlags & IML_PRESERVE_BUFFER ) == 0 ) {
			/** we  flush the buffer.**/
			aBDP [ nLink ].wFlags &= ~BDP_DATA_IN_BUFFER;
		}



		/** Check if the link has been terminated by the packet
		*** source. If it has then we free the link and return
		*** the termination code set by the source to the
		*** calling app. This allows one end of the link to tell
		*** the other end why it has terminated.
		**/

		if ( ( aBDP [ nLink ].wFlags & BDP_LINK_TERMINATED ) != 0 ) {

			nTermCode = aBDP [ nLink ].nTerminateCode;
			FreeBDPLink ( nDevice );



			return ( nTermCode );
		}


		return ( 0 );
	}


	/** Check if we have terminated.
	**/
	if ( ( aBDP [ nLink ].wFlags & BDP_LINK_TERMINATED ) != 0 ) {

		nTermCode = aBDP [ nLink ].nTerminateCode;
		FreeBDPLink ( nDevice );

		return ( nTermCode );
	}

	/** There is no data in the buffer so return IML_NO_PACKET.
	**/


	return ( IML_NO_PACKET );
}


/*************************************************************/
/** Called by both ends of the link to Terminate. The link is
*** freed on the second terminate call. On the second call
*** the error code from the first call is returned.
*** The Second call is not normally required. Typically one end
*** calls this function then when the other end makes a send or
*** get call the link is freed and the error code is passed back to
*** the caller.
**/ 



short _declspec (dllexport)  WINAPI TerminateIMLBDPStream ( short nDevice, short
								nErrorCode )

{

short	nLink;
short   nRet;


	/** Get the link from the device **/
	nLink = adiDevice [ nDevice ].nBDPLink;


	/** Check that everthing is OK with the request **/
	nRet = VerifyPacket ( nDevice, nLink );
	if (nRet != OK ) {
		return (nRet);
	}



	/** Is this the second terminate ? If so then free the link.
	*** Return the terminate code.
	**/
	if ( aBDP [ nLink ].wFlags & BDP_LINK_TERMINATED ) {

		FreeBDPLink ( nDevice );

		return ( aBDP [ nLink ].nTerminateCode );
	}

	aBDP [ nLink ].nTerminateCode = nErrorCode;
	aBDP [ nLink ].wFlags |= BDP_LINK_TERMINATED;


	return ( 0 );
}


/** This is called when SendIMLCommand has the correct flags set for
*** link creation. The message of SendIMLCommand defines buffer sizes
*** etc for the interpreter.
*** Look for a free link. If there is one then initialise
*** it and the device fields.
*** Return TRUE or FALSE if there are none free.
**/


short CreateBDPLink ( short nIMLHandle, short nDevice )

{

	short	nLink;


	/** Find a free link **/
	for ( nLink = 0; nLink < MAX_BDP_LINKS; nLink ++ ) {

		if ( aBDP [ nLink ].wFlags == BDP_LINK_FREE ) {
			break;
		}
	}


	/** No free link so return an error **/
	if ( nLink == MAX_BDP_LINKS ) {

		return ( FALSE );
	}


	/** Initialise the link **/
	aBDP [ nLink ].nTerminateCode = 0;
	aBDP [ nLink ].nDevice = nDevice;
	aBDP [ nLink ].nIMLHandle = nIMLHandle;
	aBDP [ nLink ].wFlags = BDP_LINK_BUSY;
	adiDevice [ nDevice ].nBDPLink = nLink;
	/** NB the adiDevice[].wReplyType flag is set in the SendIML
	*** Command routine.
	**/

	return ( TRUE );
}




static short FreeBDPLink ( short nDevice )

{

	short	nLink;


	/** Get the link for this device.**/
	nLink = adiDevice [ nDevice ].nBDPLink;

	/** If there is none **/	
	if ( nLink == BAD_LINK ) {
		return ( FALSE );
	}

	FreeDevice ( adiDevice [ nDevice ].nInUseBy, nDevice );
	/** Initialise the device fields **/
	adiDevice [ nDevice ].wReplyType = IML_NO_REPLY;
	adiDevice [ nDevice ].nBDPLink = BAD_LINK;



	/** Initialise the link **/
	aBDP [ nLink ].wFlags = BDP_LINK_FREE;
	aBDP [ nLink ].nTerminateCode = 0;


	return ( TRUE );
}



/** Check that the device number and link supplied with
*** the packet request are OK. 
*** Return OK or an error.
**/

short VerifyPacket ( short nDevice, short nLink ) 

{
	/** Does the device exist.
	**/

	if ( ! IsDeviceOK ( nDevice ) ) {
		return ( IML_BAD_DEVICE );
	}


	/** Check that we are expecting a BDP for this device**/
	if ( ( adiDevice [ nDevice ].wReplyType != IML_BIN_REPLY ) &&
		 ( adiDevice [ nDevice ].wReplyType != IML_BIN_DATA ) ) {

		return ( IML_LINK_ERROR );
	}

	/** We must have a link **/
	if ( nLink == BAD_LINK ) {

		return ( IML_LINK_ERROR );
	}

	/** The BUSY must be set **/
	if ( ( aBDP [ nLink ].wFlags & BDP_LINK_BUSY ) == 0 ) {

		return ( IML_LINK_ERROR );
	}

	
	return (OK);
}