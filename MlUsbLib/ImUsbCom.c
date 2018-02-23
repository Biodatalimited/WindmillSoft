/** This file handles USB transactions for all units using mlusb.sys
*** driver. It contains :
***
***  These functions open and close the usb driver
***  int   FNPREFIX OpenUSBHandle ( int nDeviceType, short nIDCode )
***  void  FNPREFIX CloseUSBHandle ( int nHandleNum )
***
***  These functions send control messages to the driver
***  BOOL  FNPREFIX UsbVendorMessage ( int nHandleNum, BYTE * ucBuf ) 
***  BOOL  FNPREFIX UsbSetParameter ( int nHandleNum, ULONG Vendor, WORD wValue, WORD wIndex )
***
***  These functions are used for streaming
***  ULONG FNPREFIX UsbStartIsoStream ( int nHandleNum, double dScanPeriod, int nChannels, 
***									WORD wNotify, WORD wBufferSA ) 
***  ULONG FNPREFIX UsbStopIsoStream ( int nHandleNum )
***  ULONG FNPREFIX UsbReadIsoch ( int nHandleNum, char* cBuffer, ULONG  ulGetBytes, 
***					                ULONG* ptulReadBytes )
***
***   These functions are used for 3304
***   BOOL FNPREFIX UsbSAListen ( int nHandleNum, int nSecAdd, char* cData, int nBytesToSend ) 
***    int FNPREFIX UsbSATalk ( int nHandleNum, int nSecAdd, char* cReply, int nBytesToGet ) 
***
***   The directory also contains UsbBdp.c which should be used with all
***   streaming handlers. It makes calls to the dll.
***
*** We can handle up to 8 different devices.
***
***   Version
***   1.0 BGO 18/2/03   Initial version. Had a problem with the release
***                     compile which defaulted to max speed. Changed to
***                     Default otherwise it locked the computer.
**/


#include <windows.h>
#include <setupapi.h>
#include <devioctl.h>
#include <basetyps.h>
#include <usbdi.h>


#include "mlusblib.h"
#include "GUIDISO.h"

/* This is same for all units */
#define VR_STREAM 0xBF
#define VR_LISTEN 0xB0
#define VR_TALK   0xB1


#define MAX_UNITS 8

HANDLE hDev [MAX_UNITS] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, 
							INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
                            INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
							INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };  /** The handle to the usb device **/
int		nMyDeviceType [MAX_UNITS];
short	nMyIDCode [MAX_UNITS];
BOOL	bUSBError [MAX_UNITS] = { TRUE, TRUE, TRUE, TRUE,
								  TRUE, TRUE, TRUE, TRUE } ;

// Local Function Prototypes
BOOL OpenDevice(
    IN       HDEVINFO                    HardwareDeviceInfo,
    IN       PSP_INTERFACE_DEVICE_DATA   DeviceInfoData,
	IN       int nHandleNum,
	IN       int nDevice,
	IN		 short nIDCode
    );
BOOL CreateDeviceFile ( int nHandleNum, int nDeviceType, short nIDCode  );

BOOL ReOpenUSBDriver ( int nHandleNum );


/** Opens a handle to the usb device.
***
*** nDevice is 751 etc
*** nIDcode is Device address 0 - 7 for 751
***
*** returns -1 for error
*** number 0 to 7 if OK
***/

int FNPREFIX OpenUSBHandle ( int nDeviceType, short nIDCode )

{
	int nIndex;

    /** Find an unused one **/
	for ( nIndex = 0; nIndex < MAX_UNITS; nIndex++ ) {
		if ( hDev[nIndex] == INVALID_HANDLE_VALUE) {
			break;
		}
	}

	if ( nIndex == 8 ) {
		return ( -1 );
	}

	if ( CreateDeviceFile ( nIndex, nDeviceType, nIDCode ) ) {
		return ( -1 );
	}

	/** Store these for the reopen function **/
	nMyDeviceType [nIndex] = nDeviceType;
	nMyIDCode [nIndex] = nIDCode;

	return ( nIndex );
}


/****************************************************
*** CreateDeviceFile ()
*** This function attempts to get hDev, a handle to the
*** USB driver. To do this it first gets hardwareDeviceInfo - an 
*** array of info on all devices with the correct GUID. Then for
*** each device in this array it gets the deviceInfoData.
*** These 2 data sets are then used to call OpenDevice for each
*** device in the array. If this succeeds in getting a valid hDev
*** we return FALSE. If it fails we return TRUE. To be valid a
*** device must have the correct nDevice type eg 751 and ID Code. 
**/

BOOL CreateDeviceFile ( int nHandleNum, int nDevice, short nIDCode  )

{
	HDEVINFO                 hardwareDeviceInfo;
	SP_INTERFACE_DEVICE_DATA deviceInfoData;
	ULONG                    ulIndex;
	BOOLEAN                  bReturn = TRUE;
    LPGUID pGuid = (LPGUID)&GUID_ML751;
    PSP_INTERFACE_DEVICE_DETAIL_DATA     functionClassDeviceData = NULL;
    ULONG   predictedLength = 0;
    ULONG    requiredLength = 0;
	



	/** Open a handle to the plug and play dev node.
	*** SetupDiGetClassDevs() returns a device information set that contains 
	*** info on all installed devices of a specified class.
	**/

	hardwareDeviceInfo = SetupDiGetClassDevs (
                           (LPGUID)&GUID_ML751,
                           NULL, // Define no enumerator (global)
                           NULL, // Define no
                           (DIGCF_PRESENT | // Only Devices present
                            DIGCF_INTERFACEDEVICE)); // Function class devices.
  
	
    /** Look at each unit in turn **/
	for ( ulIndex = 0; ulIndex < MAX_UNITS; ulIndex++) {

		/** SetupDiEnumDeviceInterfaces() returns information about device 
        *** interfaces exposed by one or more devices. Each call returns 
        *** information about one interface. The routine can be called 
        *** repeatedly to get information about several interfaces 
		*** exposed by one or more devices.
		**/
	    deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);
        
		// Get the data handle for this device
		if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                         0, // We don't care about specific PDOs
										 pGuid,
                                         ulIndex,
                                         &deviceInfoData)) {
		
			    
             // Try to open the device
			if ( OpenDevice (hardwareDeviceInfo,&deviceInfoData,
				             nHandleNum,nDevice,nIDCode) == FALSE ) {
               	/** We have opened the device
                *** hDev has been obtained.
				**/
				bReturn = FALSE;
				break;
			}
        } else {
            // Error in getting data handle
			if (ERROR_NO_MORE_ITEMS == GetLastError()) {
				break;
            }
        }
	}
   


	/** SetupDiDestroyDeviceInfoList() destroys a device information set
	**/
	SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);

	return ( bReturn );


}


/*****************************************************
Routine Description:

    Given the HardwareDeviceInfo, representing a handle to the plug and
    play information, and deviceInfoData, representing a specific usb device,
    we can now get the device path and use it in a CreateFile call. 
	If this succeeds we get hDev the handle for addressing the device.
	We use this to read the ID Code ( stored in the serial number string
	of the USB device). If it agrees with the passed in nIDCode then we
    have found the desired device. Simply return FALSE. If it does not
	agree then close the device again and return TRUE.

Arguments:

    HardwareDeviceInfo:  handle to info obtained from Pnp mgr via SetupDiGetClassDevs()
    DeviceInfoData:      ptr to info obtained via SetupDiEnumInterfaceDevice()
	nHandleNum           The index into the handle array
    nDevice              The device type
    nIDCode:             The ID code to look for in the device serial number

Return Value:

    return FALSE if we have a valid handle,
	       TRUE if we have an error.

--*/

BOOL OpenDevice(
    IN       HDEVINFO                    HardwareDeviceInfo,
    IN       PSP_INTERFACE_DEVICE_DATA   DeviceInfoData,
	IN       int nHandleNum,
	IN       int nDevice,
	IN		 short nIDCode
    )


{
    PSP_INTERFACE_DEVICE_DETAIL_DATA     functionClassDeviceData = NULL;
    ULONG                                predictedLength = 0;
    ULONG                                requiredLength = 0;
	HANDLE								 hOut = INVALID_HANDLE_VALUE;
    BYTE ucBuf[8] = {0,0,0,0,0,0,0,0};
    int nBytes;

    //
    // allocate a function class device data structure to receive the
    // goods about this particular device.
    //
    SetupDiGetInterfaceDeviceDetail (
            HardwareDeviceInfo,
            DeviceInfoData,
            NULL, // probing so no output buffer yet
            0, // probing so output buffer length of zero
            &requiredLength,
            NULL); // not interested in the specific dev-node
	

    predictedLength = requiredLength;

    functionClassDeviceData = malloc (predictedLength);
    functionClassDeviceData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);

    //
    // Retrieve the information from Plug and Play.
    //
    if (! SetupDiGetInterfaceDeviceDetail (
               HardwareDeviceInfo,
               DeviceInfoData,
               functionClassDeviceData,
               predictedLength,
               &requiredLength,
               NULL)) {
		/** If we get an error free the memory and return **/
		free( functionClassDeviceData );
        return (TRUE);
    }

    
	hDev [nHandleNum] = CreateFile (
                  functionClassDeviceData->DevicePath,
                  GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  0, // No special attributes
                  NULL); // No template file


    /** Free the memory **/
	free( functionClassDeviceData );
	
	if (INVALID_HANDLE_VALUE == hDev) {
		return (TRUE);
    }
	
		/** Now we read the device type string **/
	ucBuf[0]= 2;   //Device Type string at 2
	
	DeviceIoControl(hDev [nHandleNum],
				IOCTL_MLUSB_GET_STRING,
				ucBuf,
				8,
				ucBuf,
				16, // Need enough space for whole string
				&nBytes,
				NULL);

	/** String should be "75X Unit" **/
    if ( atoi (ucBuf) != nDevice ) {
			//It's not the correct unit so tidy up and leave
		CloseUSBHandle ( nHandleNum );
		return (TRUE);
	}

	/** Now we read the serial number string **/
	ucBuf[0]= 3;   //Serial Number string at 3
	
	DeviceIoControl(hDev [nHandleNum],
				IOCTL_MLUSB_GET_STRING,
				ucBuf,
				8,
				ucBuf,
				8,
				&nBytes,
				NULL);

    if ( (ucBuf[0] == 'I') &&
		 (ucBuf[1] == 'D') &&
		 (ucBuf[2] == nIDCode + 48 ) ) {
		/** We have found the right serial number
		*** Return success
		**/
		bUSBError [nHandleNum] = FALSE;
		return (FALSE);
	}
     
	/** The serial number must be wrong. So close the device
	*** and return failure.
	**/
	CloseUSBHandle ( nHandleNum );
	return (TRUE);

}


/** This routine sends out an 8 byte Vendor Defined message and receives
*** An 8 byte reply. The format of messages in both directions is determined
*** by the underlying driver. In general the messages reflect 8 byte packets
*** moving across USB. In the sent out message several bytes are used for 
*** internal USB purposes and should be simply set to 0 here. The general
*** arrangement is
*** Byte 0 Reserved
*** Byte 1 Defines the Function of the call
*** Bytes 2 - 5 Data
*** Bytes 6,7 Reserved
***
*** The reply message is entirely driver defined.
*** The routine returns TRUE in case of error, FALSE otherwise.
***
*** This is retained so that the 750\751 code needs no change
**/ 


BOOL FNPREFIX UsbVendorMessage ( int nHandleNum, BYTE * ucBuf ) 

{

	int nBytes;
	int nStatus;
	int nTries = 2;


	while ( nTries > 0 ) {
		/** If there was a previous USB error then attempt to
		*** reopen the device. If need be close the handle first.
		*** This allows for hot plug and unplug
		**/
		if ( bUSBError [nHandleNum]) {				
			if ( ReOpenUSBDriver (nHandleNum) ) {
				return ( TRUE );
			}
		}


		nStatus = DeviceIoControl(hDev [ nHandleNum ] ,
						IOCTL_MLUSB_VENDOR_8,
						ucBuf,
						8,
						ucBuf,
						8,
						&nBytes,
						NULL);	

		if ( ( nStatus =! 0 ) && ( nBytes == 8 ) ){
			bUSBError [nHandleNum] = FALSE;
			return ( FALSE );
		}
		
		bUSBError [nHandleNum] = TRUE;
		--nTries;

	}	

	return ( TRUE );

}

/*************************************************************/
/** This routine generates a Vendor request with the Value WORD of
*** the control packet set to wValue and the Index WORD set to wIndex.
**/

BOOL FNPREFIX UsbSetParameter ( int nHandleNum, ULONG Vendor, WORD wValue, WORD wIndex )

{
	WORD wBuf [4];

	/** Vendor request is in second byte **/
	wBuf [0] = (WORD) (Vendor * 256);
	wBuf [1] = wValue;
	wBuf [2] = wIndex;
	wBuf [3] = 0;

	return ( UsbVendorMessage ( nHandleNum, (BYTE* ) wBuf ) );

}

/***************************************************/
/** Isochronous streams work like this :
*** 1 Every millisecond the hardware sends a packet of data over USB
*** 2 The mlusb driver asks the class driver for 50 such packets
*** 3 The class driver calls back mlusb when it has the packets
*** 4 The data is copied from them into a software FIFO
*** 5 The handler reads the fifo with UsbReadIsoch
*** 6 The mlusb driver queues data requests (URBs) to the class driver.
***   When one finishes another starts.
***
*** Because we specify 50 packets then new data becomes available to the
*** handler every 50 milliseconds. We can queue more than 2 URBs but there
*** seems no point in doing so.
***
*** The fifo is 5 URBs long ie 5 * 50 milliseconds if all packets are
*** full. The handler must service it more frequently than that. 
*** 
*** This routine takes dScanPeriod, the time between scans in usecs, and
*** the number of channels in the scan. From these we can calculate a suitable
*** packet size.
*** We are also given the notify level which we pass to the driver. We
*** will not be supplied with any data less than this.
*** The secondary address of the buffer is used by 3304.
***
*** Returns a number which details the exact error see MLUSB.sys
**/

ULONG FNPREFIX UsbStartIsoStream ( int nHandleNum, double dScanPeriod, int nChannels, 
									WORD wNotify, WORD wBufferSA ) 
{
	WORD wBuf [4];
	ULONG nBytes;
	double dPacketSize;
	WORD wPacketSize = 1;

    /** Calculate the number of samples per millisecond **/
	dPacketSize = (1000 * nChannels) / dScanPeriod;

	/** Allow 20% more than needed **/
	dPacketSize = dPacketSize * 1.2;

	/** Find the next multiple of 2 above dPacketSize **/
	while ( wPacketSize < dPacketSize ) {
		wPacketSize = wPacketSize * 2;
	}

	/** limit of 128 samples per millisecond **/
	if ( wPacketSize > 128 ) {
		wPacketSize = 128;
	}


	/** The packets within the usb driver must hold data + copy + 2 byte header.
	*** They also must be binary for some usb implementations.
	*** Use 128 as a minimum this means that 16 samples ie 66 bytes max are OK.
	*** 32 or more need extra memory and the space must be left for the 2 byte
	*** header.
	**/

	if ( wPacketSize < 32 ) {
		wBuf[0] = 128;
	} else {
		wBuf[0] = 4 * wPacketSize;
		wPacketSize = wPacketSize - 4;
	}


	/** Now start the streaming driver **/
	wBuf [1] = 50;		/* Packets per buffer */
	wBuf [2] = 2;		/* Number of buffers */
	wBuf [3] = wNotify;

	DeviceIoControl(hDev [nHandleNum],
					IOCTL_MLUSB_START_ISO_STREAM,
				    wBuf,
					8,
					wBuf,
					8,
					&nBytes,
					NULL);


	/** Tell the hardware about the packet size and for 3304
	*** give it the SA of the buffer.
	**/
	UsbSetParameter ( nHandleNum, VR_STREAM, wBufferSA, wPacketSize );

	/** Error Code is in nBytes. It is defined in Mlusb **/
	return ( nBytes );
}


ULONG FNPREFIX UsbStopIsoStream ( int nHandleNum )

{
	BYTE cBuf [8];
	ULONG nBytes;

	/** Tell the hardware to stop streaming. Added to stop Dells crashing **/
	UsbSetParameter ( nHandleNum, VR_STREAM, 0, 0 );


	/** Stop the driver streaming **/
	DeviceIoControl(hDev [nHandleNum],
					IOCTL_MLUSB_STOP_ISO_STREAM,
				    cBuf,
					8,
					cBuf,
					8,
					&nBytes,
					NULL)  ;

	/** Tell the hardware to stop streaming **/
	UsbSetParameter ( nHandleNum, VR_STREAM, 0, 0 );

	return ( 0 ) ;

}
/******************************************************/
/** This reads a maximum of ulGetBytes into cBuffer. The
*** number of bytes returned is indicated in ulReadBytes.
*** This will either be 0 or greater than the notify level.
*** It returns any error.
**/

ULONG FNPREFIX UsbReadIsoch ( int nHandleNum, char* cBuffer, ULONG  ulGetBytes, 
					                ULONG* ptulReadBytes )

{
	return ( DeviceIoControl ( hDev [nHandleNum],
		            IOCTL_MLUSB_READ_ISO_BUFFER,
				    cBuffer,
					8,
					cBuffer,
					ulGetBytes,
					ptulReadBytes,
					NULL )  ) ;           
	
}



/******************************************************/
/** In 3304 this routine reads data from the specified SA
*** It returns nBytesToGet of data in cReply
*** 
*** The routine returns the number of bytes read.
**/ 


int FNPREFIX UsbSATalk ( int nHandleNum, int nSecAdd, char* cReply, int nBytesToGet ) 

{

	int nStatus;
	int nBytes;
	VENDOR sVendor;
	int	nTries = 2;

	while ( nTries > 0 ) {

		/** If there was a previous USB error then attempt to
		*** reopen the device. If need be close the handle first.
		*** This allows for hot plug and unplug
		**/
		if ( bUSBError [nHandleNum] ) {				
			if ( ReOpenUSBDriver (nHandleNum) ) {
				return ( 0 );
			}
		}

		/** First byte indicates an IN transaction **/
		sVendor.ucDirection = (BYTE) 128;
		sVendor.ucVendRequest = VR_TALK;
		sVendor.wValue = (unsigned short) nSecAdd;
		sVendor.wIndex = 0;
		sVendor.wBytes = (unsigned short) nBytesToGet;

		nStatus = DeviceIoControl(hDev [nHandleNum],
						IOCTL_MLUSB_VENDOR,
						&sVendor,
						8,
						cReply,
						nBytesToGet,
						&nBytes,
						NULL);	

		if ( nStatus =! 0 ) {
			bUSBError [ nHandleNum] = FALSE;
			return ( nBytes );
		}

		bUSBError [ nHandleNum ] = TRUE ;
		-- nTries;
	
	}

	return ( nBytes );

}


/** In 3304 this routine writes nBytesToSend of data contained in
*** cData to the specified secondary address.
*** The maximum message is 80 bytes long.
*** 
*** The routine returns TRUE in case of error.
**/ 

BOOL FNPREFIX UsbSAListen ( int nHandleNum, int nSecAdd, char* cData, int nBytesToSend ) 

{

	int nStatus;
	int nBytes;
	char ucBuf [88];
	PVENDOR pVend;
	int nTries = 2;


	while ( nTries > 0 ) {
		/** If there was a previous USB error then attempt to
		*** reopen the device. If need be close the handle first.
		*** This allows for hot plug and unplug
		**/
		if ( bUSBError [nHandleNum] ) {
				
			if ( ReOpenUSBDriver (nHandleNum) ) {
			return ( TRUE );
			}	
		}

		pVend = (PVENDOR) ucBuf;

		/** First byte indicates an OUT transaction **/
		pVend->ucDirection = 0;
		pVend->ucVendRequest = VR_LISTEN;
		pVend->wValue = nSecAdd;
		pVend->wIndex = 0;
		pVend->wBytes = (unsigned short) nBytesToSend;

		/** Copy the data into ucBuf **/
		memcpy ( ucBuf + 8, cData, nBytesToSend );

		nStatus = DeviceIoControl(hDev [nHandleNum],
						IOCTL_MLUSB_VENDOR,
						ucBuf,
						nBytesToSend + 8,
						NULL,
						0,
						&nBytes,
						NULL);	

		if ( nStatus =! 0 ) {
			bUSBError [nHandleNum] = FALSE;
			return ( FALSE );
		}
		--nTries;
		bUSBError [nHandleNum] = TRUE;
	}
    
	return ( TRUE );

}

void FNPREFIX CloseUSBHandle ( int nHandleNum )

{

	if ( hDev [ nHandleNum ] == INVALID_HANDLE_VALUE) {
		return;
	}
	CloseHandle ( hDev [nHandleNum] );
	hDev [nHandleNum] = INVALID_HANDLE_VALUE;
}


BOOL ReOpenUSBDriver ( int nHandleNum )

{

	CloseUSBHandle ( nHandleNum );
	
	return ( CreateDeviceFile ( nHandleNum, nMyDeviceType [nHandleNum],
		nMyIDCode [nHandleNum] ) ) ;
		return ( TRUE);
}