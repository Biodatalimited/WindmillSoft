/**	IMNETB.C        Low level routines for IML NetBIOS Protocol
***
***	(C) Copyright Windmill Software Ltd, 1994. All rights reserved.
***
***  This file is to be used with IMMLDRIV.C. Please read the comments
***  at the top of IMMLDRIV.C for a full explanation.  The routines in
***  this file send the IML commands over NetBIOS links to Network Microlinks
***  and retrieve the replies.
***
***  Other NetBIOS specific files (not including IMML... common files):
***     IMNETB.RC       resources
***     IMNETB.ICO      icon
***     IMNETB.DLG      dialog (About box)
***     IMNETB.DEF      linker def file
***     IMNETB.MAK      build file
***
***  The finished .EXE file is called IMNETB.EXE. It makes extensive use
***  of the functions in IMNBLIB.DLL. These are described in IMNBLIB-FS.
***
***  Version History:
***
***   October 94    Stuart Grace    First version. (based on IMSERL.C)
**/



#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "\Windmill 32\Compile Libs\imlinter.h"  /* for BDP structure and flags */
#include "\Windmill 32\Compile Libs\imnblib.h"            /* for IMNBLIB.DLL (NetBIOS interface) */

#include "\Windmill 32\Protocols\immldriv.h"
#include "imnetb.h"





/**  Local prototypes
**/

short  RegisterMyNetName ( void );
void   SendEscapeCommands ( short nDevice );



/**  Export the Windows class name and Window title to IMMLDRIV module:
**/

char *szDriverName = "IMNETB";         /* Our prog name: export to IMMLDRIV */

char *szTitle      = "Microlink Driver";        /* Our window title: export */

char *szIdent 	   = "NETBIOS 1.04";     	/* Response to ~ID command  */






/**  Import the table of Microlink devices and the path of the error logging
***  file in from the IMMLDRIV module.
**/

extern IMLDEVICE   Microlink [ NUM_DEVICES ];
extern char        szLogFile [ _MAX_PATH ];

extern HWND	   hWndMain;		/* handle of our main Window  */





/**
***  Constants relating to Ethernet and NetBIOS comms
**/

#define NET_SIGNATURE   0x25AC    /* first word of every IML NetBIOS block  */

#define NET_COMMAND   	 1000     /* Block ident for commands 		    */
#define NET_TEXT_REPLY   1001     /* Block ident for textual replies	    */
#define NET_BDP_REPLY    1002     /* Block ident for Bin Data Packt replies */
#define NET_BDP_ABORT    1003     /* Block ident for BDP stream cancel      */
#define NET_ABORT_ACK    1004     /* Block ident for stream cancel ack'ment */

#define NET_REC_TIMEOUT    5      /* NetBIOS Receive TO = 5 * 0.5 = 2.5 sec */
#define NET_SEND_TIMEOUT   5      /* NetBIOS Send TO    = 5 * 0.5 = 2.5 sec */

#define MAX_DATA_PACKET  8192
#define MAX_TEXT_LENGTH   500


#define TIMEOUT_REGISTER        3000L      /* Max 3000 msecs to reg name    */
#define TIMEOUT_CALL             250L      /* Max  250 msecs to call mlink  */
#define TIMEOUT_TRANSFER        3500L      /* Max 3500 msecs to xfer data   */
#define TIMEOUT_DEREGISTER       300L      /* Max  300 msecs to delete name */

#define MAX_REG_ATTEMPTS          10       /* Try up to 10 different names  */




/**  These structures are used to access the fields within the data blocks
***  sent & received over the network. IMLTXTBLOCK is used for IML commands,
***  IML text replies, and BDP aborts. IMLBINBLOCK is used to receive BDPs.
**/

typedef struct tagIMLTxtBlock {     /* Network block for text command/reply */

        WORD    wSignature;                       /* Windmill ident code  */
        WORD    wBlockType;                       /* Block type code      */
        WORD    wFlags;                           /* Expected reply type  */
        char    szText [ MAX_TEXT_LENGTH ];       /* Variable length text */

} IMLTXTBLOCK;


typedef struct tagIMLBinBlock {    /* Network block for Binary Data Packet */

    	WORD	wSignature;	      		   /* Windmill ident code  */
    	WORD	wBlockType;	      		   /* IML BDP block type   */
        IMLBDP	bdpPacket;	      		   /* The BDP header       */
    	char    acDataSpace [ MAX_DATA_PACKET ];   /* space for the data   */

} IMLBINBLOCK;




/**  Here is space reserved for data buffers, formatted as the structures
***  defined above.
**/

static IMLTXTBLOCK NetCmdBlock;         /* Buffer to hold comnd being sent */

static IMLBINBLOCK NetBinBlock;         /* Buffer for anything being recvd */
                                        /*  (ALL recvd blocks arrive here) */



/**  Miscellaneous globals:
**/

static short    nDriverNumber;          /* Allows multiple IMNETBs on 1 net */
static short    nNetUser;               /* Assigned to us by IMNBLIB.DLL    */
static BOOL     bNetReady;              /* TRUE when NetBIOS ready for use  */
static short  	nPendingDevice;   	/* Which device is replying  	    */
static short  	nReceiveMode;           /* Which type of reply expected:    */

#define RECEIVE_TEXT            0               /* Expecting text reply    */
#define RECEIVE_BDP	        1               /* Expecting Binary reply  */



/**  Globals used for Text replies:
**/

static char   acRecdText [ MAX_TEXT_LENGTH ];    /* Reply text received    */

static short  nReplyStatus;                     /* What is in reply buf:   */

#define REPLY_BUFFER_EMPTY      0               /* Nothing in buffer       */
#define REPLY_IN_BUFFER         1               /* Reply waiting in buffer */
#define REPLY_FAILED            2               /* Err while getting reply */



/**  Globals used during BDP streaming transfers:
**/

static short  nBDPErrorCode;                    /* Error code for this BDP  */
static BOOL   bBDPLinkCancelled;

#define NO_BDP_WAITING      -998          /* Error code before BDP arrives */





/**  One of these structures is used for each Network Microlink we are
***  managing. It tracks the current status of the NetBIOS session and any
***  active transfers.
**/

typedef struct tagLinkInfo {

    	short	nSession;       /* NetBIOS Session Num, or BAD_SESSION_NUM */
	short   nCallOp;        /* Op Num for pending CALL, or BAD_OP_NUM  */
	short   nSendOp;        /* Op Num for pending SEND, or BAD_OP_NUM  */
	short   nReceiveOp;     /* Op Num for pending RECV, or BAD_OP_NUM  */

} LINKINFO;



static LINKINFO Link [ NUM_DEVICES ];     /* One for each Microlink */











/**  OpenDriverInterface - this just initialises local data structures to
***  show we have no sessions, no replies, not registered with NetBIOS, etc.
***
***  Never fails.
**/

short  OpenDriverInterface ( short *pnHandle )
{
        short   nIndex;



        for ( nIndex = 0; nIndex < NUM_DEVICES; nIndex++ ) {

                Link [ nIndex ].nSession   = BAD_SESSION_NUM;
                Link [ nIndex ].nCallOp    = BAD_OP_NUM;
                Link [ nIndex ].nSendOp    = BAD_OP_NUM;
        	Link [ nIndex ].nReceiveOp = BAD_OP_NUM;
        }

        bNetReady = FALSE;

        nReplyStatus  = REPLY_BUFFER_EMPTY;
	nBDPErrorCode = NO_BDP_WAITING;
        bBDPLinkCancelled = FALSE;

        nDriverNumber = 0;

        return ( OK );
}







/**  This is notification from IMMLDRIV section that we will be dealing with
***  the IML device specified. This is our chance to initialise and prepare
***  for future commands. This is also called EVERY TIME WE NEED TO SEND A
***  COMMAND - this ensures that if the link to a Microlink was lost for any
***  reason, it will try to reconnect if possible. This makes quite an
***  effective error recovery mechanism.
***
***     - if not done already, then register our local name with NetBIOS
***         (should be needed on first call only).
***
***     - if session already established with specified device, just return.
***
***     - if a CALL is in progress, return (must wait for it to complete).
***
***     - if no session & no CALL active, start a CALL to this Microlink.
***
***     - We then wait for a short time (0.25 secs) looking only for
***         notification messages from IMNBLIB which tell us the CALL has
***         completed. (Do NOT yield during this time - the Apps might be
***         trying to make re-entrant calls into here). If the Microlink is
***         ready and listening, the call should succeed and establish a
***         session within this time.
***
***     - If the session is not created at the end of this timeout period
***         then return IML_NOT_READY, and we will find the results on a
***         subsequent call to this routine.
***
***  (Dealing with the notification message and analysing the result happens
***   in the func HandleNetworkEvent below. This also calls SendEscapeCommands
***   to get the Microlink correctly configured).
***
***  Returns OK if session ready to use, or IML_NOT_READY if the network link
***  is not yet available.
**/

short  PrepareLinkToDevice ( short nDevice )
{
        char    szDeviceName[ 20 ];
        DWORD   dwStartTime;
        short   nCode;
        MSG     msg;




        if ( !bNetReady ) {

                RegisterMyNetName ();      /* Will try to get NetBIOS ready */
        }

        if ( !bNetReady ) { 
                return ( IML_NOT_READY );       /* Not yet registered */
        }

        if ( Link [ nDevice ].nSession != BAD_SESSION_NUM ) {
	 
                return ( OK );          /* Link is already established */
        }

        if ( Link [ nDevice ].nCallOp != BAD_OP_NUM ) {
	 
                return ( IML_NOT_READY );    /* CALL underway - must wait */
        }


        /** No session with target device is active. We must therefore make
        *** a NetBIOS CALL to the Microlink (server)...
        **/

        sprintf ( szDeviceName, "IML_DEVICE_%d", nDevice );

		nCode = IMNBCallServer ( nNetUser,
                                 szDeviceName,
                                 NET_REC_TIMEOUT,
                                 NET_SEND_TIMEOUT,
                                 &( Link [ nDevice ].nCallOp ) );

        if ( nCode != OK ) {
				 
                return ( LogAndTranslateError ( nDevice, nCode ) );
        }


        /** We must now wait for our CALL to be answered, and process network
        *** notification messages while waiting. If the CALL is going to
        *** succeed (ie. Microlink is ready and Listening) it normally
        *** completes in a fraction of a second. If it is still pending after
        *** this time, we will have to come back later to find the result.
        *** (NetBIOS has a built-in timeout for CALLs of about 3 secs).
        ***
        *** NB: THIS IS SLIGHTLY ANTI-SOCIAL! It will tie up the CPU for up
        ***     to 0.25 sec without allowing other apps to run. Interrupts are
        ***     enabled so NetBIOS, serial comms, etc. can service interrupts.
        **/

        dwStartTime = GetTickCount ();

        do {
		if ( PeekMessage ( &msg, hWndMain, MSG_NET_EVENT,
                        		   MSG_NET_EVENT,
                                	     ( PM_NOYIELD | PM_REMOVE ) ) ) {

                	HandleNetworkEvent ( msg.wParam, msg.lParam );
        	}

	} while ( ( GetTickCount () - dwStartTime ) < TIMEOUT_CALL );


        if ( Link [ nDevice ].nSession == BAD_SESSION_NUM ) {

                return ( IML_NOT_READY );      /* Link is NOT established */
        }

	return ( OK );
}






/**  RegisterMyNetName - called from PrepareLinkToDevice if it sees bNetReady
***  is FALSE. This indicates that we have not yet registered our own name
***  with IMNBLIB.DLL and NetBIOS. This generates a name of the form
***
***     IML_DRIVER_0
***
***  and uses IMNBRegisterName to get this name into the local NetBIOS name
***  table. When the operation completes successfully, HandleNetworkEvent
***  gets the notification from IMNBLIB and sets bNetReady TRUE. If another
***  computer on the network has already done this (i.e. is also running
***  IMNETB), the operation fails and bNetReady remains FALSE. This will
***  provoke a subsequent call into here, and the second time through we
***  will try the name
***
***	IML_DRIVER_1
***
***  This can continue until a unique name is found. This routine waits
***  without yielding for the notification to arrive - with a max timeout
***  after 3 secs if nothing has happened before then. (Normally takes one
***  second or so). Will return OK (unless it gives up trying different
***  names), but the real result is in the global bNetReady.
***
***  NOTE: The name is only registered for a single LAN Adapter, so
***        ALL MICROLINKS MUST USE THE SAME LANA NUMBER !!!
**/

short  RegisterMyNetName ( void )
{
        char    szNetName[ 20 ];
        short   nDummy, nCode, nLoop, nLana;
        DWORD   dwStartTime;
        MSG     msg;




        if ( nDriverNumber > MAX_REG_ATTEMPTS ) {
    
			return ( LogAndTranslateError ( -1, ERR_TOO_MANY_DRIVERS ) );
        }


        sprintf ( szNetName, "IML_DRIVER_%d", nDriverNumber );


       	/** We now need to know which network to use. This is specified in
        *** the NetBIOS LANA field. (In Win for WorkGroups each LANA number
        *** specifies a combination of physical adapter card & transport
        *** protocol. These are defined in PROTOCOL.INI or somewhere). In
        *** general, LANA will be zero meaning the "primary" network. The
        *** LANA value is specified in ConfIML and put in the Address field.
        ***
        ***
        ***     !!! ALL MICROLINKS MUST USE THE SAME LANA NUMBER !!!
        ***
        ***
        *** We will find the first Microlink which is our responsibilty and
        *** register with NetBIOS requesting that LANA.
       	**/

        nLana = 0;

        for ( nLoop = 0; nLoop < NUM_DEVICES; nLoop++ ) {

		if ( Microlink [ nLoop ].nHardwareAddress > -1 ) {

                        nLana = Microlink [ nLoop ].nHardwareAddress;
                        break;
                }
        }

       	nCode = IMNBRegisterName ( nLana, szNetName, hWndMain, MSG_NET_EVENT,
                                   				    &nDummy );

        nDriverNumber++;        /* Try using another name next time round */

        if ( nCode != OK ) { 
			return ( LogAndTranslateError ( -1, nCode ) );
        }



        /** We must now wait for our registration to be processed. Wait for
        *** the notification message. Timeout after three seconds if nothing
        *** happened (should not occur - regn usually fails very quickly or
        *** completes within 2 secs).
        ***
        *** NB: THIS IS ANTI-SOCIAL! It will tie up the CPU for up to 2 secs
        ***     without allowing other apps to run. Interrupts are enabled
        ***     so NetBIOS, serial comms, etc. can service interrupts.
        **/

        dwStartTime = GetTickCount ();
		 
        do {
		if ( PeekMessage ( &msg, hWndMain, MSG_NET_EVENT,
                        		   MSG_NET_EVENT,
                                	     ( PM_NOYIELD | PM_REMOVE ) ) ) {

                	
					HandleNetworkEvent ( msg.wParam, msg.lParam );

                    break;      /* exit loop when done */
        	}

	} while ( ( GetTickCount () - dwStartTime ) < TIMEOUT_REGISTER );


        return ( OK );
}








/**  Commands beginning with the tilde char ( ~ ) are filtered out and
***  passed into here, as a "driver escape" command. The only command
***  implemented at present is ~ID to identify the type of link. We
***  respond with "NETBIOS 1.xx" (xx = minor version).
**/

short  ProcessTildeCommand ( short nDevice, char *szCommand,
                        				  char *szReply )
{

        if ( strnicmp ( szCommand, "~ID", 3 ) == 0 ) {

                strcpy ( szReply, szIdent );  	/* see top of file */
        } else {
                strcpy ( szReply, "Unrecognised command" );
        }

        return ( OK );
}






/**  SendDriverCommand. This is the key routine used to send a command from
***  the PC to the Network Microlink. Outline operation:
***
***     - If there is a notification from IMNBLIB in our message queue,
***         process this to get up-to-date with the network status.
***
***     - Call PrepareLinkToDevice to check if session exists, and if not
***         to try creating one.
***
***     - Allow previous SENDs or RECEIVEs on this session to complete.
***
***     - Construct network data block with header etc, and initiate a
***         SEND operation.
***
***     - If the flags indicate that a reply is wanted (Text or BDP), also
***         initiate a RECEIVE operation so we are ready to accept the
***         reply as soon as it appears.
***
***  Any errors detected are routed through LogAndTranslateError which both
***  writes the error details into the log file (if enabled) and returns an
***  appropriate generic error code (eg. IML_LINK_ERROR).
**/

short  SendDriverCommand ( short nHandle, short nDevice,
                                        char *lpszCommand, WORD unFlags )

{
	short	nCode, nIMLCode, nTotalSize, nStatus;
        DWORD   dwStartTime;
        MSG     msg;




        /** If there is a network event notification from IMNBLIB waiting in
        *** our message queue, sort this out before going any further.
        **/

	if ( PeekMessage ( &msg, hWndMain, MSG_NET_EVENT, MSG_NET_EVENT,
                                	     ( PM_NOYIELD | PM_REMOVE ) ) ) {

                HandleNetworkEvent ( msg.wParam, msg.lParam );
        }


        nStatus = PrepareLinkToDevice ( nDevice );     /* Check/create sess */

	if ( nStatus != OK ) {

                return ( nStatus );       /* Session not yet ready */
	}


        /** If the previous send or receive ops are still being processed,
        *** we wait here until they are completed. A timeout exists here
        *** for safety, in case the network mysteriously dies.
        **/

        dwStartTime = GetTickCount ();

        while ( ( Link [ nDevice ].nSendOp != BAD_OP_NUM ) ||
                ( Link [ nDevice ].nReceiveOp != BAD_OP_NUM ) ) {


		if ( PeekMessage ( &msg, hWndMain, MSG_NET_EVENT, MSG_NET_EVENT,
                                	     ( PM_NOYIELD | PM_REMOVE ) ) ) {

                	HandleNetworkEvent ( msg.wParam, msg.lParam );
        	}


                /** Time limit while waiting for Op to finish
                **/

        	if ( ( GetTickCount () - dwStartTime ) > TIMEOUT_TRANSFER ) {

			Link [ nDevice ].nSendOp    = BAD_OP_NUM;
                	Link [ nDevice ].nReceiveOp = BAD_OP_NUM;

                        return ( LogAndTranslateError ( nDevice,
                					ERR_XFER_TIMEOUT ) );
                }
	}


        /** Now complete all fields in NetCmdBlock buffer:
        **/

        NetCmdBlock.wSignature = NET_SIGNATURE;
        NetCmdBlock.wBlockType = NET_COMMAND;
        NetCmdBlock.wFlags     = unFlags;          /* flags as for COMMSLIB */

        strcpy ( NetCmdBlock.szText, lpszCommand );


        /** Total number of bytes to send =
        ***
        *** 	 6 byte header + command string + terminating zero byte
        **/

        nTotalSize = strlen ( NetCmdBlock.szText ) + 7;

        nCode = IMNBSendData ( nNetUser,
                               Link [ nDevice ].nSession,
        		       &( NetCmdBlock ),
                               nTotalSize,
                               &( Link [ nDevice ].nSendOp ) );

        nIMLCode = LogAndTranslateError ( nDevice, nCode );

        if ( nIMLCode != OK ) {

                return ( nIMLCode );    /* Failed to start send op */
        }

        nPendingDevice = nDevice;       /* Any replies will be from here */


	/** Is a reply going to be sent back? if so, prepare for it:
        **/

        if ( unFlags == IML_TEXT_REPLY ) {      /* look out for text reply */

                nReceiveMode = RECEIVE_TEXT;
                nReplyStatus = REPLY_BUFFER_EMPTY;

        	nCode = IMNBReceiveData ( nNetUser,
                               		  Link [ nDevice ].nSession,
        		       		  sizeof ( NetBinBlock ),
                               		  &( Link [ nDevice ].nReceiveOp ) );

                nIMLCode = LogAndTranslateError ( nDevice, nCode );
        }

        if ( unFlags == IML_BIN_REPLY ) {       /* look out for BDP reply */

                nReceiveMode      = RECEIVE_BDP;
                nReplyStatus      = REPLY_BUFFER_EMPTY;
                nBDPErrorCode     = NO_BDP_WAITING;
	        bBDPLinkCancelled = FALSE;

        	nCode = IMNBReceiveData ( nNetUser,
                               		  Link [ nDevice ].nSession,
        		       		  sizeof ( NetBinBlock ),
                               		  &( Link [ nDevice ].nReceiveOp ) );

                nIMLCode = LogAndTranslateError ( nDevice, nCode );
        }

        return ( nIMLCode );
}









/**  GetDriverReply. This is polled from time to time while an app is waiting
***  for a text reply. The actual reception and processing of the reply block
***  occurs in HandleNetworkEvent, which sets nReplyStatus and acRecdText.
***  All we do here is ensure that messages from IMNBLIB are passed through
***  to HandleNetworkEvent, and check on nReplyStatus.
***
***  Returns either IML_NOT_READY (not yet received), or OK when reply
***  been put into buffer, or IML_LINK_ERROR if problem. (The error log will
***  have been filled in by HandleNetworkEvent).
**/

short	GetDriverReply ( short nHandle, short *pnDevice,
                             char *lpszBuffer, short nSize, WORD unFlags )
{
        short   nCode;
        MSG     msg;




        /** If there is a network event notification from IMNBLIB waiting in
        *** our message queue, sort this out before going any further.
        **/

	if ( PeekMessage ( &msg, hWndMain, MSG_NET_EVENT, MSG_NET_EVENT,
                                	     ( PM_NOYIELD | PM_REMOVE ) ) ) {

                HandleNetworkEvent ( msg.wParam, msg.lParam );
        }


        switch ( nReplyStatus ) {       /* What has been detected? */

          default :
          case REPLY_BUFFER_EMPTY :          /* Nothing happened yet */

                nCode = IML_NOT_READY;       /* try again later */
                break;


          case REPLY_FAILED :                /* Error occurred */

                nReplyStatus = REPLY_BUFFER_EMPTY;      /* reset err flag */

                nCode = IML_LINK_ERROR;                 /* report error */
                break;


          case REPLY_IN_BUFFER :             /* Reply has been received */

                strncpy ( lpszBuffer, acRecdText, ( nSize - 1 ) );

                lpszBuffer [ nSize - 1 ] = 0;  		/* terminate string */

                nReplyStatus = REPLY_BUFFER_EMPTY;      /* reset flag */

                nCode = OK;
                break;
        }

        return ( nCode );
}







/**  This function will cancel a pending reply. After SendDriverCommand has
***  been called with IML_TEXT_REPLY, the normal course would be to call
***  GetDriverReply one or more times until a reply was successfully
***  obtained. This process can be aborted by calling AbortDriverReply, which
***  will cancel the pending RECEIVE op and reset the internal status ready
***  to start sending a new command. This function allows recovery from a
***  failed command.
**/

short  AbortDriverTextReply ( short nDevice )
{



        /** Cancel the pending RECEIVE op, if there is one for this device
        **/

       	if ( Link [ nDevice ].nReceiveOp != BAD_OP_NUM ) {

                IMNBCancelOp ( Link [ nDevice ].nReceiveOp );
        }


        /** This is an abnormal event - log it
        **/

        LogAndTranslateError ( nDevice, ERR_REPLY_ABORT );

        return ( OK );
}







/**  GetBDPFromDriver.
***
***  IMMLDRIV calls this function to retrieve a BDP we have we received, or
***  at least, get the error code if it went wrong. It calls here when it
***  finds the MSG_BDP_READY message sent from HandleNetworkEvent. This
***  occurs when the BDP is sitting in the NetBinBlock buffer, and
***  nBDPErrorCode has been set to the associated error code.
**/

short  GetBDPFromDriver ( WPARAM wParam, LPARAM lParam, LPIMLBDP *plpbdpData )
{
        short    nCode;



        /** Now see if a new BDP has arrived in NetBinBlock?
        **/

        switch ( nReplyStatus ) {

          default :
          case REPLY_BUFFER_EMPTY :             /* Nothing arrived yet */

                nCode = IML_NOT_READY;
                break;

          case REPLY_FAILED :                   /* Error occurred */

                AbortDriverBDPReply ( nPendingDevice );   /* ABORT transfer */

                nCode = IML_LINK_ERROR;
                break;


          case REPLY_IN_BUFFER :                /* New BDP ready */

	        *plpbdpData = &( NetBinBlock.bdpPacket );  /* set ptr to it */

                nCode = nBDPErrorCode;          /* This is it's status */
                break;
        }

        return ( nCode );
}












/**  SetBDPStatus - IMMLDRIV calls this after it has called GetBDPFromDriver.
***  This function tells this module what happened with the BDP just supplied
***  which implies what should be done next:
***
***     BDP_STAT_GET_NEXT     app accepted it, next BDP should be obtained
***     BDP_STAT_REPEAT       app was too busy, so repeat this BDP again
***     BDP_STAT_ABORT        app wants to end transfer - use abort protocol
**/

void  SetBDPStatus ( short nStatus )
{
        short   nCode;
	MSG     msg;



        switch ( nStatus ) {

          case BDP_STAT_GET_NEXT :    /* Current BDP processed successfully */

        	nBDPErrorCode = NO_BDP_WAITING;         /* reset flags */
                nReplyStatus  = REPLY_BUFFER_EMPTY;

		nCode = IMNBReceiveData ( nNetUser,
             		            Link [ nPendingDevice ].nSession,
       				    sizeof ( NetBinBlock ),
                       		    &( Link [ nPendingDevice ].nReceiveOp ) );

		LogAndTranslateError ( nPendingDevice, nCode );
                break;


          case BDP_STAT_REPEAT :        /* Failed to get thru' - try again */

                /** Allow the destination application to run. This should
                *** make space in COMMSLIB so this BDP will be accepted.
                *** Then post the message to our own main window again,
                *** which will prompt it to try again.
                **/

		PeekMessage ( &msg, NULL, 0, 0, PM_NOREMOVE );

                PostMessage ( hWndMain, MSG_BDP_READY, 0, 0L );
                break;


          case BDP_STAT_ABORT :         /* No more BDPs wanted */

                AbortDriverBDPReply ( nPendingDevice );
                break;
        }

        return;
}






/**  AbortDriverBDPReply. This is called when a stream of BDPs is being
***  received, and the app wants to abort this before it stops normally.
***
***  The method defined for this in the IML NetBIOS Protocol is to send
***  a special block to the relevant Microlink with the block type set to
***  NET_BDP_ABORT. When it receives this, the Microlink replies with another
***  message, with block type set to NET_ABORT_ACK. However, there might
***  be one or more normal BDP messages queued and in transit, which are too
***  late to stop. Thus we should keep accepting messages and discarding them
***  until NET_ABORT_ACK turns up. The flag bBDPLinkCancelled is set to show
***  this discarding stage.
***
***  So this routine has three stages:
***
***     - wait for any pending SENDs to this device to complete (the
***         original BSTREAM command may still be in the pipeline)
***
***     - construct a NET_BDP_ABORT block and start sending it
***
***     - ensure we have an active RECEIVE operation, to catch any remaining
***	    BDPs and the acknowledgement.
**/

short  AbortDriverBDPReply ( short nDevice )
{
        short   nCode;
        DWORD   dwStartTime;
        MSG     msg;



        /** Wait for any earlier SEND operation to complete:
        **/

        dwStartTime = GetTickCount ();

        while ( Link [ nDevice ].nSendOp != BAD_OP_NUM ) {


		if ( PeekMessage ( &msg, hWndMain, MSG_NET_EVENT, MSG_NET_EVENT,
                                	     ( PM_NOYIELD | PM_REMOVE ) ) ) {

                	HandleNetworkEvent ( msg.wParam, msg.lParam );
        	}


                /** Time limit while waiting for Op to finish
                **/

        	if ( ( GetTickCount () - dwStartTime ) > TIMEOUT_TRANSFER ) {

                        return ( LogAndTranslateError ( nDevice,
                					ERR_XFER_TIMEOUT ) );
                }
	}


        /** The required abort message has just three fields: Signature,
        *** Block type, and Flags (reserved for future).
        **/

        NetCmdBlock.wSignature = NET_SIGNATURE;
       	NetCmdBlock.wBlockType = NET_BDP_ABORT;
       	NetCmdBlock.wFlags     = 0;                /* reserved for future */

       	IMNBSendData ( nNetUser,
                       Link [ nDevice ].nSession,
       		       &( NetCmdBlock ),
               	       6,                                 /* 6 bytes */
               	       &( Link [ nDevice ].nSendOp ) );


        /** This should produce an ABORT_ACK response from the Microlink, and
        *** there might be BDPs already in transit to us. We have to
        *** accept these, otherwise the transfer will timeout and kill the
        *** session. So we make sure there is a receive operation active, but
        *** set a flag which notes that:
        ***     - if a BDP arrives, discard it
        ***     - if a NET_ABORT_ACK arrives, this transfer is complete.
        **/

        if ( Link [ nDevice ].nReceiveOp == BAD_OP_NUM ) {

		nCode = IMNBReceiveData ( nNetUser,
               		                  Link [ nDevice ].nSession,
       				          sizeof ( NetBinBlock ),
                       			  &( Link [ nDevice ].nReceiveOp ) );

       		LogAndTranslateError ( nPendingDevice, nCode );
        }

        bBDPLinkCancelled = TRUE;

        return ( OK );
}







/**  HandleNetworkEvent. This is a key routine - it gets called whenever
***  IMNBLIB.DLL has put a NET_EVENT message into our message queue. This
***  means that some NetBIOS operation has completed - either successfully
***  or it has failed.
***
***  wParam with the message tells us the type of operation - CALL, SEND, etc.
***  lParam has (in LOWORD) the Op Number which was issued when this operation
***  was initiated. For Call, Send & Recv, this stored in the Link structure
***  for the relevent device.
***
***  The action to be taken always involves calling the appropriate
***  IMNBFinish... func for this type of operation, to get the error code
***  and any returned data. This is then checked, errors are noted in the
***  log file, and global buffers & status variables are updated to show
***  what happened. Where the Op number was stored in the Link structure,
***  this is changed to BAD_OP_NUM which shows there is now no such operation
***  in progress. See comments below for details.
**/

void  HandleNetworkEvent ( WPARAM wParam, LPARAM lParam )
{
        short   nCode, nSender, nDevice, nTry, nOp;
        WORD    wDataSize;



	nOp = (short) LOWORD ( lParam );      /* This is Op which completed */


        /** If we cannot figure out which device this command relates to
        *** (should never happen), mess with the last supported device, as
        *** this is the least likely to be actually in use.
        **/

        nDevice = ( NUM_DEVICES - 1 );           /* Use this if not sure */


        switch ( wParam ) {         /* Which type of operation? */


                /** REGISTER - if this is OK, we set bNetReady to TRUE to
                ***  show network can be used (nNetUser is valid). If it
                ***  fails, we just leave bNetReady at false as this will
                ***  prompt further tries with alternative names.
                **/

          case IMNB_REGISTER:

                nCode = IMNBFinishRegistration ( nOp, &nNetUser );

                if ( nCode == OK ) {

                        bNetReady = TRUE;       /* Network can be used */
                }
                break;


                /** CALL - a call to a Microlink has either connected or has
                ***  timed out. First we find which device was being called
                ***  by this op. The session num put into the Link struct
                ***  will either be valid or BAD_SESSION_NUM. If a new sess
                ***  has been established, we immediately send the ESC comnds
                ***  needed to configure the Microlink.
                **/

          case IMNB_CALL:

                /** Must first find out which device has this operation
                *** pending:
                **/

                for ( nTry = 0; nTry < NUM_DEVICES; nTry++ ) {

                        if ( Link [ nTry ].nCallOp == nOp ) {

                                nDevice = nTry;     /* This is it! */
                                break;
                        }
                }

                nCode = IMNBFinishCall ( nOp, &( Link [ nDevice ].nSession ));

                Link [ nDevice ].nCallOp = BAD_OP_NUM;     /* op finished */

                if ( nCode != OK ) {    /* Call failed */

                        LogAndTranslateError ( nDevice, nCode );

                } else {        /* New session established successfully */

                        SendEscapeCommands ( nDevice );    /* config Mlink */
                }
                break;


                /** SEND - figure out which Microlink this was for, then get
                ***  error code. If an error occurs during a SEND, NetBIOS
                ***  always kills the session (by design).
                **/

          case IMNB_SEND:

                /** Must first find out which device has this operation
                *** pending:
                **/

                for ( nTry = 0; nTry < NUM_DEVICES; nTry++ ) {

                        if ( Link [ nTry ].nSendOp == nOp ) {

                                nDevice = nTry;     /* This is it! */
                                break;
                        }
                }

                nCode = IMNBFinishSend ( nOp );

                Link [ nDevice ].nSendOp = BAD_OP_NUM;   /* This Op finishd */

                if ( nCode != OK ) {

                        LogAndTranslateError ( nDevice, nCode );

                        /** NetBIOS error occurring during SEND causes the
                        *** session to be dropped automatically. We will
                        *** record this as a separate event, and clear the
                        *** session number to show this session is no longer
                        *** active.
                        **/

                        LogAndTranslateError ( nDevice, ERR_FAILED_SEND );

                	Link [ nDevice ].nSession = BAD_SESSION_NUM;
                }

               	break;


                /** RECEIVE - determine which link was receiving, then ask
                ***  IMNBLIB to put the data (if any) into the Binary data
                ***  buffer (which is large enough for anything). We then
                ***  analyse the signature and block type, to decide what
                ***  to do with it:
                ***
                ***  Recv Text: put it in acRecdText, and set nReplyStatus.
                ***
                ***  Recv BDP:  if wanted - set nReplyStatus & nBDPErrorCode
                ***             if discarding - just look for next message
                ***
                ***  Recv Abort-Ack : clear the discarding flag
                **/

          case IMNB_RECEIVE:

                /** Must first find out which device has this operation
                *** pending:
                **/

                for ( nTry = 0; nTry < NUM_DEVICES; nTry++ ) {

                        if ( Link [ nTry ].nReceiveOp == nOp ) {

                                nDevice = nTry;     /* This is it! */
                                break;
                        }
                }

                Link [ nDevice ].nReceiveOp = BAD_OP_NUM;    /* Op finished */


                /** Erase any previous header from the big (BDP) buffer
                **/

                memset ( &NetBinBlock, 0, 8 + sizeof ( IMLBDP ) );

               	nCode = IMNBFinishReceive ( nOp, &NetBinBlock, &wDataSize,
                                        	    		   &nSender );

               	if ( nCode != OK ) {     /* NetBIOS Receive failed */

                        nReplyStatus = REPLY_FAILED;

                	LogAndTranslateError ( nDevice, nCode );
                        break;
                }

                if ( NetBinBlock.wSignature != NET_SIGNATURE ) {

		        /** Block does NOT have correct signature at start
                        **/

                        nReplyStatus = REPLY_FAILED;

                	LogAndTranslateError ( nDevice, ERR_BAD_HEADER );
                        break;
                }

                switch ( NetBinBlock.wBlockType ) {     /* check block type */

                  case NET_BDP_REPLY :

                        if ( bBDPLinkCancelled ) {

                                /** A BDP link has just been aborted - this
                                *** packet was already in the pipeline but
                                *** it is not needed now - discard it. Start
                                *** looking for next message, because we are
                                *** expecting an Abort Acknowledge.
                                **/

				nCode = IMNBReceiveData ( nNetUser,
               		                  Link [ nDevice ].nSession,
       				          sizeof ( NetBinBlock ),
                       			  &( Link [ nDevice ].nReceiveOp ) );

       				LogAndTranslateError ( nDevice, nCode );

                                break;       /* don't need the data */
                        }

	                if ( nReceiveMode == RECEIVE_BDP ) {

        	                nReplyStatus = REPLY_IN_BUFFER;

			        /** Fill in field showing where it came from:
        			**/

        			NetBinBlock.bdpPacket.nIMLDeviceNumber =
                                			       nPendingDevice;

				nBDPErrorCode = OK;	/* assume its OK */


			        /** Check for flags set within BDP which tell
                		*** us this is the last one.
        			**/

				if ( ( NetBinBlock.bdpPacket.unBDPFlags &
                                        	   IML_BDP_END_DATA ) != 0 ) {
		
					nBDPErrorCode = END_OF_DATA;
				}

				if ( ( NetBinBlock.bdpPacket.unBDPFlags &
	                                           IML_BDP_OVERFLOW ) != 0 ) {

					nBDPErrorCode = BUFFER_OVERFLOW;
				}


                                /** Now tell our Window proc to come & get it
                                **/

        			PostMessage ( hWndMain, MSG_BDP_READY, 0, 0L);

                        } else {

                	        nReplyStatus = REPLY_FAILED;

                		LogAndTranslateError ( nDevice,
                        				   ERR_BAD_HEADER );
                        }

                        break;


                  case NET_TEXT_REPLY :

                        if ( nReceiveMode == RECEIVE_TEXT ) {

	                      	strcpy ( acRecdText,
        	                    ( (IMLTXTBLOCK *) &NetBinBlock)->szText );

                        	nReplyStatus = REPLY_IN_BUFFER;

                        } else {

                	        nReplyStatus = REPLY_FAILED;

                		LogAndTranslateError ( nDevice,
                        				 ERR_BAD_HEADER );
                        }

                        break;


                  case NET_ABORT_ACK :

                        if ( bBDPLinkCancelled ) {

                                bBDPLinkCancelled = FALSE;   /* clear flag */
                        } else {
                		LogAndTranslateError ( nDevice,
                        				   ERR_BAD_HEADER );
                        }

                        break;

                  default :

               	        nReplyStatus = REPLY_FAILED;

			LogAndTranslateError ( nDevice, ERR_BAD_HEADER );
                        break;
                }

                break;


                /** DISCONNECT - find which session was being killed, and if
                ***  it succeeded, reset session number to BAD_SESSION_NUM.
                **/

          case IMNB_DISCONNECT:

                /** Must first find out which device has this operation
                *** pending:
                **/

                for ( nTry = 0; nTry < NUM_DEVICES; nTry++ ) {

                        if ( Link [ nTry ].nReceiveOp == nOp ) {

                                nDevice = nTry;     /* This is it! */
                                break;
                        }
                }

                nCode = IMNBFinishDisconnection ( nOp );

                if ( nCode == OK ) {    /* session ended? */

                        Link [ nDevice ].nSession = BAD_SESSION_NUM;
                } else {
                	LogAndTranslateError ( nDevice, nCode );
                }
               	break;


                /** DEREGISTER - if successful, set bNetReady to FALSE.
                **/

          case IMNB_DEREGISTER:

                nCode = IMNBFinishDeregistration ( nOp );

                if ( nCode != OK ) {

                        LogAndTranslateError ( -1, nCode );
                }

                bNetReady = FALSE;

                break;
        }

        return;
}






/**  This is called from HandleNetworkEvent whenever a new session with a
***  Microlink is established. It looks at the szEscape strings read from
***  IML.INI into the Microlink structures, expands these into full IML
***  ESCAPE commands and tries to send them to the relevent device.
***
***  IML.INI contains upto four settings of this type:
***
***             ESC1 = 0000 FUNC1 PARAMS1
***             ESC2 = 0123 FUNC2 PARAMS2  etc.
***
***  This gets sent to device N as the following:
***
***             ESC N0000 FUNC1 PARAMS1
***             ESC N0123 FUNC2 PARAMS2  etc.
***
***  The replies are NOT COLLECTED, so there is no checking if the command
***  executes successfully. If the command fails to be sent over the network
***  this would be logged in the IMNETB.LOG file (if enabled).
**/

void  SendEscapeCommands ( short nDevice )
{
        char    szCmd [ 50 ];
        char	*(szPtr [ 4 ]);
        short   nCode, nEsc;


	szPtr [ 0 ] = Microlink [ nDevice ].szEscape1;
        szPtr [ 1 ] = Microlink [ nDevice ].szEscape2;
        szPtr [ 2 ] = Microlink [ nDevice ].szEscape3;
        szPtr [ 3 ] = Microlink [ nDevice ].szEscape4;


        for ( nEsc = 0; nEsc < 4; nEsc++ ) {       /* for each ESC cmnd */

	        if ( strlen ( szPtr [ nEsc ] ) > 0 ) {     /* if in use */

                	sprintf ( szCmd, "ESC %d%s", nDevice, szPtr [ nEsc ]);

			nCode = SendDriverCommand ( 0, nDevice, szCmd,
                        				      IML_NO_REPLY );

                	if ( nCode != OK ) {

                        	LogAndTranslateError ( nDevice,
                                		       ERR_ESCAPE_FAILED );

                        	return;         /* Abandon attempt to configure */
                	}
        	}

        } /* next command */


        return;
}









/**  CloseDriverInterface - called when IMNETB is closing down. This is
***  either because the user has chosen to close our icon, or Windows is
***  shutting down. We simply initiate disconnection on ALL active sessions,
***  then initiate Deregistration of our NetBIOS name. We then allow a
***  little time for IMNBLIB and NetBIOS to process this lot and tidy up
***  the network (local and remote nodes) before returning. A fixed time
***  interval (currently 0.3 secs) is allowed for this.
***
***   NB: IT IS IMPORTANT TO DO THIS!
***
***  Leaving sessions active when Windows exits can cause problems on other
***  nodes.
**/

short	CloseDriverInterface ( short nHandle )
{
        short   nIndex, nDummy;
        DWORD   dwStartTime;
        MSG     msg;



        for ( nIndex = 0; nIndex < NUM_DEVICES; nIndex++ ) {

                if ( Link [ nIndex ].nSession != BAD_SESSION_NUM ) {

                        IMNBDisconnectSession ( nNetUser,
                        			Link [ nIndex ].nSession,
                                                &nDummy );
                }
        }


        IMNBDeregister ( nNetUser, &nDummy );

        bNetReady = FALSE;


        /** Now we allow a little while for all these NetBIOS commands to
        *** be processed and we deal with all the notifications which come
        *** back.  (Half second).
        **/

        dwStartTime = GetTickCount ();

        do {
		if ( PeekMessage ( &msg, hWndMain, MSG_NET_EVENT, MSG_NET_EVENT,
                                	     ( PM_NOYIELD | PM_REMOVE ) ) ) {

                	HandleNetworkEvent ( msg.wParam, msg.lParam );
        	}

	} while ( ( GetTickCount () - dwStartTime ) < TIMEOUT_DEREGISTER );


        return ( OK );
}







/**  LogAndTranslateError. This takes two parameters:
***
***     nDevice         IML device number being accessed, or -1 for general
***                      errors not specific to any one device.
***
***     nProtocolError  an error code obtained from IMNBLIB.DLL or defined
***			 in IMNETB.H.
***
***  General errors (nDevice = -1) always get recorded in the log file with
***  the time, date & description. Errors on a specific device only get logged
***  if the Microlink.ErrorLogLevel field for that device is above zero.
***
***  The function translates the IMNBLIB error code into the most appropriate
***  generic IML error such as IML_LINK_ERROR or IML_FATAL_ERROR. This is
***  returned, so it can be passed back to COMMSLIB and the application.
**/

short  LogAndTranslateError ( short nDevice, short nProtocolError )
{
	short	nGenericError, nLogLevel;
        time_t  tTimeNow;
        char    szTimeDate [ 30 ];
        char    *szMessage;
        FILE    *pfLog;



        if ( nProtocolError == 0 ) {    /* Error code 0 means no problem */

        	return ( OK );           /* just return OK and exit */
        }


        szMessage = "";


        switch ( nProtocolError ) {


         case ERR_TOO_MANY_DRIVERS :     /* Cannot find available name */

                szMessage = "Too many IMNETB drivers on this net. ";
        	nGenericError = IML_FATAL_ERROR;
                break;

         case ERR_BAD_HEADER :           /* Bad block header received */

                szMessage = "Wrong message format received from";
        	nGenericError = IML_LINK_ERROR;
                break;

         case ERR_XFER_TIMEOUT :          /* NetBIOS op not completed */

                szMessage = "NetBIOS transfer not complete -";
        	nGenericError = IML_LINK_ERROR;
                break;

         case IMNB_TIMEOUT :           	 /* NetBIOS transfer timed out */

                szMessage = "Timeout expired -";
        	nGenericError = IML_LINK_ERROR;
                break;

         case IMNB_DATA_INCOMPLETE :     /* ML send data bigger than buffer */

                szMessage = "Data block too big from";
        	nGenericError = IML_LINK_ERROR;
                break;

         case IMNB_BAD_SESSION_NUM :     /* Session number rejected */

                szMessage = "Link no longer active to";
        	nGenericError = IML_LINK_ERROR;

                if ( nDevice > -1 ) {           /* Note session has died */

                        Link [ nDevice ].nSession = BAD_SESSION_NUM;
                }
                break;

         case IMNB_SESSION_DISCONNECTED :   /* Session gone */

                szMessage = "NetBIOS disconnected from";
        	nGenericError = IML_LINK_ERROR;

                if ( nDevice > -1 ) {           /* Note session has died */

                        Link [ nDevice ].nSession = BAD_SESSION_NUM;
                }
                break;

         case IMNB_OP_CANCELLED :           /* We cancelled this op */
         case IMNB_ALREADY_COMPLETED :      /* We wanted to cancel this op */

                return ( OK );           /* NOT AN ERROR - deliberate */


         case IMNB_TOO_MANY_NAMES : 	    /* NetBIOS name table full */

                szMessage = "Too many NetBIOS users on this machine. ";
        	nGenericError = IML_FATAL_ERROR;
                break;

         case IMNB_TOO_MANY_SESSIONS :      /* NetBIOS Sess table full */

                szMessage = "Too many network connections. ";
        	nGenericError = IML_LINK_ERROR;
                break;

         case IMNB_SERVER_NOT_READY :       /* Microlink not listening */

                szMessage = "Microlink not yet ready at";
        	nGenericError = IML_LINK_ERROR;
                break;

         case IMNB_SERVER_NOT_FOUND :       /* No such Microlink  */

                szMessage = "No Microlink found at";
        	nGenericError = IML_LINK_ERROR;
                break;

         case IMNB_SESSION_ABORTED :        /* Abnormal end of session */

                szMessage = "Net connection lost to";
        	nGenericError = IML_LINK_ERROR;
                break;

         case IMNB_NETBIOS_BUSY : 	    /* NetBIOS tied up  */

                szMessage = "NetBIOS too busy -";
        	nGenericError = IML_LINK_ERROR;
                break;

         case IMNB_TOO_MANY_OPS : 	    /* No free NCBs  */

                szMessage = "Too many NetBIOS operations -";
        	nGenericError = IML_LINK_ERROR;
                break;

         case IMNB_BAD_LAN_NUM : 	    /* Requested LANA not available */

                szMessage = "LAN Number not available -";
        	nGenericError = IML_FATAL_ERROR;
                break;

         case ERR_FAILED_SEND :           /* Failed SEND kills the session */

                szMessage = "Transfer failure killed session to";
        	nGenericError = OK;
                break;

         case ERR_REPLY_ABORT :          /* App aborted a failed command */

                szMessage = "App aborted command to";
        	nGenericError = OK;
                break;

         case ERR_ESCAPE_FAILED :        /* Could not send an ESC command */

                ErrorMessageBox ( ERR_ESCAPE_FAILED );    /* Message Box */

		szMessage = "Unable to send configuration command to";
        	nGenericError = OK;
                break;

         default :      /* Some miscellaneous unexpected error */

        	nGenericError = IML_LINK_ERROR;
                break;
        }


        if ( nDevice < 0 ) {   		/* is error logging required? */

                nLogLevel = 1;          /* general error - always log */
	} else {
                nLogLevel = Microlink [ nDevice ].nErrorLogLevel;
        }


        if ( nLogLevel > 0 ) {

        	time ( &tTimeNow );

                sprintf ( szTimeDate, "%s", ctime ( &tTimeNow ) );

        	szTimeDate [ 24 ] = 0;   /* remove trailing return */

                pfLog = fopen ( szLogFile, "at" );  /* Append Text mode */

                if ( strlen ( szMessage ) > 0 ) {

                        fprintf ( pfLog,
                                  "%s - %s IML device %d\n",
                                  szTimeDate, szMessage, nDevice );

        	} else {        /* Misc error without a message */

                	fprintf ( pfLog,
                                  "%s - Error %d on IML device %d\n",
                                  szTimeDate, nProtocolError, nDevice );
                }

        	fclose ( pfLog );
	}


        return ( nGenericError );
}
