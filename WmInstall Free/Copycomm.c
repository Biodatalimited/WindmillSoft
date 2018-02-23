#include "stdafx.h"
#include <windows.h>
#include <stdio.h>                                                           
#include <stdlib.h>
#include <string.h>
#include "\Windmill 32\Compile Libs\imlinter.h"


extern char cImlini [];

/** Allow for 32 programs. Seems plenty ! **/
SOFTWARE SoftwareEntry [32];


/***********************************************************
*** The computer code is a 16 bit value formed by reading the
*** date in Seconds since 1971 which was stored in the registry at
*** installation time.
*** This is a 32 bit value we convert to 16 bits by exclusive OR
*** of the upper and lower bytes.
***
*** The original version of this used the date of the system dir
*** but Microsoft started changing this.
**/

int GetComputerCode ( void )

{
	char * szSubKey = "SYSTEM\\CURRENTCONTROLSET\\SERVICES\\MLGPIB\\PARAMETERS";
	HKEY myKey;
	DWORD dwType;
	DWORD dwSize = 4; 
	int nComputerCode;


	/** Read the code from the registry. **/
	if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_QUERY_VALUE, &myKey)) {
		return ( 0 );
	}
	RegQueryValueEx ( myKey, "IOTYPE", NULL, &dwType, (char*) &nComputerCode, &dwSize );
	RegCloseKey ( myKey );

 
	/** Convert to 16 bits **/
	nComputerCode = nComputerCode ^ ( nComputerCode >> 16L );
	nComputerCode = nComputerCode & 0x0000FFFF;

	return ( nComputerCode );


}


/******************************************************************
*** The software code is a value stored in IML.INI which indicates
*** if the software is authorised or not. The value is written as :
*** eg. Logger = 12345
*** When this code is converted to an integer then
*** Bits 0 to 7 carry the months since JAN 99 install date
*** Bits 8 to 15 are the code derived below 
*** Bits 16 - 31 are 0.
*** The code is formed by adding together the numerical values of
*** the title characters, the total being multiplied by 2 after
*** each addition. It is then added to the computer code and the
*** unwanted bits are blanked. This code stored in IML.INI authorises
*** the software. Any other code is unauthorised.
**/


int CalculateSoftwareCode ( char* cProgramTitle )

{
	int nIndex;
	int nCode = 0;
	
	for (nIndex = 0; nIndex < (int) strlen ( cProgramTitle ); ++nIndex ) {

		nCode = nCode + cProgramTitle [nIndex];
		nCode = 2 * nCode;
	}

	nCode = nCode + GetComputerCode();

	nCode = nCode << 3;   //Added for ver 6

	nCode = nCode & 0x0000FF80;

	return (nCode);
}



/*******************************************************
*** This routine reads all the data from the SOFTWARE section of
*** a .INI or .INF file and stores it in the SoftwareEntry array 
*** of structures.
*** The data consists of
***       szName   Program Name
***       nCode    Authorisation Code
***       nDate    Install Date in Months since JAN 99.
***
*** It returns TRUE if it succeeds, FALSE if it fails
**/


int GetSoftwareDetailsFromINI ( char * cFile )

{
	char szRet [200];
	DWORD dwRet;
	int nIndex = 0;
	char* szProgramName;
	int nValue;
	
	/** This causes all the Key names of the SOFTWARE section of
	*** IML.INI to be written into the szRet buffer. It copies the
	*** keys in separated by NULL chars. The final key has 2 NULLs
	*** to mark the end. If the buffer is too small it fits in what
	*** it can and terminates with 2 NULLs giving buffer size - 2
	*** chars which is the return value.
	**/
	dwRet = GetPrivateProfileString ( "SOFTWARE", NULL, "", 
		                             szRet, 200, cFile );

	/** This is the return value if the buffer was too small **/
	if ( dwRet == 198 ) {
		return ( FALSE );
	}

	// It copies a NULL string in if it fails
	if ( strlen ( szRet) == 0 ) {
		
		// Copy in NULL to mark end of names
		strcpy ( SoftwareEntry[0].szName, ""); 
		
		return ( FALSE );
	}

	// Take a pointer to the buffer
	szProgramName = szRet;

		
	// A NULL string means end of names
	while ( strlen ( szProgramName ) != 0 ) {
		
		if ( strlen ( szProgramName ) > 19 ) {
			return ( FALSE );
		}

		// Copy the name into the structure
		strcpy ( SoftwareEntry [nIndex].szName, szProgramName );
	
		// Move to the start of the next name
		szProgramName = szProgramName + strlen(szProgramName) + 1;

		// Move to the next structure
		nIndex = nIndex + 1;
	}


	// Put in NULL to mark the end of data
	strcpy ( SoftwareEntry[nIndex].szName, "" );

	/** Now for each name in turn read its Key and fill in the
	*** rest of the structure
	**/

	nIndex = 0;

	while ( strlen ( SoftwareEntry[nIndex].szName ) != 0 ) {
		
		// Read the entry for the key
		GetPrivateProfileString ( "SOFTWARE", 
			SoftwareEntry[nIndex].szName, "None", szRet, 200, cFile );

		// Convert to a number
		nValue = atoi ( szRet );

		// Store the codes
		SoftwareEntry[nIndex].nCode = nValue & 0x0000FFF80 ;
		SoftwareEntry[nIndex].nDate = nValue & 0x00000007F ;

		// Move to the next
		++nIndex;
	}


	return ( TRUE );

}


/********************************************************
***
**/

int ReadSoftwareCode ( char* cProgramTitle )

{
	int nIndex;
	

	if ( GetSoftwareDetailsFromINI ( cImlini ) == FALSE ) {
		return ( 0 );
	}

	nIndex = 0;

	/** Look for the requested software and return its code **/
	while ( strlen ( SoftwareEntry[nIndex].szName ) != 0 ) {
		
		if ( stricmp (SoftwareEntry[nIndex].szName, cProgramTitle) == 0 ) {
			return ( SoftwareEntry[nIndex].nCode );
		}
		// Move to the next
		++nIndex;
	}

	/** Not found **/
	return (0);
}
