/**	CFINIINF.C	Nigel Roberts.		27 June, 1991
***
*** IML Configuration program CONFIML.
***
*** IML.INI and setup.inf file handling.
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
**/







#define NOCOMM		/* To avoid bit packed fields producing warnings */

#include <windows.h>

#include <string.h>
#include <stdio.h>

#include <stdlib.h>



#include "confiml.h"
#include "Copycomm.h"


extern char cImlini [];



extern GENERAL_INFO giData;	/* our general information structure. */



/** Local prototypes.
**/

static short GetHardwareFromFile ( char * szFile, short nHWNo, PHWDEV phwDev);
static short ReadIMLINIHardware ( void );
static short AppendSetupHardware ( void );
static short GetWinappModules ( char * szFile, PHWDEV phwDev );
static short Get330xDetails ( char * szFile, PHWDEV phwDev );
static short GetQuestionForUserDevice ( char * szFile, PUSER_DEV pudDev,
                                                LPQUEST pqnQn, char * szApp );

static short AddHardwareToIMLINI ( PHWDEV phwDev, short bNewHardware );
static short Add330XToIMLINI ( PHWDEV phwDev );
static short AddWinappModulesToIMLINI ( PHWDEV phwDev );
static short WriteQuestionsToIMLINI ( PUSER_DEV pudDev, char * szApp );
static short GetIMLINIWinappModules ( short nDev, short nType );
static short GetIMLINI330XDetails ( short nDev );
static short GetAnswerToQuestion ( PUSER_DEV pudDev, LPQUEST pqnQn );




/** The IML.INI file contains a list of all currently installed devices
*** This is at the end of the file. This routine prepares a list of all
*** these devices and fills in the information about them
**/ 



short	ReadIMLDevices ( void )

{

short   nDev;			/** The IML device number **/
char    szApp [ 20 ];	
char    szRet [ 120 ];
char    *szExe;


        /** Read the IML.INI file to find all the devices currently installed
        *** for each Device N find each module and create a user dev struct
        **/

        for ( nDev = 0; nDev < MAX_DEVICES; nDev ++ ) {

                /** Set imldev array element type to not known, modules to
                *** zero and set exename.
                **/
                giData.aimlDev [ nDev ].nType = DEV_FREE;
                giData.aimlDev [ nDev ].szExeName [ 0 ] = '\0';
                giData.aimlDev [ nDev ].szHWName [ 0 ] = '\0';

                /** Now look for the type for this device number. 
				*** If we don't find it then continue
                *** with the next device number. If we do then use it to
                *** determine what further details we need.
                **/

                sprintf ( szApp, "Device%d", nDev );

                if ( GetPrivateProfileString ( (LPSTR) szApp, (LPSTR) "Type",
                         (LPSTR) "", (LPSTR) szRet, 119, cImlini ) == 0 ) {

                        /** This device not present so continue with next.
                        **/
                        continue;
                }

                /** OK, determine what type this is. If it is a winapp then
                *** we need to get details for ea. module. i.e. module, qns
                *** and current answers. If it is a 330X module we just need
                *** the questions and current answers.
                **/

                if ( ( stricmp ( Despace ( szRet ), "WINAPP" ) ) == 0 ) {
                        /** This is a winapp so we need to get the modules,
                        *** Questions and answers. First get the exename.
                        **/


					giData.aimlDev [ nDev ].nType = DEV_WINAPP;

                }


               	if ( ( stricmp ( Despace ( szRet ), "X_WINAPP" ) ) == 0 ) {
                        /** This is a winapp so we need to get the modules,
                        *** Questions and answers. First get the exename.
                        **/


					giData.aimlDev [ nDev ].nType = DEV_X_WINAPP;

                }




                if ( ( stricmp ( Despace ( szRet ), "3301" ) == 0 ) ||
                        ( stricmp ( Despace ( szRet ), "3302" ) == 0 ) ||
                            ( stricmp ( Despace ( szRet ), "3303" ) == 0 ) ||
                            ( stricmp ( Despace ( szRet ), "1580" ) == 0 ) ) {

                        /** This is a 3301/2/3 or 1580 so get the details we
                        *** require.
                        **/
                        giData.aimlDev [ nDev ].nType = DEV_330X;
                }

				
				/** Collect the required data depending on the device type **/
                switch ( giData.aimlDev [ nDev ].nType ) {

					case DEV_WINAPP:
					case DEV_X_WINAPP:

                        if ( GetPrivateProfileString ( (LPSTR) szApp, (LPSTR)
                                    "Exe", (LPSTR) "", (LPSTR) szRet,
                                                 119, cImlini ) == 0 ) {

                                return ( FALSE );
                        }

                        /** We need to split the path off the front of the
                        *** exename. So look for '\'.
                        **/

                        if ( ( szExe = strrchr ( szRet, '\\' ) ) == NULL ) {
                                /** no path so just copy the exename.
                                **/
                                strcpy ( giData.aimlDev [nDev].szExeName,
                                                        Despace ( szRet ) );

                        } else {
                                /** copy from the next char after the \.
                                **/
                                szExe ++;
                                strcpy ( giData.aimlDev [nDev].szExeName,
                                                                    szExe );
                        }



                        /** Get the HW name for this device so we can find
                        *** the HW to use for the settings.
                        **/

                        if ( GetPrivateProfileString ( szApp, "Settings",
                                	"", szRet, 119, cImlini ) == 0 ) {

                                return ( FALSE );
                        }


                        strcpy ( giData.aimlDev [nDev].szHWName,
                        				Despace ( szRet ) );




                        if ( ! GetIMLINIWinappModules ( nDev,
                                	giData.aimlDev [ nDev ].nType ) ) {

                                return ( FALSE );
                        }
                        break;



                case DEV_330X :


                /** See If the type is a 3301, 2, 3 or 1580. If it is we need to
                *** get the questions and answers.
                **/

                        /** store the actual type in the exename field so
                        *** we know what it is.
                        **/
                        strcpy ( giData.aimlDev [nDev].szExeName, Despace (
                                                                 szRet ) );

                        strcpy ( giData.aimlDev [nDev].szHWName, Despace (
                                                                 szRet ) );

                        if ( ! GetIMLINI330XDetails ( nDev ) ) {
                                return ( FALSE );
                        }

                        giData.aimlDev [ nDev ].acModule [ 0 ] = MOD_INUSE;

                        break;
                }
        }

	return ( TRUE );
}




/** This routine is called at start up to get data from the IML.INI file
*** It first reads all potential hardware types from the INI file and stores
*** them in the local list.
*** If the program has been started with a command line argument specifying a
*** .INF file then this file will also be read and its contents will be appended
*** both to the local list and to the INI file. In this way new hardware is added
*** to existing INI files.
**/



short	GetHardWareDetails ( LPSTR lpszCommandLine )

{

	char *  szText = "         The ConfIML program will now be started. \n\n Use     'Add'       to specify the hardware devices that you wish to use.\n\n Use  'Directory'   to specify a default working directory for your data files.";



	/** Read the potential Hardware Types from the IML.INI file.
	**/

	if ( ! ReadIMLINIHardware () ) {
		/** Something has gone wrong so fail now.
		**/
		return ( FALSE );
	}

	/** Check the command line we have been passed to see if it contains
	*** the path of the setup file. If it does we must read the info it
	*** contains and add it to IML.INI. If it does not then we just
	*** return TRUE.
	**/

	if ( strlen ( lpszCommandLine ) == 0 ) {
		
		/** We have no command line. If we also have no devices then
		*** Confiml cannot do anything. Probably the INI file is missing.
		*** Otherwise just return true.
		**/		
		if ( giData.nHWDevices == 0) {
			return ( FALSE);
		} else {
			return ( TRUE );
		}
	}

	/** The only command line arg we allow is Install **/
	if ( strcmp ( lpszCommandLine, "Install") != 0 ) {
		return ( FALSE );
	}

	MessageBox ( NULL, szText, "Windmill Device Configuration", MB_OK );
	
	return ( TRUE );


}





short	WriteIMLINI ( void )

{

PUSER_DEV       paudDev;
short           nDev;
char            szApp [ 20 ];
char            szKey [ 30 ];
char            szStr [ 120 ];
LPQUEST         lpaqnQn;
short           nQn;
HWDEV           hwDev;




        /** Clear all devices.
        **/
        for ( nDev = 0; nDev < MAX_DEVICES; nDev ++ ) {
                sprintf ( szApp, "Device%d", nDev );
                WritePrivateProfileString ( (LPSTR) szApp, NULL, NULL,
                                                         (LPSTR) cImlini );
        }



        paudDev = (PUSER_DEV) LocalLock ( giData.haudUserDev );

        if ( paudDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        for ( nDev = 0; nDev < giData.nUserDevices; nDev ++ ) {

                /** dump this mod.
                **/

                sprintf ( szApp, "Device%d", paudDev [ nDev ].nDeviceNo );

                switch ( paudDev [ nDev ].nType ) {

                case DEV_330X :
                        /** set the type to be the exename.
                        **/
                        if ( ! WritePrivateProfileString ( (LPSTR) szApp,
                                   (LPSTR) "Type", (LPSTR) paudDev [ nDev ].
                                             szName, (LPSTR) cImlini ) ) {

                                return ( FALSE );
                        }

                        /** Now do the parameters for this device.
                        **/

                        break;


                case DEV_WINAPP :

                case DEV_X_WINAPP :


                        /** Set the type to winapp and do the exe section.
                        **/
                        if ( ! WritePrivateProfileString ( (LPSTR) szApp,
                                   (LPSTR) "Type", (LPSTR) "WINAPP", (LPSTR)
                                                              cImlini ) ) {

                                return ( FALSE );
                        }




                        /** Get the hw dev for this user dev so we can get
                        *** the install dir.
                        **/
                        if ( ! GetHWStructForUserDev ( &(paudDev [ nDev ]),
                                                                &hwDev ) ) {

                                return ( FALSE );
                        }

                        sprintf ( szStr, "%s\\%s", hwDev.szInstallDir,
                                               paudDev [ nDev ].szExeName );

                        if ( ! WritePrivateProfileString ( (LPSTR) szApp,
                                   (LPSTR) "exe", (LPSTR) szStr, (LPSTR)
                                                              cImlini ) ) {

                                return ( FALSE );
                        }


                        /** Write out the settings hw name.
                        **/

                        strcpy ( giData.aimlDev [nDev].szHWName,
                                                        hwDev.szName );


                        if ( ! WritePrivateProfileString ( szApp, "Settings",
                                	giData.aimlDev [nDev].szHWName,
                                                	cImlini ) ) {

                                return ( FALSE );
                        }




                        /** Now write out the module.
                        **/

                        sprintf ( szKey, "Module%d", paudDev[nDev].nModuleNo);

                        if ( ! WritePrivateProfileString ( (LPSTR) szApp,
                                  (LPSTR) szKey, (LPSTR)paudDev[nDev].szName,
                                                      (LPSTR) cImlini ) ) {

                                return ( FALSE );
                        }


                        break;
                }

                /** OK Now we write out the parameters i.e. fields and
                ***answers for this user device.
                **/

                if ( paudDev [ nDev ].haqnQn == NULL ) {
                        continue;
                }


                lpaqnQn = (LPQUEST) GlobalLock ( paudDev [ nDev ].haqnQn);
                if ( lpaqnQn == NULL ) {

                        HandleError ( NO_MEMORY );
                        return ( FALSE );
                }

                for ( nQn = 0; nQn < paudDev [ nDev ].nQns; nQn ++ ) {

                        Detokenise ( lpaqnQn [ nQn ].szField, szKey,
                                                paudDev [ nDev ].nModuleNo );

                        if ( ! WritePrivateProfileString ( szApp, szKey,
                        		lpaqnQn [nQn].szAns, cImlini ) ) {

                                return ( FALSE );
                        }
                }
                GlobalUnlock ( paudDev [ nDev ].haqnQn);
        }

        LocalUnlock ( giData.haudUserDev );


	return ( TRUE );
}





/******************************************************************/
/** This is used to read the potential hardware types from the IML.INI
*** to provide the Data for the ADD dialog.
**/


static short ReadIMLINIHardware ( void )
{

	short   nHWNo = 0;
	HWDEV   hwdHW;


    /** While we keep finding hardware in the IML ini file add it to the
    *** list. The GetHardwareFromFile () gets all the modules questions,
    *** etc for this piece of hardware.
    **/

    while ( GetHardwareFromFile ( cImlini, nHWNo, &hwdHW ) ) {

            if ( ! AddHardwareToList ( &hwdHW ) ) {
                    return ( FALSE );
            }
            nHWNo ++;
    }


	return ( TRUE );
}



/****************************************************************/
/** Reads hardware from the .INF file and adds it to both the
*** current list and the .INI file for future use.
**/


static short AppendSetupHardware ( void )
{

short   nHWNo = 0;
HWDEV   hwdHW;
short   bNewHardware;


        /** Get the hardware from the setup file and add it to our list.
        *** We also need to add it to the IML.INI file so  we can find it
        *** again.
        **/


        while ( GetHardwareFromFile ( giData.szSetupPath, nHWNo, &hwdHW ) ) {

                /** Put the install Directory into the HW struct.
                **/

                strcpy ( hwdHW.szInstallDir, giData.szInstallDir );

                /** Find out if we already have this hardware in our list
                **/

                bNewHardware = IsNewHardware ( &hwdHW );

                if ( bNewHardware ) {
                        /** Add it to the list as it is not in the list.
                        **/

                	if ( ! AddHardwareToList ( &hwdHW ) ) {

                        	return ( FALSE );
                	}
                }

                if ( ! AddHardwareToIMLINI ( &hwdHW, bNewHardware ) ) {

                        return ( FALSE );
                }

                nHWNo ++;
        }

	return ( TRUE );
}




/*******************************************************************/
/** Reads the hardware details from either a .INI or .INF file and adds
*** them to the list.
**/


static short GetHardwareFromFile ( char * szFile, short nHWNo, PHWDEV phwDev)

{

char    szKey [ 20 ];
char	szRet [ 120 ];


        /** Initialise the hardware struct.
        **/
        phwDev->nDeviceType = 0;
        phwDev->szName [ 0 ] = '\0';
        phwDev->szDesc [ 0 ] = '\0';
        phwDev->szInstallDir [ 0 ] = '\0';
        phwDev->szExeName [ 0 ] = '\0';
        phwDev->nModules = 0;
        phwDev->hamodModule = NULL;
        phwDev->nUnit = UNIT_MODULE;;


	/** Get details for this device number from the file specified.
	*** If we find the details then put then into the hw dev struct
	*** and return true else return false.
	**/

	sprintf ( szKey, "HW%d", nHWNo );

        szRet [ 0 ] = '\0';


	if ( GetPrivateProfileString ( (LPSTR) "Hardware", (LPSTR) szKey,
		    (LPSTR) "", (LPSTR) szRet, 119, (LPSTR) szFile ) == 0 ) {

		/** This one not found so return false.
		**/
		return ( FALSE );
	}

        /** OK this bit of hardware exists so extract the details and put
        *** them into the hw dev struct.
        *** First get the name and description.
        **/

        /** Copy the description of the end of the string before we change
        *** it by using strtok.
        **/

        strcpy ( phwDev->szDesc, Despace ( strchr ( szRet, ',' ) ) );
        strcpy ( phwDev->szName, Despace ( strtok ( szRet, " ," ) ) );


        /** OK now find the section specified by the name and sort out
        *** details of modules, questions etc.
        **/


        /** Find the install directory. We will not find this if this is the
        *** setup file so don't worry about checking the result, just copy
        *** what we get to the install dir field.
        **/
     	GetPrivateProfileString ( (LPSTR) phwDev->szName, (LPSTR)
               "InstallDir", (LPSTR) "", (LPSTR) szRet, 119, (LPSTR) szFile );

        /** OK Store the install Dir - This may be the default "".
        **/
        strcpy ( phwDev->szInstallDir, Despace ( szRet ) );


        /** find name, check the type. If winapp get the exename and modules
        *** if 3301/2/3 then get questions.
        **/

	if ( GetPrivateProfileString ( (LPSTR) phwDev->szName, (LPSTR) "Type",
		    (LPSTR) "", (LPSTR) szRet, 119, (LPSTR) szFile ) == 0 ) {

		/** This one not found so return false.
		**/
		return ( FALSE );
	}



        /** See what type this is.
        **/

        if ( ( stricmp ( Despace ( szRet ), "WINAPP" ) ) == 0 ) {
                /** This is a winapp so we need to get the exename and the
                *** modules supported.
                **/

                phwDev->nDeviceType = DEV_WINAPP;
        }


        if ( ( stricmp ( Despace ( szRet ), "X_WINAPP" ) ) == 0 ) {
                /** This is a winapp so we need to get the exename and the
                *** modules supported.
                **/

                phwDev->nDeviceType = DEV_X_WINAPP;
        }



        /** See If the type is a 3301 /2 or 3. If it is we need to get the
        *** questions for the type.
        **/

        if ( ( stricmp ( Despace ( szRet ), "3301" ) == 0 ) ||
                ( stricmp ( Despace ( szRet ), "3302" ) == 0 ) ||
                	( stricmp ( Despace ( szRet ), "3303" ) == 0 ) ||
                	( stricmp ( Despace ( szRet ), "1580" ) == 0 ) ) {

                /** This is a 3301,2, 3 or 1580 so get the details we require.
                **/
                phwDev->nDeviceType = DEV_330X;

        }




        switch ( phwDev->nDeviceType ) {

        case DEV_WINAPP:
        case DEV_X_WINAPP:


        	if ( GetPrivateProfileString ( (LPSTR) phwDev->szName,
                          (LPSTR) "ExeName", (LPSTR) "", (LPSTR) szRet, 119,
                                                   (LPSTR) szFile ) == 0 ) {

	        	/** We haven't found the exename so return false.
	        	**/
		        return ( FALSE );
	        }

                /** OK Store the exename.
                **/
                strcpy ( phwDev->szExeName, Despace ( szRet ) );



                /** Get the unit field.
                **/

                GetPrivateProfileString ( phwDev->szName, "Unit", "", szRet,
                                                   		119, szFile );

                if ( ( stricmp ( Despace ( szRet ), "DEVICE" ) ) == 0 ) {

                        phwDev->nUnit = UNIT_DEVICE;

        	} else {

                        phwDev->nUnit = UNIT_MODULE;
        	}


                /** Now get the winapp module details.
                **/

                return ( GetWinappModules ( szFile, phwDev ) );


        case DEV_330X :

                return ( Get330xDetails ( szFile, phwDev ) );
        }



        /** We don't know what the type is so return false.
        **/
        return ( FALSE );
}




/*******************************************************/
/** We have decided that the device is a WINAPP.
*** This procedure gets the details for the various modules.
**/


static short GetWinappModules ( char * szFile, PHWDEV phwDev )

{

char            szKey [ 20 ];
short           nMod = 0;
char            szApp [ 30 ];
USER_DEV        udDev;
QUEST           quQuest;


        /** Initialise our structure
       	**/
       	udDev.szDesc [ 0 ] = '\0';
       	udDev.nQns = 0;
       	udDev.haqnQn = NULL;
       	strcpy ( udDev.szExeName, phwDev->szExeName );
        strcpy ( udDev.szHWName, phwDev->szName );
        udDev.szName [ 0 ] = '\0';
       	udDev.nDeviceNo = NOT_KNOWN;
       	udDev.nModuleNo = NOT_KNOWN;
       	udDev.nType = phwDev->nDeviceType;


        /** while we find the next module name then store the name then
        *** while there is a another question get it and add it to the
        *** module / user dev. Then add the module to the HW device.
        **/

        for ( nMod = 0; nMod < MAX_MODULES; nMod ++ ) {


	        /** Initialise our structure for this module
        	**/
        	udDev.szDesc [ 0 ] = '\0';
        	udDev.nQns = 0;
        	udDev.haqnQn = NULL;
        	strcpy ( udDev.szExeName, phwDev->szExeName );
        	strcpy ( udDev.szHWName, phwDev->szName );
        	udDev.szName [ 0 ] = '\0';
        	udDev.nDeviceNo = NOT_KNOWN;
        	udDev.nModuleNo = NOT_KNOWN;
        	udDev.nType = phwDev->nDeviceType;


                sprintf ( szKey, "Mod%d", nMod );

                if ( GetPrivateProfileString ( (LPSTR) phwDev->szName,
                                (LPSTR) szKey, (LPSTR) "", (LPSTR)
                                udDev.szName, MAX_NAME_LENGTH,
                                                   (LPSTR) szFile ) == 0 ) {

                        /** This module is not present so just return.
                        **/
                        return ( TRUE );
                }


                /** Setup the app field as exename.name then get the
                *** description and the questions etc.
                *** For the description we use the default as the app name.
                **/

                sprintf ( szApp, "%s.%s", phwDev->szName, udDev.szName );

                GetPrivateProfileString ( (LPSTR) szApp, (LPSTR) "Desc",
                            (LPSTR) szApp, (LPSTR) udDev.szDesc,
                                         MAX_DESC_LENGTH, (LPSTR) szFile );


                /** OK now add the questions to the user device.
                **/

                while ( GetQuestionForUserDevice ( szFile, &udDev, &quQuest,
                                                                  szApp ) ) {

                        if ( ! AddQuestionToUserDevice ( &udDev, &quQuest )){
                                return ( FALSE );
                        }
                }

                /** Now add the module to the hardware struct.
                **/
                if ( ! AddModuleToHWDevice ( phwDev, &udDev ) ) {
                        return ( FALSE );
                }
        }

        return ( TRUE );
}


/**************************************************************/
/** We have decided that the device is a 330X. 
*** Therefore it has only one module style entry. Get the
*** details of this.
**/


static short Get330xDetails ( char * szFile, PHWDEV phwDev )

{

USER_DEV        udDev;
QUEST           quQuest;


        /** Initialise our structure.
        **/
        udDev.szDesc [ 0 ] = '\0';
        udDev.nQns = 0;
        udDev.haqnQn = NULL;
        udDev.szExeName [ 0 ] = '\0';
        udDev.szHWName [ 0 ] = '\0';
        udDev.szName [ 0 ] = '\0';
        udDev.nDeviceNo = NOT_KNOWN;
        udDev.nModuleNo = NOT_KNOWN;
        udDev.nType = phwDev->nDeviceType;


        /** We need to get the questions for the 330x device. As it is
        *** a 330x device we know it only has one 'module' so we can build
        *** the module struct by hand then add the questions to it.
        **/


        /** Put the name into the user dev struct. This is the same as the
        *** HW dev name. Also do the exe name which is also the same.
        **/

        strcpy ( udDev.szName, phwDev->szName );
        strcpy ( udDev.szExeName, phwDev->szName );
        strcpy ( udDev.szHWName, phwDev->szName );
        strcpy ( phwDev->szExeName, phwDev->szName );


        /** Get the description and store it. Use the name as the default.
        **/

        GetPrivateProfileString ( (LPSTR) udDev.szName, (LPSTR) "Desc",
                            (LPSTR) udDev.szName, (LPSTR) udDev.szDesc,
                                         MAX_DESC_LENGTH, (LPSTR) szFile );


        /** OK now add the questions to the user device.
        **/

        while ( GetQuestionForUserDevice ( szFile, &udDev, &quQuest,
                                                        phwDev->szName ) ) {

                if ( ! AddQuestionToUserDevice ( &udDev, &quQuest ) ) {
                        return ( FALSE );
                }
        }

        /** Now add the module to the hardware struct.
        **/
        if ( ! AddModuleToHWDevice ( phwDev, &udDev ) ) {
                return ( FALSE );
        }

        return ( TRUE );
}



/*********************************************************/
/** Read the question details from the INI file.
*** Return FALSE if the question is not found.
**/

static short GetQuestionForUserDevice ( char * szFile, PUSER_DEV pudDev,
                                                LPQUEST lpqnQn, char * szApp )

{

char    szRet [ 120 ];
char    szKey [ 20 ];
short   nOpt;

        /** Get the question details. These are the question, the field,
        *** the default if there is one, and any options.
        **/

        /** Initialise the question struct.
        **/

        lpqnQn->nType = 0;
        lpqnQn->nOptions = 0;
        lpqnQn->nCurrentOpt = 0;
        lpqnQn->szQn [ 0 ] = '\0';
        lpqnQn->szAns [ 0 ] = '\0';
        lpqnQn->szField [ 0 ] = '\0';
        lpqnQn->szDefault [ 0 ] = '\0';

        for ( nOpt = 0; nOpt < MAX_OPTIONS; nOpt ++ ) {
                lpqnQn->aszOpt [ nOpt ] [ 0 ] = '\0';
                lpqnQn->aszOptAns [ nOpt ] [ 0 ] = '\0';
        }

        /** Set up the key we are looking for.
        **/
        sprintf ( szKey, "Qu%d", pudDev->nQns );

        if ( GetPrivateProfileString ( szApp, szKey, "", szRet, 119, szFile ) == 0 ) {

                /** This question not found so return false.
                **/
                return ( FALSE );
        }

        /** OK store the question.
        **/
        strcpy ( lpqnQn->szQn, Despace ( szRet ) );


        /** Set up the key we are looking for.
        **/
        sprintf ( szKey, "Qu%d.field", pudDev->nQns );

        if ( GetPrivateProfileString ( szApp, szKey, "", szRet, 119, szFile ) == 0 ) {

                /** This question field not found so return false.
                **/
                return ( FALSE );
        }

        /** OK store the question field.
        **/
        strcpy ( lpqnQn->szField, Despace ( szRet ) );


        /** Now look for a default. There may not be one of these.
        **/

        /** Set up the key we are looking for.
        **/
        sprintf ( szKey, "Qu%d.default", pudDev->nQns );

        GetPrivateProfileString ( szApp, szKey, "", szRet, 119, szFile );

        /** OK store the question default.
        **/
        strcpy ( lpqnQn->szDefault, Despace ( szRet ) );


        /** Now we need to look for any options. We don't return false if
        *** we don't find an option. Set the question type to answer first.
        *** if we have any options then we reset this and count the options.
        **/

        lpqnQn->nType = QUESTION_ANSWER;
        lpqnQn->nOptions = 0;

        for ( nOpt = 0; nOpt < MAX_OPTIONS; nOpt ++ ) {

                /** Set up the key we are looking for.
                **/
                sprintf ( szKey, "Qu%d.Opt%d", pudDev->nQns, nOpt );

                if ( GetPrivateProfileString ( szApp, szKey, "", szRet, 119,
                        				szFile ) == 0 ) {

                        /** This option not found so return.
                        **/
                        return ( TRUE );
                }

                /** Store the option, reset the type and add an option to the
                *** count. Get the option answer before we fuck the string
                *** by strtok'ing it.
                **/

                strcpy ( lpqnQn->aszOptAns [ nOpt ], Despace ( strchr (
                                                            szRet, ',' ) ) );

                strcpy ( lpqnQn->aszOpt [ nOpt ], Despace ( strtok ( szRet,
                                                                  "," ) ) );

                lpqnQn->nType = QUESTION_OPTION;
                lpqnQn->nOptions ++;
        }

		

        return ( TRUE );
}








static short AddHardwareToIMLINI ( PHWDEV phwDev, short bNewHardware )


{

char    szKey [ 20 ];
char    szStr [ 120 ];


        /** We need to extract all the details for this piece of hardware
        *** and write then to the IML.INI file.
        **/

        /** Write the entry in the Hardware section. We look at the number
        *** of hw devices to work out which number to use for this one. I.e.
        *** we take one off this as it has just been incremented when we
        *** added the device to the list.
        **/

        /** If this is new hardware then we need to do the HWn stuff. If
        *** not then we don't bother.
        **/

        if ( bNewHardware ) {

        	sprintf ( szKey, "HW%d", giData.nHWDevices -1 );
        	sprintf ( szStr, "%s, %s", phwDev->szName, phwDev->szDesc );

        	if ( ! WritePrivateProfileString ( (LPSTR) "Hardware",
                       	   (LPSTR) szKey, (LPSTR) szStr, (LPSTR) cImlini)){

                	return ( FALSE );
        	}
        }

        /** Now do the section for the name. If this is a 330X type then
        *** we put the description and questions in here, there are no
        *** modules. If this is a Winapp type then we need to do the modules
        *** one by one.
        **/

        switch ( phwDev->nDeviceType ) {

        case DEV_330X :

                /** This is a 330X type so add it to the IML.INI file.
                **/

                return ( Add330XToIMLINI ( phwDev ) );


        case DEV_WINAPP :

        case DEV_X_WINAPP :

                /** This is a winapp type so add any modules with their
                *** questions to the IML.INI file.
                **/
                return ( AddWinappModulesToIMLINI ( phwDev ) );


        default :

                /** We don't know this type so return false.
                **/

                return ( FALSE );
        }
}






static short Add330XToIMLINI ( PHWDEV phwDev )

{

PUSER_DEV       paudDev;


        /** Lock the handle to the module and write out the details and the
        *** questions etc.
        **/

        /** We need to write the type to the file - this is the name in this
        *** case.
        **/
        if ( ! WritePrivateProfileString ( (LPSTR) phwDev->szName,
                                 (LPSTR) "Type", (LPSTR) phwDev->szName,
                                                   (LPSTR) cImlini ) ) {

                return ( FALSE );
        }


        paudDev = (PUSER_DEV) LocalLock ( phwDev->hamodModule );
        if ( paudDev == NULL ) {

                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /** Now write the description.
        **/

        if ( ! WritePrivateProfileString ( (LPSTR) phwDev->szName, (LPSTR)
                                    "Desc", (LPSTR) paudDev [ 0 ].szDesc,
                                                   (LPSTR) cImlini ) ) {

                return ( FALSE );
        }


        if ( ! WritePrivateProfileString ( (LPSTR) phwDev->szName,
                        (LPSTR) "InstallDir", (LPSTR) giData.szInstallDir,
                                                   (LPSTR) cImlini ) ) {

                return ( FALSE );
        }


        /** Write the questions to the section given by the name.
        **/
        if ( ! WriteQuestionsToIMLINI ( &(paudDev [ 0 ]), phwDev->szName ) ){

                LocalUnlock ( phwDev->hamodModule );
                return ( FALSE );
        }

        LocalUnlock ( phwDev->hamodModule );

        return ( TRUE );
}






static short AddWinappModulesToIMLINI ( PHWDEV phwDev )

{

PUSER_DEV       paudDev;
char            szApp [ 30 ];
char            szKey [ 30 ];
short           nMod;
char            szStr [ 20 ];


        /** lock the array of modules and write the details for each.
        *** Each module
        *** We need to write the install dir followed by the name of each
        *** module. For each module we write a section with its details.
        *** We also need the type and exename. The type is WINAPP,
        **/


        switch ( phwDev->nDeviceType ) {

        case DEV_WINAPP :

        	if ( ! WritePrivateProfileString ( (LPSTR) phwDev->szName,
                      (LPSTR) "Type", (LPSTR) "WINAPP", (LPSTR) cImlini )) {

                	return ( FALSE );
        	}
                break;

        case DEV_X_WINAPP :
        	if ( ! WritePrivateProfileString ( (LPSTR) phwDev->szName,
                      (LPSTR) "Type", (LPSTR) "X_WINAPP", (LPSTR) cImlini )) {

                	return ( FALSE );
        	}
                break;
        }



        if ( ! WritePrivateProfileString ( (LPSTR) phwDev->szName,
                        (LPSTR) "Exename", (LPSTR) phwDev->szExeName,
                                                (LPSTR) cImlini ) ) {

                return ( FALSE );
        }


        if ( ! WritePrivateProfileString ( (LPSTR) phwDev->szName,
                        (LPSTR) "InstallDir", (LPSTR) giData.szInstallDir,
                                                (LPSTR) cImlini ) ) {

                return ( FALSE );
        }



        /** Now write out the unit field.
        **/

        switch ( phwDev->nUnit ) {

        case UNIT_DEVICE :
                strcpy ( szStr, "Device" );
                break;

        case UNIT_MODULE :
        default :
                strcpy ( szStr, "Module" );
                break;
        }

        if ( ! WritePrivateProfileString ( phwDev->szName, "Unit", szStr,
                                                 	cImlini ) ) {

                return ( FALSE );
        }


        if ( phwDev->nModules == 0 ) {
                return ( TRUE );
        }


        paudDev = (PUSER_DEV) LocalLock ( phwDev->hamodModule );
        if ( paudDev == NULL ) {
                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /** Now step through each module. Write the name and description
        *** and the questions for each one.
        **/

        for ( nMod = 0; nMod < phwDev->nModules; nMod ++ ) {

                sprintf ( szKey, "Mod%d", nMod );

                if ( ! WritePrivateProfileString ( (LPSTR) phwDev->szName,
                        (LPSTR) szKey, (LPSTR) paudDev [ nMod ].szName,
                                                (LPSTR) cImlini ) ) {

                        return ( FALSE );
                }

                /** Setup the app name as devname.name and write the
                *** decription and questions
                **/

                sprintf ( szApp, "%s.%s", phwDev->szName, paudDev [
                                                          nMod ].szName );

                if ( ! WritePrivateProfileString ( (LPSTR) szApp,
                             (LPSTR) "Desc", (LPSTR) paudDev [ nMod ].szDesc,
                                                    (LPSTR) cImlini ) ) {

                        return ( FALSE );
                }


                /** Write the questions to the section given by the name.
                **/
                if ( ! WriteQuestionsToIMLINI ( &(paudDev [nMod]), szApp ) ){

                        LocalUnlock ( phwDev->hamodModule );
                        return ( FALSE );
                }
        }

        LocalUnlock ( phwDev->hamodModule );

        return ( TRUE );
}









static short WriteQuestionsToIMLINI ( PUSER_DEV pudDev, char * szApp )

{

LPQUEST         lpaqnQn;
short           nQuest;
char            szKey [ 20 ];
short           nOpt;
char            szOpt [ 2 * MAX_OPTION_LENGTH ];



        /** Check if there are any questions. If not then just return true.
        ***/
        if ( pudDev->nQns == 0 ) {
                return ( TRUE );
        }

        /** Now we need to lock the question array so we can write them out.
        **/


        lpaqnQn = (LPQUEST) GlobalLock ( pudDev->haqnQn );
        if ( lpaqnQn == NULL ) {

                HandleError ( NO_MEMORY );
                return ( FALSE );
        }

        /** Now write out each question checking which type it is.
        **/

        for ( nQuest = 0; nQuest < pudDev->nQns; nQuest ++ ) {

                /** write out the question, field and default for all
                *** questions.
                **/

                sprintf ( szKey, "Qu%d", nQuest );
                if ( ! WritePrivateProfileString ( szApp, szKey, lpaqnQn [
                        		nQuest ].szQn, cImlini ) ) {

                        return ( FALSE );
                }

                sprintf ( szKey, "Qu%d.field", nQuest );
                if ( ! WritePrivateProfileString ( szApp, szKey, lpaqnQn [
                        		nQuest ].szField, cImlini ) ) {

                        return ( FALSE );
                }

                sprintf ( szKey, "Qu%d.default", nQuest );
                if ( ! WritePrivateProfileString ( szApp, szKey, lpaqnQn [
                        		nQuest ].szDefault, cImlini ) ) {

                        return ( FALSE );
                }


                /** Now check the type. If it the question has options we
                *** need to write them out.
                **/

                switch ( lpaqnQn [ nQuest ].nType ) {


                case QUESTION_ANSWER :
                        /** This question has no options so we just break.
                        **/

                        break;


                case QUESTION_OPTION :
                        /** This question has options so we need to write
                        *** these out.
                        **/

                        for ( nOpt = 0; nOpt < lpaqnQn [ nQuest ].nOptions;
                                                                nOpt ++ ) {

                                sprintf ( szKey, "Qu%d.Opt%d", nQuest, nOpt );

                                szOpt [ 0 ] = '\0';

                                strcpy ( szOpt, lpaqnQn [ nQuest ].
                                       			aszOpt [ nOpt ] );

                                strcat ( szOpt, "," );

                                strcat ( szOpt, lpaqnQn [ nQuest ].
                                                        aszOptAns [ nOpt ] );


                                if ( ! WritePrivateProfileString ( szApp,
                                        	szKey, szOpt, cImlini ) ) {

                                        return ( FALSE );
                                }
                        }

                        break;

                default :
                        return ( FALSE );
                }
        }

        return ( TRUE );
}








static short GetIMLINI330XDetails ( short nDev )

{

USER_DEV        udDev;
QUEST           quQuest;



        /** Initialise are structure.
        **/
        udDev.szDesc [ 0 ] = '\0';
        udDev.nQns = 0;
        udDev.haqnQn = NULL;
        strcpy ( udDev.szExeName, "" );
        strcpy ( udDev.szHWName, giData.aimlDev [ nDev ].szHWName );
        strcpy ( udDev.szName, giData.aimlDev [ nDev ].szExeName );
        udDev.nDeviceNo = nDev;
        udDev.nModuleNo = 0;
        udDev.nType = DEV_330X;


        /** We need to get the questions and answers for this type. The exact
        *** type is stored in the exename field of the imldev struct array.
        *** Get the description from the type.
        **/

        GetPrivateProfileString ( (LPSTR) udDev.szName, (LPSTR) "Desc",
                            (LPSTR) udDev.szName, (LPSTR) udDev.szDesc,
                                         MAX_DESC_LENGTH, (LPSTR) cImlini );


        /** Now get the questions and answers.
        **/

        while ( GetQuestionForUserDevice ( cImlini, &udDev, &quQuest,
                                                          udDev.szName ) ) {

                if ( ! GetAnswerToQuestion ( &udDev, &quQuest ) ) {
                        return ( FALSE );
                }

                if ( ! AddQuestionToUserDevice ( &udDev, &quQuest ) ) {
                        return ( FALSE );
                }
        }

        return ( AddUserDeviceToList ( &udDev ) );
}










static short GetIMLINIWinappModules ( short nDev, short nType )

{

char            szKey [ 20 ];
short           nMod = 0;
char            szApp [ 30 ];
USER_DEV        udDev;
QUEST           quQuest;

HWDEV           hwDev;



        /** while we find the next module name then store the name then
        *** while there is a another question get it and add it to the
        *** module / user dev. Then add the module to the user device list.
        **/


        for ( nMod = 0; nMod < MAX_MODULES; nMod ++ ) {

        	/** Initialise our structure for this module.
        	**/
        	udDev.szDesc [ 0 ] = '\0';
        	udDev.nQns = 0;
        	udDev.haqnQn = NULL;
        	strcpy ( udDev.szExeName, giData.aimlDev [ nDev ].szExeName );
        	udDev.szName [ 0 ] = '\0';
        	udDev.nDeviceNo = nDev;
                udDev.nModuleNo = nMod;
        	udDev.nType = nType;

        	strcpy ( udDev.szHWName, giData.aimlDev [ nDev ].szHWName );


                sprintf ( szApp, "Device%d", nDev );

                sprintf ( szKey, "Module%d", nMod );


                if ( GetPrivateProfileString ( (LPSTR) szApp, (LPSTR) szKey,
                           (LPSTR) "", (LPSTR) udDev.szName, MAX_NAME_LENGTH,
                                                 (LPSTR) cImlini ) == 0 ) {

                        /** This module is not present so continue.
                        **/
                        continue;
                }

                giData.aimlDev [ nDev ].acModule [ nMod ] = MOD_INUSE;

                /** Setup the app field as exename.name then get the
                *** description and the questions etc.
                *** For the description we use the default as the app name.
                **/

                /** Use the HW name here.
                **/

                sprintf ( szApp, "%s.%s", udDev.szHWName, udDev.szName );

                GetPrivateProfileString ( (LPSTR) szApp, (LPSTR) "Desc",
                            (LPSTR) szApp, (LPSTR) udDev.szDesc,
                                         MAX_DESC_LENGTH, (LPSTR) cImlini );


                /** OK now add the questions to the user device.
                **/

                while ( GetQuestionForUserDevice ( cImlini, &udDev,
                                                       &quQuest, szApp ) ) {

                        if ( ! GetAnswerToQuestion ( &udDev, &quQuest ) ) {
                                return ( FALSE );
                        }

                        if ( ! AddQuestionToUserDevice ( &udDev, &quQuest )){
                                return ( FALSE );
                        }
                }




                /** Get the hardware device struct for this user dev so
                *** we can check which type it really is.
                **/

                GetHWStructForUserDev ( &udDev, &hwDev );

                udDev.nType = hwDev.nDeviceType;



                if ( ! AddUserDeviceToList ( &udDev ) ) {
                        return ( FALSE );
                }
        }

        return ( TRUE );
}







static short GetAnswerToQuestion ( PUSER_DEV pudDev, LPQUEST lpqnQn )

{

char    szRet [ 120 ];
char    szKey [ 20 ];
char    szApp [ 20 ];
short   nOpt;


        /** Set up the app name and key name. The key name is the detokenised
        *** field in the question.
        **/

        sprintf ( szApp, "Device%d", pudDev->nDeviceNo );
        Detokenise ( lpqnQn->szField, szKey, pudDev->nModuleNo );

        if ( GetPrivateProfileString ( szApp, szKey, "", szRet, 119,
                					cImlini ) == 0 ) {


                return ( FALSE );
        }

        strcpy ( lpqnQn->szAns, Despace ( szRet ) );

        /** If this is the answer to an option question then we must find
        *** which option this is and set the current option.
        **/

        if ( lpqnQn->nType == QUESTION_OPTION ) {
                /** Look through the option answers and compare the answer
                *** until we get a match.
                **/

                for ( nOpt = 0; nOpt < lpqnQn->nOptions; nOpt ++ ) {

                        if ( stricmp ( lpqnQn->aszOptAns[nOpt],
                                		lpqnQn->szAns ) == 0 ) {

                                /** We have a match so set the current option
                                *** and return.
                                **/
                                lpqnQn->nCurrentOpt = nOpt;
                                return ( TRUE );
                        }
                }
        }

        return ( TRUE );
}
