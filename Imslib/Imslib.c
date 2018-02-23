/**	IMSLIB.C	Nigel Roberts.	29 August, 1990.
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
***	IMSINTER.C - Internal functions.
***	IMSLIB.MAK - make file.
***
***	Version History.
***
***	P1.00 - 29 August, 1990 - inception.
***	P1.01 - 03 October, 1990 - 
*** Added IMSGetSetupDescription (). 
*** Changed IMSLoadSetup so it takes an extra argument, the setup name which
*** it sets.
***
***	P1.02 - 4 October, 1990 -
*** Changed setup sunmmary structure. Added bEnabled field set true if E=1
*** in setup file.
***
***	P1.03 - 08 November, 1990 -
*** Recompiled C6.
***
***	P1.04 - 29 November, 1990 -
*** Added check to see if channels in setup to load are already in use or
*** not.
***
***	P1.05 - 04 December, 1990 -
*** Restructured are internal data structures. All storage of summaries etc.
*** is now in local memory and we copy it to an global buffer to pass back to
*** an application. This means the application must now free the memory when
*** it has done with it.
***
***
***	P1.06 - 12 December, 1990 -
*** Save and load setup functions restructured and optimized.
***
***	P1.07 - 19 December, 1990 -
*** Fixed one or two little bugs.
*** Load and save setup now change cursor to hour glass.
***
***	P1.08 - 16 January, 1991 -
*** Sorted error return from load and save setup.
***
***	P1.09 - 06 March, 1991 -
*** If we get an error in WriteSetupFile it now releases all frames before
*** closing the Comms. I.e. Does not rely on CloseComms doing it.
***
*** Now no longer requires a blank line on the end of a setup file to avoid
*** loosing the last line. Also does not write a blank line on the end of
*** a setup file.
***
***
***	P1.10 - 11 March, 1991 -
*** Started to finish functions required only by setupiml.
***  IMSModifyLoadedSetupList () and IMSModifySetupSummary ().
***
***
***	P2.0 - 12 March, 1991 -
*** Added new functions to allow setups to be changed (not partitioned)
*** while loaded by applications :- IMSFinishedConfiguring ().
*** IMSEnableConfiguring () and IMSDisableConfiguring (). + new load release
***
*** Taken PeekMessage out of GetReply - could have made the library reentrant.
***
***
***	P2.01 - 22 April, 1991 -
*** Fixed enable/disable configuring mechanism.
***
***
***	P2.02 - 24 May, 1991 -
*** Now handles U line types properly. I.e. puts them in the summary but
*** does not send them to the frame on load setup. When saving a setup we
*** build the U line by hand from the contents of the summary.
***
***	P2.03 - 17 June, 1991 -
*** Fixed bugs stopping us working with frame numbers > 10.
***
***
***     P3.00 - 25 October, 1991 -
*** Added Get Setup file spec function to user interface.
***
***     P3.01 - 31 October, 1991 -
*** Tidied up storing of file spec. - set it when setup saved, - clear it
*** when setup is freed.
***
***     P3.02 - 13 January, 1992 -
*** Fixed a problem when the end of a line coincides with the end of the 4K
*** buffer used to read setups into. Fixed by making the buffer slightly
*** larger than the file.
***
***     P3.03 - 14 Febuary, 1992 -
*** Problems removing channels from a summary fixed. Removing channels caused
*** UAE on some machines - going off end of summary, When all removed could
*** not add any - summary handle realloc'ed to zero -> NULL handle which
*** cannot be realloc'ed back up again.
***
***     P3.04 - 27 Febuary, 1992 -
*** Added version stamp in RC file.
***
***     P3.05 - 27 March,1992 -
*** Altered downloading of setup files. We used to dump everything in one go
*** which is OK on GPIB and solid RS232 links wiith Xon Xoff hut on RS485 we
*** usually have no hold off mechanism and so we can overflow 3303 receive
*** buffers with large setups at high baud rates.
***
***     P3.06 - 12 November, 1992 -
*** Altered the way we request and release devies when downloading setups.
*** We used to request frames as we found them then release all when done.
*** This fucks up if we have a 3302 then 3300 frame as the 3300 handler cannot
*** get the GPIB as we have a already got it when we requested the 3302.
*** To get round this we request, use, release frames as we need them.
***
***     P3.07 - 04 December, 1992 -
*** Implemented timeout on getting IML replies as 5 seconds.
***
***
***     P4.01 - 29 March, 1993 -
*** Added monitor channel support (4.00) and IMSRequestMonitorChannel()
*** and IMSReleaseMonitorChannel() for SetupIML.
***
***     P4.02 - 08 April, 1993 -
*** Added setup name to request/release monitor channel calls so we can
*** add/remove the monitor channel from the setup. We do this so setupiml
*** does not need to set partitioning mode so it can add/remove monitors
*** while setup is idle.
***
***     P4.03 - 25 January, 1994 -
*** Change from old commslib interface to generic interface because the old
*** one is on the way out.
***
***     P4.05 - 19 April, 1994 -
*** Rebuilt for release.
***
***
***     P4.06 - 08 February, 1995 -
*** Allow for potential 6 digit channel numbers.
***
***     P4.07 - 09 May, 1995 -
*** Fixed bug in 6 digit channel numbers.
***
***     P4.08 - 06 December, 1995 -
*** Fixed bug in DisableChannel - monitors and deffault channel names
*** relating to 6 digit channel numbers.
***
***     P5.00 - July 98
*** Converted to 32 bit code. This involves defining a shared data segment
*** which cannot contain dynamically allocated data. Therefore it is not
*** possible to share setup summaries. Each app has its own copy of imslib
*** which holds only the summary required by the app for its Connect dialog.
*** If it needs to load a new setup then it must obtain from the ims files
*** summaries of all loaded setups in order to check the new setup for
*** channel conflicts. These setups are destroyed after the load.
*** The whole setup locking mechanism was rationalised from the old topsy turvy
*** version to one using the setup key array. Several small changes were made to
*** data structures and functions. Since there is no need for compatibility
*** with existing apps ( these must also be converted ) then there are no extra
*** implications.
***
***    P6.00 - Jan 2003
*** Changed to iml.ini within the install directory.
*** Added FilePath to setup summary
***    
**/





#define _WINDLL		/** It is a Windows DLL **/
#define NOCOMM		/** We don't need the COMM stuff **/
#define NOKANJI		/** We don't need this either. **/
#define NOMINMAX	/** To prevent duplicate definition **/


#include <windows.h>	/** Windows include file.**/
#include <dos.h>	/** for splitpath **/
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <memory.h>	/** for _fmemcpy **/


#include "imslib.h"
#include "Copycomm.h"
#define COMMSLIB

#include "\Windmill 32\Compile Libs\imlinter.h"


#include "imsinc.h"






/** Library version control string **/

static char *szVersionString = "6.00";



/** Initialisers for shared data area **/
#define INITKEYS  0,0,0,0,0,0,0,0,0,0
#define INITSETS  "","",0,"",FALSE
#define INITMONS  -1,0,0,0,0,0,0,0,0,0,0,0


/** Data area shared between all copies of IMS lib **/

#pragma data_seg (".sdata")


short bPartitioning = FALSE; /** partitioning semaphore **/


SETUPDETAILS asdSetup [ MAX_SETUPS ] =
{	INITSETS,INITSETS,INITSETS,INITSETS,INITSETS,
	INITSETS,INITSETS,INITSETS,INITSETS,INITSETS, };	
/** our internal details of the setups loaded.**/

char acSetupKey [ MAX_SETUPS ] [ MAX_SETUP_KEYS ] =
{	INITKEYS,INITKEYS,INITKEYS,INITKEYS,INITKEYS,
	INITKEYS,INITKEYS,INITKEYS,INITKEYS,INITKEYS, }; 

/** These keys indicate the state of each application using a particular set up.
*** Each application is given a key when it calls IMSLoadAndLock.
*** SETUP_KEY_FREE       indicates that no application has that key
*** SETUP_KEY_INUSE      indicates that an application is actively using the key
***                      eg. logger is logging
*** SETUP_KEY_CONFIG     indicates that an application has the key but will allow
***                      SetupIML to change the setup. The application will be in
***                      a non active state eg Chart stopped.
*** SETUP_KEY_CHANGED    When SetupIML modifies a setup we set this flag into
***                      keys which were otherwise at SETUP_KEY_CONFIG. When
***                      the application comes to reuse the setup it is informed
***                      that it has changed.
***
*** Key 0 has a special function in that it is always allocated to SetupIML. It can
*** have  3 values.
*** SETUP_KEY_FREE 
*** SETUP_KEY_EDIT       This means that SetupIML is editing the file.
*** SETUP_KEY_LOADED     This means the setup is loaded in hardware but not used by
***                      any application. It will be flushed if any other setup is
***                      loaded but can be used if any app needs it.
***
**/

/*****************************************************/
/** Array of structures to hold monitor data for each device **/
MON_DEV amDev [ MAX_DEVICES ] = 
{	INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS,      
	INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS,
	INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS,
	INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS, INITMONS };


char cImlini [MAX_PATH];

#pragma data_seg ()



/** Data local to this copy of IMSLIB **/

/** The setup summaries are of unknown size. They are held in local memory
*** rather than shared memory since dynamic allocation is not allowed in shared
*** memory. When a copy of IMSLIB loads a setup then it produces a summary. If
*** the setup was loaded by another copy then the summary will not exist in 
*** local memory. This copy must get it from file if needed.
**/

HANDLE hSetupSummary  [ MAX_SETUPS ];


/*************************************************************/
/**		DllMain ()
***
*** This function is the DDL initialisation function -
***
**/

BOOL APIENTRY DllMain ( HANDLE hModule , DWORD ul_reason_for_call , LPVOID lpReserved )

{	short nIndex;

	if ( ul_reason_for_call == DLL_PROCESS_ATTACH ) {
		/** Set the partitioning semaphore False. 
		**/
		for ( nIndex = 0; nIndex < MAX_SETUPS; ++nIndex ) {
			hSetupSummary [nIndex] = NULL;
		}

		GetIMLINIPath ( cImlini, MAX_PATH );
	}

	return ( TRUE );
}


/*************************************************************/
/** IMSLoadSetup
***              This function is used to load a .IMS set up file.
***  Only SetupIML calls it directly. Other Apps use IMSLoadAndLockSetup.
***  Parameters :
***       lpszPathName      Path for set up file
***       lpszSetupName     The set up file name
***       bConfigFlag       Set TRUE only by SetupIML to edit setup
**/



short _declspec (dllexport)  WINAPI IMSLoadSetup ( char * lpszPathName , char *
					lpszSetupName, short bConfigFlag )

{

	static char   szFullPath [ _MAX_PATH ];
	static char   szDrive [ _MAX_DRIVE ];
	static char   szDir [ _MAX_DIR ];
	static char   szFileName [ 9 ]; 
	static char   szExt [ _MAX_EXT ];
	static short  nSetupIndex;
	short         nIndex;
	short         nRet;
	short         nSetupState;
	HCURSOR       hSaveCursor;
	short         nLockCount;
	short         nCurrentIndex = -1;


	/** If we are partitioning then we cannot load so return partitioning.
	**/
	if ( bPartitioning ) {
		return ( IMS_PARTITIONING );
	}	

	/** copy the far path to our local one so we can split it.
	*** Use local copy incase memory shuffles knackering our far pointer.
	**/
	lstrcpy ( (LPSTR) szFullPath , lpszPathName );
	_splitpath ( szFullPath , szDrive, szDir, szFileName, szExt );

	/** Copy the file name into the setup name.
	**/
	lstrcpy ( lpszSetupName, szFileName );

	/** Is this setup already loaded. We see if we can get the index of 
	*** the filename to tell if it is.loaded
	**/
	nRet = GetIndexToSetup ( (LPSTR) szFileName , &(nSetupIndex) );
	nSetupState = GetSetupState ( nSetupIndex, &nLockCount);

	switch ( nRet ) {
		
		case OK :    /** setup already exists. **/

			/** We may not have its summary if it has been loaded 
			*** by another copy of IMSLib
			**/
			if ( hSetupSummary [nSetupIndex] == NULL ) {
				GetSummaryFromFile ( nSetupIndex );
			}
			/** do we want to configure ?.
			**/
			if ( bConfigFlag ) {
				/** Yes we do so check if setup is in use.
				**/
				if ( nSetupState == SETUP_KEY_INUSE ) {
					/** The setup is in use so we can't configure
					*** channel in use.
					**/
					return ( IMS_CHANNEL_IN_USE );
				}
				/** We are allowed to edit the setup. So put the 
				*** SetupIML key to edit
				**/
				acSetupKey [nSetupIndex][0] = SETUP_KEY_EDIT;
			}

			if ( ! bConfigFlag ) {
				/** we don't want to configure so ensure the setup is
				*** NOT being configured.
				**/
					if ( nSetupState == SETUP_KEY_EDIT ) {
					/** Being configured so return configuring.
					**/
					return ( IMS_CONFIGURING );
				}
				/** We don't need to do anything else since it is already
				*** loaded.
				**/
			}			
								
			return ( IMS_ALREADY_LOADED );

		
		case IMS_BAD_NAME :	/** Setup not found in list **/
			
	
			/** The setup is not loaded so start loading it. 
			*** We need to open the file and read it line by line. We check
			*** the type code of each line to see if it is one we need to 
			*** send to the frame and also if it is one which we need to 
			*** parse to store the info in the setup sumary.
			*** We need to create a new summary then add the channels to it.
			**/

			
			/** Only one summary will currently exist - remember it **/
			for ( nIndex = 0; nIndex < MAX_SETUPS; ++nIndex ) {
				if ( hSetupSummary [nIndex] != NULL ) {
					nCurrentIndex = nIndex;	
					break;
				}
			}

			nRet = AddSetupToList ( (LPSTR) szFileName );
		
			if ( nRet != OK ) {
				/** an error occured so return it.**/
				return ( nRet );
			}

			/** Now load the file. Set the cursor to the hour glass.**/
			SetCapture ( GetActiveWindow () );
			hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) );
			nRet = LoadSetupFile ( szFullPath, szFileName );
			SetCursor ( hSaveCursor );
			ReleaseCapture ();

			/** Get the index of this new setup. **/
			GetIndexToSetup ( (LPSTR) szFileName , &(nSetupIndex) );


			
			/** Now check the outcome of the load.
			**/
			switch ( nRet ) {
			
				case IMS_SUCCESS :	
					/** Loaded OK so set up the key if this
					*** is SetupIml
					**/
					if ( bConfigFlag ) {
						acSetupKey [nSetupIndex][0] = SETUP_KEY_EDIT;	
					}

					/** Initialise the set up changed flag **/
					asdSetup [ nSetupIndex ].bSetupChanged = FALSE;
					/** Discard all other setup summaries and free the
					*** memory.
					**/
					for ( nIndex = 0; nIndex < MAX_SETUPS; ++nIndex ) {
						if ( nIndex != nSetupIndex ) {
							if ( hSetupSummary [nIndex] != NULL ) {
								LocalFree ( hSetupSummary [nIndex] );
								hSetupSummary [nIndex] = NULL;
							}
						}
					}

					return ( nRet );
				
				default :	
					/** Need to remove the setup as we failed to
					*** load it.
					**/
					RemoveSetupFromList ( nSetupIndex );
					/** Discard all unwanted setup summaries and free the
					*** memory.
					**/
					for ( nIndex = 0; nIndex < MAX_SETUPS; ++nIndex ) {
						if ( nIndex != nCurrentIndex ) {
							if ( hSetupSummary [nIndex] != NULL ) {
								LocalFree ( hSetupSummary [nIndex] );
								hSetupSummary [nIndex] = NULL;
							}
						}
					}

					return ( nRet );
			}
			
			break; 
		
		default :	/** an error has occured so return it **/
			return ( nRet );
	}

	return ( IMS_SUCCESS );
}




/** IMSGetLoadedSetupList ()
***
*** Returns the handle to the list of setups and sets the number of setups
*** loaded. Apps use this to fill their Open setup list.
***
**/

GLOBALHANDLE _declspec (dllexport)  WINAPI IMSGetLoadedSetupList ( short * pnNumSetups )

{

	short       nSetup;
	HANDLE      hSetupList;
	PSETUPINFO  pSetupList;
	short       nCopyIndex = 0;
	short       nLockCount;



	/**  Set the number of loaded setups.
	**/
	*pnNumSetups = GetSetupCount ();


	/** if there are no setups loaded return NULL.**/
	if ( *pnNumSetups == 0 ) {
		return ( NULL );
	}


	/** allocate a global buffer big enough to store all the loaded
	*** setups. Copy the appropriate setup details into the structure
	**/

	hSetupList = GlobalAlloc ( GMEM_MOVEABLE | GMEM_ZEROINIT, (DWORD)
				 ( sizeof ( SETUPINFO ) *  *pnNumSetups ) );


	if ( hSetupList == NULL ) {
		/** Out of memory so set the number of setups to out of
		*** memory and return NULL.
		**/
		*pnNumSetups = IMS_OUT_OF_MEMORY;
		return ( NULL );
	}


	pSetupList = (PSETUPINFO) GlobalLock ( hSetupList );

	if ( pSetupList == NULL ) {
		/** Out of memory so set the number of setups to out of
		*** memory and return NULL.
		**/
		GlobalFree ( hSetupList );
		*pnNumSetups = IMS_OUT_OF_MEMORY;
		return ( NULL );
	}



	/** Ok, we can now copy the setup info from our internal data structs
	*** into the setup list.
	**/

	for ( nSetup = 0; nSetup < MAX_SETUPS; nSetup ++ ) {

		if ( GetSetupState ( nSetup, &nLockCount) == SETUP_KEY_FREE ) {
			/** This one not inuse so continue.
			**/
			continue;
		}

		strcpy ( pSetupList [ nCopyIndex ].szSetupName, 
					 asdSetup [ nSetup ].szSetupName );

		strcpy ( pSetupList [ nCopyIndex ].szDescription, 
					 asdSetup [ nSetup ].szDescription );

		strcpy ( pSetupList [ nCopyIndex ].szFileSpec,
					 asdSetup [ nSetup ].szFileSpec );

		nCopyIndex ++;

	}

	GlobalUnlock ( hSetupList );

	return ( hSetupList );
}









/**      IMSGetSetupSummary ()
***
*** Gets a handle to the setup summary.lpnChannels is set to the number of
*** channels in the setup. If the return is NULL then the reason is in 
*** pnChannels. It can be IMS_OUT_OF_MEMORY or IMS_BAD_NAME.
***
**/

GLOBALHANDLE _declspec (dllexport)  WINAPI IMSGetSetupSummary ( char * pszSetupName , 
						short  * pnChannels )
						
{

static short nIndex;
PCHANNELINFO	pSummary;
HANDLE	hSummary;
PCHANNELINFO	pLocalSummary;


	/** Get the index to the setup so we can get the handle to it from
	*** the summary table. Put the return code in lpnChannels as it goes
	*** here if there is an error.
	**/
	
	*pnChannels = GetIndexToSetup ( pszSetupName , &(nIndex) );
	
	if ( *pnChannels != OK ) {
		/** We have an error so return NULL.**/
		return ( NULL );
	}
	

	/** If this summary has no channels then we just return NULL. **/

	if ( asdSetup [ nIndex ].nChannels == 0 ) {
		*pnChannels = 0;
		return ( NULL );
	}

	
	/** OK we need to allocate a global buffer to copy the summary into.
	**/
	hSummary = GlobalAlloc ( GMEM_MOVEABLE | GMEM_ZEROINIT, (DWORD)
		( sizeof ( CHANNELINFO ) * asdSetup [ nIndex ].nChannels ));


	if ( hSummary == NULL ) {
		/**  We're out of memory so set number of channels to out of
		*** memory and return NULL.
		**/

		*pnChannels = IMS_OUT_OF_MEMORY;
		return ( NULL );
	}


	pSummary = (PCHANNELINFO) GlobalLock ( hSummary );

	if ( pSummary == NULL ) {
		/**  We're out of memory so set number of channels to out of
		*** memory and return NULL.
		**/

		*pnChannels = IMS_OUT_OF_MEMORY;
		GlobalFree ( hSummary );
		return ( NULL );
	}


	/** Now lock our internal summary so we can copy it across.**/
	pLocalSummary = (PCHANNELINFO) LocalLock ( hSetupSummary [ nIndex ] );

	if ( pLocalSummary == NULL ) {
		/**  We're out of memory so set number of channels to out of
		*** memory and return NULL.
		**/
		*pnChannels = IMS_OUT_OF_MEMORY;
		GlobalUnlock ( hSummary );
		GlobalFree ( hSummary );
		return ( NULL );
	}


	/** Now copy the summary to the global buffer.
	**/

	memcpy ( (PCHANNELINFO) pSummary, (PCHANNELINFO) pLocalSummary,
		 ( sizeof ( CHANNELINFO ) * asdSetup [ nIndex ].nChannels ));

	*pnChannels = asdSetup [ nIndex ].nChannels;

	LocalUnlock ( hSetupSummary [ nIndex ] );
	GlobalUnlock ( hSummary );

	return ( hSummary );
}	









/** IMSSaveSetup ()
***
*** Writes the named setup to the file specified by lpszPath. The file 
*** must not exist if it is to be created, it must exist if it is to be 
*** appended to. nMode specifies whether to create or append.
***
*** Returns. 
***	OK - Setup saved.
***	IMS_FILE_EXISTS - file to create already exists.
***	IMS_CANNOT_CREATE - could not create file.
***	IMS_FILE_NOT_FOUND - could not find file to append to.
***	IMS_OUT_OF_MEMORY - out of memory.
***	IMS_DISK_FULL - disk full while saving file - save failed.
***	IMS_CHANNEL_ERROR - unable to show channel in setup summary.
***	IMS_BAD_NAME - No setup with this name.
***	IMS_BAD_OPERATION - mode not append or create.
***
**/

short _declspec (dllexport)  WINAPI IMSSaveSetup ( char * lpszSetupName , char 
                                                * lpszPath , short nMode )

{

static short nIndex;
short	nRet;
short	nFileHandle;
static OFSTRUCT ofFile;
HCURSOR hSaveCursor;


	/** Check that the setup exists. It may only be in memory not in file.
	**/

	if ( ( nRet = GetIndexToSetup ( lpszSetupName , &(nIndex) ) ) != OK ) {
		/** The setup does not exist or we have encountered an error so
		*** return.
		**/
		return ( nRet );
	}
	

	switch ( nMode ) {
		
		case IMS_NEW :	/** We need to create the file. Fail if it already 
				        *** exists.
		                **/
		
			nFileHandle = OpenFile ( lpszPath , (LPOFSTRUCT) &(ofFile) , OF_EXIST );
			if ( nFileHandle != HFILE_ERROR ) {
				/** the file exists so return error.**/
				return ( IMS_FILE_EXISTS );
			}

			/** OK now create the file for writing to.**/
			nFileHandle = OpenFile ( lpszPath , (LPOFSTRUCT) &(ofFile) ,
										OF_CREATE | OF_WRITE );
			if ( nFileHandle == HFILE_ERROR ) {
				/** We failed to open the file so return cannot 
				*** create.
				**/
				return ( IMS_CANNOT_CREATE );
			}



			/** Set the file spec in the structure.
 			**/

			strcpy ( asdSetup [ nIndex ].szFileSpec, ofFile.szPathName );

			asdSetup [ nIndex ].bSetupChanged = FALSE; 

			break;


		case IMS_APPEND :
			/** The file should already exist. We add
			*** the setup onto the end.
			**/
	
			nFileHandle = OpenFile ( lpszPath , (LPOFSTRUCT) &(ofFile) ,
															OF_EXIST | OF_SHARE_COMPAT );
			if ( nFileHandle == HFILE_ERROR ) {
				/** the file does not exists so return error.
				**/
				return ( IMS_FILE_NOT_FOUND );
			}
		

			/** OK now open the file for writing to.
			**/
			nFileHandle = OpenFile ( lpszPath , (LPOFSTRUCT) &(ofFile) ,
								OF_READWRITE | OF_SHARE_COMPAT);
			if ( nFileHandle == HFILE_ERROR ) {
				/** We failed to open the file so return cannot 
				*** create.
				**/
				return ( IMS_FILE_NOT_FOUND );
			}

			/** now set the file pointer to the end of the file so we can
			*** append onto it.
			**/
			if ( _llseek ( nFileHandle , 0L , SEEK_END ) == -1L ) {
				_lclose ( nFileHandle );
				return ( IMS_FILE_NOT_FOUND );
			}

			break;
		
				
		default :	/** This is a bad mode so return BAD_OPERATION.
		            **/
			return ( IMS_BAD_OPERATION );
	}
	
	
	/** We have opened the file in the correct mode so we can now 
	*** save the setup to it and the close it.
	**/

	SetCapture ( GetActiveWindow () );
	hSaveCursor = SetCursor ( LoadCursor ( NULL, IDC_WAIT ) );

	nRet = WriteSetupToFile ( nIndex , nFileHandle );

	SetCursor ( hSaveCursor );
	ReleaseCapture ();


	_lclose ( nFileHandle );
	
	return ( nRet );
}								








/** IMSLibVersion ()
***
*** Sets the version string into lpszVersion if it is not NULL. returns the 
*** major version number of this library.
***
**/
								
short _declspec (dllexport)  WINAPI IMSLibVersion ( char  * lpszVersion, short
								 nMaxChars )

{

	if ( lpszVersion != NULL ) {
		/** We have been passed a string so copy the version string
		*** into it.
		**/

		/** Check the string is big enough.
		**/
		if ( nMaxChars > (short) strlen ( szVersionString ) ) {

			lstrcpy ( lpszVersion , (LPSTR) szVersionString );

		} else {

			strncpy ( lpszVersion, szVersionString,
							   nMaxChars -1 );

			lpszVersion [ nMaxChars -1 ] = '\0';
		}
	}
	
	/** return the first character of the version string as this is the 
	*** major version number.
	**/
	return ( (short) ( szVersionString [ 0 ] - '0' ) );
}	








/**			IMSRequestPartitioningMode ()
***
*** Check that there are no setups whose lock count is non-zero then set the
*** bPartitioning flag.
***
*** Returns.
***	OK - request accepted.
***	IMS_CHANNEL_IN_USE - a setup has non-zero lock count.
***	IMS_OUT_OF_MEMORY - could not lock setup list.
***
**/

short _declspec (dllexport)  WINAPI IMSRequestPartitioningMode ( void )

{

	short 	nIndex;
	short   nState;
	short   nLockCount;


	/** If we are already partitioning then return partitioning.
	**/

	if ( bPartitioning ) {
		return ( IMS_PARTITIONING );		
	}


	/** Go through the setups one by one. If any is in use
	*** then fail the request.
	**/
	
	for ( nIndex = 0 ; nIndex < MAX_SETUPS ; nIndex ++ ) {

		nState = GetSetupState (nIndex, &nLockCount);

		if ( nLockCount != 0 ) {
			return ( IMS_CHANNEL_IN_USE );
		}
	}

	/** OK The request is successful so set the partitioning flag and
	*** return OK.
	**/
	
	bPartitioning = TRUE;
	
	return ( OK );
}







/**			IMSReleasePartitioningMode ()
***
*** Switches the library out of partitioning mode if this mode is selected.
***
*** Returns OK or IMS_NOT_PARTITIONING.
***
**/

short _declspec (dllexport)  WINAPI IMSReleasePartitioningMode ( void )

{
	
	if ( bPartitioning ) {
		/** We are in partitioning mode so turn it off and return OK.
		**/
		bPartitioning = FALSE;
		return ( OK );
	}
	
	/** We are not in partitioning mode so return Not partitioning.
	**/
	return ( IMS_NOT_PARTITIONING );
}











/**			IMSModifyLoadedSetupList ()
***
*** Adds or removes setups from the setup list while partitioning.
*** When setup is added it is set to configuring mode.
***
*** Returns.
***	OK - setup added / removed.
***	IMS_NOT_PARTITIONING
***	IMS_BAD_NAME - add setup already exists. - remove setup not found.
***	IMS_BAD_OPERATION - not add or remove.
***
**/


short _declspec (dllexport)  WINAPI IMSModifyLoadedSetupList ( char  * lpszSetupName ,
							short nOperation )

{

static short	nIndex;
short nRet;



	/** Check we are in partitioning mode. We must be partitioning to
	*** change the setup list.
	**/

	if ( !bPartitioning ) {
		/** We are not partitioning so return Not partitioning.
		**/

		return ( IMS_NOT_PARTITIONING );
	}



	/** OK check the operation required. If it is add then we must check
	*** to see if we already have a setup of this name. If we have then
	*** return BAD NAME. otherwise we add the setup and return the
	*** result.
	*** If we are removing a setup we must see if it exists. If it does
	*** we can remove it. If it does not we return Bad name.
	*** Any other operation we return bad operation.
	**/


	switch ( nOperation ) {

	case IMS_ADD :

		/** OK does it already exist. If so then return bad name.
		**/

		if ( GetIndexToSetup ( lpszSetupName, &nIndex ) == 0 ) {
			/** Setup already exists so return bad name.
			**/

			return ( IMS_BAD_NAME );
		}

		/** OK add this setup to the list and return the result.
		**/
		nRet = AddSetupToList ( lpszSetupName );

		/** Set the keys as being edited since only SetupIml
		*** can call this function.
		**/
		if ( nRet == OK) {
			GetIndexToSetup ( lpszSetupName, &nIndex );
			acSetupKey [nIndex][0] = SETUP_KEY_EDIT;
		}

		return (nRet);



	case IMS_REMOVE :

		/** Remove the setup if it exists otherwise return bad name.
		**/

		if ( GetIndexToSetup ( lpszSetupName, &nIndex ) ==
							    IMS_BAD_NAME ) {

			/** Setup does not exist so return bad name.
			**/

			return ( IMS_BAD_NAME );
		}


		RemoveSetupFromList ( nIndex );

		return ( OK );


	default :
		/** This is a bad operation so return such.
		**/

		return ( IMS_BAD_OPERATION );

	}


	return ( OK );
}
							










/**				IMSModifySetupSummary ()
***
*** Adds, removes or changes a channel in the setup summary. Addition and
*** removal can ony be done while partitioning. Changing can only be done
*** while configuring.
***
*** Returns
***	OK - operation successful
***	IMS_NOT_PARTITIONING	- adding / removing channels
***	IMS_NOT_CONFIGURING	- changing channel
***	IMS_OUT_OF_MEMORY	- when adding
***	IMS_BAD_NAME		- Named setup not loaded
***	IMS_BAD_CHANNEL 	- channel for changing/removing not present
***	IMS_BAD_OPERATION	- not add, remove or change
***	IMS_DUPLICATE_CHANNEL.	- adding channel already in setup.
***
**/

short _declspec (dllexport)  WINAPI IMSModifySetupSummary ( char * lpszSetupName,
				PCHANNELINFO lpciChannel, short nOperation )

{


static short	nIndex;
short	nRet;


	/** Check that the named setup is loaded.
	**/

	if ( GetIndexToSetup ( lpszSetupName, &nIndex ) == IMS_BAD_NAME ) {

		/** The setup is not present so return bad name.
		**/

		return ( IMS_BAD_NAME );
	}


	/** OK what is the operation.
	**/


	switch ( nOperation ) {

	case IMS_ADD :

		/** OK we must be partitioning to do this so check.
		**/

		if ( !bPartitioning ) {

			/** We are not partitioning so fail.
			**/
			return ( IMS_NOT_PARTITIONING );
		}


		/** We can add this channel to the setup and return
		*** the result.
		**/

		switch ( nRet = IsThisChannelAvailable (nIndex, lpciChannel->
					nFrame, lpciChannel->nChannel ) ) {

		case OK :
			/** It is OK so add the channel.
			**/

			return ( AddChannelToIndexedSetup ( nIndex,
							   lpciChannel ) );

		default :
			/** We have a problem with the channel so return
			*** the error.
			**/
			return ( nRet );
		}


	case IMS_REMOVE :

		/** OK we must be partitioning to do this so check.
		**/

		if ( !bPartitioning ) {

			/** We are not partitioning so fail.
			**/
			return ( IMS_NOT_PARTITIONING );
		}


		/** We can remove this channel from the setup and return
		*** the result.
		**/


		return ( RemoveChannelFromIndexedSetup (nIndex, lpciChannel));


	case IMS_UPDATE :

		/** we must be configuring to change a channel so check.
		**/

		if ( acSetupKey [nIndex][0] != SETUP_KEY_EDIT ) {

			/** we are not configuring this setup so return.
			**/

			return ( IMS_NOT_CONFIGURING );
		}

		/** OK We can chane this channel and return the result.
		**/

		return ( ModifyChannelInIndexedSetup ( nIndex, lpciChannel ));


	default :

		/** This must be a bad operation so return such.
		**/

		return ( IMS_BAD_OPERATION );
	}


	return ( OK );
}









/** 			IMSModifySetupDescription ()
***
*** Changes the description associated with a setup if setup is in 
*** configuring mode.
***
*** Returns.
***	OK
***	IMS_NOT_CONFIGURING - not in configuring mode.
***	IMS_OUT_OF_MEMORY. - out of memory,
***	IMS_BAD_NAME. - named setup is not in the list of loaded setups.
***
**/

short _declspec (dllexport)  WINAPI IMSModifySetupDescription ( char * lpszSetupName ,
						char * lpszDescription )

{

static short	nIndex;
short	nRet;


	if ( ( nRet = GetIndexToSetup ( lpszSetupName , &(nIndex) ) ) != OK ) {
				
		/** This failed because either we are out of memory
		*** or the name was not found. nRet holds the reason
		*** so return it.
		**/
		
		return ( nRet );
	}
			
	/** We have the index so lock the setup list so we can set the 
	*** description.
	**/
	

	/** We must check that this setup is in configuring mode before we
	*** change the description.
	**/
	
	if ( acSetupKey [nIndex][0] != SETUP_KEY_EDIT ) {
	
		/** We are not configuring this setup so unlock and return
		*** not congifuring.
		**/
		return ( IMS_NOT_CONFIGURING );
	}

	ModifyIndexedSetupDescription ( nIndex , lpszDescription );

	
	return ( OK );
}







/**			IMSGetSetupDescription ()
***
*** Gets the description for the named setup if there is one.
***
*** Returns 
***	OK - description copied to string.
***	IMS_BAD_NAME - No setup of this name loaded.
***	IMS_OUT_OF_MEMORY.
***
**/

short _declspec (dllexport)  WINAPI IMSGetSetupDescription ( char * lpszSetupName,
				char  * lpszDescription, short nMaxChars )
			
{

static short	nIndex;
short	nRet;
short	nChar;


	if ( ( nRet = GetIndexToSetup ( lpszSetupName , &(nIndex) ) ) != OK ) {
				
		/** This failed because either we are out of memory
		*** or the name was not found. nRet holds the reason
		*** so return it.
		**/
		
		return ( nRet );
	}
			

	/** Copy the description from the setup list.
	**/
	
	nChar = 0;	
	while ( nChar < ( nMaxChars - 1 ) ) {
		lpszDescription [ nChar ] = asdSetup [ nIndex ].
						szDescription [ nChar ];
			
		if ( lpszDescription [ nChar ] == '\0' ) {
			break;
		}
		nChar ++;
	}
	lpszDescription [ nChar ] = '\0';
	
	
	return ( OK );
}















/*******************************************************/
/**          IMSLoadAndLockSetup ()
***
*** Used by applications other than SetupIML to load a setup. The setup is
*** initially locked i.e. not being configured. It can be enabled for
*** configuring by calling IMSEnableConfiguring (). It is freed by
*** IMSUnlockSetup ().
*** Calls IMSLoadSetup () to load the setup. Allocates a key to index the
*** setup.
*** Sets the name of the setup in lpszSetupName.
***
*** Returns - as IMSLoadSetup.
**/


short _declspec (dllexport)  WINAPI IMSLoadAndLockSetup ( LPSTR lpszPathName, LPSTR
					lpszSetupName, short  *lpnKey )

{

static short	nKey;
short	nRet;
static short	nIndex;



	nRet = IMSLoadSetup ( lpszPathName, lpszSetupName, FALSE );

	switch ( nRet ) {

		case IMS_SUCCESS :		
		case IMS_ALREADY_LOADED :
			/** OK carry on.
			**/
			break;

		default :
			/** An error has occured so return it.
			**/
			return ( nRet );
	}

	/** OK now get the setup index so we can assign a key.
	**/
	GetIndexToSetup ( lpszSetupName, &(nIndex) );

	/** look for a free key **/
	for ( nKey = 1; nKey < MAX_SETUP_KEYS; nKey ++ ) {

		if ( acSetupKey [ nIndex ] [ nKey ] == SETUP_KEY_FREE ) {

			/** This key is free to use so grab it.**/
			acSetupKey [ nIndex ] [ nKey ] = SETUP_KEY_INUSE;

			*lpnKey = nKey;

			return ( IMS_SUCCESS );
		}
	}

	return (IMS_TOO_MANY_SETUPS );

}


/*************************************************************/
/** Sent by an App when it no longer requires a setup
*** It simply changes the Setup Key from  INUSE to FREE.
***
*** Parameters :
***         lpszSetupName    The name of the set up
***                  nKey    The key earlier issued to the app
*** Return :
***       OK
***       IMS_BAD_KEY
***       IMS_BAD_NAME
**/

short _declspec (dllexport)  WINAPI IMSUnlockSetup ( LPSTR lpszSetupName, short nKey )

{

short	nRet;
static short nIndex;


	/** Check the setup key. We need the setup index to do this.**/

	if ( ( nRet = GetIndexToSetup ( lpszSetupName, &(nIndex) ) ) != OK ) {
				
		/** This failed because the name was not found. 
		**/
		return ( nRet );
	}

	/** Check that the key is in use.**/

	if ( acSetupKey [ nIndex ] [ nKey ] == SETUP_KEY_FREE ) {
		/** Invalid key so return.
		**/
		return ( IMS_BAD_KEY );
	}

	/** OK we can now free the key **/	
	acSetupKey [ nIndex ][ nKey ] = SETUP_KEY_FREE;

	return ( OK );
	
}


/************************************************************/
/** Sent by an App when it is prepared to allow SetupIML to edit the setup
*** ie by logger when it is stopped.
*** It simply changes the Setup Key from  INUSE to CONFIG.
***
*** Parameters :
***         lpszSetupName    The name of the set up
***                  nKey    The key earlier issued to the app
*** Return :
***       OK
***       IMS_BAD_KEY
***       IMS_BAD_NAME
***       IMS_CONFIGURING
**/


short _declspec (dllexport)  WINAPI IMSEnableConfiguring ( LPSTR lpszSetupName, short
								nKey )

{

	short	nRet;
	static short nIndex;


	if ( ( nRet = GetIndexToSetup ( lpszSetupName, &(nIndex) ) ) != OK ) {
				
		/** This failed because of a bad name **/

		return ( nRet );
	}

	/** Check that the key is in use.
	**/

	if ( acSetupKey [ nIndex ] [ nKey ] == SETUP_KEY_FREE ) {
		/** Invalid key so return.
		**/

		return ( IMS_BAD_KEY );
	}

	/** Already configuring **/
	if ( acSetupKey [ nIndex ] [ nKey ] == SETUP_KEY_CONFIG ) {
		return ( IMS_CONFIGURING );
	}

	/** Set it to config **/
	acSetupKey [ nIndex ] [ nKey ] = SETUP_KEY_CONFIG;

	return ( 0 );
}


/************************************************************/
/** Sent by an App when it is not prepared to allow SetupIML to edit the setup
*** ie by logger when it starts.
*** It simply changes the Setup Key from  CONFIG to INUSE.
***
*** Parameters :
***         lpszSetupName    The name of the set up
***                  nKey    The key earlier issued to the app
*** Return :
***       OK
***       IMS_BAD_KEY
***       IMS_BAD_NAME
***       IMS_CONFIGURING
**/




short _declspec (dllexport)  WINAPI IMSDisableConfiguring ( LPSTR lpszSetupName, short nKey )

{

short        nRet;
static short nIndex;


	if ( ( nRet = GetIndexToSetup ( lpszSetupName, &(nIndex) ) ) != OK ) {
				
		/** This failed because of a bad name
		**/
		return ( nRet );
	}

	/** Check if the setup is being configured.
	**/
	if ( acSetupKey [ nIndex ] [0] == SETUP_KEY_EDIT ) {
		/** Setup is being configured.
		**/
		return ( IMS_CONFIGURING );
	}

	/** Check that the key is in use.
	**/

	switch ( acSetupKey [ nIndex ] [ nKey ] ) {
		
		case SETUP_KEY_FREE :
			/** Invalid key so return.**/
			return ( IMS_BAD_KEY );

		case SETUP_KEY_INUSE :
			/** Inuse something wrong **/
			return ( IMS_NOT_CONFIGURING );
		
		case SETUP_KEY_CONFIG :
			/** Change to inuse **/
			acSetupKey [ nIndex ] [nKey] = SETUP_KEY_INUSE;
			return (OK);

		case SETUP_KEY_CHANGED :
			/** SetupIML has changed this set up so our
			*** local summary may be out of date. We must
			*** update it so that the connection dialog of
			*** our app can work properly.
			**/
			
			nRet = GetSummaryFromFile ( nIndex );

			if ( nRet != OK ) {
				return (nRet);
			}

			/** Tell the app that it has changed **/
			acSetupKey [ nIndex ] [nKey] = SETUP_KEY_INUSE;
			return ( IMS_SETUP_CHANGED );

	}

	return (OK);

}



/*********************************************************/
/** This routine is called by SetupIML when it has finished configuring.
*** It resets the acSetupKey [nIndex][0] flag which was at SETUP_KEY_EDIT.
*** If there are no apps using the setup it goes to SETUP_KEY_LOADED.
*** If there are apps it goes to SETUP_KEY_FREE and the apps go to
*** SET_UP_KEY_CHANGED
***
*** Parameters
***             Setup Name
***
*** Return 
***       OK
***       IMS_BAD_NAME
**/


short _declspec (dllexport)  WINAPI IMSFinishedConfiguring ( LPSTR lpszSetupName )

{

	short	nKey;
	short   nIndex;
	short   nLockCount;
	short	nRet;



	if ( ( nRet = GetIndexToSetup ( lpszSetupName, &(nIndex) ) ) != OK ) {
				
		/** No such set up **/
		return ( nRet );
	}

	/** Say we are not configuring **/
	acSetupKey [nIndex][0] = SETUP_KEY_FREE;

	if ( GetSetupState ( nIndex, &nLockCount ) == SETUP_KEY_FREE ) {
		/** Reset the SETUP_KEY_EDIT flag to SETUP_KEY_LOADED.
		*** This shows that the set up is loaded into the hardware
		*** but is not in use by an application.
		**/
		acSetupKey [nIndex] [0]= SETUP_KEY_LOADED ;
	} else {
		/** Set all the active keys to indicate the setup has changed.**/
		for ( nKey = 1; nKey < MAX_SETUP_KEYS; nKey ++ ) {
			if ( acSetupKey [ nIndex ] [ nKey ] == SETUP_KEY_FREE ) {
				continue;
			}
			acSetupKey [ nIndex ] [ nKey ] = SETUP_KEY_CHANGED;
		}
	}

	return ( OK );
}


/**************************************************/
/** This function is passed the index of a set up. 
*** It looks through the list of keys for that set up and
*** returns its current state. Possibilities are
*** SETUP_KEY_FREE    :- all keys FREE.
*** SETUP_KEY_INUSE   :- 1 or more keys INUSE
*** SETUP_KEY_CONFIG  :- All used keys set to CONFIG or CHANGED
*** SETUP_KEY_EDIT    :- Being edited by setupiml
*** SETUP_KEY_LOADED  :- Setup loaded but not in use
**/

short GetSetupState ( short nIndex, short* pnLockCount )

{

	short nKey;
	short nStatus = SETUP_KEY_FREE;

	/** If a setup key is not free then a app is using it
	*** count how many.
	**/
	*pnLockCount = 0;
	for ( nKey = 1; nKey < MAX_SETUP_KEYS; nKey ++) {
		if ( acSetupKey [nIndex] [nKey] != SETUP_KEY_FREE ) {
			++ *pnLockCount;
		}
	}

	/** The 0 key is reserved for setupiml. 
	*** It is normally SETUP_KEY_FREE.
	*** If setupiml has the file loaded then it is SETUP_KEY_EDIT
	*** If the file is loaded but not in use then SETUP_KEY_LOADED.
	**/

	/** Return the 0 key if it is not free **/
	if ( acSetupKey [nIndex] [0] != SETUP_KEY_FREE ) {
		return ( acSetupKey [nIndex][0] );
	}

	/** Scan the other keys **/
	for ( nKey = 1; nKey < MAX_SETUP_KEYS; nKey ++ ) {

		switch ( acSetupKey [ nIndex ] [ nKey ] ) {

			case SETUP_KEY_INUSE :
				/** If we find INUSE return it **/
				return ( SETUP_KEY_INUSE );

			case SETUP_KEY_CHANGED :
			case SETUP_KEY_CONFIG :
				/** either CONFIG or CHANGED produces CONFIG **/
				nStatus = SETUP_KEY_CONFIG;
				break;
	
		}			
	}
	/** This will return either FREE or CONFIG **/
	return ( nStatus );
}

/**********************************************/
/** This function scans through the array of setups and counts
*** those currently on the list ie not FREE. It returns the count.
**/

short GetSetupCount ( void )

{
	short nSetup;
	short nSetupCount = 0;
	short nLockCount;

	for ( nSetup = 0; nSetup < MAX_SETUPS; nSetup ++ ) {
		if ( GetSetupState ( nSetup, &nLockCount ) != SETUP_KEY_FREE ) {
			++nSetupCount;
		}
	}
	return ( nSetupCount );
}






/*****************************************************/
/** Gets the full file specification for the named set up **/


short _declspec (dllexport)  WINAPI IMSGetSetupFileSpec ( LPSTR lpszSetupName,
                        		LPSTR lpszBuffer, short nMaxLength )

{

short	nRet;
static short nIndex;
short   nPathLength;


	if ( ( nRet = GetIndexToSetup ( lpszSetupName, &(nIndex) ) ) != OK ) {
				
		/** This failed because either we are out of memory
		*** or the name was not found. nRet holds the reason
		*** so return it.
		**/

		return ( nRet );
	}

        /** get the file spec out of our internal summary structure.
        **/

        nPathLength = strlen ( asdSetup [ nIndex ].szFileSpec );

        /** Check the buffer is big enough to hold the file spec. If not then
        *** return buffer too small.
        **/

        if ( nMaxLength < ( nPathLength + 1 ) ) {
                return ( IMS_BUFFER_TOO_SMALL );
        }

        strcpy ( lpszBuffer, asdSetup [ nIndex ].szFileSpec );

        if ( asdSetup [ nIndex ].bSetupChanged ) {
                return ( IMS_SETUP_CHANGED );
        }

        return ( OK );
}



/********************************************************************/
/** Monitor Channels have channel numbers which are dynamically allocated.
*** The .IMS file merely provides the application with a name. This function
*** is called by an app to have a monitor channel created in the front end
*** software. The number of the created channel is entered into the PCHANNELINFO
*** structure.
***/





short _declspec (dllexport)  WINAPI IMSRequestMonitorChannel ( char *lpszSetupName,
                                                PCHANNELINFO lpciChannel )


{

	static short   nIMLHandle;
	short   nRet;
	short   bPartitionFlag = bPartitioning;

	/** Open a path through commslib **/
	nRet = OpenIMLInterface ( &nIMLHandle );
	if ( nRet != 0 ) {

		/** failed to open so return.
		**/
		return ( nRet );
	}

	/** Do the main task **/
	nRet = RequestMonitorChannel ( lpciChannel, nIMLHandle );

	CloseIMLInterface ( nIMLHandle );


	if ( nRet != 0 ) {
		/** we have an error so return. **/
        return ( nRet );
	}


	/** Now add the channel to the setup summary.
	*** Cheat by setting the partitioning flag so we can add the
	*** channel to the setup. We set it back again when we have done.
	**/

	bPartitioning = TRUE;
	nRet = IMSModifySetupSummary ( lpszSetupName, lpciChannel, IMS_ADD );
	bPartitioning = bPartitionFlag;

	return ( nRet );
}


/*******************************************************************/
/** We don't want the monitor channel anymore. Reverse the previous function
**/


short _declspec (dllexport)  WINAPI IMSReleaseMonitorChannel ( char * pszSetupName,
                                                PCHANNELINFO pciChannel )


{

	static short   nIMLHandle;
	short   nRet;
	short   bPartitionFlag = bPartitioning;


    nRet = OpenIMLInterface ( &nIMLHandle );

	if ( nRet != 0 ) {
		/** failed to open so return.**/
		return ( nRet );
	}

	/** Do the main task **/
	nRet = ReleaseMonitorChannel ( pciChannel, nIMLHandle );

	CloseIMLInterface ( nIMLHandle );


    if ( nRet != 0 ) {

		/** Something has gone wrong so return.**/
        return ( nRet );
    }


	/** OK we can remove the monitor channel from the setup now.
	*** Cheat by setting the partitioning flag so we can add the
	*** channel to the setup. We set it back again when we have done.
	**/

	bPartitioning = TRUE;
	nRet = IMSModifySetupSummary ( pszSetupName, pciChannel, IMS_REMOVE );
	bPartitioning = bPartitionFlag;

	return ( nRet );
}


/*************************************************************
*** This routine performs the security check. Passed a program title
*** it calculates the correct software code, reads that stored in 
*** IML.INI and compares them. If they are the same it returns TRUE
*** otherwise it returns FALSE.
**/

short _declspec (dllexport)  WINAPI IMSClear ( char * cProgramTitle )

{ int nSoftwareCode;

	nSoftwareCode = CalculateSoftwareCode ( cProgramTitle );

	if ( nSoftwareCode == ReadSoftwareCode ( cProgramTitle ) ) {
		return (TRUE);
	}

	return (FALSE);


}