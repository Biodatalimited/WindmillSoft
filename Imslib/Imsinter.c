/**  IMSINTER.C  Nigel Roberts. 29 August, 1990.
***
*** Intelligent Microlink Applictions Setup Management Library.
***
*** This is a Dynamic Link Library for use by Windows applications.
*** It manages the loading and saving of setups. It builds setup summaries
*** which can be interrogated by applications requiring information about
*** the setups loaded.
***
*** Assocciated Files.
*** 
*** IMSLIB.H - Exported function prototypes and #defines used by apps.
*** IMSLIB.DEF - Module definition file.
*** IMSINC.H - Private function prototypes and #defines
*** IMSLIB.C - Exported functions.
*** IMSLIB.MAK - make file.
***
***
*** This file contains all the internal functions required to implement the
*** library.
***
***
*** Version History.
***
*** P1.00 - 29 August, 1990 - inception.
***
**/



#define _WINDLL     /** It is a Windows DLL **/
#define NOCOMM      /** We don't need the COMM stuff **/
#define NOKANJI     /** We don't need this either. **/
#define NOMINMAX    /** To prevent duplicate definition **/


#include <windows.h>  /** Windows include file.**/
#include <string.h>   /** for strnset, strcpy & strlen **/
#include <stdlib.h>   /** For itoa **/

#include "imslib.h"

#define COMMSLIB

#include "\Windmill 32\Compile Libs\imlinter.h"
#include "imsinc.h"


/** External variables declared in IMSLIB.C **/

extern short bPartitioning;  /** partitioning semaphore **/

extern SETUPDETAILS asdSetup [ MAX_SETUPS ];  
/** our internal details of the setups loaded. **/

extern char acSetupKey [ MAX_SETUPS ] [ MAX_SETUP_KEYS ];


extern HANDLE hSetupSummary [ MAX_SETUPS ];


/** GetIndexToSetup ()
***
*** Searches the array of setup structures for the setup name.
*** If it finds a match and the set up is in use it returns
*** its index.
***
*** Returns.
*** OK - setup name found.
*** IMS_OUT_OF_MEMORY - out of memory when locking setup list.
*** IMS_BAD_NAME - setup name not found in list.
***
**/

short GetIndexToSetup ( char * lpszSetupName , short * nIndex )

{

	short  nSetup;

	/** now search through the setup list to find the name.**/
	
	for ( nSetup = 0 ; nSetup < MAX_SETUPS ; nSetup ++ ) {
		
		/** compare the name of this setup with the one provided.
		**/
		if ( ( lstrcmpi ( lpszSetupName , (LPSTR) asdSetup [
					nSetup ].szSetupName ) ) == 0 ) {
						
			/** We have found a match so set the index. unlock
			*** the setup list handle and return OK.
			**/
			*nIndex = nSetup;
			return ( OK );
		}
	}
	
	/** We have not found a match so unlock the setup list and return bad 
	*** name.
	**/
		
	return ( IMS_BAD_NAME );
}



/***************************************************************/
/** 			AddChannelToIndexedSetup ()
***
*** Inserts a channel into a setup summary in channel order. If the channel
*** is already present we return duplicate channel and leave everything as it
*** is.
*** If no name is specified we build it from the frame and channel.
***
*** Returns.
*** OK - channel added.
*** IMS_OUT_OF_MEMORY.
*** IMS_DUPLICATE_CHANNEL - already in setup - Warning error.
***
**/

short AddChannelToIndexedSetup ( short nSetupIndex, PCHANNELINFO lpciChannel )

{

short	nChannel;
HANDLE	hSummary;
PCHANNELINFO	pciSummary;
short	nInsertIndex;
static char szDefName [ 10 ];




	/** Check if we have a name supplied. If not the we build the
	*** default from the frame and channel.
	**/

	if ( strlen ( lpciChannel->szName ) == 0 ) {

		/** We have no name so create the default.
		**/
		CreateChannelNumberString ( lpciChannel, lpciChannel->szName );
	}

	/** OK now get the handle of the setup summary we want to add to.
	**/
	hSummary = hSetupSummary [ nSetupIndex ];

	/** Check if the handle is NULL. If it is we need to allocate space
	*** for the first channel. We then copy the details into the summary,
	*** update the handle and set number of channels to 1, Unlock and 
	*** return OK.
	**/
	
	if ( hSummary == NULL ) {
		hSummary = LocalAlloc ( LMEM_MOVEABLE | LMEM_ZEROINIT ,
					(WORD) sizeof ( CHANNELINFO ) );
					
		if ( hSummary == NULL ) {
			return ( IMS_OUT_OF_MEMORY );
		}
		
		/** Store the handle in the table and set the number of channels
		*** to 1.
		**/
		hSetupSummary [ nSetupIndex ] = hSummary;
		asdSetup [ nSetupIndex ].nChannels = 1;
		
		/** Lock the summary and copy the details in.
		**/
		if ( ( pciSummary = (PCHANNELINFO) LocalLock ( hSummary ) )
								== NULL ) {
			/** We're out of memory.
			**/
			return ( IMS_OUT_OF_MEMORY );
		}

		/** STRUCTURE ASSIGNMENT !!!
		**/
		pciSummary [ 0 ] = *lpciChannel;
		LocalUnlock ( hSummary );

		asdSetup [ nSetupIndex ].bSetupChanged = TRUE;

		return ( OK );
	}
	
	/** now lock the summary **/
	if ( ( pciSummary = (PCHANNELINFO) LocalLock ( hSummary ))== NULL ) {
		/** We're out of memory so return. Must unlock the summary 
		*** table first.
		**/
		return ( IMS_OUT_OF_MEMORY );
	}

	
	/** Now we look through the channels to find where to insert the new
	*** one. We also check that it does not already exist in the setup.
	*** We insert it in frame, module, channel order.
	**/
	
	/** insert at the end if not set in the loop.
	**/
	nInsertIndex = asdSetup [ nSetupIndex ].nChannels;
	
	for ( nChannel = 0 ; nChannel < asdSetup [ nSetupIndex ].nChannels ;
								nChannel ++ ) {


		/** If the frame number is less than this one then we need
		*** to insert before it.
		**/
		if ( lpciChannel->nFrame < pciSummary [ nChannel ].nFrame ) {
			nInsertIndex = nChannel;
			break;
		}

		/** Compare the frame number. If the new one is greater then
		*** continue.
		**/
		if ( lpciChannel->nFrame > pciSummary [ nChannel ].nFrame ) {
			continue;
		}
		
		/** Check that this channel  is not already in the setup.
		**/
		if ( lpciChannel->nChannel == pciSummary [nChannel ].nChannel){
			/** This channel is already in the setup so leave it as
			*** it is and return duplicate channel. We must unlock
			*** everything first.
			**/
			LocalUnlock ( hSummary );
			return ( IMS_DUPLICATE_CHANNEL );
		}

		/** Now do the channel number. If the new one is less than
		*** this one then we need to insert before this one so 
		*** remember where we are and break out of the loop.
		**/
		if ( lpciChannel->nChannel < pciSummary [ nChannel ].nChannel){
			nInsertIndex = nChannel;
			break;
		}
	}
	
	/** OK We now know where we need to insert the new channel. We need to
	*** increase the size of the array, move channels up the array so we
	*** can insert the new one and increment the number of channels in the 
	*** setup.
	*** We must unlock the summmary so we can realloc it then relock it. 
	*** we must put the new handle back into our summary table.
	**/

	asdSetup [ nSetupIndex ].nChannels ++;
	
	LocalUnlock ( hSummary );
	hSummary = LocalReAlloc ( hSummary , (WORD) ( asdSetup [
			nSetupIndex ].nChannels * sizeof ( CHANNELINFO ) ) ,
						LMEM_MOVEABLE | LMEM_ZEROINIT );
		
	if ( hSummary == NULL ) {
		/** realloc failed - we must be out of memory. Unlock everything
		*** and return.
		**/
		return ( IMS_OUT_OF_MEMORY );
	}

	/** store the new handle.
	**/
	hSetupSummary [ nSetupIndex ] = hSummary;
	
	/** Now move the channels up. Need to relock the summary.
	**/
	
	if ( ( pciSummary = (PCHANNELINFO) LocalLock (hSummary )) == NULL ) {
		/** We're out of memory so return. Must unlock the
		*** summary table first.
		**/
		return ( IMS_OUT_OF_MEMORY );
	}


	/** Move the channels up.
	**/
	for ( nChannel = asdSetup [ nSetupIndex ].nChannels -1; nChannel >
						nInsertIndex; nChannel -- ) {
	
		/** STRUCTURE ASSIGNMENT **/
		pciSummary [ nChannel ] = pciSummary [ nChannel -1 ];
	}


	/** insert the new one.
	**/
	pciSummary [ nInsertIndex ] = *lpciChannel;
	
	/** Unlock everything and return.
	**/
	LocalUnlock ( hSummary );

	asdSetup [ nSetupIndex ].bSetupChanged = TRUE;

	return ( OK );
}







/**			RemoveChannelFromIndexedSetup ()
***
***  Deletes a channel from the setup.
***
*** Returns.
***	OK - channel deleted.
***	IMS_OUT_OF_MEMORY
***	IMS_BAD_CHANNEL - channel not in setup.
***
**/

short RemoveChannelFromIndexedSetup ( short nSetupIndex, PCHANNELINFO 
								lpciChannel )

{

short	nChannel;
HANDLE	hSummary;
PCHANNELINFO	pciSummary;
static short	nDeleteIndex;
short 	nRet;


	/** Get the index of the channel to delete.
	**/
	
	switch ( ( nRet = GetChannelIndexInIndexedSetup ( nSetupIndex , 
					lpciChannel , &(nDeleteIndex) ) ) ) {
								
	case OK : break;

	default : return ( nRet );
	}

	/** We need the handle to the summary which is stored in our summary
	*** table. To get it we must lock our table.
	**/

	/** OK now get the handle of the setup summary we want to change.
	**/
	
	hSummary = hSetupSummary [ nSetupIndex ];
	
	/** now lock the summary **/
	if ( ( pciSummary = (PCHANNELINFO) LocalLock ( hSummary ))== NULL ) {
		/** We're out of memory so return. Must unlock the
		*** summary table firts.
		**/
		return ( IMS_OUT_OF_MEMORY );
	}
	
	/** OK we now know where the channel is so we can now move the others
	*** down. If we still have channels in the setup we can resize the
	*** array otherwise we can set the handle to null.
	**/
	
	/** Move the channels down.
	**/

        /** Loop to channels -1 so we don't try and copy from off the end
        *** of the summary.
        **/

	for ( nChannel = nDeleteIndex ; nChannel < asdSetup [ nSetupIndex ].
						nChannels - 1; nChannel ++ ) {
	
		/** STRUCTURE ASSIGNMENT **/
		pciSummary [ nChannel ] = pciSummary [ nChannel + 1 ];
	}
	
	/** now resize the array 
	**/	
	asdSetup [ nSetupIndex ].nChannels --;

	LocalUnlock ( hSummary );

        /** If we no longer have any channels then don't realloc the
        *** summary to zero.
        **/

        if ( asdSetup [ nSetupIndex ].nChannels > 0 ) {

		hSummary = LocalReAlloc ( hSummary , (WORD) ( asdSetup [
			nSetupIndex ].nChannels * sizeof ( CHANNELINFO ) ) ,
						LMEM_MOVEABLE | LMEM_ZEROINIT );
        }


	/** store the new handle.
	**/
	hSetupSummary [ nSetupIndex ] = hSummary;
	
	asdSetup [ nSetupIndex ].bSetupChanged = TRUE;

	return ( OK );
}








/**			ModifyChannelInIndexedSetup ()
***
***  modifies a channel in the setup.
***
*** Returns.
***	OK - channel modified.
***	IMS_OUT_OF_MEMORY
***	IMS_BAD_CHANNEL - channel not in setup.
***
**/

short ModifyChannelInIndexedSetup ( short nSetupIndex, PCHANNELINFO 
								lpciChannel )
								
{

	HANDLE	        hSummary;
	PCHANNELINFO    pciSummary;
	static short    nChanIndex;
	short           nRet;


	/** Get the index of the channel to change.
	**/
	
	switch ( ( nRet = GetChannelIndexInIndexedSetup ( nSetupIndex , 
					lpciChannel , &(nChanIndex) ) ) ) {
								
		case OK : break;

		default : return ( nRet );
	}


	/** OK now get the handle of the setup summary we want to change.
	**/
	
	hSummary = hSetupSummary [ nSetupIndex ];
	
	/** now lock the summary **/
	if ( ( pciSummary = (PCHANNELINFO) LocalLock ( hSummary ))== NULL ) {
		/** We're out of memory so return. Must unlock the
		*** summary table first.
		**/
		return ( IMS_OUT_OF_MEMORY );
	}


	/** OK we can now set the channel.
	**/
	/** STRUCTURE ASSIGNMENT **/
	pciSummary [ nChanIndex ] = lpciChannel [ 0 ];
	
	LocalUnlock ( hSummary );

    asdSetup [ nSetupIndex ].bSetupChanged = TRUE;

	return ( OK );
}






/**			GetChannelIndexInIndexedSetup ()
***
*** Searches through the setup to find the index of the channel.
*** Sets pnChanIndexto the channel index.
***
*** Returns.
***	OK - channel found.
***	IMS_OUT_OF_MEMORY
***	IMS_BAD_CHANNEL - channel not in this setup.
***
**/

short GetChannelIndexInIndexedSetup ( short nSetupIndex , PCHANNELINFO 
					lpciChannel , short * pnChanIndex )

{

short	nChannel;
HANDLE	hSummary;
PCHANNELINFO	pciSummary;


	/** OK now get the handle of the setup summary we want to look in.**/
	hSummary = hSetupSummary [ nSetupIndex ];
	
	/** now lock the summary **/
	if ( ( pciSummary = (PCHANNELINFO) LocalLock ( hSummary ))== NULL ) {
		return ( IMS_OUT_OF_MEMORY );
	}
	
	/** Now we look through the channels to find the one we want
	**/	
	for ( nChannel = 0 ; nChannel < asdSetup [ nSetupIndex ].nChannels ;
								nChannel ++ ) {
				
		/** Compare the frame number. If not equal then
		*** continue.
		**/
		if ( lpciChannel->nFrame != pciSummary [ nChannel ].nFrame ) {
			continue;
		}
		
		/** Check if this is the channel we want
		**/
		if ( lpciChannel->nChannel == pciSummary [nChannel ].nChannel){
			/** This channel is the one we want
			**/
			*pnChanIndex = nChannel;
			LocalUnlock ( hSummary );
			return ( OK );
		}
	}

	/** We have not found it **/
	LocalUnlock ( hSummary );

	return ( IMS_BAD_CHANNEL );
}







/*************************************************************************/
/**			AddSetupToList ()
***
*** Creates a new empty setup with no channels.
*** It also flushes out any loaded setup which is not in 
*** use by an application. The only reason for doing this is that we are about
*** to load another channel. To do this we need summaries of all loaded
*** setups in order to check that setups do not share channels.
***
*** Returns.
***	OK - setup added.
***	IMS_TOO_MANY_SETUPS.
***
**/

short AddSetupToList ( char * lpszSetupName  )

{

	short	nSetup;
	short   nKey;
	short   nLockCount;

	/** Before we can load the setup we must have a summary of
	*** all loaded setups to check that channels are only in
	*** one setup.
	**/
	for ( nSetup = 0; nSetup < MAX_SETUPS; nSetup ++ ) {			
		
		/** Any setup not in active use can be removed **/
		if ( GetSetupState (nSetup, &nLockCount) == SETUP_KEY_LOADED ) {
			RemoveSetupFromList ( nSetup );
		}
				
		/** If its not free we must get the summary **/
		if ( nLockCount != 0) {
			/** If we do not have it already **/
			if ( hSetupSummary [nSetup] == NULL ) {
				GetSummaryFromFile ( nSetup );
			}
		}
		
	}
		
	/** Find a free index for this setup.**/	
	for ( nSetup = 0; nSetup < MAX_SETUPS; nSetup ++ ) {
		/** if it is free **/
		if ( GetSetupState ( nSetup, &nLockCount) == SETUP_KEY_FREE ) {	
			break;
		}
	}

	/** Give up if there is no room for a new setup **/
	if (nSetup == MAX_SETUPS) {
		return ( IMS_TOO_MANY_SETUPS );
	}

	/** initialise the fields in the structure.**/
	lstrcpy ( (LPSTR) asdSetup [ nSetup ].szSetupName ,lpszSetupName );		
	asdSetup [ nSetup ].nChannels = 0;
	asdSetup [ nSetup ].szFileSpec [ 0 ] = '\0';
	asdSetup [ nSetup ].bSetupChanged = FALSE;

	hSetupSummary [ nSetup ] = NULL;
	/** Set the keys to FREE. **/
	for ( nKey = 0; nKey < MAX_SETUP_KEYS; ++nKey ) {
		acSetupKey [nSetup] [nKey] = SETUP_KEY_FREE;
	}

	return ( OK );
}





/**********************************************************************/
/**  RemoveSetupFromList ()
***
***  Removes the indexed setup from the list and deletes all the monitor channels
***  it held. The summary memory is freed and the list and keys are initialised.
***
***  Returns.
***  OK 
***  IMS_OUT_OF_MEMORY.
***
**/

short RemoveSetupFromList ( short nIndex )

{


	short          nChan;
	PCHANNELINFO   pciSummary;
	static short   nIMLHandle;
	short          nRet;
	short          nKey;
	short          nError;

	

	/** We need to clear out any channels in the setup.
	**/

	if ( asdSetup [ nIndex ].nChannels > 0 ) {
		
		
		/** If we do not have the file summary we must get it **/
		if ( hSetupSummary [nIndex] == NULL ) {
			nError = GetSummaryFromFile ( nIndex );
			if (nError != OK ) {
				return (nError);
			}

		}
	
	
		/** there are channels in this setup so free them up.
		***
		*** We need to run through the channels and check for
		*** monitors. If we find any we need to release them.
		*** We need a commlib handle to do this.
		**/

		nRet = OpenIMLInterface ( &nIMLHandle );

		if ( nRet != 0 ) {
			/** failed to open so return.**/
			return ( nRet );
		}

		if ( ( pciSummary = (PCHANNELINFO) LocalLock ( hSetupSummary [
		                     nIndex ] ))== NULL ) {
			/** We're out of memory so return.**/
			return ( IMS_OUT_OF_MEMORY );
		}

	
		/** Now we look through the channels to find monitors**/
		for ( nChan = 0; nChan < asdSetup [ nIndex ].nChannels; nChan ++ ) {
				
			/** Check the type code. If N then we have a monitor
			*** so we need to release it.
			**/
			if ( pciSummary [ nChan ].cTypeCode == 'N' ) {
				ReleaseMonitorChannel ( &(pciSummary [nChan]), nIMLHandle );
			}
		}

		LocalUnlock ( hSetupSummary [ nIndex ] );
		LocalFree ( hSetupSummary [ nIndex ] );
		CloseIMLInterface ( nIMLHandle );
	}

	/** Initialise the set up structure **/
	lstrcpy ( (LPSTR) asdSetup [ nIndex ].szSetupName ,"" );		
	asdSetup [ nIndex ].nChannels = 0;
	asdSetup [ nIndex ].szFileSpec [ 0 ] = '\0';
	asdSetup [ nIndex ].bSetupChanged = FALSE;
	hSetupSummary [ nIndex ] = NULL;
	/** Set the keys to FREE. **/
	for ( nKey = 0; nKey < MAX_SETUP_KEYS; ++nKey ) {
		acSetupKey [nIndex] [nKey] = SETUP_KEY_FREE;
	}
	
	
	return ( OK );
}




/***********************************************/
/**   ModifyIndexedSetupDescription ()
***
***   Changes the description associated with a setup. It is passed the
***   the index of the setup and a pointer to the new description string.
***
***   Returns.
***   OK
***   IMS_OUT_OF_MEMORY. - out of memory,
***   IMS_BAD_NAME. - named setup is not in the list of loaded setups.
***
**/

short ModifyIndexedSetupDescription ( short nIndex, char * pszDescription )

{


	/** We must ensure that the description string is not too long
	*** Then we copy the string into our setup list and then unlock the list.
	**/
	if ( ( strlen ( pszDescription ) ) > MAX_DESCRIPTION_LENGTH ) {
	
		/** it is too long so truncate it.
		**/
		pszDescription [ MAX_DESCRIPTION_LENGTH ] = '\0';
	}
	
	/** Copy the string in **/
	strcpy ( (LPSTR) asdSetup [ nIndex ].szDescription ,pszDescription );

	asdSetup [ nIndex ].bSetupChanged = TRUE;

	return ( OK );
}
