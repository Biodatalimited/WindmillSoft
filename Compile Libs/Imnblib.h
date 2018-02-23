/**
***  IMNBLIB.H          Header file for IML NetBIOS Library (IMNBLIB.DLL)
***
***  (C) Copyright Windmill Software Limited, 1996. All rights reserved.
***
***  This file defines the constants and exported function prototypes
***  required to use IMNBLIB.DLL.  To be included in all source files
***  which call IMNBLIB.DLL.
***
***  History:
***
***     June 1994       Stuart Grace     Created for IMNBLIB.DLL ver 1.00
***   August 1996       Stuart Grace     Extended for IMNBLIB.DLL ver 2.00
**/





/**  Return codes
***
***  NetBIOS return codes are between 0 and 255. These are translated
***  to 3000-3255 to avoid existing error codes. Any code in this range
***  not defined below has the meaning described in NetBIOS Reference Book.
**/


#if !defined ( OK )

#define OK              0

#endif


#define IMNB_ERROR_BASE         3000

#define IMNB_BAD_BUF_SIZE      		(IMNB_ERROR_BASE + 0x01)
#define IMNB_BUFFERS_FULL      		(IMNB_ERROR_BASE + 0x02)
#define IMNB_TIMEOUT	       		(IMNB_ERROR_BASE + 0x05)
#define IMNB_DATA_INCOMPLETE   		(IMNB_ERROR_BASE + 0x06)
#define IMNB_BAD_SESSION_NUM		(IMNB_ERROR_BASE + 0x08)
#define IMNB_NO_RESOURES			(IMNB_ERROR_BASE + 0x09)
#define IMNB_SESSION_DISCONNECTED	(IMNB_ERROR_BASE + 0x0A)
#define IMNB_OP_CANCELLED			(IMNB_ERROR_BASE + 0x0B)
#define IMNB_DMA_FAILED				(IMNB_ERROR_BASE + 0x0C)
#define IMNB_NAME_IN_USE_LOCAL		(IMNB_ERROR_BASE + 0x0D)
#define IMNB_TOO_MANY_NAMES			(IMNB_ERROR_BASE + 0x0E)
#define IMNB_NAME_HAS_SESSIONS		(IMNB_ERROR_BASE + 0x0F)
#define IMNB_NAME_NOT_FOUND			(IMNB_ERROR_BASE + 0x10)
#define IMNB_TOO_MANY_SESSIONS		(IMNB_ERROR_BASE + 0x11)
#define IMNB_SERVER_NOT_READY		(IMNB_ERROR_BASE + 0x12)
#define IMNB_SERVER_NOT_FOUND		(IMNB_ERROR_BASE + 0x14)
#define IMNB_NAME_NOT_VALID			(IMNB_ERROR_BASE + 0x15)
#define IMNB_NAME_IN_USE_REMOTE		(IMNB_ERROR_BASE + 0x16)
#define IMNB_NAME_DELETED			(IMNB_ERROR_BASE + 0x17)
#define IMNB_SESSION_ABORTED		(IMNB_ERROR_BASE + 0x18)
#define IMNB_NAME_CONFLICT			(IMNB_ERROR_BASE + 0x19)
#define IMNB_INCOMPATIBLE_NODE		(IMNB_ERROR_BASE + 0x1A)
#define IMNB_NETBIOS_BUSY			(IMNB_ERROR_BASE + 0x21)
#define IMNB_TOO_MANY_OPS			(IMNB_ERROR_BASE + 0x22)
#define IMNB_BAD_LAN_NUM			(IMNB_ERROR_BASE + 0x23)
#define IMNB_ALREADY_COMPLETED		(IMNB_ERROR_BASE + 0x24)
#define IMNB_NAME_RESERVED			(IMNB_ERROR_BASE + 0x25)
#define IMNB_CANNOT_CANCEL			(IMNB_ERROR_BASE + 0x26)
#define IMNB_HARDWARE_FAULT			(IMNB_ERROR_BASE + 0x40)

/**  All error codes from 0x40 to 0xFE are Hardware or System faults
**/

#define IMNB_OP_IN_PROGRESS			(IMNB_ERROR_BASE + 0xFF)


/**  Errors within IMNBLIB itself:
**/

#define IMNB_OUT_OF_MEMORY			(IMNB_ERROR_BASE + 300)
#define IMNB_BAD_USER_NUM			(IMNB_ERROR_BASE + 301)
#define IMNB_BAD_OP_NUM				(IMNB_ERROR_BASE + 302)
#define IMNB_TOO_MANY_USERS			(IMNB_ERROR_BASE + 303)
#define IMNB_BAD_MODE				(IMNB_ERROR_BASE + 304)
#define IMNB_BAD_WINDOW				(IMNB_ERROR_BASE + 305)
#define IMNB_BAD_POINTER			(IMNB_ERROR_BASE + 306)




/**  Notification message codes
**/

#define IMNB_REGISTER      1
#define IMNB_LISTEN        2
#define IMNB_CALL          3
#define IMNB_RECEIVE       4
#define IMNB_SEND          5
#define IMNB_DISCONNECT    6
#define IMNB_DEREGISTER    7
#define IMNB_RECVDATAGRAM  8		/* version 2.00 */




/**  Special values:
**/

#define IMNB_GROUP_NAME    4096         /* used with IMNBRegisterName (Ver 2.00) */
#define IMNB_ANY_SESSION    500         /* used with IMNBReceiveData 		*/
#define BAD_OP_NUM          -99         /* Op Num if op failed to start     */
#define BAD_SESSION_NUM       0         /* Session Num if failed to connect */




/**  Function prototypes
**/

#if defined ( __cplusplus )

extern "C" {

#endif



	
/*#ifdef IMNLIB*/

	#define FNPREFIX     _declspec (dllexport)  WINAPI
/*#else
    #define FNPREFIX     _declspec (dllimport)  WINAPI 
#endif*/


short FNPREFIX IMNBRegisterName ( short nLAN, LPCSTR lpszMyName, HWND hWnd, UINT wMsg,
																				 short  *lpnOpNum );

short FNPREFIX IMNBFinishRegistration ( short nOpNum, short  *lpnNetUser );

short FNPREFIX IMNBListenForClient ( short nNetUser, LPCSTR lpszClientName, short nRecTimeOut,
 																short nSendTimeOut, short  *lpnOpNum );

short FNPREFIX IMNBFinishListen ( short nOpNum, short  *lpnSession, LPSTR lpszClientName );

short FNPREFIX IMNBCallServer ( short nNetUser, LPCSTR lpszServerName, short nRecTimeOut,
														 		short nSendTimeOut, short  *lpnOpNum );

short FNPREFIX IMNBFinishCall ( short nOpNum, short  *lpnSession );

short FNPREFIX IMNBReceiveData ( short nNetUser, short nSession, WORD wMaxSize, 
																					short *lpnOpNum );

short FNPREFIX IMNBFinishReceive ( short nOpNum, LPVOID lpvBuffer, LPWORD lpwActualSize,
																				 short  *lpnSession );

short FNPREFIX IMNBSendData ( short nNetUser, short nSession, LPVOID lpvBuffer, WORD wSize,
																					 short  *lpnOpNum );
short FNPREFIX IMNBFinishSend ( short nOpNum );

short FNPREFIX IMNBDisconnectSession ( short nNetUser, short nSession, short  *lpnOpNum );
short FNPREFIX IMNBFinishDisconnection ( short nOpNum );

short FNPREFIX IMNBRecvDatagram ( short nNetUser, WORD wMaxSize, short  *lpnOpNum );
short FNPREFIX IMNBFinishRecvDatagram ( short nOpNum, LPVOID lpvBuffer, LPWORD lpwActualSize,
																				 		LPSTR lpszSender );

short FNPREFIX IMNBTransmitDatagram ( short nNetUser, LPCSTR lpszDestName, LPVOID lpvBuffer,
 																							  WORD wSize );
short FNPREFIX IMNBCancelOp ( short nOpNum );

short FNPREFIX IMNBDeregister ( short nNetUser, short  *lpnOpNum );
short FNPREFIX IMNBFinishDeregistration ( short nOpNum );

short FNPREFIX IMNBGetVersion ( LPSTR lpszBuffer, short nSize );

short FNPREFIX IMNBSetMode ( short nMode );
short FNPREFIX IMNBGetStatus ( short nLAN, LPCSTR lpszName, LPVOID lpvBuffer, short nMaxSize );

#if defined ( __cplusplus )

}

#endif

