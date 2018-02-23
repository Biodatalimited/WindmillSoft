/**
***	ATini.c			IMLAPPS-IML Applications Toolkit
***
***	IMLAPPS-ATK-P-2.00	Andrew Thomson
***
***	This file contains the source for the following toolkit
***	functions :
***
***		ATKLoadINIColors
***		ATKSaveINIColors
***
***
***	The functions in this module should be used by
***	Windmill applications to read and write data to the
***	WINDMILL.ini file.
***
***	To use this module :
***
***
***	- Specify the include file 'ATKit.h' in the module
***	  which will call the functions. This should be located
***	  after including <windows.h>
***
***
***	- Include the following toolkit files in the build list :
***
***		ATini.c, ATerror.c
***
***
***
***
***	Associated Files
***
***	        ATkit.h       - Contains the Fn prototypes, structure
***	              		declarations and manifest constants.
***
***	        ATerror.c     - Used to report errors which occur in this
***	                        module.
***
***	        ATkit.rc      - Resource Text Strings used by above.
***
***	VERSION HISTORY :
***
***	16 February  93    ATK-P-2.00	First Issue of this module
***
**/
#define NOKANJI
#define NOCOMMS
#define IML_APPLICATION

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ATkit.h"


/**     Intrinsic Function Prototypes
**/


/**
***     Private Variables, accessable only to THIS module.
**/


/**
***	ATKLoadINIColours
***
***
***	This function loads the RGB values from the [Windmill.colours ]
***	section of the Windmill.ini file and stores them a ATKCOLORINFO
***	structure passed in to the function by reference.
***	Two colours, a foreground and a background colour exist for
***	each of the currently supported alarm states.
***
*** INI file no longer used just set defaults.
***
***
***	ARGUMENTS
***
***	  PATKCOLOURINFO	A pointer to an ATKCOLOURINFO structure.
***
***
***	RETURN VALUE		OK
***
**/
short ATKLoadINIColours ( PATKCOLOURINFO pcolColours )
{


	/**	Normal Alarm State Text and Background Colors **/
	pcolColours->crNormalText = RGB (  0, 0, 0 );
	pcolColours->crNormalBk = RGB ( 255, 255, 255 );


	/**	Warning Alarm State Text and Background Colors **/
	pcolColours->crWarningText =  RGB ( 255, 255, 255 );
	pcolColours->crWarningBk = RGB ( 0, 0, 255);


	/**	Critical Alarm State Text and Background Colors**/
	pcolColours->crCriticalText = RGB ( 255, 255, 255);
	pcolColours->crCriticalBk = RGB ( 255, 0, 0 );

	return ( OK );
}





