/**
***     IMSERL.H        Header file for IMSERL Serial driver for IML
***
***  See IMLSERL.C for details.
**/




#define RX_QUEUE_SIZE   1500    /* Windows serial recv queue size (chars) */
#define TX_QUEUE_SIZE    800    /* Windows serial xmit queue size (chars) */




/**  These are internal errors or events which produce detailed messages in
***  the error logging file, but then get translated into generic errors
***  like IML_FATAL_ERROR etc before being returned to COMMSLIB.
**/

#define ERR_COM_PORT_NUMBER     1001    /* User com port out of range	*/
#define ERR_GET_STATE           1002    /* GetCommState failed 		*/
#define ERR_SET_STATE           1003    /* SetCommState failed 		*/
#define ERR_REPLY_ABORT         1004    /* App aborted a failed command	*/
#define ERR_PKT_ADDRESS         1005    /* Packet addrsd to wrong node	*/
#define ERR_PKT_CRC	        1006    /* Packet failed CRC test	*/
#define ERR_PKT_SEQ_NUM         1007    /* Packet sequencing error	*/
#define ERR_PKT_TYPE      	1008    /* Packet of unexpected type	*/
#define ERR_MAX_RETRIES      	1009    /* Exhausted transmisn retries	*/
#define ERR_NAK_RECEIVED      	1010    /* Microlink sent NAK back	*/
#define ERR_PKT_UNFINISHED     	1011    /* Packet just stopped arriving	*/
#define ERR_ACK_TIMEOUT     	1012    /* ACK did not arrive		*/
#define ERR_WRONG_PROTOCOL     	1013    /* Attempt to use BDP on "N"	*/
#define ERR_CCO_FAILED     	1014    /* Failed to switch to "C"      */
#define ERR_CANNOT_ABORT     	1015    /* No incoming packets to abort	*/
#define ERR_PKT_REPEAT     	1016    /* Packet recd twice - ACK lost	*/




/**  States for the text reply receiver state machine:
**/

#define RXS_SLEEPING        0      	/* not looking for anything         */
#define RXS_WAIT_START      1           /* searching for start sequence     */
#define RXS_IN_TEXT         2           /* accumulating text - look for end */




/**  Return code from GetNextCommChar
**/

#define GETNEXT_OK       0       /* Retreived a char, no problem       	 */
#define GETNEXT_NONE     1       /* No chars available       		 */
#define GETNEXT_ERROR    2       /* Windows Comm driver reported problem */



/**
**/

#define MAX_PKT_SIZE       265

#define PKT_COMP_MESSAGE   (BYTE) 0xA0
#define PKT_PART_MESSAGE   (BYTE) 0x90	/* ORed with seq number */
#define PKT_END_MESSAGE	   (BYTE) 0x88	/* ORed with seq number */
#define PKT_ACK		   (BYTE) 0x18
#define PKT_NAK		   (BYTE) 0x30
#define PKT_ABORT	   (BYTE) 0x60
#define PKT_ABAK	   (BYTE) 0xC0

#define COMP_ID_CHAR	   (BYTE) 0x21	/* Destination ID for computer */

#define VALID_TYPE_BYTES   "\xA0\x90\x91\x92\x93\x94\x95\x96\x97\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F\x18\x30\x60\xC0"
#define VALID_ID_BYTES	   "!0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"


#define PACKET_TIMEOUT	   400L	 /* 400 ms max pause allowed inside packet */

#define ACK_TIMEOUT	   550L	 /* 550 ms allowed for ACK to be returned  */
                                 /*  (means baud rate must be 4800 or more)*/

#define MAX_SEND_ATTEMPTS    6   /* Try up to six times to get packet sent */

#define ABAK_TIMEOUT	   250L	 /* 250 ms allowed for ABAK to be returned */

#define MAX_ABORT_ATTEMPTS  10   /* Try up to ten times to abort transfer  */


#define MAX_BDP_SIZE       8192  /* 8K BDP is max supported */

#define MAX_ABORT_WAIT    5000L  /* Give up trying to abort after 5 secs   */


/**  Prototype of local functions in IMSERL.C
**/

short   LogAndTranslateError ( short nDevice, DWORD dwProtocolError );
char    GetFrameIDChar ( short nFrame );
