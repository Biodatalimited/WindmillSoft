/**
***     ATmSec.c              	IMLAPPS-IML Applications Toolkit
***
***     IMLAPPS-ATK-P-3.02	Andrew Thomson
***
***     This file contains the source for the following toolkit
***     functions :
***
***     GetMillisecondCount
***
***     The functions in this module should be used by
***     Windmill applications to read and write data from
***     IML channels.
***
***
***     To use this module :
***
***
***     - Include the line 'rcinclude ATKit.rc' in the
***       application's Resource file.
***
***
***     - Specify the include file 'ATKit.h' in the module
***       which will call the functions. This should be located
***       after including <windows.h>
***
***
***     - Include the following toolkit files in the build list :
***
***             ATmSec, ATerror.c
***
***
***
***
***     Associated Files
***
***             ATkit.h       - Contains the Fn prototypes, structure
***                             declarations and manifest constants.
***
***             ATerror.c     - Used to report errors which occur in this
***                             module.
***
***             ATkit.rc      - Resource Text Strings used by above.
***
***     VERSION HISTORY :
***
***     17 September 91    ATK-P-1.00   First Version Written.
***     16 February  93    ATK-P-2.00   Alarm Support Included
***     21 January   94    ATK-P-3.00   No message-boxes on comms failures.
***                                     Reads each channel even if comms errs
***     19 January   95    ATK-P-3.01   Performs REP only if READ failed
***                                     received. Removes one last messagebox
***                                     displayed if REP failed.
***     19 April     96    ATK-P-3.02   New implementation of GetMillisecondCount fn
***                                     which calls COMMSLIB GetIMLMillisecondCount fn
**/



#include <windows.h>
#include <windowsx.h>
#include "\Windmill 32\Compile Libs\IMLinter.h"   /* ...using the COMMSLIB Dynalink                */

#include "atkit.h"




static DWORD   GetTick ( void );


/**  GetMillisecondCount. This void function returns an unsigned 32-bit number
***  which constantly increments at 1 mS intervals. The absolute value is of
***  no significance, but the DIFFERENCE between two values indicates the
***  approximate number of milliseconds elapsed. Can be used for timeouts etc.
**/
DWORD GetMillisecondCount ( VOID )

{
	
	/**	Calls Commslib Timer Function
	**/
	return ( GetIMLMillisecondCount () );
}





 