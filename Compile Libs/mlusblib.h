/***

Module Name:

    MlUsb.h

Abstract:

    Driver-defined special IOCTL's 
	Plus prototypes for ImUsbCom.c

Environment:

    Kernel & user mode

**/

#include "\Windmill 32\Compile libs\imlinter.h"
#ifndef MLUSBH_INC
#define MLUSBH_INC

#define MLUSB_IOCTL_INDEX  0x0000


#define IOCTL_MLUSB_GET_CONFIG_DESCRIPTOR     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   MLUSB_IOCTL_INDEX,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
                                                   
#define IOCTL_MLUSB_RESET_DEVICE   CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   MLUSB_IOCTL_INDEX+1,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)                                                              
                                                   
#define IOCTL_MLUSB_RESET_PIPE  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   MLUSB_IOCTL_INDEX+2,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS) 
                                                          
#define IOCTL_MLUSB_STOP_ISO_STREAM     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   MLUSB_IOCTL_INDEX+3,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_MLUSB_START_ISO_STREAM     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   MLUSB_IOCTL_INDEX+4,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)                                                   

#define IOCTL_MLUSB_GET_DEVICE_DESCRIPTOR     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   MLUSB_IOCTL_INDEX+5,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
#define IOCTL_MLUSB_GET_STRING   CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   MLUSB_IOCTL_INDEX+6,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_MLUSB_VENDOR_8	CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                   MLUSB_IOCTL_INDEX+7,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
#define IOCTL_MLUSB_VENDOR	CTL_CODE(FILE_DEVICE_UNKNOWN, \
                                                   MLUSB_IOCTL_INDEX+8,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_MLUSB_READ_ISO_BUFFER     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   MLUSB_IOCTL_INDEX+12,\
                                                   METHOD_OUT_DIRECT,  \
                                                   FILE_ANY_ACCESS)


#endif // end, #ifndef MLUSBH_INC


/** Structure for use with vendor request calls **/
typedef struct VendorHeader {
	BYTE ucDirection;          //Defines in or out
	BYTE ucVendRequest;        //The Vendor request code
	WORD wValue;               //User Defined value
	WORD wIndex;               //User Defined Index
	WORD wBytes;               //Bytes to send or get
} VENDOR, *PVENDOR;


#define FNPREFIX	_declspec (dllexport) WINAPI


int  FNPREFIX OpenUSBHandle ( int nDevice, short nIDCode );
void FNPREFIX CloseUSBHandle ( int nHandleNum );


/** Control read and write **/
BOOL FNPREFIX UsbVendorMessage ( int nHandleNum, BYTE * ucBuf );
int  FNPREFIX UsbSATalk ( int nHandleNum, int nSecAdd, char* cReply, int nBytesToGet ) ;
BOOL FNPREFIX UsbSAListen ( int nHandleNum, int nSecAdd, char* cData, int nBytesToSend );  
BOOL FNPREFIX UsbSetParameter ( int nHandleNum, ULONG Vendor, WORD wValue, WORD wIndex );


/** Streaming functions **/
ULONG FNPREFIX UsbReadIsoch ( int nHandleNum, char* cBuffer, ULONG  ulGetBytes, 
					                ULONG* ptulReadBytes );
ULONG FNPREFIX UsbStartIsoStream ( int nHandleNum, double dScanPeriod, int nChannels, 
									WORD wNotify, WORD wBufferSA ); 
ULONG FNPREFIX UsbStopIsoStream ( int nHandleNum );

/** BDP functions **/
void  BDP_ReadDataFromUSB ( void );
short  BDP_Initialise ( short nMinPac, short nMaxPac,
					 WORD hWnd, WORD wMsg );
short  BDP_DespatchDataPacket ( LPIMLBDP fpBDP );



