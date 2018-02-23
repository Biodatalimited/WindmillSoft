/** COMMAIN.C       IMLAPPS Communications Library.
***
*** COMSLIB-P2      Nigel Roberts.
***
*** This is the main module of the communications Dynamic Link Library.
*** It contains the entry points of the library. These are the initialisation
*** routine, and the routines accessible to an application.
***
*** Associated Files are:-
***	COMUTIL.C    - Assignment and checking channel numbers.
***	COMMBDP.C    - Binary packet transfer
***	MSEC.C       - Millisecond Timer
***	COMDEF.H     - #defined constants - library specific
***	COMPROTO.H   - Library specific function prototypes
***
*** This library is compiled using Microsoft C6, nMake and LINK.
***
***
*** VERSION HISTORY.
***
*** 20 March 1990 	IMLAPPS-COMMSLIB-P1.0 	- Inception.
***
***
*** 1.01, 1.02 - Alpha test versions 
***
*** 25 May, 1990. 1.03 - All ints changed to shorts for portability reasons.
*** 		No other changes to the code.
***
*** 12 June, 1990. 1.04 - When end of reply received then the reply status is 
***		reset to no reply expected in GetMicrolinkReply().
***
*** 21 June, 1990. 1.05 - Fixed properly what was supposed to be fixed in 1.04.
***
*** 21 August, 1990. 1.06 - 
***	Upgraded to Windows 3.
***	Changed to use the GPIBLIB DLL
***	Request/ReleaseMicrolinkComms () - argument added to indicate which 
*** 	frame is being requested.
***
*** 13 September, 1990. 1.07 - Changed function StateOfCommsPort () to be more
***	use to the test harness. 
***	Fixed a bug whereby if request one frame/port and then send a command
***	to another get Out of memory as we tried to lock the message buffer
***	for the wrong port which had not been alloc'ed.
***
*** 	One application can request several frames at once. However it is upto
***	the application to manage command/reply pairs. I.e. it must not send
***	a command with a reply while it is still awaiting a reply.
***
***	The Message buffer is now one per frame not one per port as before.
***	This is to allow more than one frame to be requested at any one time.
***
***	The nStatus field in the Microlink struct is now used as a semaphore
***	It can be set to :- NO_DEVICE - not used/available.
***	SEM_FREE - present but unrequested; Channel No - number of the 
***	channel which requested it.
***	We have also added a field to the port struct - nLockCount - This is
***	a count of the times that this port has been requested by one channel
***	so we can free a frame and only free the port if the lock count is
***	zero - To allow one app to request multiple frames and release them 
***	again one at a time.
***
*** 18 September, 1990 - 1.08.
***	Added frame number to be dumped to file in Request/Release/SendCommand
***	functions.
***
*** 20 September, 1990 - 1.09.
***	Open serial port () changed because BuildCommDCB stopped working. As
***	a result the Baud rate, parity, data bits etc are now hard coded t
***	9600, non, and 8 respectively. THIS WILL NEED CHANGING BEFORE 
***	RELEASE.
***
*** 08 October, 1990 - 1.10.
***	Added new debugging mode. Mode 2. Dumps all serial reads/writes to file
***	szDumpFile. Dumping implemented in commess.
***  
***	Fixed a bug in BuildSerialReply (). If got part reply then it used to
***	loose it or if got address as last char of one read block it did
***	not remember to look for : only at start of next.
***
*** 17 October, 1990 - 1.11
***	In request the range of the address requested is now checked 
***
*** 02 November, 1990 - 1.12
***	In build GPIB reply we allocate slightly larger blocks than we neen to
***	allow us to zero terminate strings. - We did thjis fore serial replies 
***	sometime previously.
***
*** 29 November, 1990 - 1.13
***	Added BaudRate to port structure so we can again set the baud rate
***	from the win.ini file. Variable set when we Get the info from the
***	Win.ini.
***
***
*** 07 December, 1990 - 1.14
***	Changed GetGPIBReply (). We now remove the \n at the end of the reply
***	- the character received with EOI - if we get transfer stopped due to
***	EOI.
***
***
*** 12 December, 1990 - 1.15
***	Added a new function in commess to check the free space in the
***	transmit queue before sending data as WriteComm overwrites data in
***	the buffer if there is insufficient space.
***
***
*** 14 December, 1990 - 1.16
***	Totally rewritten BuildSerialReply () to fix a timing dependant bug
***	which meant we sometimes missed the end of reply caussing us to
***	timeout without realising we had actually received all the reply.
***
***
*** 09 January, 1991 - 1.17
***	Changed WEP so that on exit we don't bother doing any tidying. If we
***	do and we are about to exit windows we knacker things and windows
***	does not exit properly hanging the machine.
***	Fixed SendGPIBCommand () - increased the timeout and we now get the
***	right GPIB completion code if we timeout.
***	Changed MAX_PORTS to 5 so we can now handle Com3 and 4.
***
***
*** 21 January, 1991 - 1.18
***	Lenghtened the timeouts on GPIB send and receive command / reply.
***	Send command is 2.5 secs and get reply is 5 secs. We had problems
***	with timeouts when downloading large setups from imslib.
***
***
*** 26 Febuary, 1991 - 1.19
***	Enabled interrupts when opening GPIB drivers.
***	Check for terminating character (CR) in get GPIB reply.
***
*** 18 March, 1991 - 1.20
***	In release we now use GlobalFlags to see how many times to unlock
***	ahMessageBuffer [ ].
***
*** 5 April, 1991
***	Changed Version function to be nVer = Version(LPSTR lpzsVer, nChars).
***
*** 23 April, 1991 - 1.21
***	Implemented mechanism to communicate with Windows IML interpreters.
***	These are treated as another port type. Max Ports is now 6. Port 5
***	is the route to windows IML interpreters.
***
*** 10 May, 1991 - 1.22
***	GetInfoFromWININI now also gets info on WINAPPS and Microlinks from
***	IML.INI.
***
*** 13 May, 1991 - 1.23
***	Always use SendMessage to send stuff to Winapps not postmessage.
***
*** 17 June, 1991 - 1.24
***	Increased MAX_DEVICES to 36 from 10 - should have done this
***	before.
***
*** 20 June, 1991 - 1.25
***	If a Winapp has been closed we now restart it when we open the
***	first channel.
***
*** 26 June, 1991 - 1.26
***	We now only look in IML.INI for Devices. We do not look for Microlink
***	sections in WIN.INI or IML.INI.
***
***__________________________________________________________________________
***--------------------------------------------------------------------------
***
***
*** 27 September, 1991 - 2.0
***     Added the generic IML interface including BDP support.
***
***
*** 23 October, 1991 - 2.01
***     Fixed one or two bugs in the generic interface.
***
*** 27 Febuary, 1992 - 2.02
***     Added version stamp info in RC file.
***
*** 03 March, 1992 - 2.03
***     Added sending DEL command at start of each serial transmission for
***     Dave for use with 3303 rs485 modules
***
*** 26 March, 1992 - 2.04
***     Added 3303 support. I.e. treat this as a separate serial device and
***     do the adition of dels for this device only.
***
*** 17 July, 1992 - 2.05
***     As for 3303 we have added support for 1580 control boards.
***
***
*** 22 September, 1992 - 2.06
***     Windows 3.1 effectively closes the serial port when the opening
***     application exits. Ths leaves any other apps who want to use
***     the port up shit creak. We get round this by execing an application
***     which calls us back so we can open the serial ports then. This
***     means that the ports 'belong' to the app we exec'ed. It gives us
***     its window handle so we can post it a message to kill it - freeing
***     the ports when our open count hits zero.
***
*** 25 November, 1992 - 2.07
***     Fixed some problems with the commopener.
***     Added new generic function AbortIMLReply.
***
*** 27 November, 1992 - 2.08
***     Removed unnecessary global allocs for GPIB stuff and tydied up
***     Global alloc, realloc and compact - checking for null and compacting
***     to required size - not -1.
***
***
***     2.09 - compiled debug looking for bugs.
***
*** 13 May, 1993 - 2.10
***     Changed get reply so it does not return partial serial replies.
***     Returns the lot when the end received. Put it back again.
***     Shortened all the dump strings.
***
*** 11 June, 1993 - 2.11
***     Fixed bug in GetIMLReply which appeared to cause UEA with scan
***     doing group reads on serial systems. String terminator lost while
***     shuffling buffers.
***
*** 23 June, 1993 - 2.12
***     BDP link stuff. While we have a BDP link active on a device we
***     must lock the device so it cannot be used until the BDP link has
***     been terminated. Need to set bReplyPending true in generic interface
***     in Comver2.c
***
***     Also added mode 3 - dubugging aid - dumps to outputdebugstring.
***     Changed the dump functions to macros which check if the dump mode
***     is non zero before calling the functions - speed it up when not
***     dumping.
***
***
*** 15 July, 1993 - 3.00
***     slimmed down commslib - GPIB and Serial stuff removed to Winapps
***     Old interface removed, now generic only. Basically re-written.
***
***     3.01 interim release.
***
*** 19 April 1994 - 3.02
***     Built for release.
***
*** 20 November 1995 - 3.03
***     Fixed bug in sending commands to microlink drivers which are
***     busy. Bug shown up when Stuart fixed his Makelong bug - drivers
***     can now report errors.
***
***     Also fixed bug in OpenImlInterface - Was being re-entered while
***     WinExec'ing IML interpretors. Should be re-entrant now. No statics
***     used.  
***
***	29 April, 1996 - 3.04
***		Added timing function - GetTickCount ()   
***
*** may 1996 - 4.00 
***		Changed interface to IML interpretors to try and fix lock ups in windows 95.
***		Changed SendMessages to PostMessages followed by peekmessage with handler calling us to return
***		the completion code via AckIMLCommand - causing us to finish peek message loop
*** 
*** 17 May, 1996 - 4.01
***		Added timeouts to peekmessage loops in send command to interpreter stuff.
***
*** 2 August 1996 - 4.02  (SRG)
***		Changed SendIMLCommand slightly to stop re-entrancy problems. Commands with no reply were
***		not setting nInUseBy field - this allowed other calls to come in while it was waiting in the
***		PeekMessage loop for acknowledgement. Now sets and clears this field for EVERY command.
***
*** 30 September, 1996 - 4.03
***		Added check for number of peekmesages as well as timeout for posting messages of to interpreters.
***		in both SendIMLComand and GetIMLReply.
***
*** July 1998 - 5.00
***     Converted to 32 bit version. Involves declaring shared data
***     explicitly.
***     Added Mutex to signal when devices are busy. This allows the
***     queueing app to shut down while it waits for the device.
***
*** July 1999 - 5.01
***		Tidied up BDP logic. In particular the mutex, applied when
***     the binary transfer is started, is now released in FreeBDPLink
***
*** April 2000 - 5.02
***     0.5 Second Delay added after launching devices from IML.INI.
***     This was needed to allow serial protocol to send out messages
***     to devices needed error correction protocol. Previously caused
***     problems with 4 or more devices.
***
*** Sept 2001 - 5.03
***     MAX_REPLY_SIZE increased to 1024 bytes to allow for DHW
***     replies with many 800 modules 
***
*** Jan 2003 - 5.04
***		Add GetIMLINIPath added to allow the ini file to be put in
***		the install directory.
***
**/




#define NOMINMAX   1 /** to prevent min max redefinition **/


#define VERSION_STRING    "6.00 COMMSLIB"
/** Here for ease of finding **/

#define COMMSLIB

/** Include Files 
**/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <stdlib.h>


/** include files which are specific to this code
**/

#include "comdef.h"     /**library specific #defined constants and structures**/
#include "comproto.h"   /** library specific function prototypes - non static**/


#include "\Windmill 32\Compile Libs\imlinter.h"   /** Generic IML interface inc file **/


/** Device initialisation constant **/
#define DEVINIT NULL,IML_NO_REPLY,DEVICE_FREE,0,"",FALSE,FALSE,BAD_LINK,0 
/** BDP initialistion constant **/

#define BDPINIT BAD_HANDLE,BAD_DEVICE,0,BDP_LINK_FREE,""

/********************************************************/
/********************************************************/
/** This data is shared between all copies of Commslib.
*** Each application which uses Commslib has an 
*** independent copy of everything else.
*** It must all be initialised.
**/





#pragma data_seg (".sdata")




/** Array of DeviceInfo structs indexed on device number. These hold
*** information on the Devices with which the library can communicate.
*** 32 Devices Allowed for.
**/
IMLDEVICE adiDevice [MAX_DEVICES] =  
	{  DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,
	   DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,
	   DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,
	   DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT,DEVINIT

	};

/** The binary data packets **/
BDPLINK aBDP [ MAX_BDP_LINKS ] = {BDPINIT,BDPINIT,BDPINIT,BDPINIT};

/** each bit set to 1 indicates a handle in use **/
unsigned long lHandleUsage = 0;	

short   bInitialiseOK = FALSE;
/** Flag used to indicate that the devices have been loaded
*** 
**/

/** Mutex handles 1 per device **/
HANDLE hMutex [ MAX_DEVICES ];



char cDebugStrings [MAX_DEBUG_STRINGS][256] = { "","","","","","","","",
												"","","","","","","","",
												"","","","","","","","",
												"","","","","","","","" };
 
unsigned char cDebugBytes [MAX_DEBUG_STRINGS] = { 0,0,0,0,0,0,0,0,0,0 };
int	 nDebugIndex = 0;						/** Next place to read **/
int  nDebugCount = 0;




#pragma data_seg ()

/*******************************************************/
/*******************************************************/

/** This data is specific to this copy **/

/** This copy of commslib belongs to one application. It
*** can only talk to one device at once.
**/
static short nLocalDevice; /** Device we are currently talking to **/
   



/** Codes for IML Windows messages **/
static UINT wIMLCommandWaiting;
static UINT wIMLAbortCommand;
static UINT wIMLReplyWanted;



/** LibMain ()
*** 
***
*** Library Initialisation routine. 
***
*** This code is used when the process is created
*** It initialises all local variables ie variables in this copy of the DLL.
*** Variables shared between copies are initialised in the declaration above.
*** 
**/

BOOL APIENTRY DllMain ( HANDLE hModule , DWORD ul_reason_for_call , LPVOID lpReserved )

{
	int nIndex;


	if ( ul_reason_for_call == DLL_PROCESS_ATTACH ) {

		
		/** Register the windows messages.**/

		wIMLCommandWaiting = RegisterWindowMessage ( "IML_COMMAND_WAITING" );

		wIMLAbortCommand = RegisterWindowMessage ( "IML_ABORT_COMMAND" );

		wIMLReplyWanted = RegisterWindowMessage ( "IML_REPLY_WANTED" );

	
		for ( nIndex = 0; nIndex < MAX_DEVICES; ++nIndex ) {
			hMutex [nIndex] = NULL;	
		}
	}


/**	for ( nIndex = 0; nIndex < MAX_DEBUG_STRINGS; ++nIndex ) {
		cDebugStrings [nIndex][0]= 0;
	}
	
**/	  
	return ( TRUE );
}








/**    OpenIMLInterface ()
***
*** An application calls this function to open a communication channel
*** to the IML devices. It returns an error if it cannot perform the function.
*** It fills in the Channel number if successful and returns 0.
*** It loads the IML interpreters if they are not already loaded.
*** In a working system it can only fail if the IML handles are all used.
**/

short _declspec (dllexport)  WINAPI OpenIMLInterface ( short *pnHandle )

{

short	nRetCode = 0;
DWORD   dwMsecStart;



	/** Check if initialisation has been done
	**/
	if ( !bInitialiseOK ) {		

		/** Set the flag so we don't do it again **/
		bInitialiseOK = TRUE;		


		/** Get the device details from IML.INI 
		*** and load them. 
		**/		
		nRetCode = GetDevicesFromIMLINI ();
		if ( nRetCode != OK ) {			
			return ( nRetCode );
		}
		
		/** Time delay to ensure all devices are OK before starting
		*** app. Needed in particular for Serial Protocol. 
		*** See SFR 5008
		**/

		dwMsecStart = MillisecondCount ();
		while ( ( MillisecondCount () - dwMsecStart ) < 500 ) { }

	
	}


	/** Now see if we have a IML channel available **/
	if ( ! AssignIMLHandle ( pnHandle ) ) {
		return ( IML_NO_HANDLES );
	}
	
	return ( 0 );
}







/**   CloseIMLInterface ()
***
***
*** This function closes the communications channel.
***
*** It takes a channel number as its sole argument. This should correspond to 
*** the number returned by the Open call. It checks that the number is valid.
*** If it is it releases it so it can be used again.
***
*** Return value can be 0 - ok,
*** IML_BAD_HANDLE.
***
**/

short _declspec (dllexport)  WINAPI CloseIMLInterface ( short nIMLHandle )

{

	
	/** check the channel number to see if it is OK. Return BAD channel no
	*** if it is not OK.
	**/
	if ( ! IsIMLHandleOK ( nIMLHandle ) ) {
		return ( IML_BAD_HANDLE );
	}

	/** Free up the iml handle.**/
	ReleaseIMLHandle ( nIMLHandle );
		
	return ( 0 );
}







#pragma optimize ( "", off )

/********************************************************/
/**   SendIMLCommand ()
***
***
*** Send Command To Microlink function.
***
***
*** It returns a CompletionCode. This can be 
***     0
***     IML_BAD_DEVICE
***     IML_BAD_HANDLE
***     IML_NOT_READY
***     IML_FATAL_ERROR
***
*** The routine first checks that :
*** - The device is within the maximum range
*** - It is not still doing the previous command
*** - The IML handle is OK
*** - The device exists
*** - The device is not in use by another handle
*** - The windows handle for the device is good
*** It then sends the windows message and gets
*** the reply.
**/

short _declspec (dllexport)  WINAPI SendIMLCommand ( short nIMLHandle, short nDevice,
                                        char *lpszCommand, WORD unFlags )

{

	DWORD       ulCommandNumber;
	short nRet;


	/** check device exists **/	
	if ( !IsDeviceOK ( nDevice ) ) {
		return ( IML_BAD_DEVICE );
	}

	/** check the channel number to see if it is OK. Return BAD Handle
	*** if it is not OK.
	**/
	if ( ! IsIMLHandleOK ( nIMLHandle ) ) {
		return ( IML_BAD_HANDLE );
	}

	/** There must be no Binary link to this device since
	*** we are trying for ASCII transfer.
	**/
	if ( adiDevice [nDevice].nBDPLink != BAD_LINK ) {
		return (IML_NOT_READY );
	}

	/** If we are still doing the previous command to this device
	*** then return not ready.
	**/
	if ( adiDevice [nDevice].nDevAck  == -1 ) {
		return ( IML_NOT_READY );
	}

	/** Get access to the device if the mutex will let us.
	*** We dwell in this routine using very little processor
	*** resource if the device is busy. There is a time out of
	*** 5 seconds
	**/

	if ( ClaimDevice ( nIMLHandle, nDevice ) == FALSE ) {
		return ( IML_NOT_READY);
	}

		
	/** This copy of Commslib is talking to - **/
	nLocalDevice = nDevice;

	/** Update the device structure **/
	adiDevice [ nLocalDevice ].wReplyType = unFlags;
	adiDevice [ nLocalDevice ].bReplyReceived = FALSE;
	adiDevice [ nLocalDevice ].wCommandNo ++;

	/** We need to put the command into our internal buffer
	*** for this device so it can be picked up by the interpreter.
	**/
	strcpy ( adiDevice [ nDevice ].szBuffer, lpszCommand );


	switch ( unFlags ) {

		case IML_TEXT_REPLY :
		case IML_NO_REPLY :	
			break;
		case IML_BIN_REPLY :
			if ( ! CreateBDPLink ( nIMLHandle, nDevice ) ) {
				return ( IML_FATAL_ERROR );
			}
			break;
		case IML_BIN_DATA :
			if ( ! CreateBDPLink ( nIMLHandle, nDevice ) ) {
				return ( IML_FATAL_ERROR );
			}
			break;
		default :
			/** Prevents hang ups if uFlags has an invalid value **/
			return ( IML_NO_REPLY_EXPECTED );
	}


	/** Now send the interpreter a command waiting message so it will
	*** come and get the command.
	**/

	if ( adiDevice [ nDevice ].hWndInterpreter == NULL ) {
		/** We don't have the window handle return error **/
		return ( IML_BAD_DEVICE );
	}

	/** Create the command number **/
	ulCommandNumber = MAKELONG ( nDevice, adiDevice [ nDevice ].wCommandNo );

	/** Set waiting for device ack flag **/
	adiDevice [nDevice].nDevAck = -1;  

	
	/** Send the message to the App with a 5 second timeout. **/
	if ( !SendMessageTimeout ( adiDevice [ nDevice ].hWndInterpreter, wIMLCommandWaiting, (WPARAM) unFlags,
		(LPARAM) ulCommandNumber, SMTO_NORMAL, 10000, NULL ) ) {
			adiDevice [nDevice].nDevAck = IML_TRANSMISSION_TIMEOUT;
	}
	
	/** If we haven't got an ack substitute this error code **/
	if ( adiDevice [nDevice].nDevAck == -1 ) {
		adiDevice [nDevice].nDevAck = IML_TRANSMISSION_TIMEOUT;				
	}



	/** Check return code. If not OK then we must free
	*** the device and reset the various flags.
	*** 
	*** Must also do this if no reply is needed.
	**/

	nRet = adiDevice [nDevice].nDevAck;
	if ( ( nRet != 0 ) || 
		     ( unFlags == IML_NO_REPLY ) ) {	/* Failed or no reply */

		FreeDevice ( nIMLHandle, nDevice);
	}
	
	/** Return the error code **/
    	
	return ( nRet );
}


/*******************************************************************/  
/** This routine is called by interpreters to acknowledge the completion
*** of a command sent to them.
**/  
  
short _declspec (dllexport)  WINAPI AckIMLCommand ( DWORD ulCommandNumber, short nError )

{
	
	short   nDevice;


	//Get the device number from the command number - it is the loword.
	nDevice = LOWORD ( ulCommandNumber );
	
	/** Store the acknowledgement **/
	adiDevice [nDevice].nDevAck = nError;


	return ( 0 );
}



#pragma optimize ( "", on )






/**       GetIMLReply ()
***
***
*** This function is polled to get the reply to a command. 
*** 
*** It takes a channel number as its sole argument. This should be the value
*** set by the opencomms call.
*** 
*** 
*** It sets anCompletionCode which should be read by GetCommsError Fn.
***
*** anCompletionCode can have values:
***	COMMS_DEVICE_NOT_OPEN
***	COMMS_DEVICE_BUSY
***	COMMS_NO_REPLY_EXPECTED
***	COMMS_NO_REPLY_RECEIVED
***	COMMS_BAD_CHANNEL_NUMBER
***	COMMS_OUT_OF_MEMORY
***
**/

short _declspec (dllexport)  WINAPI GetIMLReply ( short nIMLHandle, short 
                *lpnDevice, char *lpszBuffer, short nSize, WORD unFlags )

{
    short nRet;

	/** check the channel number to see if it is OK. Return BAD Handle
	*** if it is not OK.
	**/
	if ( ! IsIMLHandleOK ( nIMLHandle ) ) {
			return ( IML_BAD_HANDLE );
	}


	/** Is this the same as the user wants.**/
	if ( *lpnDevice != nLocalDevice ) {
		return ( IML_BAD_DEVICE );
	}


	/** Does this device have a reply for this handle. Or are we
	*** expecting one.
	**/
	if ( adiDevice [ nLocalDevice ].nInUseBy != nIMLHandle ) {
		return ( IML_NO_REPLY_EXPECTED );
	}

	if ( adiDevice [ nLocalDevice ].wReplyType != IML_TEXT_REPLY ) {
		return ( IML_NO_REPLY_EXPECTED );
	}

	
	/** Now see if we have the reply.**/	
	if ( ! adiDevice [ nLocalDevice ].bReplyReceived ) {

		/** We don't have the reply yet so send the handler a 
		*** wake up call.
		**/
		
		adiDevice [nLocalDevice].nDevAck = -1;

		/** Send the message to the App with a 5 second timeout. **/
		if ( !SendMessageTimeout ( adiDevice [ nLocalDevice ].hWndInterpreter, wIMLReplyWanted, (WPARAM) unFlags,
			0, SMTO_NORMAL, 10000, NULL ) ) {
			adiDevice [nLocalDevice].nDevAck = IML_TRANSMISSION_TIMEOUT;
		}

		
		/** Sort return code. It may contain an error so we should
		*** pass it up if it does.
		**/
        nRet = adiDevice [nLocalDevice].nDevAck;

		if ( nRet != 0 ) { 
			/** Dont reset anything here the app will abort
			*** if it doesn't like the error code.
			**/
			return ( nRet );
		}

		if ( ! adiDevice [ nLocalDevice ].bReplyReceived ) {
			return ( IML_NOT_READY );
		}
	}

	/** OK We should have the reply so copy it to the user supplied
	*** buffer and return OK or Buffer too small.
	**/

	if ( strlen ( adiDevice [ nLocalDevice ].szBuffer ) > (size_t) nSize ) {

		/** buffer too small for reply.**/
		return ( IML_BUFFER_TOO_SMALL );
	}


	strcpy ( lpszBuffer, adiDevice [ nLocalDevice ].szBuffer );

	/** Let go of the mutex.
	**/
	FreeDevice (nIMLHandle, nLocalDevice );

	return ( OK );
}







/**      AbortIMLReply ()
***
***
*** This function is used to abort a reply which is no longer required.
***
*** It resets the data structures and frees any memory allocated for the reply
*** for the channel number specified by nIMLHandle.
***
*** It returns anCompletionCode which can be
***	0
***	COMMS_DEVICE_NOT_OPEN
***	COMMS_NO_REPLY_EXPECTED
***	COMMS_BAD_CHANNEL_NUMBER
***
**/

short _declspec (dllexport)  WINAPI AbortIMLReply ( short nIMLHandle, short nDevice )

{

	DWORD       ulCommandNumber;  
	short nRet;

	/** check the channel number to see if it is OK. Return BAD Handle
	*** if it is not OK.
	**/
	if ( ! IsIMLHandleOK ( nIMLHandle ) ) {
			return ( IML_BAD_HANDLE );
	}



	/** Does the device exist.**/
	if ( ! IsDeviceOK ( nDevice) ) {
		return ( IML_BAD_DEVICE );
	}


	if ( adiDevice [ nDevice ].nInUseBy != nIMLHandle ) {
		/** This device is being used by another app.**/
		return ( IML_BAD_DEVICE );
	}


	/** Check that we are expecting a reply for this channel **/

	if ( adiDevice [ nDevice ].wReplyType != IML_TEXT_REPLY ) {
		return ( IML_NO_REPLY_EXPECTED );
	}
	
	/** OK We are expecting a reply for this channel so we can now terminate
	*** it.
	**/

	ulCommandNumber = MAKELONG ( nDevice, adiDevice [ nDevice ].wCommandNo );


	adiDevice [nDevice].nDevAck  = -1;

	/** Send the message to the App with a 5 second timeout. **/
	if ( !SendMessageTimeout ( adiDevice [ nDevice ].hWndInterpreter, wIMLAbortCommand,0,
		(LPARAM) ulCommandNumber, SMTO_NORMAL, 10000, NULL ) ) {
			adiDevice [nDevice].nDevAck = IML_TRANSMISSION_TIMEOUT;
	}

	nRet = adiDevice [nDevice].nDevAck;
	
	
	FreeDevice ( nIMLHandle, nDevice );
	
	return ( nRet );
}






/**************************************************/
/** Just copies the version string into a buffer **/


short _declspec (dllexport)  WINAPI GetIMLInterfaceVersion ( short nHandle, char *
                                                   szBuffer, short nSize )

{

short   nLen;
char *  szVer;

	if ( szBuffer != NULL ) {

		nLen = strlen ( VERSION_STRING );

		if ( nSize > nLen ) {
			strcpy ( szBuffer, VERSION_STRING );
		}

		strncpy ( szBuffer, VERSION_STRING, ( nSize - 2 ) );
		szBuffer [ nSize - 1 ] = '\0';
	}

	szVer = VERSION_STRING;
	return ( ( szVer [ 0 ] - '0' ) );
}



/**************************************************************/
/** Fills in LowMap and HighMap with bits which indicate which
*** IML devices 0 to 63 are present. Always returns OK.
**/


short _declspec (dllexport)  WINAPI GetIMLDeviceMap ( short nHandle, long  *
                                             plLowMap, long *plHighMap )

{

	short    nDevice;


	*plLowMap  = 0x00000000L;
	*plHighMap = 0x00000000L;	/* high map NOT USED in this version */
	

	/** Look in our Device stuctures to see which are in use.
	**/
	
	for ( nDevice = 0 ; nDevice < MAX_DEVICES ; nDevice++ ) {

		if ( adiDevice [ nDevice ].bExists ) {

			*plLowMap |= (long) ( 0x00000001L << nDevice );
			continue;
		}
	}

	return ( OK );
}









/**        CommsOperationMode ()
***
***
*** This function sets the operation mode of the library. It is not used by 
*** applications. It can be accessed by the library test harness to set the
*** mode of operation of the library.
***
*** Function removed at change to 5.0
**/

short _declspec (dllexport)  WINAPI CommsOperationMode ( short nMode , LPSTR lpszModeStr )

{

	return ( 0 );
}

/****************************************************/
/** Called by the interpreter after it has been told that a command is waiting.
*** The command is copied into the passed buffer if it is big enough.
**/

short _declspec (dllexport)  WINAPI GetIMLCommandToExecute ( DWORD ulCommandNumber,
					LPSTR lpszBuffer, short nSize )


{

	short   nDevice;


	// Get the device number from the command number - it is the loword.

	nDevice = LOWORD ( ulCommandNumber );  

	if ( nSize < (short) strlen ( adiDevice [ nDevice ].szBuffer ) ) {
		/** The buffer is too small so return error.
		**/		
		return ( IML_BUFFER_TOO_SMALL );
	}

	/** Get a copy of the command string **/
	strcpy ( lpszBuffer, adiDevice [ nDevice ].szBuffer );


	return ( 0 );
}

/*****************************************************/
/** Called by the interpreter to pass a reply string.
*** It simply copies the string into the buffer if it is big enough.
*** It also sets the reply received flag.
**/


short _declspec (dllexport)  WINAPI ReplyToIMLCommand ( DWORD ulCommandNumber,
                                        LPSTR lpszBuffer )

{

	short   nDevice;


	// Get the device number from the command number - it is the loword.

	nDevice = LOWORD ( ulCommandNumber );

	adiDevice [ nDevice ].bReplyReceived = TRUE;

	strcpy ( adiDevice [ nDevice ].szBuffer, lpszBuffer );


	return ( 0 );
}


/*******************************************************/
/** Called by the interpreter to register its windows handle 
*** There used to be an Ex version of this. Not needed with the
*** change to 32 bit.
**/


short _declspec (dllexport)  WINAPI RegisterIMLInterpreter( HWND hWndInterpreter,
                                      short nDeviceNumber )

{

	if ( ! IsDeviceOK ( nDeviceNumber) ) {
		/** Something has gone wrong here so return address error.
		**/
		return ( IML_BAD_DEVICE );
	}

	adiDevice [ nDeviceNumber ].hWndInterpreter = hWndInterpreter;

	
	return ( 0 );
}



/***********************************************************/
/** Called by interpreter when it closes **/


short _declspec (dllexport)  WINAPI UnRegisterIMLInterpreter ( short nDeviceNumber )

{

	if ( ! IsDeviceOK ( nDeviceNumber ) ) {
		/** Something has gone wrong here so return address error.
		**/
		return ( IML_BAD_DEVICE );
	}

	adiDevice [ nDeviceNumber ].hWndInterpreter = NULL;

	return ( 0 );
}

/*****************************************************************/
/** Get the full path to IML.INI.
*** dwSize is the size of the supplied buffer
**/

short _declspec (dllexport)  WINAPI GetIMLINIPath ( char* cFilePath, DWORD dwSize )

{
	return ( GetIniPath ( cFilePath, dwSize ));

}



/*****************************************************************/
/** Get the full path to the working directory.
*** dwSize is the size of the supplied buffer
**/

short _declspec (dllexport)  WINAPI GetIMLDirPath ( char* cFilePath, DWORD dwSize )

{

HKEY  myKey;
char * szSubKey = "SOFTWARE\\WINDMILL SOFTWARE\\WINDMILL";
DWORD dwType;
DWORD dwLength;

	dwLength = dwSize;

	/** Read the working dir from the registry.**/
	if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_QUERY_VALUE, &myKey)) {
		return ( 1 );
	}
	RegQueryValueEx ( myKey, "WORKINGDIR", NULL, &dwType, cFilePath, &dwLength );
	RegCloseKey ( myKey );

	if ( cFilePath [ strlen (cFilePath) - 1 ] == '\\' ) {
		cFilePath [ strlen (cFilePath) - 1 ] = 0;
	}

	if ( (strlen(cFilePath) + 10) > dwSize ) {
		return ( 1 );
	}

	if ( strlen ( cFilePath ) == 0 ) {
		return ( 1 );
	}

	return (OK);

}