#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>




/*****************************************************/
/** The function writes to the sub key which hes been previously created
*** by Install shield. It simply puts the time in.
*** It is called by 
**/


CHAR WINAPI WmSetKey ( HWND hwnd, LPSTR szSrcDir, LPSTR szSupport,
					   LPSTR szInst, LPSTR szRes )
{
	
	char * szSubKey = "SYSTEM\\CURRENTCONTROLSET\\SERVICES\\MLGPIB\\PARAMETERS";
	HKEY myKey;
	DWORD dwTime;

	dwTime = time(NULL);

	if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_SET_VALUE, &myKey)) {
		return ( 0 );
	}
	RegSetValueEx ( myKey, "DMATYPE", 0, REG_DWORD, (char*) &dwTime, 4 );
	RegCloseKey ( myKey );

	return ( 1 );

}
	
/***********************************************************
*** This routine reads the original key.
*** It limits its size so that we don't overflow.
**/

int _declspec (dllexport)  WINAPI WmGetKey ( DWORD* pdwKey )

{

	char * szSubKey = "SYSTEM\\CURRENTCONTROLSET\\SERVICES\\MLGPIB\\PARAMETERS";
	HKEY myKey;
	DWORD dwType;
	DWORD dwSize;

	dwSize = 4;

	if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_QUERY_VALUE, &myKey)) {
		*pdwKey = 0; 
		return ( 0 );
	}

	RegQueryValueEx ( myKey, "DMATYPE", 0, &dwType, (char*) pdwKey, &dwSize );		
	
	RegCloseKey ( myKey );


	/** Make sure that we can convert to 2 ints **/
	*pdwKey = *pdwKey & 0x7fff7fff;

	return ( 1 );

}


/***********************************************************
*** This routine reads the original key.
*** It limits its size so that we don't overflow.
**/

int _declspec (dllexport)  WINAPI WmPutKey ( void )

{

	char * szSubKey = "SYSTEM\\CURRENTCONTROLSET\\SERVICES\\MLGPIB\\PARAMETERS";
	HKEY myKey;
	DWORD dwTime;
	DWORD dwResult;

	dwTime = time(NULL);


	if ( RegCreateKeyEx ( HKEY_LOCAL_MACHINE, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, 
							KEY_ALL_ACCESS, NULL, &myKey, &dwResult )) { 
		return ( 0 );
	}

	RegSetValueEx ( myKey, "DMATYPE", 0, REG_DWORD, (char*) &dwTime, 4 );
		
	
	RegCloseKey ( myKey );


	return ( 1 );

}






