/** COMDEF.H       IMLAPPS Communications library
***
*** COMSLIB-P3     Nigel Roberts.
***
*** This file contains the #define constants required by the modules wich make 
*** up the communications library. This include file is shorternal to the library.
***
***
*** Associated files - COMMAIN.C
***                    COMUTIL.C
***                    COMMBDP.C
***                    MSEC.C
***                    COMPROTO.H
***                    COMDEF.H
**/






#define MAX_DEVICES         32    /* 36 max supported IML devices */
#define MAX_IML_USERS       32    /* 32 Max number of IML users */
#define MAX_BDP_LINKS       4     /* max number of BDP links */



#define MAX_COMMAND_SIZE    255
#define MAX_REPLY_SIZE      1024  /* Was 255 on version 5.02 */



#define BAD_HANDLE      -1      /** value to indicate a bad file handle **/
#define BAD_LINK        -1      /* Bad BDP link number */
#define BAD_DEVICE      -1      /* bad device number */






/** DeviceInfo structs. These hold information on the Microlinks with which
*** the library can communicate.
**/

typedef struct DeviceInfoTag {

	HWND    hWndInterpreter; /* window handle of iml interpreter app.*/
	WORD    wReplyType;      /* type of reply if any */
	short   nInUseBy;        /* IML handle using this device */
	WORD    wCommandNo;      /* Command sequence number */
	char    szBuffer [ MAX_REPLY_SIZE ];    /* command/reply buffer */
	short   bReplyReceived;  /* Reply in reply buf */
	short   bExists;         /* This device exists. */      
	short   nBDPLink;        /* BDP link number */    
	short   nDevAck;         /* Set by device when it replies */

} IMLDEVICE;


typedef struct BDPLinkTag {
        short   nIMLHandle;         /** IML handle **/
        short   nDevice;            /** Device using packet **/
        short   nTerminateCode;  
        WORD    wFlags;
		char    cBuffer [8400];

} BDPLINK;



#define DEVICE_FREE             MAX_DEVICES + 100       /** well out of the
                                                        *** way of range of
                                                        *** devices.
                                                        **/


#define BDP_LINK_FREE           0x0000
#define BDP_LINK_BUSY   		0x0001
#define BDP_LINK_TERMINATED     0x0002
#define BDP_DATA_IN_BUFFER      0x0004


#define MAX_DEBUG_STRINGS 32





