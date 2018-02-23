/**
***     IMNETB.H        Header file for IMNETB NetBIOS driver for IML
***
***  See IMNETB.C for details.
**/






/**  These are internal errors or events which produce detailed messages in
***  the error logging file, but then get translated into generic errors
***  like IML_FATAL_ERROR etc before being returned to COMMSLIB.
**/

#define ERR_TOO_MANY_DRIVERS    1001    /* Cannot find any name to use	 */
#define ERR_BAD_HEADER          1002    /* Block Signature or Type wrong */
#define ERR_XFER_TIMEOUT        1003    /* NetBIOS op did not complete   */
#define ERR_REPLY_ABORT         1004    /* App aborted a failed command	 */
#define ERR_FAILED_SEND         1005    /* Failed SEND kills the session */
#define ERR_ESCAPE_FAILED       1006    /* Unable to send initial ESC    */



/**  Prototype of local functions in IMNETB.C
**/

short   LogAndTranslateError ( short nDevice, short nProtocolError );
