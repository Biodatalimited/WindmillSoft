/**	IMLINTER.H		Header file for IML App-to-Kernel Interface
***
***  Defines the function prototypes and return codes, as specified in
***  GENIML - WP 2.0. Should be included in apps making calls and in libraries
***  implementing the functions.
***
***	History
***
***  22 April 1991	Stuart Grace	First Version (ASCII only).
***  26 Sept  1991	Stuart Grace	Support for Binary Data Packets added.
***  30 Sept  1991      Nigel Roberts   Made dual use. I.e include in apps or
***                                     in DLL.
***
***  01 November, 1991  Nigel Roberts.  Rearranged so can be used for commslib
***                                     Winapps and interpreters.
***
***  May 1996   Updated for use with COMMSLIB 4.00 and Visual C++
**/





#if defined ( __cplusplus )

extern "C" {

#endif

#ifndef RC_INVOKED

#pragma pack ( 1 )

#endif


/**
***   The Binary Data Packet structure:
**/

typedef struct {

	short	nTotalPacketLength;		/* Header Size + Data Size */
	short	nDataLength;			/* Data Size in bytes	   */
	short	nBDPFormat;			/* BDP Format  = 1	   */
	short	nIMLDeviceNumber;
	short	nBDPErrorCode;
	WORD	unBDPFlags;
	DWORD	ulByteCount;
	BYTE	aucBinaryData [ 1 ];		/* VARIABLE LENGTH array   */
	
 } IMLBDP,  * LPIMLBDP,  * PIMLBDP;






/**
***   Error Codes
**/


#ifndef OK
#define OK	0
#endif

#define	IML_CONFIG_ERROR		16
#define	IML_NOT_READY			17
#define IML_NOT_OPEN                    18
#define	IML_BAD_HANDLE			19
#define	IML_REPLY_PENDING		20
#define	IML_NO_REPLY_EXPECTED		21
#define IML_TRANSMISSION_TIMEOUT 22
#define	IML_BAD_DEVICE			23
#define	IML_NO_HANDLES			24
#define	IML_NO_DEVICES			25
#define	IML_FATAL_ERROR			27
#define	IML_LINK_ERROR			29
#define	IML_BUFFER_TOO_SMALL		30
#define IML_NO_FREE_BUFFER		33
#define IML_NO_PACKET			34
#define IML_PACKET_TOO_BIG		35





/**
***   Flags passed in to SendIMLCommand:
**/


#define	IML_NO_REPLY	0x0000		/* no more data and no reply	*/
#define	IML_REPLY	0x0001		/*   (obsolete - do not use)	*/
#define	IML_TEXT_REPLY	0x0001		/* command has ASCII reply	*/
#define	IML_BIN_REPLY	0x0002		/* command has BDP reply	*/
#define	IML_BIN_DATA	0x0004		/* BDP data follows command	*/
#define IML_LOCKOUT     0x0008          /* No release comms on send command */
                                        /* or get reply */



/**
***   Flags passed in to SendIMLBinaryDataPacket:
**/


#define	IML_FAIL_IF_FULL	0x0000		/* Normal action */
#define	IML_OVERWRITE_IF_FULL	0x4000		/* Force packet through */


/**
***   Flags passed in to GetIMLBinaryDataPacket:
**/


#define	IML_CLEAR_BUFFER	0x0000		/* Normal action */
#define	IML_PRESERVE_BUFFER	0x2000		/* Leave packet in buffer */


/**
***   Flags in unBDPFlags field of binary data packet:
**/


#define	IML_BDP_OVERFLOW	0x0001
#define IML_BDP_END_DATA	0x0002






#if defined ( COMMSLIB ) 
#define CALLTYPE  _declspec (dllexport) WINAPI
#else
#define CALLTYPE  _declspec (dllimport) WINAPI
#endif


short CALLTYPE OpenIMLInterface ( short far *pnHandle );
/** Called by an app to get a Commslib handle **/

short CALLTYPE CloseIMLInterface ( short nHandle );
/** Called by an app to release a Commslib handle **/

short CALLTYPE GetIMLDeviceMap ( short nHandle, long far * plLowMap,
							long far *plHighMap );
/** Called by an Setupiml to get a bit map of devices **/

short CALLTYPE SendIMLCommand ( short nHandle, short nDevice,
                                        char far *szCommand, WORD unFlags );
/** Called by an app to send a command **/
 
  
short CALLTYPE AckIMLCommand ( DWORD ulCommandNumber, short nError );
/** This routine is called by interpreters to acknowledge the completion
*** of a command sent to them.
**/ 

short CALLTYPE GetIMLReply ( short nHandle, short far *nDevice,
                             char far *szBuffer, short nSize, WORD unFlags );
/** Called by an app to get a reply **/

short CALLTYPE GetIMLInterfaceVersion ( short nHandle, char far *
                                                   szBuffer, short nSize );
/** Gets a version number **/

short CALLTYPE AbortIMLReply ( short nHandle, short nDevice );
/** Called by an app to cancel a reply **/


DWORD CALLTYPE GetIMLMillisecondCount ( void );
/** Called by an app to get an incrementing count of milliseconds **/


short CALLTYPE GetIMLCommandToExecute ( DWORD ulCommandNumber,
                              LPSTR lpszBuffer, short nSize );
/** Called by an IML interpreter when it has been told that a command is ready.**/

short CALLTYPE ReplyToIMLCommand ( DWORD ulCommandNumber,
                              LPSTR lpszBuffer );
/** Called by an IML interpreter to send a reply.**/

short CALLTYPE RegisterIMLInterpreter ( HWND hWndInterpreter,
                               short nDeviceNumber );
/** Called by an IML interpreter on loading to register itself.**/

short CALLTYPE UnRegisterIMLInterpreter ( short nDeviceNumber );
/** Called by an IML interpreter on unloading to unregister itself.**/

short _declspec (dllexport)  WINAPI GetIMLINIPath ( char* cFilePath, DWORD dwSize );
/** Get the path to the IML.INI file **/

short _declspec (dllexport)  WINAPI GetIMLDirPath ( char* cFilePath, DWORD dwSize );
/** Get the path to the Working Dir **/

/** BDP function prototypes.
**/
short CALLTYPE SendIMLBinaryDataPacket ( LPIMLBDP lpbdpSend, WORD unFlags);

short CALLTYPE GetIMLBinaryDataPacket ( short nDevice, LPIMLBDP
                                      lpbdpGet, short nSize, WORD unFlags );

short CALLTYPE TerminateIMLBDPStream ( short nDevice, short nErrorCode );


/** Debug for Comiml **/
int   CALLTYPE ReadDebugString ( char* cDebugReply );

void  CALLTYPE WriteDebugString ( char* cDebugSend, int nSize );


       
#ifndef RC_INVOKED

#pragma pack ()

#endif
       

#if defined ( __cplusplus )

}

#endif

