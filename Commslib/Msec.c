

/**  This function returns a count of milliseconds, with a resolution of one millisec.
***  The returned DWORD is the exact number of milliseconds since Windows was started.
***  It will wrap around after 4,294,967 seconds, which is about 7 weeks.
***
***  The caller should ALWAYS SUBTRACT AN OLD VALUE FROM THE NEW VALUE, to obtain the
***  difference (i.e. number of milliseconds elapsed). This will work correctly even
***  if the counter wraps around.
***
***  The QueryPerformanceCounter WIN32 function returns a number
***  which is a count of a high frequency clock. The QueryPerformanceFrequency
***  returns the frequency of that clock. The count starts when windows starts.
***  These fuctions may not be supported on all platforms.
**/

#include <windows.h>
#include <winbase.h>
#include "comproto.h"



DWORD _declspec (dllexport)  WINAPI GetIMLMillisecondCount ( void )  

{
	return ( MillisecondCount () );
}



/**************************************************
*** Separate routine used to allow easy calling from within 
*** Commslib.
**/


DWORD MillisecondCount ( void )

{
	LARGE_INTEGER Frequency, CurrentCount;
	double fCurrentCount, fFrequency;
	double f32bit = 0xFFFFFFFF;

	QueryPerformanceFrequency ( &Frequency );
	QueryPerformanceCounter ( &CurrentCount );
	
	/** Convert the 64 bit integers to float **/
	fFrequency = Frequency.LowPart + Frequency.HighPart * f32bit;
	fCurrentCount = CurrentCount.LowPart + CurrentCount.HighPart * f32bit;
 
	/** Convert to milliseconds **/
	fCurrentCount = (fCurrentCount * 1000) / fFrequency;

	/** Subtract any overflows. Division may give rounding errors **/
	while ( fCurrentCount > f32bit ) {
		fCurrentCount = fCurrentCount - f32bit;
	}

	/** Return a 32 bit value **/
	return ( (DWORD) fCurrentCount );

}