/**
***	ATlimits.c		IMLAPPS-IML Applications Toolkit
***
***	IMLAPPS-ATK-P-1.02	Andrew Thomson
***
***	This file contains the source for the following toolkit
***	functions :
***
***	        ATKChannelLimits
***
***
***	The function in this module should be used by
***	Windmill applications to obtain the minimum and maximum
***	values obtainable for a list of channels.
***
***
***	To use this module :
***
***
***	- Specify the include file 'ATKit.h' in the module
***	  which will call the functions. This should be located
***	  after including <windows.h>
***
***	- Specify the include files 'IMSLIB.h' and 'IMLInter.h'
***	  if you have not already done so.
***
***	- Include the following toolkit files in the build list :
***
***	        ATlimits, ATerror.c
***
***
***
***
***	Associated Files
***
***	        ATkit.h       - Contains the Fn prototypes, structure
***	              		declarations and manifest constants.
***
***	        ATerror.c     - Used to Report errors occuring within
***	                        this module.
***
***	VERSION HISTORY :
***
***	17 September 91    ATK-P-1.00   First Version Written.
***
***
**/
#define NOKANJI
#define NOCOMMS
#define IML_APPLICATION

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ATkit.h"
#include "\Windmill 32\Compile Libs\imslib.h"
#include "\Windmill 32\Compile Libs\IMLInter.h"


extern BOOL bDestroy;

/**     Intrinsic Function Prototypes
**/



/**
***	ATKChannelLimits
***
***	Interrogates the IML Device, extracting the minimum and maximum
***	obtainable values for each channel in the given channel list. The
***	limits are stored as strings szMin and szMax in the ATKCHANINFO
***	structures.
***
***	NOTE    szMin will be derived from the value associated with the
***	lowest minimum input which is acceptable at the module front end.
***	Similarly, szMax is derived from the maximum input value acceptable
***	by the front end. The minimum & maximum 'front-end' values may then
***	be scaled / linearised to give the final limits. For this reason
***	you should not assume that szMin will be lower than szMax. This may
***	not be the case.
***
***
***	ARGUMENTS :
***
***	  hWndParent            The Window Handle of the calling
***				application.
***
***	  nIMLHandle            The IML Device handle. The application
***	                        must obtain this handle by calling
***	                        IMLOpenInterface.
***
***	  pciChanList           Near Pointer to an array of ATKCHANINFO
***	                        structures. A min & max limits string will
***	                        be obtained for each channel structure in
***	                        the array and the string placed in the
***				structure. Note the Channel Name, Channel
***	                        Number and Channel Device Number must be
***	                        set in each structure before calling this
***	                        function.
***
***	  nChans                The number of structures in the above
***				array.
***
***	RETURN VALUE :
***
***	        Number of channels which incurred errors whilst
***	        attempting to extract the units.
**/
short ATKChannelLimits ( HWND hWndParent, short nIMLHandle,
				PATKCHANINFO pciChanList, short nChans )
{
	char    szCommand    [ ATK_SHOW_CMD_LEN + 1 ];
	char    szReply      [ ATK_LIMITS_REPLY_LEN + 1 ];
	WORD    wFlags;
	short   nResult;
	HANDLE  hInst;
	short   nErrorCount;
	short   nChanInError;
	short   nIndex;



	hInst = (HINSTANCE) GetWindowLong ( hWndParent, GWL_HINSTANCE );

	wFlags = IML_TEXT_REPLY;   /* Expect a reply with SHOW cmd */

	nErrorCount = 0;           /* no errors yet                */


	/**     Loop round each ATKCHAN Info structure in the
	***     array.
	***
	**/
	for ( nIndex = 0; nIndex < nChans; nIndex++ ) {


		/**     Send the LIMITS command to the channel specified in
		***     the ATKCHANINFO structure.
		**/
		sprintf ( szCommand, "LIM %s",
				pciChanList [ nIndex ].szChannel );



		/**
		***     Send the IML command.
		**/
		nResult = ATKProcessIMLCommand ( hWndParent,
					nIMLHandle,
					szCommand,
					pciChanList [ nIndex ].nDeviceNumber,
					szReply,
					ATK_LIMITS_REPLY_LEN,
					wFlags );

		if ( bDestroy ) {
			return (0);
		}

		if ( nResult != OK ) {

			/**     An error has occured in ATKProcessIMLCommand
			***     It will have already been reported to the user
			***     by that function. This error will probably
			***     occur on other channels so don't attempt to
			***     continue..
			**/
			strcpy ( pciChanList [ nIndex ].szMin,
			               ATK_DEFAULT_MIN_LIMIT );

			strcpy ( pciChanList [ nIndex ].szMax,
			                ATK_DEFAULT_MAX_LIMIT );
			nErrorCount++;


			/**     Propergate the error  down to the
			***     remaining channels then break
			***     without attempting to process these
			***     channels
			**/
			for ( nChanInError = nIndex+1; nChanInError
			                        < nChans; nChanInError++ ) {


				/**     Give Error Chans default values
				**/
				strcpy ( pciChanList [ nChanInError ].szMin,
						ATK_DEFAULT_MIN_LIMIT );

				strcpy ( pciChanList [ nChanInError ].szMax,
						ATK_DEFAULT_MAX_LIMIT );

				nErrorCount++;
			}
			break;

		}



		/**     Extract the Limits strings from the Reply
		***     The returned string will contain the two
		***	values e.g. :
		***
		***              -10.24 10.24
		***
		***     or the string "Unrecognised Command"
		***
		**/
		if ( strspn ( szReply, "\t\n\r 1234567890eE.+-" ) ==
						strlen ( szReply ) ) {

			/**     Only-numerical characters in reply
			***     Assume this is a valid reply and not
			***     error text.
			**/

			/**     Extract the two values, assuming
			***     a space seperator...
			**/
			sscanf ( szReply, "%s %s",
				pciChanList [ nIndex ].szMin,
				pciChanList [ nIndex ].szMax );

		} else {

			/**     Invalid reply recieved,
			***	use default values
			**/
			strcpy ( pciChanList [ nIndex ].szMin,
					ATK_DEFAULT_MIN_LIMIT );

			strcpy ( pciChanList [ nIndex ].szMax,
					ATK_DEFAULT_MAX_LIMIT );
		}

	} /* Next Channel */

	return ( nErrorCount );
}
