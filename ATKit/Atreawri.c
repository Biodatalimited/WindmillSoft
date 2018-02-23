/**
***     ATreawri.c              IMLAPPS-IML Applications Toolkit
***
***     IMLAPPS-ATK-P-3.01	 Andrew Thomson
***
***     This file contains the source for the following toolkit
***     functions :
***
***             ATKReadChannel
***             ATKSetReadList
***             ATKReadList
***             ATKWriteChannel
***             ATKSetWriteList
***             ATKWriteList
***
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
***     - Specify the include files 'IMSLIB.h' and 'IMLInter.h'
***       if you have not already done so.
***
***     - Include the following toolkit files in the build list :
***
***             ATreawri, ATerror.c
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
**/
#define NOKANJI
#define NOCOMMS
#define IML_APPLICATION

#include <windows.h>
#include <windowsx.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <sys\types.h>
#include <sys\timeb.h>
#include "ATkit.h"
#include "\Windmill 32\Compile Libs\imslib.h"
#include "\Windmill 32\Compile Libs\IMLInter.h"

extern BOOL bDestroy;

/**     Intrinsic Function Prototypes
**/
void  ATKGetTime ( PATKCHANINFO pciRead );
short ReadChannel ( HWND hWndParent, short nIMLHandle, PATKCHANINFO pciRead );
short ReadChannelAndAlarmState ( HWND hWndParent,
				     short nIMLHandle, PATKCHANINFO pciRead );
static short GetMonitorChanType ( char *szMonitorName );


/**
***     Private Variables, accessable only to THIS module.
**/
static  LOCALHANDLE       hPrivateReadList      = NULL;
static  LOCALHANDLE       hPrivateWriteList     = NULL;
static  PATKCHANINFO      pciPrivateReadList    = NULL;
static  PATKCHANINFO      pciPrivateWriteList   = NULL;
static  short             nPrivateReadListSize  = 0;
static  short             nPrivateWriteListSize = 0;
static  BOOL              abDeviceSupportsAlarms[ ATK_MAX_DEVICES ];
static  LPATKMONITORCHAN  lpmcMonitorTable      = (LPATKMONITORCHAN) NULL;


/**
***     ATKReadChannel
***
***
***     This function takes a single time-stamped reading from
***     the channel specified in the given ATKCHANINFO structure.
***
***     The ATKCHANINFO structure must contain the channel's name
***     the channel number and the device number.
***
***
***     If successful, the channel value and timestamp are stored
***     in the structure otherwise if the function fails to obtain
***     a valid, an IML error code is stored in the structure's
***     'nError' field.
***
***
***     ARGUMENTS
***
***       hWndParent            The Window Handle of the calling
***                             application.
***
***       nIMLHandle            The IML Device handle. The application
***                             must obtain this handle by calling
***                             IMLOpenInterface.
***
***       pciRead               Near Pointer to a ATKCHANINFO
***                             structure which must contain the
***                             IML channel's name, number and
***                             device number. The value, timestamp
***                             and error fields will be set by
***                             this fn.
***
***       bReadAlarmState       If set TRUE the 'nAlarmState' field
***                             will also be set. This indicates whether
***                             the current reading triggered an IML
***                             alarm. The field will take one of
***                             the following values :
***
***                                     ALARM_NORMAL
***                                     ALARM_WARNING
***                                     ALARM_CRITICAL
***
***
***
***     RETURN VALUE
***
***       TRUE                  A reading was sucessfully taken.
***                             The reading will be stored in the
***                             structure's szValue field and the
***                             timestamp in the ulTime and nMSec
***                             fields. ( see Function Spec for
***                             details on timestamps )
***
***
***       FALSE                 An error occured whilst attempting
***                             to read the channel. The IML error
***                             code is returned in the structure's
***                             nError field.
**/
short ATKReadChannel ( HWND hWndParent,
	short nIMLHandle, PATKCHANINFO pciRead, BOOL bReadAlarmState )
{

	static short    bInitialized = FALSE;
	short           nDevice;



	if ( ( !bInitialized ) &&
			( lpmcMonitorTable == (LPATKMONITORCHAN) NULL ) ) {

		/**
		***     Assume No device support alarms until
		***     we actually locate Monitor Channels
		***     for the device.
		***
		***
		**/
		for ( nDevice = 0; nDevice < ATK_MAX_DEVICES; nDevice++ ) {

			abDeviceSupportsAlarms [ nDevice ] = FALSE;
		}

	}



	/**     This function supports both IML devices which
	***     support IML alarms and IML devices which do not.
	***
	***     We have two intrinic functions one utilisizing
	***     the standard IML READ command and the other
	***     the newer IML AREAD command.
	***
	***     Use the latter if requested ( bReadAlarmStates == TRUE )
	***     and the device actually has IML Alarm support.
	***
	**/
	if ( ( bReadAlarmState == TRUE ) &&
		( abDeviceSupportsAlarms [ pciRead->nDeviceNumber ] ) ) {


		return ( ReadChannelAndAlarmState ( hWndParent,
						nIMLHandle, pciRead ) );

	} else {

		return ( ReadChannel ( hWndParent, nIMLHandle, pciRead ) );

	}

}





/**
***     ATKSetReadList
***
***     This function specifies a list of channels to be read by
***     the associated toolkit function : ATKReadList.
***     This function implements any optimisations for reading
***     data for the list of channels.
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
***       pciReadList           Near Pointer to an array of ATKCHANINFO
***                             structures which should be read by a
***                             call to ATKReadList. Each structure in
***                             the array must contain the channel
***                             name, the IML channel number and the
***                             device number.
***
***       nChans                Specifies the size of the ATKCHANINFO
***                             array.
***
**/
short ATKSetReadList ( HWND hWndParent,
		 short nIMLHandle, PATKCHANINFO pciReadList, short nChans )

{
	short           nIndex;         /* Used to index ATKCHANINFO array */

	/**
	***     Create one or more IML Macros, to read all of
	***     the channels specified in pciReadList. One Macro
	***     will be needed for each IML Device in use.
	***
	***     Optimisation may be included in the Macros.
	***     e.g.
	***
	***          Using Ranges on READ cmds
	***          Reading multiple devices in Parallel.
	***
	***     MACRO Names will be named 'TRn' were 'n' is the
	***     device no. The Macro(s) will be invoked by
	***     ATLReadList.
	**/


	/**
	***     ---------oooo  I M P O R T A N T  oooo--------
	***
	***
	***     In this initial version, we will not implement
	***     the macro facility, instead keeping an internal
	***     list of channels to be read. The list will be
	***     accessible to ATKReadList which will use a READ
	***     command for each channel in the list.
	***
	***
	***
	***
	***     We will implement marcos when the IML macro
	***     facility has been enhanced.
	**/



	if ( hPrivateReadList != NULL ) {

		/**     Destroy the existing List
		**/
		LocalUnlock ( hPrivateReadList );
		LocalFree   ( hPrivateReadList );
	}

	/**     Create the new list
	**/
	hPrivateReadList =  LocalAlloc ( LMEM_MOVEABLE,
				(WORD) nChans * sizeof ( ATKCHANINFO ) );

	if ( hPrivateReadList == NULL ) {

		nPrivateReadListSize = 0;
		pciPrivateReadList   = NULL;

		return ( ATK_ERR_NO_LOCAL_MEMORY );
	}


	/**     Lock the list. This will remain locked until either
	***     this function is recaled with a new list or the
	***     application terminates...
	**/
	pciPrivateReadList =  (PATKCHANINFO) LocalLock ( hPrivateReadList );

	if ( pciPrivateReadList == NULL ) {

		LocalFree ( hPrivateReadList );

		hPrivateReadList        = NULL;
		nPrivateReadListSize    = 0;

		return ( ATK_ERR_NO_LOCAL_MEMORY );
	}




	/**     Fill the list with the channel details...
	***     We will use straight-forward structure assignment
	***     to copy the applications ATKCHANINFO array into
	***     our own private copy..
	**/
	for ( nIndex = 0; nIndex < nChans; nIndex++ ) {

		pciPrivateReadList [ nIndex ] = pciReadList [ nIndex ];
	}


	/**     Store the size of the ReadList.
	**/
	nPrivateReadListSize = nChans;

	/**     Return with no errors
	**/
	return ( OK );

}





/**
***     ATKReadList
***
***     This function take a reading for each channel specified in the
***     current 'ReadList'. The ReadList is created by a call to
***     the ATKSetReadList function
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
***       pciReadList           Near Pointer to an array of ATKCHANINFO
***                             structures in which the readings are
***                             to be stored.
***
***       bReadAlarmState       If set TRUE the 'nAlarmState' field
***                             will also be set. This indicates whether
***                             the current reading triggered an IML
***                             alarm. The field will take one of
***                             the following values :
***
***                                     ALARM_NORMAL
***                                     ALARM_WARNING
***                                     ALARM_CRITICAL
***
***      RETURN VALUE           Number of channels which returned errors.
***
***
**/
short ATKReadList ( HWND hWndParent, short nIMLHandle,
			PATKCHANINFO pciReadList, BOOL bReadAlarmStates )
{

	short   nIndex;
	short   nErrorCount;
	BOOL	bChannelRead;

	/**
	***     ---------oooo  I M P O R T A N T  oooo--------
	***
	***
	***     In this initial version, we will not implement
	***     the IML macro facility, instead keeping an internal
	***     list of channels to be read. The list will be
	***     created by ATKSetReadList. This func will send
	***     a READ command for each channel in the list.
	***
	***
	***     We will implement marcos when the IML macro
	***     facility has been enhanced.
	**/

	/**     We will also keep a running total of channels in error.
	**/
	nErrorCount = 0;


	for ( nIndex = 0; nIndex < nPrivateReadListSize; nIndex++ ) {

		bChannelRead = ATKReadChannel ( hWndParent, nIMLHandle,
			   &( pciReadList [ nIndex ] ), bReadAlarmStates );

		/**    Classify the type of error.
		***    If we have an IML error simply increment the
		***    error count and continue with the next channel.
		***    For any other type of error, abort the fn.
		**/
		if ( ( !bChannelRead ) &&
		   ( pciReadList [ nIndex ].nError >= FIRST_IML_EXEC_ERR ) &&
		   ( pciReadList [ nIndex ].nError <= LAST_IML_OTHERS_ERR ) ) {

			/**     An error has occured in the IML Device
			***     This will probably not effect other channels
			***     so just increment the error count.
			**/
			nErrorCount++;


		} else if ( !bChannelRead ) {


			/**     ATKReadChannel has failed and the channel
			***     could NOT be read. Since this error is
			***     probably a Comms or memory error ( Not an IML
			***	error ) other channels may or may not fail.
			***	Pre-version 3 we copied the error code to all
			***	remaining channels and called break to exit the
			***	for loop. This stops other devices being read
			***	so this was removed and the code simply incs
			***	the error count as above.
			**/

			nErrorCount++; /* Inc the error count for this chan */


		}

	} /* Next Channel in List */

	return ( nErrorCount );
}




/**
***     ATKWriteChannel
***
***
***     This function takes a single time-stamped reading from
***     the channel specified in the given ATKCHANINFO structure.
***
***     The ATKCHANINFO structure must contain the channel's name
***     the channel number and the device number.
***
***
***     If successful, the channel value and timestamp are stored
***     in the structure otherwise if the function fails to obtain
***     a valid, an IML error code is stored in the structure's
***     'nError' field.
***
***
***     ARGUMENTS
***
***       hWndParent            The Window Handle of the calling
***                             application.
***
***       nIMLHandle            The IML Device handle. The application
***                             must obtain this handle by calling
***                             IMLOpenInterface.
***
***       pciRead               Near Pointer to a ATKCHANINFO
***                             structure which must contain the
***                             IML channel's name, number and
***                             device number. The value, timestamp
***                             and error fields will be set by
***                             this fn.
***
***     RETURN VALUE
***
***       TRUE                  A reading was sucessfully taken.
***                             The reading will be stored in the
***                             structure's szValue field and the
***                             timestamp in the ulTime and nMSec
***                             fields. ( see Function Spec for
***                             details on timestamps )
***
***
***       FALSE                 An error occured whilst attempting
***                             to read the channel. The IML error
***                             code is returned in the structure's
***                             nError field.
**/
short ATKWriteChannel ( HWND hWndParent,
				short nIMLHandle, PATKCHANINFO pciWrite )
{
	char    szCommand [ ATK_READ_CMD_LEN + 1 ];
	char    szReply   [ ATK_REP_CMD_REPLY_LEN + 1 ];
	WORD    wFlags;
	short   nFieldsConverted;
	short   nResult;
	HANDLE  hInst;
	long    lError;




	hInst = (HINSTANCE) GetWindowLong ( hWndParent, GWL_HINSTANCE );

	wFlags = IML_NO_REPLY;   /* No Reply expected from WRITE cmd  */





	/**     Send the WRITE command to the channel specified in
	***     the ATKCHANINFO
	***     structure.
	**/
#if defined DEMO_VERSION
	/**  If compiling a demonstartion version replace
	***  the normal 'WRITE' command with the '#$' command
	**/
	sprintf ( szCommand, "#$ %s %s",
				pciWrite->szChannel, pciWrite->szValue );
#else
	sprintf ( szCommand, "WRI %s %s",
				pciWrite->szChannel, pciWrite->szValue );
#endif


	/**
	***     Send the IML command. We may need to
	***     make repeated calls to ATKProcessIMLCommand
	***     since it may already be processing an existing
	***     command....
	**/
	nResult = ATKProcessIMLCommand ( hWndParent,
				 nIMLHandle,
				 szCommand,
				 pciWrite->nDeviceNumber,
				 NULL,
				 MAX_NAME,
				 wFlags );

	if ( bDestroy ) {
		return (0);
	}


	sprintf ( szCommand, "WRI %s %s",
				pciWrite->szChannel, pciWrite->szValue );


	if ( nResult != OK ) {

		/**     An error has occured in ATKProcessIMLCommand
		***     It will have already been reported to the user
		***     by that function. Set the ATKCHANINFO structures
		***     error value and exit this routine.
		**/
		pciWrite->nError = nResult;
		strcpy ( pciWrite->szValue, "WRITE Failed" );

		return ( FALSE );
	}







	/**     Determine whether the WRITE command was successfull
	***     by sending a REPORT command with no parameters. This
	***     reports only the error code left by the previous
	***     command.
	**/
	strcpy ( szCommand, "REP" );
	wFlags = IML_TEXT_REPLY;        /* Reply expected with REP cmd */


	/**
	***     Send the IML command. We may need to
	***     make repeated calls to ATKProcessIMLCommand
	***     since it may already be processing an existing
	***     command....
	**/
	nResult = ATKProcessIMLCommand ( hWndParent,
				 nIMLHandle,
				 szCommand,
				 pciWrite->nDeviceNumber,
				 szReply,
				 ATK_REP_CMD_REPLY_LEN,
				 wFlags );

	if ( bDestroy ) {
		return (0);
	}



	if ( nResult != OK ) {

		/**     An error has occured in ATKProcessIMLCommand
		***     It will have already been reported to the user
		***     by that function. Set the ATKCHANINFO structure's
		***     error value and exit the routine.
		**/
		strcpy ( pciWrite->szValue, "WRITE Failed" );
		pciWrite->nError = nResult;

		return ( FALSE );
	}

	/**     Finally return the outcome of this function
	***     TRUE is we managed to write valid data or
	***     FALSE if not.
	***     Temporary lError added for 32 bit since type int
	***     ie long in 32 bit is needed. Alternative would be to
	***     put a long into the structure.
	**/
	nFieldsConverted = sscanf ( szReply, "%d", &lError  );
	
	pciWrite->nError = (short) lError;


	if ( nFieldsConverted == 0 ) {

		/**     Invalid reply to the REPORT command
		**/
		pciWrite->nError = ATK_ERR_INVALID_IML_REPLY;
		strcpy ( pciWrite->szValue, "WRITE Failed" );

		/** MessageBox ( hInst, NULL, ATK_ERR_INVALID_IML_REPLY,
		***        NULL, MB_OK | MB_ICONEXCLAMATION );
		**/

	}

	return ( ( pciWrite->nError == 0 ) ? TRUE : FALSE );

}






/**
***     ATKSetWriteList
***
***     This function specifies a list of channels which will
***     send a data value to an IML device whenever the associated
***     toolkit function, ATKWriteList, is called.
***     This function implements any optimisations for writing
***     data from the list of channels.
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
***       pciWriteList          Near Pointer to an array of ATKCHANINFO
***                             structures. Each channel described in the
***                             array may then be processed by calling
***                             ATKWriteList. Each structure in
***                             the array must contain the channel
***                             name, the output value string, the IML
***                             channel number and the device number.
***
***
***       nChans                Specifies the size of the ATKCHANINFO
***                             array.
***
**/
short ATKSetWriteList ( HWND hWndParent,
		 short nIMLHandle, PATKCHANINFO pciWriteList, short nChans )

{
	short           nIndex;         /* Used to index ATKCHANINFO array */

	/**
	***     Create one or more IML Macros, to send data, for
	***     all of the channels specified in pciWriteList. One
	***     Macro will be needed for each IML Device in use.
	***
	***     Optimisation may be included in the Macros.
	***     e.g.
	***
	***          Using Ranges on WRITE cmds
	***          Reading multiple devices in Parallel.
	***
	***     MACRO Names will be named 'TWn' were 'n' is the
	***     device no. The Macro(s) will be invoked by
	***     ATLReadList.
	**/


	/**
	***     ---------oooo  I M P O R T A N T  oooo--------
	***
	***
	***     In this initial version, we will not implement
	***     the macro facility, instead keeping an internal
	***     list of channels which will write data. The list
	***     will be accessible to ATKWriteList which will
	***     use a WRITE command for each channel in the list.
	***
	***
	***
	***
	***     We will implement marcos when the IML macro
	***     facility has been enhanced.
	**/



	if ( hPrivateWriteList != NULL ) {

		/**     Destroy the existing List
		**/
		LocalUnlock ( hPrivateWriteList );
		LocalFree   ( hPrivateWriteList );
	}

	/**     Create the new list
	**/
	hPrivateWriteList =  LocalAlloc ( LMEM_MOVEABLE,
				(WORD) nChans * sizeof ( ATKCHANINFO ) );

	if ( hPrivateWriteList == NULL ) {

		nPrivateWriteListSize = 0;
		pciPrivateWriteList   = NULL;

		return ( ATK_ERR_NO_LOCAL_MEMORY );
	}


	/**     Lock the list. This will remain locked until either
	***     this function is recaled with a new list or the
	***     application terminates...
	**/
	pciPrivateWriteList =  (PATKCHANINFO) LocalLock ( hPrivateWriteList );

	if ( pciPrivateWriteList == NULL ) {

		LocalFree ( hPrivateWriteList );

		hPrivateWriteList       = NULL;
		nPrivateWriteListSize   = 0;

		return ( ATK_ERR_NO_LOCAL_MEMORY );
	}




	/**     Fill the list with the channel details...
	***     We will use straight-forward structure assignment
	***     to copy the applications ATKCHANINFO array into
	***     our own private copy..
	**/
	for ( nIndex = 0; nIndex < nChans; nIndex++ ) {

		pciPrivateWriteList [ nIndex ] = pciWriteList [ nIndex ];
	}


	/**     Store the size of the WriteList.
	**/
	nPrivateWriteListSize = nChans;

	/**     Return with no errors
	**/
	return ( OK );

}





/**
***     ATKWriteList
***
***     This function output a data value for each channel specified in
***     the current 'WriteList'. The WriteList is created by a call to
***     the ATKSetWriteList function
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
***       pciWriteList          Near Pointer to an array of ATKCHANINFO
***                             structures in which the output values are
***                             stored.
***     RETURN VALUE :
***
***             Number of channels which incurred errors whilst
***             attempteing to write data.
***
**/
short ATKWriteList ( HWND hWndParent, short nIMLHandle,
						PATKCHANINFO pciWriteList )
{

	short   nIndex;
	short   nErrorCount;
	BOOL    bDataSent;

	/**
	***     ---------oooo  I M P O R T A N T  oooo--------
	***
	***
	***     In this initial version, we will not implement
	***     the IML macro facility, instead keeping an internal
	***     list of channels to be read. The list will be
	***     created by ATKSetWriteList. This func will send
	***     a WRITE command for each channel in the list.
	***
	***
	***     We will implement marcos when the IML macro
	***     facility has been enhanced.
	**/


	/**     We will also keep a running total of channels in error.
	**/
	nErrorCount = 0;

	for ( nIndex = 0; nIndex < nPrivateWriteListSize; nIndex++ ) {

		bDataSent = ATKWriteChannel ( hWndParent, nIMLHandle,
					       &( pciWriteList [ nIndex ] ) );

		/**    Classify the type of error.
		***    If we have an IML error simply increment the
		***    error count and continue with the next channel.
		***    For any other type of error, abort the fn.
		**/
		if ( ( !bDataSent ) &&
		  ( pciWriteList [ nIndex ].nError >= FIRST_IML_EXEC_ERR ) &&
		  ( pciWriteList [ nIndex ].nError <= LAST_IML_OTHERS_ERR ) ) {



			/**     An error has occured in the IML Device
			***     This will probably not effect other channels
			***     so just increment the error count.
			**/
			nErrorCount++;


		} else if ( !bDataSent ) {

			/**	ATKWriteChannel has failed and the channel
			***	could NOT be updated. Since this error is
			***	probably a Comms or memory error ( Not an IML
			***	error ) other channels may or may not fail.
			***	Pre-version 3 we copied the error code to all
			***	remaining channels and called break to exit the
			***	for loop. This stops other devices being
			***	processed so this was removed and the code
			***	simply incs the error count as above.
			**/

			nErrorCount++; /* Inc the error count for this chan */


		}

	} /* Next Channel In List */



	return ( nErrorCount );
}







/**
***     ATKGetTime
***
***     Time Stamps the given ATKCHANINFO structure.
***
***     The 'ulTime' field is set to the number of Secs since 00:00:00
***     GMT, January 1st, 1970.
***
***     The 'mSecs' field is set to the number of millisecs that have
***     elapsed since the above second. This value is significant to
***     hundreds of a millisec. ( The last digit is always zero ).
***
***     The function compenstaes for local time ( PTZ ) and daylight
***     saving.
***
***
***
***     ARGUMENTS :
***
***             pciRead      Near Pointer to a ATKCHANINFO
***                          structure which will be time
***                          stamped.
***
***     RETURN VALUE :
***
***             none.
***
***
**/
void ATKGetTime ( PATKCHANINFO pciRead )

{

	struct timeb timTimeStamp;


	ftime ( &timTimeStamp );


	/** Time in Secs since 00:00:00 GMT, January 1st, 1970
	**/
	pciRead->ulTime = (DWORD) timTimeStamp.time;

	/**     Adjust from Local Time
	***     ( which is by default in the Pacific Time Zone )
	**/
	pciRead->ulTime -= ( timTimeStamp.timezone * 60L );
	if ( timTimeStamp.dstflag ) {

		/**     Daylight saving in operation
		***     Add 3600 Seconds to convert
		***     GMT to BST
		***
		**/
		pciRead->ulTime += ( 60L * 60L );
	}

	/** Fraction of a second in milliseconds. The last digit
	*** is always zero, as millitm is incremented to the
	*** nearest one-hundredth of a second.
	**/
	pciRead->nMSecs = timTimeStamp.millitm;

	return;
}





/**
***     ReadChannel
***
***
***     This local function takes a single time-stamped reading from
***     the channel specified in the given ATKCHANINFO structure.
***
***     The ATKCHANINFO structure must contain the channel's name
***     the channel number and the device number.
***
***
***     If successful, the channel value and timestamp are stored
***     in the structure otherwise if the function fails to obtain
***     a valid, an IML error code is stored in the structure's
***     'nError' field.
***
***
***     ARGUMENTS
***
***       hWndParent            The Window Handle of the calling
***                             application.
***
***       nIMLHandle            The IML Device handle. The application
***                             must obtain this handle by calling
***                             IMLOpenInterface.
***
***       pciRead               Near Pointer to a ATKCHANINFO
***                             structure which must contain the
***                             IML channel's name, number and
***                             device number. The value, timestamp
***                             and error fields will be set by
***                             this fn.
***
***     RETURN VALUE
***
***       TRUE                  A reading was sucessfully taken.
***                             The reading will be stored in the
***                             structure's szValue field and the
***                             timestamp in the ulTime and nMSec
***                             fields. ( see Function Spec for
***                             details on timestamps )
***
***
***       FALSE                 An error occured whilst attempting
***                             to read the channel. The IML error
***                             code is returned in the structure's
***                             nError field.
**/
short ReadChannel ( HWND hWndParent, short nIMLHandle, PATKCHANINFO pciRead )
{
	char    szCommand [ ATK_READ_CMD_LEN + 1 ];
	char    szReply   [ ATK_READ_CMD_REPLY_LEN + 1 ];
	WORD    wFlags;
	short   nFieldsConverted;
	short   nResult;
	HANDLE  hInst;
	int		nTempInt;



	hInst = (HINSTANCE) GetWindowLong ( hWndParent, GWL_HINSTANCE );

	wFlags = IML_TEXT_REPLY;   /* Expect a reply with READ cmd */





	/**     Send the READ command to the channel specified in
	***     the ATKCHANINFO structure.
	**/
#if defined DEMO_VERSION
	/** If compiling the Demo Version of an Application
	*** use the '#*' command instead of the normal READ
	**/
	sprintf ( szCommand, "#* %s", pciRead->szChannel );
#else
	sprintf ( szCommand, "REA %s", pciRead->szChannel );
#endif


	/**
	***     Send the IML command.
	**/
	nResult = ATKProcessIMLCommand ( hWndParent,
				 nIMLHandle,
				 szCommand,
				 pciRead->nDeviceNumber,
				 szReply,
				 ATK_READ_CMD_REPLY_LEN,
				 wFlags );
				 
	if ( bDestroy ) {
		return (0);
	}

	if ( nResult != OK ) {

		/**     An error has occured in ATKProcessIMLCommand
		***     It will have already been reported to the user
		***     by that function. Set the ATKCHANINFO structures
		***     error value and exit this routine.
		**/
		pciRead->nError = nResult;
		strcpy ( pciRead->szValue, "READ Failed" );

		return ( FALSE );
	} else {
		/**     Copy as much of the reply as possible into
		***     the channel structure. NOTE we need to use
		***     a large reply buffer for the READ command in 
		***     case we recieve a long  reply. If the buffer
		***     is too small we will recieve a 
		***     COMMS_MORE_REPLY_TO_COME  error from the commslib
		***     which is not the error code we really want.
		**/
		strncpy ( pciRead->szValue, szReply, MAX_NAME );

		/**     NULL terminate the response
		***     ( the array is MAX_NAME + 1 )
		**/
		pciRead->szValue [ MAX_NAME ] = '\0';


	}



	/**     Store the time-stamp
	**/
	ATKGetTime ( pciRead );


	if ( szReply [ 0 ] != 'R' ) {


		/**	We did not receive a READ failed string
		***	so assume data okay.
		**/
		pciRead->nError = OK ;

	} else {

		/**	Looks like a READ Failed
		***	Determine whether the READ command was successfull
		***	by sending a REPORT command with no parameters. This
		***	reports only the error code left by the previous
		***	command.
		**/
		strcpy ( szCommand, "REP" );


		/**
		***	Send the IML REP command to the IML Device.
		**/
		nResult = ATKProcessIMLCommand ( hWndParent,
				 nIMLHandle,
				 szCommand,
				 pciRead->nDeviceNumber,
				 szReply,
				 ATK_READ_CMD_REPLY_LEN,
				 wFlags );

		if ( bDestroy ) {
			return (0);
		}



		if ( nResult != OK ) {

			/**	An error has occured in ATKProcessIMLCommand
			***	It will have already been reported to the user
			***	by that function. Set the ATKCHANINFO structure's
			***	error value and exit the routine.
			**/
			strcpy ( pciRead->szValue, "READ Failed" );
			pciRead->nError = nResult;

			return ( FALSE );
		}




		nFieldsConverted = sscanf ( szReply, "%d", &nTempInt );

		pciRead->nError = (short) nTempInt;
		
		if ( nFieldsConverted == 0 ) {

			/**	Invalid reply to the REPORT command
			**/
			pciRead->nError = ATK_ERR_INVALID_IML_REPLY;
			strcpy ( pciRead->szValue, "READ Failed" );



		}

	}

	/**	Finally return the outcome of this function
	***	TRUE is we managed to obtain valid data or
	***	FALSE if not.
	**/
	return ( ( pciRead->nError == 0 ) ? TRUE : FALSE );


}




/**
***     ReadChannelAndAlarmState
***
***
***     This local function takes a single time-stamped reading from
***     the channel specified in the given ATKCHANINFO structure and
***     additionally determines the alarm state of the channel.
***
***     The ATKCHANINFO structure must contain the channel's name
***     the channel number and the device number.
***
***
***     If successful, the channel value and timestamp are stored
***     in the structure otherwise if the function fails to obtain
***     a valid, an IML error code is stored in the structure's
***     'nError' field.
***
***
***     ARGUMENTS
***
***       hWndParent            The Window Handle of the calling
***                             application.
***
***       nIMLHandle            The IML Device handle. The application
***                             must obtain this handle by calling
***                             IMLOpenInterface.
***
***       pciRead               Near Pointer to a ATKCHANINFO
***                             structure which must contain the
***                             IML channel's name, number and
***                             device number. The value, timestamp
***                             alarm state and error fields will be
***                             set by this fn.
***
***     RETURN VALUE
***
***       TRUE                  A reading was sucessfully taken.
***                             The reading will be stored in the
***                             structure's szValue field, the alarm
***                             state in the nAlarmState field and the
***                             timestamp in the ulTime and nMSec
***                             fields. ( see Function Spec for
***                             details on timestamps )
***
***
***       FALSE                 An error occured whilst attempting
***                             to read the channel. The IML error
***                             code is returned in the structure's
***                             nError field.
**/
short ReadChannelAndAlarmState ( HWND hWndParent, short nIMLHandle,
						PATKCHANINFO pciRead )

{
	char    szCommand [ ATK_READ_CMD_LEN + 1 ];
	char    szReply   [ ATK_READ_CMD_REPLY_LEN + 1 ];

	char    *szValue;       /* Pointer into reply string */
	char    *szMonitorChan; /* Pointer into reply string */

	WORD    wFlags;
	short   nFieldsConverted;
	short   nResult;
	HANDLE  hInst;

	int		nTempInt;




	hInst = (HINSTANCE) GetWindowLong ( hWndParent, GWL_HINSTANCE );

	wFlags = IML_TEXT_REPLY;   /* Expect a reply with READ cmd */





	/**	Send the AREAD command to the channel specified in
	***     the ATKCHANINFO structure.
	**/
#if defined DEMO_VERSION
	/** If compiling the Demo Version of an Application
	*** use the '#%' command instead of the normal READ
	**/
	sprintf ( szCommand, "#%% %s", pciRead->szChannel );
#else
	sprintf ( szCommand, "ARE %s", pciRead->szChannel );
#endif


	/**
	***     Send the IML command.
	**/
	nResult = ATKProcessIMLCommand ( hWndParent,
				 nIMLHandle,
				 szCommand,
				 pciRead->nDeviceNumber,
				 szReply,
				 ATK_READ_CMD_REPLY_LEN,
				 wFlags );
			
	if ( bDestroy ) {
		return (0);
	}
				 

	if ( nResult != OK ) {

		/**     An error has occured in ATKProcessIMLCommand
		***     It will have already been reported to the user
		***     by that function. Set the ATKCHANINFO structures
		***     error value and exit this routine.
		**/
		pciRead->nError = nResult;
		strcpy ( pciRead->szValue, "AREAD Failed" );

		return ( FALSE );
	} else {

		/**  Tokenize the reply extracting
		***  the value and upto two monitor
		***  channel names. e.g.
		***
		***  '9.133\00802/Temp_High'
		**/
		szValue = strtok ( szReply, "\\/" );


		/**  Unless a monitor channel name is specified,
		***  the channel is assumed NOT to be in an alarm
		***  condition.
		***
		***  A reply may contain more than one monitor
		***  channel name. If one of these is monitoring
		***  a 'critical' alarm ( U=C attribute ), then
		***  the channel is in a Critical alarm state
		***  regardless of the type of the other monitor
		***  channel(s).
		**/
		pciRead->nAlarmState = ALARM_NORMAL;

		while  ( ( ( szMonitorChan =
			strtok ( NULL, "\\/"    ) ) != NULL )
			 && ( pciRead->nAlarmState != ALARM_CRITICAL ) ) {

			/**     This channel is in alarm
			***     Determine type of alarm
			**/
			pciRead->nAlarmState =
				     GetMonitorChanType ( szMonitorChan );
		}



		/**     Copy as much of the reply as possible into
		***     the channel structure. NOTE we need to use
		***     a large reply buffer for the READ command in 
		***     case we recieve a long  reply. If the buffer
		***     is too small we will recieve a 
		***     COMMS_MORE_REPLY_TO_COME  error from the commslib
		***     which is not the error code we really want.
		**/
		strncpy ( pciRead->szValue, szValue, MAX_NAME );

		/**     NULL terminate the response
		***     ( the array is MAX_NAME + 1 )
		**/
		pciRead->szValue [ MAX_NAME ] = '\0';





	}



	/**     Store the time-stamp
	**/
	ATKGetTime ( pciRead );



	/**
	***	Do we need to send a REP command ?
	***	i.e. Reply is 'AREAD Failed'
	**/
	if ( szReply [ 0 ] != 'A' ) {


		/**	We did not receive an AREAD failed string
		***	so assume data okay.
		**/
		pciRead->nError = OK ;

	} else {

		/**	Looks like an AREAD Failed
		***	Determine whether the AREAD command was successfull
		***	by sending a REPORT command with no parameters. This
		***	reports only the error code left by the previous
		***	command.
		**/
		strcpy ( szCommand, "REP" );


		/**
		***	Send the IML REP command to the IML Device.
		**/
		nResult = ATKProcessIMLCommand ( hWndParent,
				 nIMLHandle,
				 szCommand,
				 pciRead->nDeviceNumber,
				 szReply,
				 ATK_READ_CMD_REPLY_LEN,
				 wFlags );

		if ( bDestroy ) {
			return (0);
		}



		if ( nResult != OK ) {

			/**	An error has occured in ATKProcessIMLCommand
			***	It will have already been reported to the user
			***	by that function. Set the ATKCHANINFO structure's
			***	error value and exit the routine.
			**/
			strcpy ( pciRead->szValue, "AREAD Failed" );
			pciRead->nError = nResult;

			return ( FALSE );
		}


		nFieldsConverted = sscanf ( szReply, "%d", &nTempInt );

		pciRead->nError = (short) nTempInt ;
		
		if ( nFieldsConverted == 0 ) {

			/**	Invalid reply to the REPORT command
			**/
			pciRead->nError = ATK_ERR_INVALID_IML_REPLY;
			strcpy ( pciRead->szValue, "AREAD Failed" );



		}

	}

	/**	Finally return the outcome of this function
	***	TRUE is we managed to obtain valid data or
	***	FALSE if not.
	**/
	return ( ( pciRead->nError == 0 ) ? TRUE : FALSE );


}






/**
***     ATKBuildMonitorTable
***
***     This function builds a table of ATKMONITORCHAN structures,
***     detailing the monitor channels available in the current IMS setup.
***     The table is not used directly by the calling application
***     but is referenced by ATKReadChannel and ATKReadList to determine
***     what type of alarm, Warning or Critical, a connected channel
***     has activated.
***
***     Call this function AFTER loading an initially loading an IML setup
***     or if the setup may have edited by SetupIML.
***     An IML setup MUST have been loaded prior to calling this function.
***
***     Use fn. ATKDestroyMonitorTable to release the table after use.
***     OR before opening a new IML setup.
***
***
***     Return value : OK or non-zero error code if unable to build the
***                    table.
***
***
**/
short ATKBuildMonitorTable ( char *szIMSSetup )
{

	short           nSummaryItems, nMonitorTotal;
	short           nChannel, nTableElement;
	short           nDevice;
	GLOBALHANDLE    ghSummary;
	PCHANNELINFO   lpSummary;


	/**     If a table already exists, free it and set the pointer
	***     the table pointer to NULL, indicating no table now exists.
	**/
	if ( lpmcMonitorTable != (LPATKMONITORCHAN) NULL ) {

		ATKDestroyMonitorTable ( );

	}


	/**
	***     Assume No device support alarms until
	***     we actually locate Monitor Channels
	***     for the device.
	**/
	for ( nDevice = 0; nDevice < ATK_MAX_DEVICES; nDevice++ ) {

		abDeviceSupportsAlarms [ nDevice ] = FALSE;
	}


	/**     Request a summary from IMSLib
	**/
	ghSummary = IMSGetSetupSummary ( (LPSTR) szIMSSetup,
							&nSummaryItems );

	if ( ghSummary == NULL ) {

		/**     The summary could not be obtained
		***     Error code stored in nSummaryItems
		**/
		return ( nSummaryItems );
	}



	/**     Lock the summary array
	**/
	lpSummary = (PCHANNELINFO) GlobalLock ( ghSummary );
	if ( lpSummary == NULL ) {

		GlobalFree ( ghSummary );

		return ( ATK_ERR_NO_GLOBAL_MEMORY );

	}

	/**     Interrogate the summary array, searching for
	***     enabled channels of the required type(s)
	***
	***     Perform two passes of the summary, the first
	***     to determine how many enabled monitor channels
	***     exist.
	**/
	nMonitorTotal = 0;      /* Initialize Total */

	for ( nChannel = 0; nChannel < nSummaryItems; nChannel++ ) {


		if ( ( (char) toupper ( (int) lpSummary [ nChannel ].cTypeCode )
			== 'N' ) && ( lpSummary [ nChannel ].bEnabled ) ) {

			nMonitorTotal++;
		}


	}


	/**     Create and initilise an array of Monitor channels
	***     Create one additional element in which we will
	***     place an null channel name to mark the end of the table.
	**/
	lpmcMonitorTable = (LPATKMONITORCHAN) GlobalAllocPtr ( GMEM_MOVEABLE,
				(DWORD) sizeof ( ATKMONITORCHAN )
						* ( nMonitorTotal + 1 ) );

	/**     Create the table
	**/
	if ( lpmcMonitorTable == NULL ) {

		/**   No memory
		**/
		return ( ATK_ERR_NO_GLOBAL_MEMORY );

	}






	/**     Perform a second pass of the summary
	***     initializing the elements of the table.
	**/
	nTableElement = 0;
	for ( nChannel = 0; nChannel < nSummaryItems; nChannel++ ) {


		if ( ( (char) toupper ( (int) lpSummary [ nChannel ].cTypeCode )
			== 'N' ) && ( lpSummary [ nChannel ].bEnabled ) ) {





			/**     The Monitor channel name
			**/
			lstrcpy (
			 lpmcMonitorTable [ nTableElement ].lpszMonitorName,
			 (LPSTR) lpSummary [ nChannel ].szName );






			/**     The type of alarm being monitored
			***     encoded into the first characater
			***     of the units string.
			**/
			switch ( lpSummary [ nChannel ].szUnits [ 0 ] ) {

			default :
				lpmcMonitorTable [ nTableElement ].
						nAlarmType = ALARM_NORMAL;

				break;

			case 'c' :
			case 'C' :
				lpmcMonitorTable [ nTableElement ].
						nAlarmType = ALARM_CRITICAL;

				break;

			case 'w' :
			case 'W' :
				lpmcMonitorTable [ nTableElement ].
						nAlarmType = ALARM_WARNING;

				break;
			}








			/**     This IML Device must support alarms
			***     and may therefore use the AREAD command
			**/
			abDeviceSupportsAlarms
				[ lpSummary [ nChannel ].nFrame ] = TRUE;


			nTableElement++;   /* Next Monitor Channel */

		}


	}

	/**     Terminate the table
	**/
	lpmcMonitorTable [ nTableElement ].lpszMonitorName [ 0 ] = '\0';




	GlobalUnlock ( ghSummary );
	GlobalFree   ( ghSummary );

	return ( OK );

}






/**
***     ATKDestroyMonitorTable
***
***     This function destroys the table of ATKMONITORCHAN structures,
***     created by ATKBuildMonitorTable;
***
***     Return value : OK or non-zero error code if unable to destroy the
***                    table.
**/
short ATKDestroyMonitorTable ( VOID )
{

	if ( lpmcMonitorTable != (LPATKMONITORCHAN) NULL ) {

		GlobalFreePtr  ( lpmcMonitorTable );
		lpmcMonitorTable = (LPATKMONITORCHAN) NULL;
	}
	return ( OK );

}





/**     GetMonitorChanType
***
***     Searches the table of ATKMONITORCHAN structures pointed to
***     by global 'lpmcMonitorTable' for the entry with channel name
***     szMonitorName.
***
***     Return Value :  The function returns the 'alarm type' of the
***                     monitor channel as configured by SetupIML.
***                     Currently this may be :
***
***                             ALARM_NORMAL
***                             ALARM_WARNING
***                             ALARM_CRITICAL
**/
static short GetMonitorChanType ( char *szMonitorName )
{
	short   nIndex;



	nIndex = 0;     /* Index to first monitor channel */

	/**     Now search through the table of Monitor Channel structs
	***     looking for a match.
	**/
	while ( lpmcMonitorTable [ nIndex ].lpszMonitorName [ 0 ] != '\0' ) {


		if ( lstrcmp ( (LPSTR) szMonitorName,
		       lpmcMonitorTable [ nIndex ].lpszMonitorName ) == 0 ) {

			/**     Match found, return the alarm type..
			**/

			GlobalUnlockPtr ( lpmcMonitorTable ); /* Unlock tab */

			return ( lpmcMonitorTable [ nIndex ].nAlarmType );



		}
		nIndex++;   /* Try next Monitor Channel */
	}



	return ( ALARM_NORMAL );        /* No match return default state */
}
