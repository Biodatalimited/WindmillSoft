/**
***     WPDO_CYC.c     ATK-PANELS
***     Support for 'Auto' cycling mode.
***     Andrew Thomson
***
***     This module holds miscellaneous functions used to start,
***     stop and handle 'auto' cycling.
***
***
***      Associated Files : 	
***
***      Version History :
***
**/

#define  NOMINMAX
#define  NOKANJI
#define  NOCOMM
#define  STRICT
#define  CTLAPIS
#define  IML_APPLICATION

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>   /* Contains ULONG_MAX constant			*/


#include <conio.h>   /* for inp() and outp() 			*/


#include "\Windmill 32\Compile Libs\imlinter.h"   /* ...using the COMMSLIB Dynalink		 */
#include "\Windmill 32\Compile Libs\imslib.h"   /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATkit\atkit.h"      /* Application Toolkit Header/Prototypes         */


#include "WPDO_def.h"   /* Manifest Constants and Structure Declarations */

/* Functions Prototypes*/



/**	Externally defined Globals Variables and structures
**/
extern HINSTANCE  hInst;        /* Program Instance Handle	    */
extern HWND       hwndMain;     /* Program Window Handle	    */


/**	Application Details
**/
extern PANELINFO    panDigitalOut;

/**	Channel Connection Information
**/
extern CONNECTIONS conninfo;





/**
***	StartAutoCycle
***
***	Starts the cycling operation.
***
***	Return OK or defined ERR code ( see WPDO_Def.h)
***
*** Not needed any more now we use windows timers

**/


short StartAutoCycle ( VOID )


{
	DWORD	dwMillisecsNow;



	/**	Do NOT allow the Hardware configuration
	***	   to change while we are logging............
	**/


	/**	Get the current time and use this
	***	set a pseudo last time which will result
	***	in a first read being taken immediately.
	**/
	dwMillisecsNow = GetMillisecondCount ( );


//	panDigitalOut.dwMillisecsLastCycle =
//			dwMillisecsNow - panDigitalOut.dwMillisecsInterval;




	return ( OK );

}









/**
***	ServiceAutoCycle
***
***	Determines if new cycle data is required. If so the fn calls
***	the WinApp_ReadChannel function directly to obtain and paint the
***	new data.
***
*** The status variable seems to have been messed around with.
*** IDLE in logger terms meant doing nothing whilst
*** AUTO meant running. In the conversion to Digital out this
*** was lost. It is always IDLE and also taking readings. The
*** change to a windows timer then caused problems since it caused
*** readings to be taken during dialogs. To overcome this whilst
*** not interfering with the original scheme I have added a BUSY
*** status to be invoked when doing connection dialogs etc.
***
***	Return OK
**/
short ServiceAutoCycle ( VOID )
{

	static	BOOL	bInFunction;

//	short		nElapsedCycles;



	/**	Stop this function being reenterant
	**/
	if ( bInFunction ) {
		
		/**	Already 'in this function' data
		**/
		return ( OK );
	}

	if ( panDigitalOut.nStatus != STATUS_IDLE ) {
		return ( OK );
	}

	
	/** Now processing this fn
	*** so set the interlock
	**/
	bInFunction = TRUE;	/* Now processing this fn-set the interlock */
	

		/**     Take new Reading, first adjust
		***     the 'Time Last Scan' value. This stores
		***     the time of the current scan.
		**/
//		panDigitalOut.dwMillisecsLastCycle +=
//			  ( (DWORD) nElapsedCycles *
//			     panDigitalOut.dwMillisecsInterval );

		panDigitalOut.bReportErrors = FALSE;

		/**	Read all channels. Use SendMessage
		***	to read ASAP.
		**/
		FORWARD_WM_READCHAN ( hwndMain,
			     conninfo.nNumConnected, SendMessage );

		panDigitalOut.bReportErrors = TRUE;
		
		panDigitalOut.nStatus = STATUS_IDLE;

		bInFunction = FALSE;
		return ( OK );


}





/**
***	StopAutoCycle
***
***	Stops the cycling operation.
***
***	Return OK or defined ERR code ( see WPDO_Def.h)
**/
short StopAutoCycle ( VOID )
{


	/**	No action required on Cycling Output Panels..
	**/


	return ( OK );
}
