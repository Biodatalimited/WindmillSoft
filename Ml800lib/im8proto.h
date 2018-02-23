

/** Routines in Imserl800 **/

short  SendSerialCommand ( short nModule, char *lpszCommand );
short  GetSerialReplyID ( short nModule, char *lpszBuffer );
short  GetSerialReply ( short nModule, char *lpszBuffer );
short	GetSerialReplyBinary ( short nModule, int nBytes, char *lpszBuffer );
short  SendSerialCommandIDList ( short nModule, char *lpszCommand );
short	GetSerialReplyIDList ( char *lpszBuffer );

short  LogAndTranslateError ( short nModule, DWORD dwProtocolError, short nType );
DWORD MillisecondCount ( void );

/** Type codes for Log and Translate error **/
#define ERROR_INTERNAL_TYPE   1
#define ERROR_COMM_TYPE       2


/** Serial port errors not associated with a module **/
#define	GENERAL_ERROR	-1

/** Buffer size used for data transfer **/
#define MAX_BUFFER	81


/** Serial Port Parameters **/

#define	RX_QUEUE_SIZE 1024
#define	TX_QUEUE_SIZE 1024


#define MLINK_BAUD_1200    1     /* serial link at  1200 baud  */
#define MLINK_BAUD_2400    2     /*   "     "   at  2400 baud  */
#define MLINK_BAUD_4800    3     /*   "     "   at  4800 baud  */
#define MLINK_BAUD_9600    4     /*   "     "   at  9600 baud  */
#define MLINK_BAUD_19200   5     /*   "     "   at 19200 baud  */


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

#define	ERR_TX_TIMEOUT		1017	/*The data didn't go */
#define ERR_RX_TIMEOUT		1018	/*The data didn't come */
#define ERR_MTIME			1019	/*Problem with clock routines */
#define ERR_RX_OVERFLOW     1020    /*Receiver overflow*/