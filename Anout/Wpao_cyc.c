/**
***	WPAO_CYC.c		ATK-PANELS
***				Support for 'Auto' cycling mode.
***     Andrew Thomson
***
***	This module holds miscellaneous functions used to start,
***	stop and handle 'auto' cycling.
***
***
***	Associated Files : 	
***
***	Version History :
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
#include <limits.h>	/* Contains ULONG_MAX constant			*/

#include <conio.h>	/* for inp() and outp() 			 */


#include "\Windmill 32\Compile Libs\IMLinter.h"	/* ...using the COMMSLIB Dynalink		 */
#include "\Windmill 32\Compile Libs\imslib.h"    /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATkit\ATKit.h"      /* Application Toolkit Header/Prototypes         */


#include "WPAO_def.h"	/* Manifest Constants and Structure Declarations */
			/* Functions Prototypes				 */



/**	Externally defined Globals Variables and structures
**/
extern HINSTANCE  hInst;		/* Program Instance Handle	    */
extern HWND	  hwndMain;		/* Program Window Handle	    */


/**	Application Details
**/
extern PANELINFO	panAnalogOut;

/**	Channel Connection Information
**/
extern CONNECTIONS conninfo;





/**
***	StartAutoCycle
***
***	Starts the cycling operation.
***
***	Return OK or defined ERR code ( see WPAO_Def.h)
**/
short StartAutoCycle ( VOID )
{
	DWORD		dwMillisecsNow;



	/**	Do NOT allow the Hardware configuration
	***	to change while we are logging............
	**/



	/**	Get the current time and use this
	***	set a pseudo last time which will result
	***	in a first read being taken immediately.
	**/
	dwMillisecsNow = GetIMLMillisecondCount ( );


	panAnalogOut.dwMillisecsLastCycle =
			dwMillisecsNow - panAnalogOut.dwMillisecsInterval;




	return ( OK );

}









/**
***	ServiceAutoCycle
***
***	Determines if new cycle data is required. If so the fn calls
***	the WinApp_ReadChannel function directly to obtain and paint the
***	new data.
***
***	Return OK
**/
short ServiceAutoCycle ( VOID )
{

	static	BOOL	bInFunction;

	
	/** Must be in idle mode ie have a loaded setup **/
	if ( panAnalogOut.nStatus != STATUS_IDLE ) {
		return ( OK);
	}
	
	
	/**	Stop this function being reenterant
	**/
	if ( bInFunction ) {
		
		/**	Already 'in this function' data
		**/
		return ( OK );
	}

	/** Now processing this fn
	*** so set the interlock
	**/
	bInFunction = TRUE;	/* Now processing this fn-set the interlock */
	

	/**	Read all channels. Use SendMessage
	***	to read ASAP.
	**/
	FORWARD_WM_READCHAN ( hwndMain,
			conninfo.nNumConnected, SendMessage );

	bInFunction = FALSE;
	return ( OK );

    

}





/**
***	StopAutoCycle
***
***	Stops the cycling operation.
***
***	Return OK or defined ERR code ( see WPAO_Def.h)
**/
short StopAutoCycle ( VOID )
{


	/**	No action required on Cycling Output Panels..
	**/


	return ( OK );
}
