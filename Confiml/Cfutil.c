/**	CFUTIL.C	Nigel Roberts.		27 June, 1991
***
*** IML Configuration program CONFIML.
***
*** Utility functions. E.g. Detokenise, CreateSettingsDlg
***
*** Associated files:-
***	CFMIAN.C -	main module.
***	CFDEVDLG.C -	Main device selection dialog.
***	CFSETDLG.C -	Settings dialog.
***	CFINIINF.C -	INI and INF file handling.
***	CFUTIL.C -	Assorted utilities eg detokenising.
***	CONFIML.H -	Function prototypes structure defns #defines etc.
***	CFDLG.H -	#defines for dialogs.
***	CONFIML.DLG -	Dialog scripts.
***	CONFIML.RC -	Resource file.
***	CONFIML.DEF -	Definitions file.
***	CONFIML.ICO -	Program icon file.
***	CONFIML.MAK -	Make file.
***
***
*** Version History.
***
*** P1.00 - 27 June, 1991
***	Conception.
***
**/







#define NOCOMM		/* To avoid bit packed fields producing warnings */

#include <windows.h>

#include <string.h>
#include <stdio.h>
#include <memory.h>


#include "CONFIML.H"




static short GetFreeModule ( short nDev, short * pnMod );







extern GENERAL_INFO giData;	/* our general information structure. */










char *	Detokenise ( char * lpszWithTokens, char *szTokensReplaced, short
								nMod )

{

char *  szTok;
char    szMod [ 10 ];


        /** Scan the string for tokens, If we find one then we replace it
        *** with the appropriate string in the replaced string.
        *** We scan the with tokens string looking for $(MOD). We copy upto
        *** this into the replaced string then insert the appropriate string.
        *** NOTE. We must not change the with tokens string.
        **/


        /** ||||||||||||||| NOTE |||||||||||||||||||||||||||||||||||
        *** This next bit of code relies on the token we are replacing being
        *** longer than what we are replacing it with. If this is no the
        *** case we bugger things up as we overwrite the bit of string just
        *** after the token as we are doing it in place.
        **/

        szTokensReplaced [ 0 ] = '\0';

        strcpy ( szTokensReplaced, lpszWithTokens );

        sprintf ( szMod, "%d", nMod );

        while ( ( szTok = strstr ( szTokensReplaced, "$(" ) ) != NULL ) {

                /** Ok terminate the copy at the start of the token then
                *** add the mod string. Shift szTok to point to the next bit
                *** of the string then add this to the replaced string.
                **/

                *szTok = '\0';

                strcat ( szTokensReplaced, szMod );
                szTok += strlen ( "$(MOD)" );
                strcat ( szTokensReplaced, szTok );
        }

	return ( szTokensReplaced );
}









/** Remove any white space or commas from the front of the string.
**/

char *  Despace ( char * szStr )

{

        while (( *szStr == ' ' ) || ( *szStr == '\t' ) || ( *szStr == ',' )) {
                szStr ++;
        }

        return ( szStr );
}





short   AddModuleToHWDevice ( PHWDEV phwDev, PUSER_DEV pudDev )

{

PUSER_DEV       paudDev;


        /** If we don't already have a handle then allocate some memory for
        *** the array of structures.
        **/

        if ( phwDev->hamodModule == NULL ) {

             	phwDev->hamodModule = LocalAlloc ( LMEM_MOVEABLE |
                        		LMEM_ZEROINIT, sizeof ( USER_DEV ) );

                if ( phwDev->hamodModule == NULL ) {
                        HandleError ( NO_MEMORY );
                        return ( FALSE );
                }
        }


        /** Ensure that the memory block is big enough.
        **/

        phwDev->hamodModule = LocalReAlloc ( phwDev->hamodModule, (
                        sizeof ( USER_DEV ) * ( phwDev->nModules + 1 ) ),
                                         		LMEM_MOVEABLE );


        if ( phwDev->hamodModule == NULL ) {

                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        paudDev = (PUSER_DEV) LocalLock ( phwDev->hamodModule );

        if ( paudDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }


        CopyUserDevice ( (PUSER_DEV) &(paudDev [phwDev->nModules]), pudDev );

        /** Free the memory used for the questions if any.
        **/

        if ( pudDev->haqnQn != NULL ) {
                GlobalFree ( pudDev->haqnQn );
                pudDev->haqnQn = NULL;
        }



        /**** ||||||||||| STRUCTURE ASSIGNMENT !!!!!!!
        *** paudDev [ phwDev->nModules ] = *pudDev;
        **/

        LocalUnlock ( phwDev->hamodModule );

        phwDev->nModules ++;

        return ( TRUE );

}




/***************************************************************/
/** The details of a question have been read from IML.INI.
*** Store them and increment the number of questions for this device.
**/ 


short   AddQuestionToUserDevice ( PUSER_DEV pudDev, LPQUEST lpquQuest)

{

LPQUEST       lpaqnQn;


        /** If we don't already have a handle then allocate some memory for
        *** the array of structures.
        **/

        if ( pudDev->haqnQn == NULL ) {


             pudDev->haqnQn = GlobalAlloc ( GMEM_MOVEABLE | GMEM_ZEROINIT,
                                              	(DWORD) sizeof ( QUEST ) );

                if ( pudDev->haqnQn == NULL ) {

                        HandleError ( NO_MEMORY );
                        return ( FALSE );
                }
        }


        /** Ensure that the memory block is big enough.
        **/


        pudDev->haqnQn = GlobalReAlloc ( pudDev->haqnQn, (DWORD) ( sizeof
        		( QUEST ) * ( pudDev->nQns + 1 ) ), GMEM_MOVEABLE );


        if ( pudDev->haqnQn == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        lpaqnQn = (LPQUEST) GlobalLock ( pudDev->haqnQn );

        if ( lpaqnQn == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /**** ||||||||||| STRUCTURE ASSIGNMENT !!!!!!! ***/
        lpaqnQn [ pudDev->nQns ] = *lpquQuest;

        GlobalUnlock ( pudDev->haqnQn );

        pudDev->nQns ++;

		

        return ( TRUE );

}




short   AddHardwareToList ( PHWDEV phwDev )

{

PHWDEV       pahwDev;


        /** If we don't already have a handle then allocate some memory for
        *** the array of structures.
        **/

        if ( giData.hahwHardware == NULL ) {
             	giData.hahwHardware = LocalAlloc ( LMEM_MOVEABLE |
                                        LMEM_ZEROINIT, sizeof ( HWDEV ) );

                if ( giData.hahwHardware == NULL ) {
                        HandleError ( NO_MEMORY );
                        return ( FALSE );
                }
        }


        /** Ensure that the memory block is big enough.
        **/

        giData.hahwHardware = LocalReAlloc ( giData.hahwHardware, (
                        sizeof ( HWDEV ) * ( giData.nHWDevices + 1 ) ),
                                         		LMEM_MOVEABLE );


        if ( giData.hahwHardware == NULL ) {

                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        pahwDev = (PHWDEV) LocalLock ( giData.hahwHardware );

        if ( pahwDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /**** ||||||||||| STRUCTURE ASSIGNMENT !!!!!!! ***/
        pahwDev [ giData.nHWDevices ] = *phwDev;

        LocalUnlock ( giData.hahwHardware );

        giData.nHWDevices ++;

        return ( TRUE );

}





short   IsNewHardware ( PHWDEV phwDev )

{

PHWDEV       	pahwDev;
short           nHW;

        if ( giData.nHWDevices == 0 ) {
                /** We have no HW Devs so this must be new HW so return TRUE.
                **/
                return ( TRUE );
        }


        pahwDev = (PHWDEV) LocalLock ( giData.hahwHardware );

        if ( pahwDev == NULL ) {

                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /** Look through the list to see if we have the same name. If we
        *** have then unlock and return false.
        **/

        for ( nHW = 0; nHW < giData.nHWDevices; nHW ++ ) {
                if ( stricmp ( pahwDev [nHW].szName, phwDev->szName )==0 ){

                        /** We have a match so return false.
                        **/
                        LocalUnlock ( giData.hahwHardware );
                        return ( FALSE );
                }
        }


        LocalUnlock ( giData.hahwHardware );

        /** No match was found so return TRUE.
        **/
        return  ( TRUE );
}





short   AddUserDeviceToList ( PUSER_DEV pudDev )

{

PUSER_DEV       paudDev;


        /** If we don't already have a handle then allocate some memory for
        *** the array of structures.
        **/

        if ( giData.haudUserDev == NULL ) {
             	giData.haudUserDev = LocalAlloc ( LMEM_MOVEABLE |
                                        LMEM_ZEROINIT, sizeof ( USER_DEV ) );

                if ( giData.haudUserDev == NULL ) {
                        HandleError ( NO_MEMORY );
                        return ( FALSE );
                }
        }


        /** Ensure that the memory block is big enough.
        **/

        giData.haudUserDev = LocalReAlloc ( giData.haudUserDev, (
                        sizeof ( USER_DEV ) * ( giData.nUserDevices + 1 ) ),
                                         		LMEM_MOVEABLE );


        if ( giData.haudUserDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev );

        if ( paudDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /**** ||||||||||| STRUCTURE ASSIGNMENT !!!!!!! ***/
        paudDev [ giData.nUserDevices ] = *pudDev;

        LocalUnlock ( giData.haudUserDev );

        giData.nUserDevices ++;

        return ( TRUE );

}





short   RemoveUserDeviceFromList ( short nDev )

{

PUSER_DEV       paudDev;
short           nHold;
short           nMod;



        if ( giData.haudUserDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev );

        if ( paudDev == NULL ) {
                return ( FALSE );
        }

        /** Free up any questions associated with the device we are removing.
        *** Also free the module and possibly the iml device.
        **/

        if ( paudDev [ nDev ].haqnQn != NULL ) {
                GlobalFree ( paudDev [ nDev ].haqnQn );
                paudDev [ nDev ].haqnQn = NULL;
        }

        giData.aimlDev[ paudDev[nDev].nDeviceNo ].acModule[
                                      paudDev[nDev].nModuleNo ] = MOD_FREE;

        /** If all the modules are free for this device then free the device.
        **/

        for ( nMod = 0; nMod < MAX_MODULES; nMod ++ ) {
                if (  giData.aimlDev[ paudDev[nDev].nDeviceNo ].acModule[
                                                  nMod ] != MOD_FREE ) {

                        break;
                }
        }

        if ( nMod == MAX_MODULES ) {
                /** All the modules were free so clear the iml device.
                **/
                giData.aimlDev [ paudDev[nDev].nDeviceNo ].szExeName[0]='\0';
                giData.aimlDev [ paudDev[nDev].nDeviceNo ].szHWName[0]='\0';
                giData.aimlDev [ paudDev[nDev].nDeviceNo ].nType = DEV_FREE;
        }


        /** Copy the devices passed the one we wish to delete down one.
        **/

        giData.nUserDevices --;

        for ( nHold = nDev; nHold < giData.nUserDevices; nHold ++ ) {

                /**** ||||||||||| STRUCTURE ASSIGNMENT !!!!!!! ***/
                paudDev [ nHold  ] = paudDev [ nHold + 1 ];
        }

        LocalUnlock ( giData.haudUserDev );

        if ( giData.nUserDevices == 0 ) {
                /** Don't bother resizing the block - just return.
                **/
                return ( TRUE );
        }

        /** resize the the memory block.
        **/

        giData.haudUserDev = LocalReAlloc ( giData.haudUserDev, ( sizeof (
                     USER_DEV ) * ( giData.nUserDevices ) ), LMEM_MOVEABLE );


        if ( giData.haudUserDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        return ( TRUE );

}








short   GetHWStructForUserDev ( PUSER_DEV pudDev, PHWDEV phwDev )

{

PHWDEV          pahwDev;
short   nHW;



        /** Scan the hardware list to find the module specified in the
        *** user dev struct. When we find it copy the HW struct to phwDev
        *** and return TRUE. If we don't find it the n return false.
        **/

        pahwDev = (PHWDEV) LocalLock ( giData.hahwHardware );
        if ( pahwDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /** Now step through the hw list looking at each module in the
        *** HH struct.
        **/

        for ( nHW = 0; nHW < giData.nHWDevices; nHW ++ ) {


		/** Is this the one we are looking for ?
                **/

                if ( stricmp ( pudDev->szHWName, pahwDev[nHW].szName) == 0 ) {

                        /** This is the one so copy the HW, unlock
                        *** and return true.
                        **/

                        /**** |||||| Structure assignment |||||**/
                        *phwDev = pahwDev [ nHW ];

                        LocalUnlock ( giData.hahwHardware );

                        return ( TRUE );
                }

        }

        /** We have not found it so unlock and return false.
        **/

        LocalUnlock ( giData.hahwHardware );

        return ( FALSE );
}








short CopyUserDevice ( PUSER_DEV pudTo, PUSER_DEV pudFrom )

{

short           nQn;
LPQUEST         lpaqnQn;


        /*** |||||||| Structure assignment |||||| ***/
        *pudTo = *pudFrom;
        pudTo->haqnQn = NULL;
        pudTo->nQns = 0;

        if ( pudFrom->nQns > 0 ) {
                /** get the questions and add them to our copy device.
                **/


                lpaqnQn = (LPQUEST) GlobalLock ( pudFrom->haqnQn );

                if ( lpaqnQn == NULL ) {

                        HandleError ( NO_MEMORY );
                        return ( FALSE );
                }

                for ( nQn = 0; nQn < pudFrom->nQns; nQn ++ ) {

                        if ( ! AddQuestionToUserDevice ( pudTo,
                                                 &(lpaqnQn [ nQn ]) ) ) {

                                GlobalUnlock ( pudFrom->haqnQn );
                                return ( FALSE );
                        }
                }

                GlobalUnlock ( pudFrom->haqnQn );
        }

        return ( TRUE );
}









short   GetDeviceAndModuleNumbers ( PUSER_DEV pudDev )

{

short   nDev;
short   nMod;



        /** search for the device and module numbers. If the device type is
        *** 330x then we need a free device for this. If it is a winapp then
        *** we first look at the exenames to see if we already have a
        *** device capable of handling the user dev. if we have we need to
        *** have a free module. If we don't then we need a new device.
        **/

        switch ( pudDev->nType ) {

        case DEV_330X :

                /** OK we need to find a free device.
                **/

                for ( nDev = 0; nDev < MAX_DEVICES; nDev ++ ) {
                        if ( giData.aimlDev [ nDev ].nType != DEV_FREE ) {
                                /** This one is in use so continue.
                                **/
                                continue;
                        }

                        /** OK we can use this one.
                        **/
                        pudDev->nDeviceNo = nDev;
                        pudDev->nModuleNo = 0;

                        strcpy ( giData.aimlDev [ nDev ].szExeName, pudDev->
                                                               szExeName );

                        strcpy ( giData.aimlDev [ nDev ].szHWName, pudDev->
                                                               szHWName );

                        giData.aimlDev [ nDev ].nType = DEV_330X;
                        giData.aimlDev [ nDev ].acModule [ 0 ] = MOD_INUSE;

                        return ( TRUE );
                }
                return ( FALSE );


        case DEV_WINAPP :
        case DEV_X_WINAPP :

                /** OK Scan the iml devs to find a match for the exename.
                **/

                /** Use the HW name not the exename.
                ***
                *** If the Unit is device then we need a new device - no
                *** need to scan through the devices we already have.
                **/


               	for ( nDev = 0; nDev < MAX_DEVICES; nDev ++ ) {

                       	if ( ( giData.aimlDev [ nDev ].nType != DEV_WINAPP )
                                && ( giData.aimlDev [ nDev ].nType !=
                        				DEV_X_WINAPP ) ) {

                               	/** This one is not a winapp so continue.
                               	**/
                       	 	continue;
                       	}

                       	if ( ( stricmp ( giData.aimlDev [ nDev ].szHWName,
                                		pudDev->szHWName ) ) != 0 ) {

                               	/** This one is not a match so continue.
                               	**/
                               	continue;
                       	}

	              	/** OK This one matches so see if there is a
                        *** free module.
                       	**/

                       	if ( ! GetFreeModule ( nDev, &nMod ) ) {
                               	/** There is no free module in this
                                *** device so break.
                               	**/
                               	break;
                       	}

	                /** OK we can use this device and module.
                        **/

                	pudDev->nDeviceNo = nDev;
                        pudDev->nModuleNo = nMod;
                        giData.aimlDev [ nDev ].acModule [ nMod ] =
                                				MOD_INUSE;

                        return ( TRUE );

                }


                /** OK we need to find a free device.
                **/

                for ( nDev = 0; nDev < MAX_DEVICES; nDev ++ ) {
                        if ( giData.aimlDev [ nDev ].nType != DEV_FREE ) {
                                /** This one is in use so continue.
                                **/
                                continue;
                        }

                        /** OK we can use this one.
                        **/
                        pudDev->nDeviceNo = nDev;
                        pudDev->nModuleNo = 0;

                        strcpy ( giData.aimlDev [ nDev ].szExeName, pudDev->
                                                               szExeName );

                        strcpy ( giData.aimlDev [ nDev ].szHWName, pudDev->
                                                               szHWName );

                        giData.aimlDev [ nDev ].nType = pudDev->nType;
                        giData.aimlDev [ nDev ].acModule [ 0 ] = MOD_INUSE;

                        return ( TRUE );
                }
        }


        return ( FALSE );
}










static short GetFreeModule ( short nDev, short * pnMod )

{

short nMod;

        for ( nMod = 0; nMod < MAX_MODULES; nMod ++ ) {

                if ( giData.aimlDev [ nDev ].acModule [ nMod ] == MOD_FREE ) {
                        /** This one is free so grab it.
                        **/
                        giData.aimlDev [ nDev ].acModule [ nMod ] = MOD_INUSE;
                        *pnMod = nMod;
                        return ( TRUE );
                }
        }

        return ( FALSE );
}
