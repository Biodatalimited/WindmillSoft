/**	IMSFILE.C	Nigel Roberts.	29 August, 1990.
***
***	Intelligent Microlink Applictions Setup Management Library.
***
***	This is a Dynamic Link Library for use by Windows applications.
***	It manages the loading and saving of setups. It builds setup summaries
***	which can be interrogated by applications requiring information about
*** 	the setups loaded.
***
***	Assocciated Files.
***	
***	IMSLIB.H - Exported function prototypes and #defines used by apps.
***	IMSLIB.DEF - Module definition file.
***	IMSINC.H - Private function prototypes and #defines
***	IMSLIB.C - Exported functions.
***	IMSLIB.MAK - make file.
***
***
***	This file contains all the file and communications functions required 
***	to implement the library.
***
***
***	Version History.
***
***	P1.00 - 29 August, 1990 - inception.
***
**/



#define _WINDLL     /** It is a Windows DLL **/
#define NOCOMM      /** We don't need the COMM stuff **/
#define NOKANJI     /** We don't need this either. **/
#define NOMINMAX    /** To prevent duplicate definition **/


#include <windows.h> /** Windows include file.**/

#include <io.h>      /** for read **/
#include <stdlib.h>
#include <math.h>    /** for atol .. **/
#include <string.h>  /** for strstr .. **/
#include <ctype.h>   /** for isdigit, isupper. **/


#include <sys\types.h>
#include <sys\stat.h>   /** for fstat **/



#include "imslib.h"

#define COMMSLIB

#include "\Windmill 32\Compile Libs\imlinter.h"

#include "imsinc.h"






#define MAX_MICROLINKS	36


/** External variables declared in IMSLIB.C **/

extern short bPartitioning;  /** partitioning semaphore **/




extern SETUPDETAILS asdSetup [ MAX_SETUPS ];
/** our internal details of the setups loaded.**/

extern HANDLE hSetupSummary [ MAX_SETUPS ];







static short  ReadFileIntoBuffer ( char *szFullPath, HANDLE * pBufferHandle );
static short  RequestDeviceDoError ( short nDev, short nCommHandle );
static void   ExtractChannelInfoFromLine ( char * szLine, PCHANNELINFO
								pciChannel );
static short  DoDefineCommand ( char * szLine, short nDev, short nCommHandle );
static short  GetLineFromBuffer ( LPSTR lpszBuf, short * pnBufIndex,
							char * szLine );



/*********************************************************/
/** This function allocates a dynamic memory buffer and loads
*** the whole file into it. It provides a handle to the buffer to
*** the calling function. It returns OK or an error code.
**/

static short ReadFileIntoBuffer ( char *szFullPath, HANDLE * pBufferHandle )

{	
	HFILE    hSetupHandle;
	HANDLE   hBuf;
	DWORD    dwBufferSize;
	LPSTR    lpszBuf;
	WORD     wRead;

	/** We must open the file so we can read it. If the open fails then
	*** we return file not found.
	**/
	hSetupHandle = _lopen ( szFullPath, OF_READ  );

	if ( hSetupHandle == HFILE_ERROR ) {
		return ( IMS_FILE_NOT_FOUND );
	}

	/** allocate ourselves a buffer to read the file into. Get the size
	*** of the file first and make the buffer slightly larger.
	**/

	dwBufferSize = GetFileSize ( (HANDLE) hSetupHandle, NULL);

	/** Get memory for the buffer. Fail if not enough.
	**/
	if ( ( hBuf = GlobalAlloc ( GMEM_MOVEABLE, ( dwBufferSize + 10L ) ) ) == NULL ) {
		_lclose ( hSetupHandle );
		return ( IMS_OUT_OF_MEMORY );
	}

		
	/** not enough memory so fail.**/	
	if ( ( lpszBuf = GlobalLock ( hBuf ) ) == NULL ) {
		GlobalFree ( hBuf );
		_lclose ( hSetupHandle );
		return ( IMS_OUT_OF_MEMORY );
	}


	/** Read the file into the buffer. Tidy up in case of error **/
	if ( ( wRead = _lread ( hSetupHandle, lpszBuf, dwBufferSize ) ) < dwBufferSize ) {
		_lclose ( hSetupHandle );
		GlobalUnlock ( hBuf );
		GlobalFree ( hBuf );
		return ( IMS_FILE_NOT_FOUND );
	}

	/** We have finished with the file so close it.**/
	_lclose ( hSetupHandle );

	/** Terminate the buffer with '\0'**/
	lpszBuf [ dwBufferSize ] = '\0';
	
	/** Unlock the buffer **/
	GlobalUnlock ( hBuf );	

	/** Return the handle **/
	*pBufferHandle = hBuf;
	
	return ( OK );

}


/***************************************************************/
/** This routine opens the file specified in szFullPath. It then copies it
*** into a buffer. Data is then read from the buffer line by line.
*** Each line is interpreted and its contents are put into the set up summary.
*** Each line results in a define command which is sent to the hardware.
**/


short LoadSetupFile ( char *szFullPath , char *szSetupName )

{

	static short       nSetupIndex;
/*	static OFSTRUCT    ofSetup; */
	static CHANNELINFO ciChannel;
	static char        szLine [ 120 ];
	static char        szTemp [10];
	static HANDLE      hBuf;
	static short       abFrameInUse [ MAX_MICROLINKS ];
	static short       nCommHandle;
	static short       nError;
	static short       nBufIndex;
	short              nLoop;
	short              nRet;
	LPSTR              lpszBuf;

	/** Read the file into a memory buffer
	*** Gives a handle to the buffer.
	**/
	nRet = ReadFileIntoBuffer ( szFullPath, &hBuf );

	if ( nRet != OK ) {
		return ( nRet );
	}
	
	/** open the comms to talk to the hardware **/
	nRet = OpenIMLInterface ( &nCommHandle );
	
	/** failed to open so return.**/
	if ( nRet != 0 ) {
		GlobalFree ( hBuf );
		return ( nRet );
	}

	/** lock the buffer **/	
	if ( ( lpszBuf = GlobalLock ( hBuf ) ) == NULL ) {
		GlobalFree ( hBuf );
		return ( IMS_OUT_OF_MEMORY );
	}


	/** Clear the frame in use array **/
	for ( nLoop = 0 ; nLoop < MAX_MICROLINKS ; nLoop ++ ) {
		abFrameInUse [ nLoop ] = FALSE;
	}

	/** Get the setup index.**/
	GetIndexToSetup ( szSetupName, &(nSetupIndex ) );
	
	nBufIndex = 0;

	/** Get each line from the buffer in turn.
	**/
	while ( GetLineFromBuffer ( lpszBuf, &nBufIndex, szLine ) ) {

		/** What sort of line is this. Look at first char to find
		*** out.
		**/
		switch ( szLine [ 0 ] ) {
			
			case 'D' :	/** Setup description so set the description.
				**/
				ModifyIndexedSetupDescription ( nSetupIndex,
						 	&(szLine [ 2 ]) );						
				break;
			case 'N' :      /** This is a monitor channel.**/
			case 'A' :
			case 'I' :
			case 'O' :
			case 'V' :
			case 'B' :
			case 'R' :
			case 'T' :
			case 'M' :
			case 'U' :
				/** This is a channel which needs to go in the setup
				*** This function fills the Channel info structure
				**/
				ExtractChannelInfoFromLine ( szLine, &ciChannel );
				/** Check if this is an alarm monitor channel. If so then
				*** we need to request a monitor and then fill in
				*** the module and channel numbers in the line
				*** so it can be sent out.
				**/
				if ( szLine [ 0 ] == 'N' ) {
					nError = RequestMonitorChannel ( &ciChannel, nCommHandle );
					if ( nError != 0 ) {
						CloseIMLInterface ( nCommHandle );
						GlobalUnlock ( hBuf );
						GlobalFree ( hBuf );
						return ( nError );
					}
					/** Get the Channel Number String **/
					CreateChannelNumberString ( &ciChannel, szTemp );
					/** Add it to the line **/
					strncpy ( &szLine[2] , szTemp , strlen (szTemp ) );
				}

				/** We need to check if we can use this channel.
				*** Return an error if it is already in use.
				**/
				if (( IsThisChannelAvailable ( nSetupIndex,
				                 ciChannel.nFrame, ciChannel.nChannel )) !=0 ){

					CloseIMLInterface ( nCommHandle );
					GlobalUnlock ( hBuf );
					GlobalFree ( hBuf );
					return ( IMS_CHANNEL_IN_USE );
				}

				if ( AddChannelToIndexedSetup ( nSetupIndex, 
						(PCHANNELINFO) &(ciChannel) ) != OK ) {

                                /** Sort out WHAT WE DO HERE.
                                *********************************
                                ***/

					break;
				}

				/** If szLine [ 0 ] is U i.e. line type U then we
				*** don't send this one to the frame.
				**/

				if ( szLine [ 0 ] == 'U' ) {
					break;
				}


				/** OK send the define command to the frame. This is
				*** the line with Define replacing the type code.
				***
				*** Check if the frame is there if we have not already
				*** done so.
				**/

				if ( !abFrameInUse [ ciChannel.nFrame ] ) {

					/** Check if it is there **/
					nError = RequestDeviceDoError ( ciChannel.nFrame, nCommHandle );
					/** Quit if it is not there **/
					if ( nError != 0 ) {
						CloseIMLInterface ( nCommHandle );
						GlobalUnlock ( hBuf );
						GlobalFree ( hBuf );
						return ( nError );
					}
					/** Set the frame in use flag to avoid this loop
					*** in future.
					**/
					abFrameInUse [ ciChannel.nFrame ] = TRUE;
				}
				/** Send the define command **/
				nError = DoDefineCommand ( szLine, ciChannel.nFrame, nCommHandle );
				
				if ( nError != 0 ) {
					CloseIMLInterface ( nCommHandle );
					GlobalUnlock ( hBuf );
					GlobalFree ( hBuf );
					return ( nError );
				}

				break;

			default :	/** ignore this line **/
				break;
		}
	}



	GlobalUnlock ( hBuf );
	GlobalFree ( hBuf );
	CloseIMLInterface ( nCommHandle );

	/** Set the file spec in the structure.**/
/*	strcpy ( asdSetup [ nSetupIndex ].szFileSpec, ofSetup.szPathName );*/
	strcpy ( asdSetup [ nSetupIndex ].szFileSpec,szFullPath );

	return ( IMS_SUCCESS );
}


/****************************************************************/
/** This function gets summary data only from an IMS file. It is required
*** when IMSlib is asked to load a file when another file is already loaded.
*** That file must have been loaded by another copy of IMSlib. We need a
*** summary of the already loaded file to check that channels do not clash.
*** Ordinary channels are easy but monitors require a SHOW command in order
*** to discover their dynamically allocated channel numbers.
**/


short GetSummaryFromFile ( short nSetupIndex )

{

	static CHANNELINFO ciChannel;
	static char        szLine [ 120 ];
	static char        szImlText [120];
	static HANDLE      hBuf;
	static short       nCommHandle;
	static short       nError;
	static short       nBufIndex;
	short              nRet;
	LPSTR              lpszBuf;
	

	/** We must start the file read with 0 channels. This will
	*** increment as the channels are read.
	**/

	asdSetup [ nSetupIndex ].nChannels = 0;
	
	/** Read the file into a memory buffer
	*** Gives a handle to the buffer.
	**/
	nRet = ReadFileIntoBuffer ( asdSetup [nSetupIndex].szFileSpec, &hBuf );

	if ( nRet != OK ) {
		return ( nRet );
	}
	
	/** open the comms to talk to the hardware **/
	nRet = OpenIMLInterface ( &nCommHandle );
	
	/** failed to open so return.**/
	if ( nRet != 0 ) {
		GlobalFree ( hBuf );
		return ( nRet );
	}

	/** lock the buffer **/	
	if ( ( lpszBuf = GlobalLock ( hBuf ) ) == NULL ) {
		GlobalFree ( hBuf );
		return ( IMS_OUT_OF_MEMORY );
	}


	
	nBufIndex = 0;

	/** Get each line from the buffer in turn.
	**/
	while ( GetLineFromBuffer ( lpszBuf, &nBufIndex, szLine ) ) {

		/** What sort of line is this. Look at first char to find
		*** out.
		**/
		switch ( szLine [ 0 ] ) {
			
			case 'N' :    
			case 'A' :
			case 'I' :
			case 'O' :
			case 'V' :
			case 'B' :
			case 'R' :
			case 'T' :
			case 'M' :
			case 'U' :
				/** This is a channel which needs to go in the setup
				*** This function fills the Channel info structure
				**/
				ExtractChannelInfoFromLine ( szLine, &ciChannel );
				/** Check if this is an alarm monitor channel. If so then
				*** we need to find the channel number which it has been dynamically
				*** allocated. To do this we use the channel name with a show command.
				**/
				if ( szLine [ 0 ] == 'N' ) {
					/** Prepare and send the SHOW command **/
					strcpy ( szImlText, "SHO " );
					strcat ( szImlText, ciChannel.szName );
					nError = SendIMLCommand ( nCommHandle, ciChannel.nFrame,
						                            szImlText, IML_TEXT_REPLY );
					if ( nError != 0 ) {
						CloseIMLInterface ( nCommHandle );
						GlobalUnlock ( hBuf );
						GlobalFree ( hBuf );
						return ( nError );
					}
					/** Get the reply leaving space at the start of the string
					*** to add the type.
					**/
					nError = GetReply ( nCommHandle, ciChannel.nFrame,
                        	&(szImlText [ 2 ]), ( sizeof ( szImlText ) -2 ) );
					if ( nError != 0 ) {
						CloseIMLInterface ( nCommHandle );
						GlobalUnlock ( hBuf );
						GlobalFree ( hBuf );
						return ( nError );
					}
					
					/** Add the type so that we can now reuse the function
					*** for getting channel info.
					**/
					szImlText [0] = 'N';
					szImlText [1] = ' ';
					ExtractChannelInfoFromLine ( szImlText, &ciChannel );					
				}

				/** Put the channel into the summary **/
				if ( AddChannelToIndexedSetup ( nSetupIndex, 
						(PCHANNELINFO) &(ciChannel) ) != OK ) {

                                /** Sort out WHAT WE DO HERE.
                                *********************************
                                ***/

					break;
				}

				
				if ( nError != 0 ) {
					CloseIMLInterface ( nCommHandle );
					GlobalUnlock ( hBuf );
					GlobalFree ( hBuf );
					return ( nError );
				}

				break;

			default :	/** ignore this line **/
				break;
		}
	}



	GlobalUnlock ( hBuf );
	GlobalFree ( hBuf );
	CloseIMLInterface ( nCommHandle );


	return ( OK );
}






/*********************************************************/
/** This routine copies data from the file buffer into the line string.
***
*** Parameters
***          lpszBuf       Pointer to text buffer
***          *pnBufIndex...Index to characters in that buffer
***          szLine        Pointer to string to hold the line
**/

static short GetLineFromBuffer ( LPSTR lpszBuf, short * pnBufIndex, char * szLine )

{

	short   nLineIndex = 0;

	/** scan the buffer starting at the buf index copying into
	*** szline until we get to the end of a line. inc buf index to step
	*** over the \r \n at the end of the line.
	**/

	/** Look for CR **/
	while ( lpszBuf [ *pnBufIndex ] != '\r' ) {

		/** problems with 0 **/
		if ( lpszBuf [ *pnBufIndex ] == '\0' ) {
			return ( FALSE );
		}

		/** Copy into the line string **/
		szLine [ nLineIndex ] = lpszBuf [ *pnBufIndex ];

		nLineIndex ++;
		(*pnBufIndex) ++;
	}

	/** Add a terminal 0 **/
	szLine [ nLineIndex ] = '\0';
	/** Step over CR LF **/
	(*pnBufIndex) += 2;

	return ( TRUE );
}




/**********************************************************/
/** Send out a DEFINE command to the IML device.
*** Follow it with a REPORT command to check for errors.
***
*** Parameters
***          szLine       pointer to define command text
***          nDev         IML device to send command to
***          nCommHandle  Handle allocated to us by Commslib
***
*** Return
***       OK or error code
**/



static short DoDefineCommand ( char * szLine, short nDev, short nCommHandle )

{

	static char  szCommand [ 120 ];
	static char  szRep [ 80 ];
	short        nTries = 0;
	short        nLoadError;
	static short nError;




	strcpy ( szCommand, "DEF " );
	strcat ( szCommand, &szLine [ 1 ] );


	while ( nTries < 5 ) {

		nTries ++;
		nError = SendIMLCommand ( nCommHandle, nDev, szCommand, IML_NO_REPLY );

		/** We have an error so check what sort it is. If it
		*** is non critical then we can try the define
		*** command again.
		**/

		switch ( nError ) {

			case 0 :
				break;
			default :
				return ( nError );
		}


		/** Now do a rep to see if the commamd worked ok.
		**/

		nError = SendIMLCommand ( nCommHandle, nDev, "rep", IML_TEXT_REPLY );
		if ( nError != 0 ) {
			return ( nError );
		}


		nError = GetReply ( nCommHandle, nDev, szRep, sizeof (szRep ) );
		if ( nError != 0 ) {
			return ( nError );
		}

		nLoadError = atoi ( szRep );
		if ( nLoadError == 0 ) {
			/** Define command sent and understood so we can
			*** return OK.
			**/
			return ( 0 );
		}
	}

	/** if we fail to send the define we can return IMS_LOAD_ERRORS.
	**/
	return ( IMS_LOAD_ERRORS );

}




/******************************************************/
/** Send out the IML ERR command. This should always generate a reply.
*** Only used to check that comms to frame are OK.
**/ 

static short   RequestDeviceDoError ( short nDev, short nCommHandle )

{

	static char     szStr [ 20 ];
	static short   nError;



	/** Frame requested OK so send errors command. And get the reply -
	*** check for any errors. If we get an error then release frame
	*** and return the error........ .
	**/

	nError = SendIMLCommand ( nCommHandle, nDev, "ERR", IML_TEXT_REPLY );


	if ( nError != 0 ) {
		return ( nError );
	}


	nError = GetReply ( nCommHandle, nDev, szStr, sizeof ( szStr ) );

	/** Check for an error. If we find one then release the frame
	*** and return the error.
	**/

	if ( nError != 0 ) {
		/** We have an error so release all and return the error.
		**/
		return ( nError );
	}

	return ( 0 );
}

/************************************************************/
/** This routine is passed a line of text from the IMS file.
*** It extracts the data and fills in a Channel info structure
*** It gets :   Channel Type
***             Frame Number
***             Channel Number
***             Name
***             Units
***             Enable
***
*** Parameters
***           szLine      Pointer to the line of text
***           pciChannel  Pointer to the channel info structure to fill
**/



static void ExtractChannelInfoFromLine ( char * szLine, PCHANNELINFO pciChannel )

{

	char*   szPtr;
	short   nChar;
	long    lChan;


	/** [0] is channel type. **/
	pciChannel->cTypeCode = szLine [ 0 ];
	/** [1] is space **/


	/** Cope with potential 6 digit channel numbers.
	*** If we have XXXX as the channel number then this is a monitor
	*** channel and we can only get the frame number.
	*** The same code works with 5 digit numbers.
	**/


	if ( szLine [ 4 ] == 'x' ) {

		pciChannel->nFrame = atoi ( &(szLine [ 2 ]) );

	} else {

		lChan = atol ( &(szLine [ 2 ]) );

		pciChannel->nFrame = (short) ( lChan / 10000L );

		pciChannel->nChannel = (short) ( lChan % 10000L );
	}



	/** find the name of the channel.
	**/

	szPtr = strstr ( szLine , " N=" );

	/** increment pointer passed ' N='
	**/

	szPtr += 3;      /** Point to first char of name **/
	nChar = 0;       /** Initialise character count **/
	/** Copy the name chars 1 by 1 into the channel info structure.
	*** Cope with unexpected CR,LF, and too long strings
	**/
	while ( szPtr [ nChar ] != ' ' ) {

		if ( ( szPtr [ nChar ] == '\r' ) ||
		            ( szPtr [ nChar ] == '\n' ) ) {

			break;
		}

		pciChannel->szName [ nChar ] = szPtr [ nChar ];

		if ( nChar == MAX_NAME_LENGTH ) {
			break;
		}
		nChar ++;
	}

	/** Add a terminal 0 **/
	pciChannel->szName [ nChar ] = '\0';

	/** If no name specified then copy the frame and channel number
	*** into the name string. This may be 5 or 6 bytes long.
	**/

	if ( nChar == 0 ) {

		if ( pciChannel->nFrame > 9 ) {

			strncpy ( pciChannel->szName , &szLine [2] , 6 );

			/** Must terminate the string as strncpy
			*** doesn't do it.
			**/

			pciChannel->szName [ 6 ] = '\0';

		} else {

			strncpy ( pciChannel->szName , &szLine [2] , 5 );

			/** Must terminate the string as strncpy
			*** doesn't do it.
			**/

			pciChannel->szName [ 5 ] = '\0';
		}
	}


	/** find the units of the channel.
	**/

	szPtr = strstr ( szLine , " U=" );

	/** increment pointer passed ' U='
	**/

	szPtr += 3;
	nChar = 0;
	
	/** Copy the units into the channel info structure **/
	while ( szPtr [ nChar ] != ' ' ) {

		pciChannel->szUnits [ nChar ] = szPtr [ nChar ];

		if ( nChar == MAX_NAME_LENGTH ) {
			break;
		}

		nChar ++;
	}

	/** Add a terminal 0 **/
	pciChannel->szUnits [ nChar ] = '\0';

	/** Is this channel enabled or disabled.
	**/
	szPtr = strstr ( szLine , " E=" );
	/** increment pointer passed ' E='
	**/

	szPtr += 3;
	pciChannel->bEnabled = ( szPtr [ 0 ] == '1' ) ? TRUE : FALSE;


	return;
}

/********************************************************************/
/** When channels do not have a name specified in the set up then we
*** must create a default name from the frame and channel numbers. 
*** This name appears on the Connect dialog of Apps.
**/


void CreateChannelNumberString ( PCHANNELINFO pciChannel, char* szName )


{	
	short nFrame, nChannel;


	nFrame = pciChannel->nFrame;
	nChannel = pciChannel->nChannel;


	if ( nFrame > 9 ) {

		szName [ 0 ] = (char) ( '0' + ( nFrame / 10 ) );
		szName [ 1 ] = (char) ( '0' + ( nFrame % 10 ) );
		szName [ 2 ] = (char) ( '0' + ( nChannel / 1000 ) );
		szName [ 3 ] = (char) ( '0' + ( nChannel / 100 ) %10);
		szName [ 4 ] = (char) ( '0' + ( nChannel / 10 ) % 10);
		szName [ 5 ] = (char) ( '0' + ( nChannel % 10 ) );
		szName [ 6 ] = '\0';

	} else {

		szName [ 0 ] = (char) ( '0' + ( nFrame ) );
		szName [ 1 ] = (char) ( '0' + ( nChannel / 1000 ) );
		szName [ 2 ] = (char) ( '0' + ( nChannel / 100 ) %10);
		szName [ 3 ] = (char) ( '0' + ( nChannel / 10 ) % 10);
		szName [ 4 ] = (char) ( '0' + ( nChannel % 10 ) );
		szName [ 5 ] = '\0';
	}

}



/**			WriteSetupToFile ()
***
*** Writes the indexed setup to the file specified by the handle.
***
*** Returns
***	OK - setup written to file.
***	IMS_OUT_OF_MEMORY - not enough memory - setup not written.
***	IMS_DISK_FULL - disk full writing setup.
***	IMS_CHANNEL_ERROR - unable to show channel in setup summary.
***
**/


short WriteSetupToFile ( short nIndex , short nFileHandle )

{
	
	short        abFrameInUse [ MAX_MICROLINKS ];
	short        nCharsWritten;
	short        nStrLen;
	short        nLoop;
	static short nCommHandle;
	HANDLE       hSummary;
	PCHANNELINFO pciSummary;
	short        nChannels;
	static char  szCommand [ 20 ];
	static char  szBuf [ 100 ];
	static short nError;
	static char  szTerm [ 3 ];
	short        nRet;
	static char  szChan [ 10 ];



	/** We need to lock the setup summary then find which frames are
	*** included in it so we can request all of them. We then go through
	*** each channel doing a show, checking the reply, then appending the
	*** appropriate type code and writing the line to the file.
	**/


	hSummary = hSetupSummary [ nIndex ];
	nChannels = asdSetup [ nIndex ].nChannels;
	
	pciSummary = (PCHANNELINFO) LocalLock ( hSummary );
	if ( pciSummary == NULL ) {
		/** we're out of memory **/
		return ( IMS_OUT_OF_MEMORY );
	}


	/** Which frames do we need? First clear the array **/
	for ( nLoop = 0 ; nLoop < MAX_MICROLINKS ; nLoop ++ ) {
		abFrameInUse [ nLoop ] = FALSE;
	}

	for ( nLoop = 0 ; nLoop < nChannels ; nLoop ++ ) {
		abFrameInUse [ pciSummary [ nLoop ].nFrame ] = TRUE;
	}

	/** OK we now know which setup we are using and which frames are in the
	*** summary. We now open the comms and request all the frames in use.
	*** We release everything if we cannot request a frame and return 
	*** COMMS_DEVICE_BUSY.
	**/

    nRet = OpenIMLInterface ( &nCommHandle );
	if ( nRet != 0 ) {
		/** failed to open so return. **/
		LocalUnlock ( hSummary );
		return ( nRet );
	}





	/** We have use of the commslib so we now send out a show command
	*** for each channel. The reply is the settings for this channel so
	*** we write this to the file prepending the type code.
	**/

	szTerm [ 0 ] = '\r';   /** CR LF  terminator **/
	szTerm [ 1 ] = '\n';
	szTerm [ 2 ] = '\0';



	/** First write the file type details followed by the setup description.
	**/

	strcpy ( szBuf, "F 01 IMSLIB " );

	_lwrite ( nFileHandle, szBuf, strlen ( szBuf ) );

	IMSLibVersion ( szBuf, 50 );

	strcat ( szBuf, szTerm );

	_lwrite ( nFileHandle, szBuf, strlen ( szBuf ) );


	/** Write the setup description to the file
	**/
	
	strcpy ( szBuf, "D " );

	strcat ( szBuf, asdSetup [ nIndex ].szDescription );
	strcat ( szBuf, szTerm );

	_lwrite ( nFileHandle, szBuf, strlen ( szBuf ) );


	/** Now get the info on the channels in the setup.
	**/	

	strcpy ( szCommand, "SHO " );

	for ( nLoop = 0 ; nLoop < nChannels ; nLoop ++ ) {

        strcpy ( szCommand, "SHO " );

		/** The default channel name is also the string used to express
		*** channel number in commands.
		**/
		CreateChannelNumberString ( ( pciSummary + nLoop) , szChan );

        strcat ( szCommand, szChan );


		/** Check if this is type U. If it is we don't get the
		*** channel details from the frame. Instead we get them
		*** from the summary.
		**/

		if ( pciSummary [ nLoop ].cTypeCode == 'U' ) {
			/** Build the line for the file by hand.
			**/

			strcpy ( szBuf , "U " );
			strcat ( szBuf, &(szCommand [ 4 ] ) );
			strcat ( szBuf, " N= E=0 U=" );

			strcat ( szBuf, szTerm );

			nStrLen = strlen ( szBuf );

			nCharsWritten = _lwrite ( nFileHandle, (LPSTR)szBuf,
								   nStrLen);
		
			if ( nCharsWritten != nStrLen ) {
				LocalUnlock ( hSummary );

				CloseIMLInterface ( nCommHandle );
				return ( IMS_DISK_FULL );
			}

			continue;
		}


		/** OK Send the show command.
		**/

		nError = SendIMLCommand ( nCommHandle, pciSummary [
				  nLoop ].nFrame, szCommand, IML_TEXT_REPLY );

		if ( nError != 0 ) {
			/** We have an error so tidy up and return the error.
			**/

			CloseIMLInterface ( nCommHandle );
			LocalUnlock ( hSummary );
			return ( nError );
		}


        /** Get the reply.
        *** REMEMBER to leave the first two chars of the buffer free
        *** for the type code to go in.
        *** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
         **/

		nError = GetReply ( nCommHandle, pciSummary [ nLoop ].nFrame,
                        	&(szBuf [ 2 ]), ( sizeof ( szBuf ) -2 ) );

		if ( nError != 0 ) {

			CloseIMLInterface ( nCommHandle );
			LocalUnlock ( hSummary );
			return ( IMS_CHANNEL_ERROR );
		}
		

                /** Put the type code at the front.
                **/

		szBuf [ 0 ] = pciSummary [ nLoop ].cTypeCode;
        szBuf [ 1 ] = ' ';

        if ( pciSummary [ nLoop ].cTypeCode == 'N' ) {

			/** This is a monitor so stamp out the channel
			*** number before we write it to file.
			**/
			if ( pciSummary [ nLoop ].nFrame > 9 ) {

				szBuf [ 4 ] = 'x';
                szBuf [ 5 ] = 'x';
                szBuf [ 6 ] = 'x';
	            szBuf [ 7 ] = 'x';
            } else {
                szBuf [ 3 ] = 'x';
                szBuf [ 4 ] = 'x';
                szBuf [ 5 ] = 'x';
                szBuf [ 6 ] = 'x';
            }
        }


		strcat ( szBuf, szTerm );

		nStrLen = strlen ( szBuf );

		nCharsWritten = _lwrite ( nFileHandle, (LPSTR)szBuf, nStrLen);
		
		if ( nCharsWritten != nStrLen ) {
			LocalUnlock ( hSummary );

			CloseIMLInterface ( nCommHandle );
			return ( IMS_DISK_FULL );
		}
	}

	CloseIMLInterface ( nCommHandle );

	LocalUnlock ( hSummary );
	
	return ( OK );	
}


/**********************************************************/
/** This routine examines all the setups to see if any of them are
*** already using the passed in channel. It needs to know which
*** setup is enquiring in order to exclude it from the search.
***
*** Parameters :
***        nSetupIndex   The index of the set up
***        nFrame        The frame number
***        nChannel      The channel number
*** Return
***       OK or IMS_CHANNEL_IN_USE 
**/


short IsThisChannelAvailable ( short nSetupIndex, short nFrame, short
							    nChannel )

{

short	nSetup;
static short nIndex;
static CHANNELINFO ciChannel;
static short nLockCount;


	ciChannel.nFrame = nFrame;
	ciChannel.nChannel = nChannel;

	/** if there are no setups loaded then we can just return OK.
	**/
	if ( GetSetupCount == 0 ) {
		return ( 0 );
	}

	/** look for the channel in all the setups currently loaded.
	**/

	for ( nSetup = 0; nSetup < MAX_SETUPS; nSetup ++ ) {

		/** Check if setup is free. If it is then continue.**/
		if ( GetSetupState ( nSetup, &nLockCount) == SETUP_KEY_FREE) {
			continue;
		}

		/** If this is the setup we are trying to load then don't
		*** check the channel.
		**/

		if ( nSetup == nSetupIndex ) {
			continue;
		}

		/** Try and get the index of the channel within the setup.
		*** It will return IMS_BAD_CHANNEL if it does not exist.
		**/
		if ( GetChannelIndexInIndexedSetup ( nSetup, (PCHANNELINFO)
					&ciChannel, (short *) &nIndex ) ==
							IMS_BAD_CHANNEL ) {

			/** the channel is not in this setup so try the next
			*** one.
			**/
			continue;
		}

		/** The channel is in this set up.
		*** Is this setup locked/ inuse or not. If it is not being
		*** used then we can get rid of it and return 0 to indicate
		*** that the channel is OK to use. If the setup is being used
		*** then we must return Channel in  use and fail.
		**/

		if ( nLockCount == 0 ){
			/** this setup is not in use by a program so get rid of it and
			*** return OK since the channel cannot be in another setup.
			**/
			RemoveSetupFromList ( nSetup );
			return ( OK );
		}

		/** the setup is in use so we must return channel in use.
		**/
		return ( IMS_CHANNEL_IN_USE );
	}

	/** We have searched through all the setups and not found the channel
	*** so we are free to use it.
	**/


	return ( OK );
}


/********************************************************/
/** Get the response to an IML command from commslib.
*** Commslib will reply IML_NOT_READY if it has not arrived.
*** Handles any errors and implements a time out.
*** Reply in szBuf.
***
*** Parameters :
***       nCommsChannel   CommsChannel allocated by Commslib
***       nDev            Device to get reply from
***       szBuf           Pointer to buffer for reply
***       nSize           Buffer size
***
*** Return
***       Error Code or OK
**/

short GetReply ( short nCommsChannel, short nDev, char * szBuf, short nSize )

{

unsigned long   ulTime;
short           nRet;
static short    nDevice;

	nDevice = nDev;

	/** Initialise time out **/
	ulTime = GetCurrentTime ();

	/** loop to wait for reply **/
	while ( TRUE ) {

		/** Ask for reply **/
		nRet = GetIMLReply ( nCommsChannel, &nDevice, szBuf, nSize, 0 );
		switch ( nRet ) {

			case 0 : return ( 0 ); /** success **/

			case IML_NOT_READY :
				/** No reply yet so try again.
				**/
				break;

			default :	/** We have a problem.**/
				AbortIMLReply ( nCommsChannel, nDev );
				return ( nRet );
		}


		/** Check for timeout.**/
		if ( ( GetCurrentTime () - 5000L ) > ulTime ) {
			/** Time out so cancel reply **/
			AbortIMLReply ( nCommsChannel, nDev );
			return ( IML_LINK_ERROR );
		}
	}

	return ( 0 );
}



#if 0


/***************************************************/
/** Convert frame characters 0-9 and A-Z to number 0-35 
***
*** Parameter
***      cFrame  0-9  and A-Z
***
*** Return
***       number 0-35
**/

short GetFrameNumberFromChar ( char cFrame )

{

short	nFrame = 100;

	if ( isdigit ( cFrame ) ) {
		nFrame = (short) cFrame - (short) '0';
		return ( nFrame );
	}

	if ( isupper ( cFrame ) ) {
		nFrame = (short) cFrame - (short) 'A';
		nFrame += 10;
	}

	return ( nFrame );
}


/*************************************************************/
/** Frames are numbered 0-9 and A-Z. Convert the number 0-35
*** to the correct character.
***
*** Parameter :
***     nFrame   Frame number 0-35
***
*** Return :
***     Character 0-9  A-Z
**/


char GetFrameCharFromNumber ( short nFrame )

{

char cFrame;

	if ( nFrame < 10 ) {
		cFrame = (char) ( nFrame + '0' );
		return ( cFrame );
	}

	nFrame -= 10;
	cFrame = 'A';
	cFrame += (char) nFrame;

	return ( cFrame );
}



#endif
