/**
***	ATmonit.c		  IMLAPPS-IML Applications Toolkit
***
***	IMLAPPS-ATK-P-2.01	Andrew Thomson
***
***	This file contains the source for the following toolkit
***	functions :
***
***		ATKGetAlarmLevels
***
***
***	The functions in this module should be used by
***	Windmill applications to determine the Warning
***	and Critical alarm levels for a specified IML
***	channel.
***
***
***	To use this module :
***
***
***	- Include the line 'rcinclude ATKit.rc' in the
***	  application's Resource file.
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
***		ATmonit, ATerror.c
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
***	12 March 1993	 ATK-P-2.00	First Version of this Module.
***	29 July  1993	 ATK-P-2.01	Correction to work with devices > 0
**/
#define NOKANJI
#define NOCOMMS
#define IML_APPLICATION

#include <windows.h>
#include <windowsx.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ATkit.h"
#include "\Windmill 32\Compile Libs\imslib.h"
#include "\Windmill 32\Compile Libs\IMLInter.h"


extern BOOL bDestroy;



/**     Intrinsic Function Prototypes
**/
static void GetAttribute ( char *szSHOWReply, char *szDest, char cAttribute );


/**
***	Private Variables, accessable only to THIS module.
**/


/**
***	ATKGetAlarmLevels
***
***
***	This function takes ...
***
***	ARGUMENTS
***
***	  hWndParent            The Window Handle of the calling
***	                        application.
***
***	  nIMLHandle            Handle to COMMSLIB library
***
***
***
***
***	RETURN VALUE
**/
short ATKGetChannelAlarmLevels ( HWND hWndParent, short nIMLHandle,
			PATKCHANINFO pciChan, PATKALARMLEVELS palLevels )
{
	char		szCommand    [ ATK_SHOW_CMD_LEN + 1 ];
	char		szSHOWReply  [ ATK_SHOW_REPLY_LEN + 1 ];
	char		szMONReply   [ ATK_MON_REPLY_LEN + 1 ];
	HINSTANCE	hInst;
	WORD		wFlags;
	short		nResult;
	short		nSourceIndex, nDestIndex;

	char		szInputChan   [ MAX_NAME + 1 ];
	char		szMonitorChan [ MAX_NAME + 1 ];
	char		szOutputChan  [ MAX_NAME + 1 ];
	char		szAttrib      [ MAX_NAME + 1 ];





	/** Default return value
	**/
	palLevels->bCriticalLevel = FALSE;
	palLevels->bWarningLevel  = FALSE;



	hInst = (HINSTANCE) GetWindowLong ( hWndParent, GWL_HINSTANCE );

	wFlags = IML_TEXT_REPLY;   /* Expect a reply with SHOW cmd */





	/**	Send the MONITOR command to the channel specified in
	***	the ATKCHANINFO structure.
	**/
	sprintf ( szCommand, "MON %s", pciChan->szChannel );


	/**
	***	   Send the IML command.
	**/
	nResult = ATKProcessIMLCommand ( hWndParent,
	                         nIMLHandle,
	                         szCommand,
				 pciChan->nDeviceNumber,
				 szMONReply,
				 ATK_MON_REPLY_LEN,
	                         wFlags );

	if ( bDestroy ) {
		return (0);
	}


	if ( nResult != OK ) {

		/**	An error has occured in ATKProcessIMLCommand
		***	It will have already been reported to the user
		***	by that function.
		**/
		return ( nResult );
	}



	/**	Interperate the Reply from the MONITOR command
	***	It may look as follows :
	***	The string will look as follows :
	***
	***
	***	   "Unrecognised command MON"	- No alarm support
	***
	***	   "-"				- no monitors in use
	***
	***	   "XXXXX YYYYY ZZZZZ XXXXX YYYYY ZZZZZ etc. etc.
	***
	***	 where XXXXX is the channel number of the Monitor Input.
	***	       YYYYY is the Monitor channel number.
	***	       ZZZZZ is the output channel ( or 'N' if none )
	***
	**/
	if ( ( szMONReply [ 0 ] == '-' )
		|| ( szMONReply	[ 0 ] == 'U' ) || ( szMONReply [ 0 ] == 'u' ) ) {

		/** Monitors not supported, or are not configured.
		**/
		return ( OK );
	}


	nSourceIndex = 0;
	while ( szMONReply [ nSourceIndex ] != '\0' ) {


		/**	Extract the three fields for each MONITOR
		***	channel assocaited with our Input Channel.
		**/


		/***	First the Input Channel Number......
		**/
		nDestIndex = 0;
		while ( ( szMONReply [ nSourceIndex ] != ' '  ) &&
				( szMONReply [ nSourceIndex ] != '\0' ) ) {


			/**	Copy this field into a new string
			**/
			szInputChan [ nDestIndex++ ] =
					szMONReply [ nSourceIndex++ ];
		}
		szInputChan [ nDestIndex ] = '\0'; /* Terminate new string */


		/**	Advance over space characters in the Source buffer
		**/
		while ( szMONReply [ nSourceIndex ] == ' ' ) {

			nSourceIndex++;
		}












		/**
		***	...Next the Monitor channel Number...
		**/
		nDestIndex = 0;
		while ( ( szMONReply [ nSourceIndex ] != ' ' ) &&
				( szMONReply [ nSourceIndex ] != '\0' ) ) {

			szMonitorChan [ nDestIndex++ ] =
					szMONReply [ nSourceIndex++ ];
		}
		szMonitorChan [ nDestIndex ] = '\0';




		/**	Advance over space characters in the Source buffer
		**/
		while ( szMONReply [ nSourceIndex ] == ' ' ) {

			nSourceIndex++;
		}










		/**
		***	..Lastly the Output channel Number
		**/
		nDestIndex = 0;
		while ( ( szMONReply [ nSourceIndex ] != ' ' ) &&
				( szMONReply [ nSourceIndex ] != '\0' ) ) {

			szOutputChan [ nDestIndex++ ] =
					szMONReply [ nSourceIndex++ ];
		}
		szOutputChan [ nDestIndex ] = '\0';



		/**	Advance over space characters in the Source buffer
		**/
		while ( szMONReply [ nSourceIndex ] == ' ' ) {

			nSourceIndex++;
		}





		/**
		***	Check the input channel matches the requested
		***	channel...
		***	Assumes Input & Monitor channels reside in same device
		***	(corrected version 2.01 July 1993)
		***	The device number may be 1 or 2 characters ( upto 30 )
		***	so strip off the last 4 characters to give the
		***	module/channel number for the comparison March 95
		**/
		if ( atoi ( &(szInputChan [ strlen ( szInputChan ) - 4 ] ) )
					    == pciChan->nChannelNumber ) {

			/**	This monitor channel is being USED
			***	to monitor OUR input. Lets find the
			***	attributes of th monitor channel.
			**/


			/**	Send the SHOW command to the Monitor
			***	Channel requesting the 'T' and 'U'
			***	attributes.
			**/
			sprintf ( szCommand, "SHO %s T S U",
							szMonitorChan );

			/**
			***	   Send the IML command.
			**/
			nResult = ATKProcessIMLCommand ( hWndParent,
			         nIMLHandle,
			         szCommand,
				 pciChan->nDeviceNumber,
				 szSHOWReply,
				 ATK_SHOW_REPLY_LEN,
			         wFlags );


			if ( bDestroy ) {
				return (0);
			}

			if ( nResult != OK ) {

				/**	An error has occured in ATKProcessIMLCommand
				***	It will have already been reported to the user
				***	by that function.
				**/

				return ( nResult );
			}



			/**	Check the 'U=' attribute first.
			***	This first character MUST be 'W'
			***	of 'C' ( Warning or critical )
			**/
			GetAttribute ( szSHOWReply, szAttrib, 'U' );
			switch ( szAttrib [ 0 ] ) {

				case 'w' :
				case 'W' :

					/**	Check the 'T=' attribute first.
					***	This will be either 'H' or 'L'
					***	if monitoring an alarm
					**/
					GetAttribute ( szSHOWReply, szAttrib, 'T' );

					switch ( szAttrib [ 0 ] ) {

						case 'h' :
						case 'H' :

							GetAttribute ( szSHOWReply,
									szAttrib, 'S' );

							palLevels->dWarningLevel =
									atof ( szAttrib );

							palLevels->nWarningType	=
									ALARM_HIGH;

							palLevels->bWarningLevel = TRUE;

							break;

						case 'l' :
						case 'L' :


							GetAttribute ( szSHOWReply,
									   szAttrib, 'S' );

							palLevels->dWarningLevel =
									atof ( szAttrib );

							palLevels->nWarningType
									= ALARM_LOW;

							palLevels->bWarningLevel = TRUE;

							break;

						default :

							break;
					}
					break;

				case 'c' :
				case 'C' :

					/**	Check the 'T=' attribute first.
					***	This will be either 'H' or 'L'
					***	if monitoring an alarm
					**/
					GetAttribute ( szSHOWReply, szAttrib, 'T' );

					switch ( szAttrib [ 0 ] ) {

					case 'h' :
					case 'H' :

						GetAttribute ( szSHOWReply,
								szAttrib, 'S' );

						palLevels->dCriticalLevel =
								atof ( szAttrib );

						palLevels->nCriticalType = ALARM_HIGH;

						palLevels->bCriticalLevel = TRUE;

						break;

					case 'l' :
					case 'L' :


						GetAttribute ( szSHOWReply,
								   szAttrib, 'S' );

						palLevels->dCriticalLevel =
								atof ( szAttrib );

						palLevels->nCriticalType = ALARM_LOW;

						palLevels->bCriticalLevel = TRUE;


						break;

					default :
						break;
					}
					break;
			}
		}

	} /* End of reply */


	return ( OK );
}




/**
***	GetAttribute
***
***	Returns by reference the attribute field identified by cAttribute.
***
***	szShowReply	Points to the reply from a SHOW command.
***	szDest		Pointer to string to receive attribute field
***	cAttribute	Attribute required.
***
***	No return value.
**/
static void GetAttribute ( char *szSHOWReply, char *szDest, char cAttribute )
{
	char	szSearchFor [ 3 ];	/* Search string	*/
	short	nSource, nDest; 	/* Char array indexs	*/
	char	*szAttrbStart;		/* Pointer into reply	*/





	/**  Default return string to NULL length
	***  in case attribute not found
	**/
	szDest [ 0 ] = '\0';


	/**
	***	Build the identifing string to search for.
	***	e.g "T=\0'
	**/
	szSearchFor [ 0 ] = (char) toupper ( (int) cAttribute );
	szSearchFor [ 1 ] = '=';
	szSearchFor [ 2 ] = '\0';


	/**	Search the reply string
	**/
	szAttrbStart = strstr ( szSHOWReply, szSearchFor );
	if ( szAttrbStart != NULL ) {


		nSource = 2;	/* Skip 'T=' or whatever in source */
		nDest	= 0;

		/**	Copy the attribute field to the destination
		***	string
		***
		**/
		while ( ( szAttrbStart [ nSource ] != '\0' ) &&
				( szAttrbStart [ nSource ] != ' ' ) ) {

			szDest [ nDest++ ] = szAttrbStart [ nSource++ ];

		}
		szDest [ nDest ] = '\0';   /* Terminate Destination string */

	}
	return;
}
