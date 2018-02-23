#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "c:\Windmill 32\Confiml\Copycomm.h"

/** Prototypes **/
DWORD IsFilePresent ( char* cPath, char* cFileName, char* cExt );
DWORD CreateHardwareList ( char* cPath );
DWORD CreateSoftwareList ( char* cPath );
DWORD AddDateToSoftwareList ( void );
DWORD LookForDevices ( char* szSrcDir, char* szInstDir );
DWORD CopyDevices ( char* cFileName, char* cInstPath );
DWORD IsDeviceOK ( char* cFileName, char* cDevice );
DWORD UpDateIniFile ( char* cPath );
DWORD SetComputerCode ( void );
DWORD MarkFile ( char* cExeName, char* cPath );
DWORD QueryMarkFile ( char* cExeName, char* cPath );
DWORD  AddUnauthorised ( void );
void RemoveSlash ( char* cPath );

/** Full path to iml.ini **/
char cImlini [MAX_PATH];
/** Full path to iml.bak. This is copy of existing ini **/
char cImlbak [MAX_PATH];
/** General use **/
char cFilePath [MAX_PATH];
/** Source with no \ **/
char szSrcDirx [MAX_PATH];
/** Install with no \ **/
char szInstx [MAX_PATH];


/** General use **/
char cSection [1024];



/*****************************************************/
/** This function is called before any files are copied.
*** We scan the handler list and the Application list looking
*** for exe files that already exist in the install dir. Any we
*** find are marked as NORMAL. If they are included in the install
*** then they will be overwritten and that will set the ARCHIVE flag.
*** After installation we can prepare a list of unchanged files. eg
*** REPLAY might have been in the directory but was not in the install.
*** As a result we put it on the not authorised list. This will still
*** allow it to work but it will say it is not authorised. It will have
*** a 1 : 1999 install date.
*** 
*** A return value of 0 causes the installation to exit.
*** The return was originally typed as CHAR ?
**/


/** For Free Trial simply check that no Windmill is present
**/

LONG WINAPI WmDelete ( HWND hwnd, LPSTR szSrcDir, LPSTR szSupport,
					   LPSTR szInst, LPSTR szRes )
{
	
	char cKey [5];
	char cHandler [20];
	char cExeName [12];
	char cCaption [80];
	int  nRet;
	char cTemp [20];
	int nIndex = 0;
	int nHardware = 0;
	BOOLEAN bFound = TRUE;

	strcpy ( szInstx, szInst );
	RemoveSlash ( szInstx ); 
	
	
	if ( GetComputerCode  () || IsFilePresent (szInstx,"iml",".ini" ) ) {

		strcpy ( cCaption, "Windmill is already on this computer.\r"  );
		strcat ( cCaption, "Install the free trial on a different one" );
		MessageBox ( GetFocus(), cCaption, "Install Exit", MB_OK );

		return (0);
	}

	return (1);


}

/***************************************************/
/** If the file cExeName exists in the directory
*** indicated by cPath then mark it NORMAL. They are
*** usually ARCHIVE.
**/


DWORD MarkFile ( char* cExeName, char* cPath )

{
	DWORD dwAttributes;

	
	/** Get the full path to the exe into cFilePath **/
	strcpy ( cFilePath, cPath );
	strcat ( cFilePath, "\\" );
	strcat ( cFilePath, cExeName );
	strcat ( cFilePath, ".exe" );

	dwAttributes = GetFileAttributes ( cFilePath );
		

	if ( dwAttributes != 0xFFFFFFFF ) {
		SetFileAttributes ( cFilePath, FILE_ATTRIBUTE_NORMAL );
		return (1);
	}

	return (0);

}

/***************************************************/
/** Files which were present in the install directory before copying
*** were marked by changing them from ARCHIVE to NORMAL. If they were
*** overwritten by the install then they will now be ARCHIVE again. 
*** We check the cExeName file on cPath to see if it is still NORMAL.
*** If it is we display a message and return 0.
*** Otherwise we return 1.
**/


DWORD QueryMarkFile ( char* cExeName, char* cPath )

{
	DWORD dwAttributes;
	char  cCaption [100];

	
	/** Get the full path to the exe into cFileB **/
	strcpy ( cFilePath, cPath );
	strcat ( cFilePath, "\\" );
	strcat ( cFilePath, cExeName );
	strcat ( cFilePath, ".exe" );

	dwAttributes = GetFileAttributes ( cFilePath );
		

	if ( dwAttributes == FILE_ATTRIBUTE_NORMAL ) {
		strcpy ( cCaption, cExeName );
		strcat ( cCaption, "  is already present but not in the new installation." );
		MessageBox ( GetFocus(), cCaption, "Install Warning", MB_OK );
		return (0);
	}

	return (1);

}


/***************************************************/
/** The main installation routine **/

/** For free trial use fixed IML.INI.
*** Change the install directory entries
*** Authorise the programs
*** Finally invalidate ComIML
**/

CHAR WINAPI WmInstall ( HWND hwnd, LPSTR szSrcDir, LPSTR szSupport,
					   LPSTR szInst, LPSTR szRes )

{

	strcpy ( szInstx, szInst );
	RemoveSlash ( szInstx ); 

	/** Initialise the path to iml.ini **/
	strcpy ( cImlini, szInstx );
	strcat ( cImlini, "\\iml.ini" );




	/** Change the directory for Comiml**/
	WritePrivateProfileString ( "COMIML", "InstallDir",szInstx, cImlini);
	strcpy ( cImlbak, szInstx );
	strcat ( cImlbak, "\\COMIML" );
	WritePrivateProfileString ( "Device0", "exe",cImlbak, cImlini);
	
	/** Change the directory for siggen **/
	WritePrivateProfileString ( "IMSIGGEN", "InstallDir",szInstx, cImlini);
	strcpy ( cImlbak, szInstx );
	strcat ( cImlbak, "\\IMSIGGEN" );
	WritePrivateProfileString ( "Device1", "exe",cImlbak, cImlini);


	/** Change the imd file directory **/
	strcpy ( cImlbak, szInstx );
	strcat ( cImlbak, "\\SetUps\\None.imd" );
	WritePrivateProfileString ( "COMIML.None", "IMDFile",cImlbak, cImlini);


	GetSoftwareDetailsFromINI ( cImlini );

	/** Set the new version of the computer code **/
	SetComputerCode ();

	AuthoriseAllPrograms ( 7 );


	/** Write to IML.INI to deauthorize ComIML **/
	WritePrivateProfileString ( "SOFTWARE","ComIML", "25181", cImlini );


	return(1);
	
}


/************************************************/
/** This routine searches on the path specified by cPath
*** for a file cFileName. The .exe extension is automatically
*** added. 
*** If it exists then it returns 1 else
*** it returns 0.
**/

DWORD IsFilePresent ( char* cPath, char* cFileName, char* cExt )

{
	char cBuffer [80];
	char* cName;
	
	return ( SearchPath ( cPath, cFileName, cExt, 
		80, cBuffer, &cName ) );

}




/*************************************************/
/** This routine searches through the handler list contained within IML.INI. 
*** For each handler on the list we
*** 1 Get the name of its EXE file
*** 2 See if it is present in the install dir and unmarked.
*** 3 If it is not then move to next handler
*** 4 Read the handlers Hwlist key. This is a new field.
*** 5 Add it to the hardware list
*** 6 Read the handler Softlist key. This is a new field.
*** 7 Add it to the software list
*** 8 We stop the process when we get to the end of the
*** handler list.
*** 9 We delete the handler list
***
*** The new fields 'Desc' and 'Software' were added to the handler to
*** allow the new installation method to work. The 'Desc' field data was
*** originally contained within the Hardware section of the individual
*** inf files. The Software field data was contained in the Software
*** section of the individual inf files.
**/

DWORD CreateHardwareList ( char* cPath )

{

	char cKey [5];
	char cHandler [20];
	char cExeName [12];
	char cBuffer [80];
	int  nRet;
	char cTemp [20];
	int nIndex = 0;
	int nHardware = 0;
	int nMod;
	BOOLEAN bFound = TRUE;

	while ( bFound ) {
	
		/** Handlers list contains entries
		***  HA0=IMSIGGEN
		***  HA1=IMML75X 
		***  etc
		**/
		strcpy ( cKey, "HA");
		itoa (nIndex, cTemp, 10);
		strcat ( cKey, cTemp);

		nRet = GetPrivateProfileString( "Handlers", cKey, "",
			                         cHandler, 20, cImlini);
		if ( nRet == 0 ) {
			bFound = FALSE;
			break;
		}

		/** Get the ExeName from the handler details **/
		nRet = GetPrivateProfileString( cHandler, "Exename", "",
									 cExeName, 12, cImlini );

		/** Do we have the handler exe file and is it one we have
		*** just installed. 
		**/
		if ( (IsFilePresent (cPath, cExeName, ".exe" )) &&
			 (QueryMarkFile ( cExeName, cPath ))          ) {
			/** Yes we do so add the handler to the hardware list **/

			/** First get the entry for the list this is
			*** from the Hwlist key
			**/	
			nRet = GetPrivateProfileString ( cHandler, "Hwlist", "",
				             cBuffer, 80 , cImlini );
				       
			/** Prepare the Key HW0 etc**/
			strcpy ( cKey, "HW");
			itoa (nHardware, cTemp, 10);
			strcat ( cKey, cTemp);

			/** Add it to the hardware list**/
			WritePrivateProfileString("Hardware",cKey, cBuffer, cImlini);

			nRet = GetPrivateProfileString ( cHandler, "Softlist", "",
									cBuffer, 80, cImlini );

			/** Add it to the software list also**/
			WritePrivateProfileString("Software",cBuffer,"0",cImlini);

			/** Write in the install dir **/
			WritePrivateProfileString(cHandler, "InstallDir", cPath, cImlini);

			/** Next hardware entry **/
			++nHardware;
		
		} else {
			
			/** The handler is not present so we delete the unused
			*** entry. First all the modules. Then the main entry.
			**/

			for ( nMod = 0; nMod<32; ++nMod ) {

				/** Prepare the Mod0 etc**/
				strcpy ( cKey, "Mod");
				itoa (nMod, cTemp, 10);
				strcat ( cKey, cTemp);

				/** Read the module name **/
				if ( GetPrivateProfileString( cHandler, cKey, "", 
					         cTemp, 20, cImlini ) != 0 ) {
					/** Prepare the section name **/
					strcpy (cBuffer,cHandler);
					strcat (cBuffer, ".");
					strcat (cBuffer,cTemp);
					/** Delete it **/
					WritePrivateProfileSection(cBuffer,NULL,cImlini);
				}
			}

			/** Delete the whole handler **/
			WritePrivateProfileSection(cHandler,NULL,cImlini);			
		}
		
		/** Next handlers entry **/
		++nIndex;
	}

	/** Delete the handlers section **/
	WritePrivateProfileString( "Handlers", NULL, NULL, cImlini );

	/** Return the number of handlers found **/
	return ( nIndex );
}

/******************************************************/
/** Search through the Applications list of the ini file.
*** If a file with that name is present then give it an entry in the
*** software section. Finally delete the applications list.
**/


DWORD CreateSoftwareList ( char* cPath ) 

{
	char cKey[5];
	char cTemp[10];
	char cExeName[12];
	int nIndex = 0;
	int nRet;
	BOOLEAN bFound = TRUE;
		
	while ( bFound ) {
	
		/** Applications list contains entries
		***  AP0=Graphics
		***  AP1=Replay
		***  etc
		**/
		strcpy ( cKey, "AP");
		itoa (nIndex, cTemp, 10);
		strcat ( cKey, cTemp);

		nRet = GetPrivateProfileString( "APPLICATIONS", cKey, "",
			                         cExeName, 12, cImlini);

		if ( nRet == 0 ) {
			bFound = FALSE;
			break;
		}

		/** Do we have the application exe file **/
		if ( IsFilePresent (cPath, cExeName, ".exe" ) ) {
			/** Yes we do so add the app to the software list **/

			/** Add it to the software list if its not marked 
			*** The softlist entry for apps is the exename
			**/
			if ( QueryMarkFile ( cExeName, cPath) ) {
				WritePrivateProfileString("Software",cExeName,"0",cImlini);
			}
		}
		
		/** Next software entry **/
		++nIndex;
	}

	/** Delete the applications section **/
	WritePrivateProfileString ( "APPLICATIONS",NULL,NULL,cImlini);

	

	/** Return the number of programs found **/
	return ( nIndex );	

}


/**********************************************/
/** Add the contents of the CopyControl section to the end
*** of the software list. This is done simply to allow us to use the
*** old installation software that was so arranged.
**/

DWORD AddDateToSoftwareList ( void )

{
	char cBuffer [12];

	/** Copy the contents of the COPYCONTROL TO SOFTWARE **/
	GetPrivateProfileString( "COPYCONTROL", "Year", "",
			                         cBuffer, 12, cImlini);
	WritePrivateProfileString( "SOFTWARE", "Year",
									 cBuffer, cImlini);
	GetPrivateProfileString( "COPYCONTROL", "Month", "",
			                         cBuffer, 12, cImlini);
	WritePrivateProfileString( "SOFTWARE", "Month",
									 cBuffer, cImlini);
	GetPrivateProfileString( "COPYCONTROL", "Duration", "",
			                         cBuffer, 12, cImlini);
	WritePrivateProfileString( "SOFTWARE", "Duration",
									 cBuffer, cImlini);

	/** Delete the copy control section **/
	WritePrivateProfileSection ( "COPYCONTROL", NULL, cImlini );

	return (0);
}

/************************************************/
/** There are several occasions on which we want to copy devices
*** from existing files into the new .ini file. We use the following
*** order of precedence.
*** 1 if the install disk contains Devices.ini then use these
*** 2 if the install dir already has an IML.INI use these
*** 3 if the Windows dir has an IML.INI use these.
***
*** This means that we can install a Devices set up from disk or
*** we can use the existing one on the computer or
*** we can use the old style one from the system dir.
**/

DWORD LookForDevices ( char* szSrcDir, char* szInstDir )

{


	/** If we have a file on the disk then use it **/

	if ( IsFilePresent ( szSrcDir, "Devices", ".ini")) {
		strcpy ( cFilePath, szSrcDir );
		strcat ( cFilePath, "\\Devices.ini");	

		return ( CopyDevices ( cFilePath, szInstDir ) );
	}

	if ( IsFilePresent ( szInstDir, "Iml", ".bak")){		
		return ( CopyDevices ( cImlbak, szInstDir ) );
	}


/**	 This would be a version 4 install. Don't want to know

	GetWindowsDirectory ( cFilePath, MAX_PATH );
	if ( IsFilePresent ( cFilePath, "iml", ".ini")){
		strcat ( cFilePath, "\\iml.ini");		
		return ( CopyDevices ( cFilePath, szInstDir ) );
	}
**/

	return (0);

}

/*******************************************************/
/** Look for devices 0 to 31 in the specified file and
*** copy their sections into iml.inf.
*** We change the exe key since the install directory may be
*** different.  
*** Return 0 if no problems
***        1 if we find a installed device that we are not installing
**/

DWORD CopyDevices ( char* cFileName, char* cInstPath )

{
	int  nDevice;
	char cTemp [20];
	char cDevice [12];


	for ( nDevice = 0; nDevice < 32; ++nDevice ) {
		
		/** Prepare the section name **/
		strcpy ( cDevice, "Device" );
		itoa (nDevice, cTemp, 10);
		strcat ( cDevice, cTemp );

		/** If the device exists and is in the new installation
		*** then add it to IML.INF. If it does not exist or is not
		*** in the installation then continue.
		**/
		if ( IsDeviceOK ( cFileName, cDevice ) != 1) {
			continue;
		}

		/** Read the section **/
		GetPrivateProfileSection ( cDevice, cSection, 1023, cFileName);

		/** copy it to the inf file **/		
		WritePrivateProfileSection ( cDevice, cSection, cImlini );

		/** Read the exe entry. This is the full path to the exe file **/
		GetPrivateProfileString(cDevice,"exe","", cSection, 256, cImlini);

		/** Get the required exe name **/
		_splitpath(cSection, NULL, NULL, cTemp, NULL);

		/** Prepare the new entry **/
		strcpy (cSection, cInstPath);
		strcat (cSection, "\\");
		strcat (cSection, cTemp);

		/** Write it **/
		WritePrivateProfileString ( cDevice, "exe", cSection, cImlini);

		/** If its ComIML we write in a None module
		*** This is an IMD file that does nothing but allows us to have a 
		*** preinstalled Device which won't crash. The file is in the install
		*** directory so we must set its path
		**/
		if (strcmp ( cTemp, "COMIML" ) == 0 ) {
			strcpy ( cSection, cInstPath );
			strcat ( cSection, "\\None.imd" );
			WritePrivateProfileString ( "COMIML.None", "IMDFile", cSection, cImlini );
			/** Also add the description **/
			WritePrivateProfileString ( "COMIML.None", "Desc","None$ : Empty File - replace with your own IMD file",cImlini );
			/** Must also change the module in the device **/
			WritePrivateProfileString ( "COMIML", "Mod0", "None", cImlini );
		}

		/** If this is the 800 handler **/
		if ( strcmp  ( cTemp, "IMML800" ) == 0 ) {

			/** We may not have a modem entry if its an early version **/
			if ( GetPrivateProfileInt (cDevice,"M0P3", 255, cImlini ) == 255) {
				/** Add one **/
				WritePrivateProfileString ( cDevice, "M0P3", "0", cImlini ); 
			}
		}
	}

	return (0);
}

/****************************************************/
/** We are passed 
*** cFilename the file to look in
*** cDevice the Device section name Device0 etc
*** We read the device settings that contain the handler name
*** We check that the handler is being installed.
*** We return 0 if there is no such device
***           1 if there is and it needs copying
***           2 if it is not on the hardware list
**/

DWORD IsDeviceOK ( char* cFileName, char* cDevice )

{
	int nRet;
	char cSettings [20];
	char cHardware [20];

	
	/** If we don't have a device at that number return **/
	nRet = GetPrivateProfileString( cDevice,"Settings","",
		                      cSettings, 20, cFileName);
	if ( nRet == 0) {
		return ( 0 );
	}

	/** We have a device the handler name is in cSettings.
	*** If we can read the Exename then the handler is
	*** present.
	**/
	if ( GetPrivateProfileString( cSettings, "Exename", "",
		cHardware, 20, cImlini ) ) {
		return (1);
	}

	/** Warn the user **/
	MessageBox(GetFocus(), "A device has been found that is not in your new installation.", "Install Warning", MB_OK );

	return (2);
}



/********************************************************/
/** Makes a registry entry of IOTYPE at the sub key below. This
*** is simply the 32 bit seconds since 1970 of the computer. It serves
*** to identify the computer since you must also copy this obscure entry
*** if you wish to have authorised software.
**/ 


DWORD SetComputerCode ( void )

{
	char * szSubKey = "SYSTEM\\CURRENTCONTROLSET\\SERVICES\\MLGPIB\\PARAMETERS";
	HKEY myKey;
	DWORD dwTime;

	dwTime = time(NULL);

	if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_SET_VALUE, &myKey)) {
		return ( 1 );
	}
	RegSetValueEx ( myKey, "IOTYPE", 0, REG_DWORD, (char*) &dwTime, 4 );
	RegSetValueEx ( myKey, "INTADD", 0, REG_DWORD, (char*) &dwTime, 4 );

	RegCloseKey ( myKey );



	return ( 0 );

}


void RemoveSlash ( char* cPath )

{
	int	nLength;

	nLength = strlen ( cPath );

	if ( *(cPath + nLength - 1) == '\\' ) {
		*(cPath + nLength - 1) = 0;
	}
}
