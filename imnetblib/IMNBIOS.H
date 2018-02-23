/**
***  IMNBIOS.H          NetBIOS header for use in IMNBLIB.DLL
***
***
***  This file defines the NetBIOS Network Control Block (NCB)
***  and the NCB command codes used for all NetBIOS interactions.
***
***
***  History:
***
***     June 1994    Stuart Grace    Created for IMNBLIB.DLL ver 1.00
***   August 1996    Stuart Grace    Extended for IMNBLIB.DLL ver 2.00
**/





#define MAX_USERS       20      /* Allow upto 20 simultaneous users */

#define MAX_OPS         50      /* Allow upto 50 simultaneous pending NCBs */


#pragma pack( 1 )		/* Ensure structures a aligned on single byte boundries */


typedef struct tagNetUser {

    short	nStatus;	      /* current status of this entry		*/
    short	nLanAdapter;	  /* LANA		    		   			*/
    char	szName [ 17 ];	  /* NetBIOS name (padded with spaces)	*/
    short	nNameNumber;	  /* number for our NetBIOS name		*/
    HWND	hWndNotify;	      /* hWnd to send notify msg to			*/
    UINT	wNotifyMsg;	      /* value of msg to send				*/

} NETUSER;



/**  Values for nStatus field of NETUSER structure:
**/

#define USER_FREE               0       /* not in use 					*/
#define USER_REGISTERING        1       /* Registration in progress 	*/
#define USER_ACTIVE             2       /* Fully registered 			*/
#define USER_UNREGISTERING      3       /* In process of deregistering 	*/





/**  Network Control Block structure
***
**/


#define NCBNAMESIZE	   16	  /* Exact length of a NetBIOS name	  	 */
                              /* (padded with spaces - no zero byte) */

#define NUM_BYTES_IN_NCB   64     /* Size of standard NCB block */




typedef struct tagNetOp {
                                /* These are our private fields:         */

    short		nUser;	      	/* Index into NetUser array of owner     */
    WORD        wParam;         /* type of op - for wParam of notify msg */

                                /* Here is a standard NetBIOS N.C.B.:    */
                                /*   (do not change any of this)         */

    unsigned char   ncb_command;	    		/* command code		    	*/
    unsigned char   ncb_retcode;	    		/* return code		    	*/
    unsigned char   ncb_lsn;		    		/* local session number	    */
    unsigned char   ncb_num;		    		/* number of our network name */
    char   *	    ncb_buffer; 	       		/* address of data buffer */
    unsigned short  ncb_length; 	        	  /* size of data buffer    */
    char	    	ncb_callname [ NCBNAMESIZE ]; /* remote netname  	    */
    char	    	ncb_name [ NCBNAMESIZE ];     /* our netname  	    	*/
    unsigned char   ncb_rto;		          	  /* recv timeout count     */
    unsigned char   ncb_sto;		          	/* send timeout count     */
    unsigned long   ncb_post;		        	/* Async notification ptr   */
    unsigned char   ncb_lana_num;	        	/* LAN Adapter number       */
    unsigned char   ncb_cmd_cplt;	        	/* pending/complete flag    */
    unsigned char   ncb_reserve [ 14 ];     	/* reserved, used by BIOS   */

} NETOP;



#define OP_NOT_IN_USE	-1      /* NetOp.nUser value for available op */




/**
***	NCB Command codes
***
***  One of these must go in the ncb_command field of every NCB.
***
***  NOTE: This is not the full set: certain obscure commands are missing.
**/

#define NCB_ASYNCH		0x80		/* high bit set == asynchronous   */

#define NCB_CALL 		0x10		/* NCB CALL				  */
#define NCB_LISTEN		0x11		/* NCB LISTEN			  */
#define NCB_HANGUP		0x12		/* NCB HANG UP			  */
#define NCB_SEND 		0x14		/* NCB SEND				  */
#define NCB_RECV 		0x15		/* NCB RECEIVE			  */
#define NCB_RECVANY		0x16		/* NCB RECEIVE ANY		  */
#define NCB_SENDDG		0x20		/* NCB SEND DATAGRAM	  */
#define NCB_RECVDG		0x21		/* NCB RECEIVE DATAGRAM	  */
#define NCB_ADDNAME		0x30		/* NCB ADD NAME 		  */
#define NCB_DELNAME		0x31		/* NCB DELETE NAME		  */
#define NCB_ASTAT		0x33		/* NCB ADAPTER STATUS	  */
#define NCB_SESSSTAT	0x34		/* NCB SESSION STATUS	  */
#define NCB_CANCEL		0x35		/* NCB CANCEL			  */
#define NCB_ADDGRNAME	0x36		/* NCB ADD GROUP NAME	  */


#pragma pack( )		/* Default structure alignment */

