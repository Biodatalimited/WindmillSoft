
#define COMMSLIB


#define NOMINMAX	1	/** to prevent min max redefinition error **/


/** Include Files 
**/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>





/** include files which are specific to this code
**/

#include "comdef.h"	/**library specific #defined constants and structures**/
#include "\Windmill 32\Compile Libs\imlinter.h"	/** error codes and exported function prototypes **/
#include "comproto.h"	/** library specific function prototypes - non ststic**/


extern char cDebugStrings [MAX_DEBUG_STRINGS][256];
extern unsigned char cDebugBytes [];
extern int	nDebugIndex;				/** Next place to read **/
extern int  nDebugCount;


/** Read the next debug string into cDebugReply
*** Return the number of bytes in the reply
*** Called by ComDebug
**/

int _declspec (dllexport)  WINAPI ReadDebugString ( char* cDebugReply )

{

	int nReply;

	/** No strings to send **/
	if ( !nDebugCount )	{ return (0); }


	/** Copy the reply **/
	nReply = cDebugBytes [nDebugIndex];
	memcpy ( cDebugReply, cDebugStrings[nDebugIndex], nReply );

	/** Increment the index for the next reply string **/
	++nDebugIndex;
	if ( nDebugIndex >= MAX_DEBUG_STRINGS  ) { nDebugIndex = 0; }

	/** One less string **/
	--nDebugCount;

	/** Reset to 0 just to keep things simple **/
	if ( ! nDebugCount ) { nDebugIndex = 0; }


	/** Return bytes in reply **/
	return ( nReply ) ;

}


/** Write a new debug string into the store
*** Don't write if its already full
*** Called by ComIML
**/


void _declspec (dllexport)  WINAPI WriteDebugString ( char* cDebugSend, int nSize )

{
	int nWriteIndex;

	/** Check if its full **/
	if ( nDebugCount >= MAX_DEBUG_STRINGS ) { return; }

	/** Work out where to write **/
	nWriteIndex  = nDebugIndex + nDebugCount;
	
	if ( nWriteIndex >= MAX_DEBUG_STRINGS ) {
		nWriteIndex = nWriteIndex - MAX_DEBUG_STRINGS;
	}

	if ( ( nWriteIndex < 0) || ( nWriteIndex >= MAX_DEBUG_STRINGS ) ) { return; }

	/** Copy it in **/
	memcpy ( cDebugStrings[nWriteIndex], cDebugSend, nSize );
	cDebugBytes [nWriteIndex] = nSize;

	/** One more written **/
	++nDebugCount;


}
