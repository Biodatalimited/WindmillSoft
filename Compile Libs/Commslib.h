/** COMMSLIB.H 		IMLAPPS Communications library
***
*** COMSLIB-P1		Nigel Roberts.
***
*** This file contains the #define constants and function prototypes required 
*** by an application inorder for it it be able to use the communications 
*** library.
***
***
*** Other files required to use the communications library are:-
***
***	COMMSLIB.LIB - Communications library Import Library.
***	COMMSLIB.EXE - Communications Dynamic Link Library.
***
***
*** NOTE - This file must be #included after WINDOWS.H as it uses types defined
*** in Windows.H. ( HMEM - LPSTR ).
**/


#define COMMS_NOT_OPEN			0xFFF0
#define COMMS_HARDWARE_NOT_PRESENT	0x0010
#define COMMS_DEVICE_BUSY		0x0011
#define COMMS_DEVICE_NOT_OPEN		0x0012
#define COMMS_BAD_CHANNEL_NUMBER	0x0013
#define COMMS_NO_REPLY_RECEIVED		0x0014
#define COMMS_NO_REPLY_EXPECTED		0x0015
#define COMMS_TRANSMISSION_TIMEOUT	0x0016
#define COMMS_ADDRESS_ERROR		0x0017
#define COMMS_OUT_OF_MEMORY		0x0018
#define COMMS_NO_MICROLINKS		0x0019
#define COMMS_GPIB_INSTALL_ERROR        0x001A
#define COMMS_GPIB_HARDWARE_ERROR       0x001B
#define COMMS_BAD_BAUD_RATE		0x001C
#define COMMS_TRANSMISSION_ERROR	0x001D
#define COMMS_MORE_REPLY_TO_COME	0x001E
#define COMMS_NOT_REQUESTED		0x001F
#define COMMS_GPIB_VERSION_ERROR	0x0020






#define GPIB_FRAME	0
#define SERIAL_FRAME	1
#define WINAPP_FRAME	2




/** the function prototypes of the functions asseccible to an application 
*** are as follows:-
**/


short _declspec (dllexport)  WINAPI OpenMicrolinkComms ( void );
/** This function is used to open a communications channel. 
**/


short _declspec (dllexport)  WINAPI CloseMicrolinkComms ( short nChannelNo );
/** This function closes the communications channel.
**/


short _declspec (dllexport)  WINAPI SendMicrolinkCommand ( short nChannelNo , short nIMLDev ,
					LPSTR lpszCommand , short bReply );

/** This function is used to send commands to a specific Microlink.
**/


HANDLE _declspec (dllexport)  WINAPI GetMicrolinkReply ( short nChannelNo );
/** This function is polled to get the reply to a command. 
**/


short _declspec (dllexport)  WINAPI AbortMicrolinkReply ( short nChannelNo );
/** This function is used to abort a reply which is no longer required.
**/


short _declspec (dllexport)  WINAPI GetMicrolinkCommsError ( short nChannelNo );
/** This function reports the error code set by the previous call to the 
*** library.
**/


short _declspec (dllexport)  WINAPI GetMicrolinkCommsVersion ( LPSTR lpszVersion,
						       short nMaxChars );


short _declspec (dllexport)  WINAPI RequestMicrolinkComms ( short nChannelNo , short nIMLDev );
/** This function is used to request permission to use the comms channels.
**/

short _declspec (dllexport)  WINAPI ReleaseMicrolinkComms ( short nChannelNo , short nIMLDev );
/** This function is called to release the comms channels when finished with.
**/



short _declspec (dllexport)  WINAPI MicrolinkFrameDetails ( short nIMLDev, short far
				*lpnType, short far * lpnRealAddress );

/** this function is used to obtain details of the Microlink frame.
**/





short _declspec (dllexport)  WINAPI GetIMLCommandToExecute ( DWORD ulCommandNumber,
					LPSTR lpszBuffer, short nSize );

short _declspec (dllexport)  WINAPI ReplyToIMLCommand ( DWORD ulCommandNumber,
							LPSTR lpszBuffer );



short _declspec (dllexport)  WINAPI RegisterIMLInterpreter ( HWND hWndInterpreter,
						short nDeviceNumber );


short _declspec (dllexport)  WINAPI UnRegisterIMLInterpreter ( short nDeviceNumber );
