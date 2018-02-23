/**
***     ATtimer.c               IMLAPPS-IML Applications Toolkit
***
***     IMLAPPS-ATK-P-1.02     Andrew Thomson
***
***     This file contains the source for the following toolkit
***     functions :
***
***             ATKGetNearestTimerInterval
***
***
***
***     The functions in this module should be used by
***     Windmill applications to establish valid timer event
***     intervals.
***
***
***     To use this module :
***
***     - Specify the include file 'ATKit.h' in the module
***       which will call the functions. This should be located
***       after including <windows.h>
***
***     - Include the following toolkit files in the build list :
***
***             ATtimer.c
***
***
***
***
***     Associated Files
***
***             ATKIT.h       - Contains the Fn prototypes, structure
***                             declarations and manifest constants.
***
***
***     VERSION HISTORY :
***
***     17 September 91    ATK-P-1.00   First Version Written.
***
***
**/
#define NOKANJI
#define NOCOMMS

#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "ATkit.h"
#include "\Windmill 32\Compile Libs\commslib.h"
#include "\Windmill 32\Compile Libs\imslib.h"

/**
***  ATKGetNearestTimerInterval
***
***  This func sets a given timer interval , in mSecs, to the
***  nearest attainable interval using the 55mSec timer tick.
***
***  The achievable interval is returned by reference in pulTime.
***  The return value specifies the difference between the
***  requested interval and the achievable interval represented
***  as parts per thousand. This may be used by the application
***  to determine whether the change is acceptable.
***
***  ARGUMENTS
***      pulTime        Pointer to unsigned long which on entry
***                     contains the requested interval in mSecs
***                     and on exit contains the nearest acheivable
***                     interval, in mSecs.
***
***
***  RETURN VALUE
***
***     The error between the requested and the nearest attainable
***     interval expressed as tenths of a percent ( parts per thousand )
***
**/
short ATKGetNearestTimerInterval ( DWORD *pulTime )
{
	double    dRequested;
	double    dNearestIntv;
	double    dDifference;
	double    dPartsPerThouError;
	LONG      lNumTicks;


	/**  Use doubles rather than longs 
	***  to minimise cumulative errors
	***  with long intervals ( > 30 secs )
	**/
	dRequested = (double) *pulTime;


	/**     Establish how many actual ticks we can
	***     use to represent the requested interval
	***     Remembering to round to the nearest whole
	***     tick....
	***
	**/
	lNumTicks = (LONG) ( ( dRequested / DOS_TICK_INTV ) + 0.5 );

	if ( lNumTicks == 0L ) {

		/**     Disallow intervals smaller than DOS_TICK_INTV
		**/
		lNumTicks = 1L;
	}

	/**     Multiply this back up by the DOS_TICK_INTV to obtain
	***     the actual duration. Note the new Interval will be a
	***     multiple of DOS_TICK_INTV ( 55 mSecs )
	**/
	dNearestIntv = (double) lNumTicks * DOS_TICK_INTV;

	/**     Establish how many mSecs difference exists
	***     between the actual duration and the application
	***     requested duration
	**/
	dDifference  = 	fabs ( dRequested - dNearestIntv );


	/**     Express the difference as as 'Parts per thousnad'
	***     (tenths of a percent )
	**/
	if ( dRequested < 1.0e-99 ) {
		/** Prevent division by zero
		**/
		dPartsPerThouError = 1000.0;
	} else {
		dPartsPerThouError = dDifference * 1000.0 / dRequested;
	}

	/**     Return the NEW interval by reference
	***     Rounding up to nearest integer value
	**/
	*pulTime = (DWORD) ( dNearestIntv + 0.5 );


	/**     Return error between requested and nearest
	***     interval as tenths of a percent
	***   ( parts per thousand )
	**/
	return ( (short) dPartsPerThouError  );
}
