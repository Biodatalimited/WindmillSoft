/** COMPROTO.H 		IMLAPPS Communications library
***
**/



/** COMMAIN.C - Main Module - entry points to library.
**/

short _declspec (dllexport) WINAPI CommsOperationMode ( short nMode , char * lpszModeStr );


/** MSEC.C **/
DWORD MillisecondCount ( void );

/** this is the initialisation function called from libentry
**/

BOOL APIENTRY DllMain ( HANDLE hModule , DWORD ul_reason_for_call , LPVOID lpReserved );



/** The following are the function prototypes of the Private Functions.
**/

/** COMUTIL.C - This module contains the following functions
**/

short IsDeviceOK ( short nDevice );
/** This function checks if a device number belongs to a valid device **/

short AssignIMLHandle ( short * pnIMLHandle );
/** This function checks which channels are inuse and returns the channel 
*** number if one is available or zero.
**/

short IsIMLHandleOK ( short nIMLHandle );
/** This function checks the validity of a communications channel number.
**/

short ReleaseIMLHandle ( short nIMLHandle );
/** This function frees the channel number, if it has been assigned, so that it
*** can be used again.
**/

short ClaimDevice ( short nIMLHandle, short nDevice );
/** Claims use of the IML device . Returns
*** TRUE if the calling handle has the device.
**/

short FreeDevice ( short nIMLHandle, short nDevice );
/** Removes the IMLHandle from the queue when the app has finished with
*** the device.
**/


short GetIniPath ( char* cFileName, DWORD dwSize ); 
/** Get the path to IML.INI **/

short LoadIMLInterpretors ( void );
short GetDevicesFromIMLINI ( void );

#if defined ( DEMO_VERSION )
short KillIMLInterpretors ( void );
#endif




short CreateBDPLink ( short nHandle, short nDevice );
