/** IMSMON.C    Monitor  handleing for IMSLib.
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


#define COMMSLIB

#include "\Windmill 32\Compile Libs\imlinter.h"

#include "imsinc.h"










/************************************************************************/
/**
typedef struct MonDevTag {
	short   nFirstModule;                       
	long    lModuleMap;
	long    lChannelMap [ MAX_MONITOR_MODS ];
} MON_DEV;

***  The management of this data structure is the chief task of this file
***  We maintain one such structure for each IML device.
***  The nFirstModule field is the number of the first module which we
***  can allocate as a monitor module. All physical modules have lower numbers.
***  The lModuleMap parameter provides a bit map of monitor modules which we
***  have created.
***  eg if lModule map bit 2 is set then a monitor module exists with a number
***  of  nFirstModule + 2.
***  The lChannelMap parameter provides a bit map of allocated channels for a
***  monitor module. eg if bit 5 is set then channel 5 of the module is in use.  
***
***  A utility CountMapBits () works out how many modules or channels have been
***  allocated.
**/



extern MON_DEV amDev [ MAX_DEVICES ];      /** One structure for each device **/





static short CountMapBits ( long lMap );
static short FindFirstMonitor ( short nDev, short nIMLHandle );
static short GetMonitorChannel ( short nDev, short * pnMod, short * pnChan, short nIMLHandle );

static short CreateMonitor ( short nDev, short nMod, short nIMLHandle );
static short DestroyMonitor ( short nDev, short nMod, short nIMLHandle );

static short DisableChannel ( short nDev, short nChannel, short nIMLHandle );


/******************************************************/
/** This counts how many bits are set in lMap and returns the count **/

short CountMapBits ( long lMap )

{
	short nBitCount = 0;
	short nIndex;

	for ( nIndex = 0; nIndex < 32; ++nIndex ) {
		if ( lMap & 1 ) {
			++nBitCount;
		}
		lMap = lMap >> 1;
	}

	return ( nBitCount );
}


/***********************************************************************/
/** We don't want this monitor channel anymore.
*** Mark it as not used
*** Decrease the number of channels in the module.
*** If there are none left destroy the module.
*** Send the IML messages to do these things.
**/

short   ReleaseMonitorChannel ( PCHANNELINFO lpchMon, short nIMLHandle )

{

	short	nChan;
	short	nMod;
	short	nDev;
	long    lTemplate;


	/** mark the channel as now free and send E=0 to turn it off.
	*** If this is the last channel in the module then kill the
	*** module.
	**/


	nDev = lpchMon->nFrame;                /** The device number **/
	nMod = lpchMon->nChannel / 100;        /** The physical module number **/
	nChan = lpchMon->nChannel % 100;       /** The channel number **/

	nMod -= amDev [ nDev ].nFirstModule;   /** The array index module number **/


	/** Set a template to the correct channel bit **/
	lTemplate = 1 << nChan;
	/** Cancel the channel from the map **/
	amDev [ nDev ].lChannelMap [ nMod ] = 
	          amDev [ nDev ].lChannelMap [ nMod ] - lTemplate	;  


	/** Disable the physical channel by sending IML  DEF E=0 and flush name N=. **/
	DisableChannel ( nDev, lpchMon->nChannel, nIMLHandle );

	/** If there are no channels left in the module kill it **/
	if ( CountMapBits ( amDev [nDev].lChannelMap [nMod]) == 0 ) {

		/** Send the IML command to kill it **/
		DestroyMonitor ( nDev, (short)( nMod + amDev [ nDev ].nFirstModule), nIMLHandle );

		/** Set a Template to the correct module bit **/
		lTemplate = 1 << nMod;

		/** Module not created **/
		amDev [ nDev ].lModuleMap = amDev[ nDev ].lModuleMap - lTemplate;
	}

	return ( 0 );
}


/*************************************************************/
/** Create a monitor channel.
*** The CHANNELINFO structure specifies the device to which it belongs.
**/

short   RequestMonitorChannel ( PCHANNELINFO lpchMon, short nIMLHandle )

{

	static short   nMod;
	static short   nChan;
	short          nRet;
	short          nDev;



	/** The specified device.**/
	nDev = lpchMon->nFrame;


	/** If the first module field is -1 then there are no monitor modules
	*** on this device. ( for any setup )
	**/

	if ( amDev [ nDev ].nFirstModule == -1 ) {

		/** We need to find the first module we can use as a monitor
		*** Fills in the nFirstModule field of the structure.
		**/

		nRet = FindFirstMonitor ( nDev, nIMLHandle );

		if ( nRet != 0 ) {

			/** Something has gone wrong so return.**/
			return ( nRet );
		}
	}


	/** Now find a free monitor channel - this may create a new monitor
	*** module if necessary.
	**/
	nRet = GetMonitorChannel ( nDev, &nMod, &nChan, nIMLHandle );

	if ( nRet != 0 ) {

		/** Something has gone wrong so return.**/
		return ( nRet );
	}


	/** Now work out the actual IML channel number for the monitor
	*** channel. This is Chan + ( 100 * ( nMod + FirstMod )).
	**/

	lpchMon->nChannel = nChan + ( 100 * ( nMod + amDev [ nDev ].nFirstModule ) );


	return ( 0 );
}
/*******************************************************************/
/** Do a DHW command on the device to find what hardware is present.
*** If we find any monitors then kill them. Remember the last module
*** number and add one - this is the first number we can use for
*** monitor modules we create. This routine is only used when the first monitor
*** is created by any setup.
**/


static short FindFirstMonitor ( short nDev, short nIMLHandle )

{

	short         nRet;
	static char   szReply [ 512 ];
	char *        pszStr;
	short         nMod;
	short         nLastModule;


	/** Send DHW command **/
	nRet = SendIMLCommand ( nIMLHandle, nDev, "DHW", IML_TEXT_REPLY );

	if ( nRet != 0 ) {
		return ( nRet );
	}


	/** Now get the reply.**/
	nRet = GetReply ( nIMLHandle, nDev, szReply, sizeof ( szReply ) );

	if ( nRet != 0 ) {
		return ( nRet );
	}


	/** Now parse the reply. If we find any monitors then kill them.
	*** The reply is of the form 'modno modname, n, nChans' repeated
	*** for each module. We are interested in the module numbers and
	*** names - if Monitor.
	**/

	nLastModule = 0; /** Initialise the nLastModule **/

	pszStr = strtok ( szReply, " " ); /** Find the string to the first space **/

	nMod = atoi ( pszStr );   /** Get the module number **/

	pszStr = strtok ( NULL, " " );  /** The next string **/

	/** If its a monitor module kill it. **/
	if ( ( strnicmp ( pszStr, "Monitor", 7 ) ) == 0 ) {

		DestroyMonitor ( nDev, nMod, nIMLHandle );

	} else {
		/** This becomes the last module **/
		nLastModule = nMod;
	}

	pszStr = strtok ( NULL, " " );
	pszStr = strtok ( NULL, " " );


	while ( pszStr != NULL ) {

		pszStr = strtok ( NULL, " " );  /** This one gives NULL **/

		if ( pszStr == NULL ) {
			/** Check if we have reached the end of the list
			*** of modules. Ensure we don't end up with a mod
			*** of zero by atoi on NULL.
			**/

			break;
		}

		nMod = atoi ( pszStr );            /** Get the module number **/
		pszStr = strtok ( NULL, " " );

		/** If its a monitor destroy it **/
		if ( ( strnicmp ( pszStr, "Monitor", 7 ) ) == 0 ) {

			DestroyMonitor ( nDev, nMod, nIMLHandle );
		} else {

			/** Store the last module number **/
			nLastModule = nMod;

		}
		/** Step over the unwanted fields **/
		pszStr = strtok ( NULL, " " ); 
		pszStr = strtok ( NULL, " " );

	}


	/** We want the first free one. It must be one more than the last**/

	amDev [ nDev ].nFirstModule = nLastModule + 1;

	return ( 0 );
}






static short GetMonitorChannel ( short nDev, short * pnMod, short * pnChan,
							short nIMLHandle )

{

	short   nMod;
	short   nChan;
	short   nRet;
	short   nChannelCount;
	long    lTemplate;


	/** Scan our table of monitor channels for a free one. If we find
	*** one then set the mod and chan and return. If we don't then we
	*** need to create a new monitor module.
	**/



	/** Do we have a free channel in an existing module. If so use it.**/

	for ( nMod = 0; nMod < MAX_MONITOR_MODS; nMod ++ ) {

		/** How many channels used in this module **/
		nChannelCount = CountMapBits ( amDev [ nDev ].lChannelMap [nMod] );

		/** Check if monitor exists. if not continue to next one.**/
		if ( nChannelCount == 0 ) {
			continue;
		}

		/** Check if the monitor is fully used. if so skip to next **/
		if ( nChannelCount == MAX_MONITOR_CHANS ) {
			continue;
		}



		/** OK look for a free channel.**/
		lTemplate = 1;
		for ( nChan = 0; nChan < MAX_MONITOR_CHANS; nChan ++ ) {

			/** If map bit is not set the channel is free **/
			if ( !(amDev [nDev].lChannelMap [nMod] & lTemplate) ) {

				/** This one is free so set it busy and return; **/
				amDev [nDev].lChannelMap [nMod ] = 
					                 amDev [nDev].lChannelMap [nMod ] + lTemplate;

				*pnMod = nMod;
				*pnChan = nChan;

				return ( 0 );
			}
			/** Move the template to the next bit **/
			lTemplate = lTemplate << 1;
		}

	}



	/** OK no free monitor channel exists so we need to create another.
	*** Check we can create another.
	**/

	if ( CountMapBits ( amDev [ nDev ].lModuleMap ) >= MAX_MONITOR_MODS ) {

		return ( IMS_TOO_MANY_MONITORS );
	}


	/** Find the first free module.**/
	nMod = 0;
	lTemplate = 1;
	/** Look for the first 0 bit in the module map **/
	while (  amDev [ nDev ].lModuleMap & lTemplate ) {
		nMod ++;
		lTemplate = lTemplate << 1;
	}


	nRet = CreateMonitor ( nDev, (short)(nMod + amDev [ nDev ].nFirstModule), nIMLHandle );

	if ( nRet != 0 ) {
		return ( nRet );
	}

	/** Mark the module and channel as busy **/
	amDev [ nDev ].lModuleMap = amDev [ nDev ].lModuleMap + lTemplate;
	amDev [ nDev ].lChannelMap [nMod] = 1;


	/** Return the values **/
	*pnChan = 0;
	*pnMod = nMod;


	return ( 0 );
}


/********************************************************/
/** Sends the IML command to create a new monitor module
*** Parameters
***         nDev         The IML device
***         nMod         The module to create
***         nIMLHandle   The Commslib handle
**/


static short CreateMonitor ( short nDev, short nMod, short nIMLHandle )

{

	static char szCmd [ 40 ];
	static char szNum [ 8 ];


	/** do a CHW to add a new monitor to the device.
	**/

	strcpy ( szCmd, "CHW " );
	strcat ( szCmd, itoa ( nMod, szNum, 10 ) );
	strcat ( szCmd, " Monitor" );

	return ( SendIMLCommand ( nIMLHandle, nDev, szCmd, IML_NO_REPLY ) );
}


/******************************************************/
/** Sends the IML command to destroy a monitor module **/

static short DestroyMonitor ( short nDev, short nMod, short nIMLHandle )

{

	static char szCmd [ 40 ];
	static char szNum [ 8 ];


	/** do a CHW to remove the monitor module. **/

	strcpy ( szCmd, "CHW " );
	strcat ( szCmd, itoa ( nMod, szNum, 10 ) );
	strcat ( szCmd, " NONE" );

	return ( SendIMLCommand ( nIMLHandle, nDev, szCmd, IML_NO_REPLY ) );
}




/****************************************************/
/** Send the IML DEFINE command to disable a channel **/


static short DisableChannel ( short nDev, short nChannel, short nIMLHandle )

{


	static char szCmd [ 30 ];
	static char szNum [ 10 ];



	strcpy ( szCmd, "DEF " );



	if ( nDev > 9 ) {

		szNum [ 0 ] = (char) ( '0' + ( nDev / 10 ) );

		szNum [ 1 ] = (char) ( '0' + ( nDev % 10 ) );

		szNum [ 2 ] = (char) ( '0' + ( nChannel / 1000 ) );
		szNum [ 3 ] = (char) ( '0' + ( nChannel / 100 ) % 10 );
		szNum [ 4 ] = (char) ( '0' + ( nChannel / 10 ) % 10 );
		szNum [ 5 ] = (char) ( '0' + ( nChannel % 10 ) );
		szNum [ 6 ] = '\0';

	} else {

		szNum [ 0 ] = (char) ( '0' + ( nDev ) );

		szNum [ 1 ] = (char) ( '0' + ( nChannel / 1000 ) );
		szNum [ 2 ] = (char) ( '0' + ( nChannel / 100 ) % 10 );
		szNum [ 3 ] = (char) ( '0' + ( nChannel / 10 ) % 10 );
		szNum [ 4 ] = (char) ( '0' + ( nChannel % 10 ) );
		szNum [ 5 ] = '\0';
	}

	strcat ( szCmd, szNum );
	strcat ( szCmd, " E=0 N=" );

	return ( SendIMLCommand ( nIMLHandle, nDev, szCmd, IML_NO_REPLY ) );

}
