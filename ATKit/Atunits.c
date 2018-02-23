/**
***     ATunits.c               IMLAPPS-IML Applications Toolkit
***
***     IMLAPPS-ATK-P-3.00	Andrew Thomson
***
***     This file contains the source for the following toolkit
***     functions :
***
***             ATKChannelUnits
***
***
***     The functions in this module should be used by
***     Windmill applications to obtain the units for
***     a list of channels.
***
***
***     To use this module :
***
***
***     - Specify the include file 'ATKit.h' in the module
***       which will call the functions. This should be located
***       after including <windows.h>
***
***     - Specify the include files 'IMSLIB.h' and 'IMLInter.h'
***       if you have not already done so.
***
***     - Include the following toolkit files in the build list :
***
***             ATunits, ATerror.c
***
***
***
***
***     Associated Files
***
***             ATKIT.h       - Contains the Fn prototypes, structure
***                             declarations and manifest constants.
***
***             ATerror.c     - Used to Report errors occuring within
***                             this module.
***
***     VERSION HISTORY :
***
***     17 September 91    ATK-P-1.00   First Version Written.
***     21 January   94    ATK-P-3.00   On/Off capitalized for consistancy
***                                     with SetupIML
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
***     ATKChannelUnits
***
***     Interrogates the IML Device, extracting the Units string
***     for each channel in the given channel list.
***
***     ARGUMENTS :
***
***       hWndParent            The Window Handle of the calling
***                             application.
***
***       nIMLHandle            The IML Device handle. The application
***                             must obtain this handle by calling
***                             IMLOpenInterface.
***
***       pciChanList           Near Pointer to an array of ATKCHANINFO
***                             structures. A units string will be
***                             obtained for each channel structure in
***                             the array and the string placed in the
***                             structure. Note the Channel Name, Channel
***                             Number and Channel Device Number must be
***                             set in each structure before calling this
***                             function.
***
***       nChans                The number of structures in the above
***                             array.
***
***     RETURN VALUE :
***
***             Number of channels which incurred errors whilst
***             attempting to extract the units.
**/
short ATKChannelUnits ( HWND hWndParent, short nIMLHandle,
                        PATKCHANINFO pciChanList, short nChans )
{
	char    szCommand    [ ATK_SHOW_CMD_LEN + 1 ];
	char    szReply      [ ATK_SHOW_REPLY_LEN + 1 ];
	char    *szUnitsAttr;
	char    *szSeparator;
	WORD    wFlags;
	short   nResult;
	HANDLE  hInst;
	short   nErrorCount;
	short   nChanInError;
	short   nSourceIndex, nDestIndex, nIndex;



	hInst = (HINSTANCE) GetWindowLong ( hWndParent, GWL_HINSTANCE );

	wFlags = IML_TEXT_REPLY;   /* Expect a reply with SHOW cmd */

	nErrorCount = 0;           /* no errors yet                */


	/**     Loop round each ATKCHAN Info structure in the
	***     array.
	***
	**/
	for ( nIndex = 0; nIndex < nChans; nIndex++ ) {


		/**     Send the SHOW command to the channel specified in
		***     the ATKCHANINFO structure.
		**/
		sprintf ( szCommand, "SHO %s U",
		              pciChanList [ nIndex ].szChannel );



		/**
		***     Send the IML command.
		**/
		nResult = ATKProcessIMLCommand ( hWndParent,
		                 nIMLHandle,
		                 szCommand,
		                 pciChanList [ nIndex ].nDeviceNumber,
		                 szReply,
		                 ATK_SHOW_REPLY_LEN,
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
			pciChanList [ nIndex ].szUnits [ 0 ] = '\0';
			nErrorCount++;


			/**     Propergate the error  down to the
			***     remaining channels then break
			***     without attempting to process these
			***     channels
			**/
			for ( nChanInError = nIndex+1; nChanInError
			                        < nChans; nChanInError++ ) {


				pciChanList
				    [ nChanInError ].szUnits [ 0 ] = '\0';

				nErrorCount++;
			}
			break;

		}



		/**     Extract the Units string from the Reply
		***     The string will look as follows :
		***
		***              0:01200 U=Volts
		***
		***     We are only interested in the string
		***     following the '=' character.
		***
		**/
		szUnitsAttr = strstr ( szReply, "=" );
		szUnitsAttr++;	/* Advance over the '=' character */

		/** Also check for a separator character
		**/
		szSeparator = strstr ( szUnitsAttr, "/" );


		/** Transfer the string segment of interest
		*** to the Channel structure.
		**/
		if ( szUnitsAttr == NULL ) {

			/**     Show Failed...
			**/
			pciChanList [ nIndex ].szUnits [ 0 ] = '\0';
			nErrorCount++;

			continue;  /* ...Try Next Channel */
		}


		if ( ( pciChanList [ nIndex ].bDigital == TRUE ) &&
					 ( szSeparator != NULL ) ) {


			/** This is a digital channel whose
			*** 'U' attribute contains user
			*** specified strings for the
			*** on/off states.
			*** The two strings are separated by
			*** a '/' character.
			**/



			nSourceIndex = 0; /* Index first charcter of attrib */
			nDestIndex   = 0; /* Index first charcter of szOn   */

			/** Copy character by character until
			*** divider character reached
			**/
			while ( szUnitsAttr [ nSourceIndex ] != '/' ) {

				pciChanList [ nIndex ].szOn [ nDestIndex++ ] =
						szUnitsAttr [ nSourceIndex++ ];

			}
			/** Terminate 'On' str
			**/
			pciChanList [ nIndex ].szOn [ nDestIndex ] = '\0';


			/** Now repeat for szOff string,
			*** copying what remains of the
			*** attribute string.
			**/
			nSourceIndex++; /* Skip the divider character */
			nDestIndex = 0; /* Index first char of szOff  */

			/** Copy character by character until
			*** string terminator is reached
			**/
			while ( szUnitsAttr [ nSourceIndex ] != '\0' ) {

				pciChanList [ nIndex ].szOff [ nDestIndex++ ]
					     = szUnitsAttr [ nSourceIndex++ ];

			}

			/** Terminate 'Off' str
			**/
			pciChanList [ nIndex ].szOff[ nDestIndex ] = '\0';



			/** Since we have On/Off state names
			*** then we do not have any units.
			**/
			pciChanList [ nIndex ].szUnits [ 0 ] = '\0';


		} else {

			/** This is a channel whose 'U' attribute
			*** is used to store a standard 'Units'
			*** string.
			**/


			/** Copy the reply following the '=' char
			*** into the 'szUnits' field and give the
			*** on/off strings default values.
			**/
			strcpy ( pciChanList [ nIndex ].szUnits, szUnitsAttr );
			strcpy ( pciChanList [ nIndex ].szOn,  "ON" );
			strcpy ( pciChanList [ nIndex ].szOff, "OFF" );

		}


	} /* Next Channel */

	return ( nErrorCount );
}
