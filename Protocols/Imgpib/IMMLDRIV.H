/**	IMMLDRIV.H      Header file for use with IMMLDRIV.C
***
**/



#define IDM_ABOUT               16

#define MAX_COMMAND		200
#define MAX_REPLY		350


#define IDS_ERR_REGISTER_CLASS    1
#define IDS_ERR_CREATE_WINDOW     2
#define IDS_LOADS_WHEN_NEEDED     3
#define IDS_DEV_NUMBER_MISSING    4
#define IDS_REG_MESSAGE_FAILED    5
#define IDS_ERR_GET_COMMAND       6
#define IDS_ERR_POST_REPLY        7
#define	IDS_ERR_UNREGISTER        8
#define	IDS_ERR_REG_COMMS         9
#define	IDS_CLOSE_WARNING        10
#define	IDS_TOO_MANY_DEVICES     11

#define FATAL_STARTUP_ERROR	 999		/* added for kit 2.02 */



#define BUFFER_OVERFLOW	  118	/* IML error: cannot send BDPs fast enough */
#define END_OF_DATA	  120	/* IML error: all requested data supplied  */



/**  These are internal message we can send to our own window in order to
***  get things done.
**/

#define MSG_USE_DEVICE   (WM_USER+1)    /* Add new device to our list       */
#define MSG_BDP_READY    (WM_USER+2)    /* Send Bin Data Packet to COMMSLIB */
#define MSG_NET_EVENT    (WM_USER+3)    /* Pass to NetBIOS driver           */




/**  Structure used to hold details of a Microlink
**/


typedef struct {

        short   nHardwareAddress;       /* COMx or -1 if not our device	*/
        short   nMicrolinkType;         /* 0 = RS232; 1 = RS485		*/
        short   nErrorLogLevel;         /* 0 = no log; 1 = error log	*/
        short   nRate;		        /* baud rate code   		*/
        short   nExtraInfo;             /* 0 = N protocol; 1 = C prtcol */
        char    szEscape1 [ 20 ];       /* ESC command to send to Mlink */
        char    szEscape2 [ 20 ];       /*  "     "    "   "   "    "   */
        char    szEscape3 [ 20 ];       /*  "     "    "   "   "    "   */
        char    szEscape4 [ 20 ];       /*  "     "    "   "   "    "   */

 } IMLDEVICE;



#define  NUM_DEVICES     32             /* Max device number = 32 */



/**  Values which the nMicrolinkType & nExtraInfo fields can take:
**/

#define MLINK_DEFAULT     0      /* Usual default type for this driver	*/
#define MLINK_RSI         1      /* nMicrolinkType for RS485 (RSI) 	*/

#define MLINK_CRC         1      /* nExtraInfo for CRC Protocol 	*/



/**  Values which can go into nExtraInfo field:
**/

#define MLINK_BAUD_1200    1     /* serial link at  1200 baud  */
#define MLINK_BAUD_2400    2     /*   "     "   at  2400 baud  */
#define MLINK_BAUD_4800    3     /*   "     "   at  4800 baud  */
#define MLINK_BAUD_9600    4     /*   "     "   at  9600 baud  */
#define MLINK_BAUD_19200   5     /*   "     "   at 19200 baud  */
#define MLINK_BAUD_38400   6     /*   "     "   at 38400 baud  */




/**  Prototype for utility function in IMMLDRIV.C
**/

short	ErrorMessageBox ( short nCode );



/**  Prototypes for the low-level transmit/receive functions
**/

short	OpenDriverInterface ( short *pnHandle );
short	PrepareLinkToDevice ( short nDevice );
short   ProcessTildeCommand ( short nDevice, char *szCommand,
                        				  char *szReply );
short	SendDriverCommand ( short nHandle, short nDevice,
                                        char *szCommand, WORD unFlags );
short	GetDriverReply ( short nHandle, short *nDevice,
                             char *szBuffer, short nSize, WORD unFlags );
short	AbortDriverTextReply ( short nDevice );
short	AbortDriverBDPReply  ( short nDevice );
short	GetBDPFromDriver ( WORD wParam, long lParam, LPIMLBDP *plpbdpData );
short	CloseDriverInterface ( short nHandle );
void    BackgroundProcess ( void );


/**  The following are only used by the NetBIOS driver...
**/

void    HandleNetworkEvent ( WPARAM wParam, LPARAM lParam );
void    SetBDPStatus ( short nStatus );

#define  BDP_STAT_GET_NEXT     1        /* Please get next BDP 	 	*/
#define  BDP_STAT_REPEAT       2        /* Please repeat same BDP again */
#define  BDP_STAT_ABORT        3      	/* Please abort this BDP reply  */
