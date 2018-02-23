/** This file contains the code used to implement the software
*** security system. 
**/



#include <windows.h>

#include <stdio.h>                                                           
#include <stdlib.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <time.h>
#include "Resource.h"
#include "Confiml.h"
#include "Copycomm.h"



extern GENERAL_INFO giData;	/* our general information structure. */
extern SOFTWARE SoftwareEntry [32];
extern char cImlini [];






/** Code to convert 5 bit binary to alpha characters **/
const char cAlphaCode [33] = "nhg53bqzc9twmeiayp2fj8u6kxdv74sr";


/*************************************************
*** This routine is used to convert alpha code back to 5 bit binary.
*** It is passed a single alpha code character. It scans the alpha
*** code string and returns the equivalent 5 bit code. If the char
*** is not alpha code it returns -1.
**/



int AlphaDecode ( char cCode )

{
	int nIndex;

	for ( nIndex = 0; nIndex < 32; ++nIndex ) {

		if ( cAlphaCode[nIndex] == cCode ) {
			return ( nIndex);
		}
	}

	return ( -1 );


}






/*********************************************************
***
**/

int GetNumberofReleases (void)

{
	int nRet;
	char cBuffer[10];

	/** See if we have a HWX key **/
	nRet = GetPrivateProfileString ( "Hardware", "HWX", "", 
		                             cBuffer, 10, cImlini );
			
	/** If we don't then create it with releases set to 0 **/
	if ( nRet == 0 ) {
		return (0); 		
	} else {
		return ( AlphaDecode ( cBuffer[0] ) & 0x00000007 );
	}

}

/*********************************************************
*** This routine authorises all programs stored in the structure by
*** writing their codes  to IML.INI. The act of writing adds them if
*** they are not already present in IML.INI It assumes that the 
*** SoftwareEntry structure has already been filled. It returns true 
*** if it succeeds, false if it fails.
**/


int AuthoriseAllPrograms ( int nReleases )

{	
	int nIndex = 0;
	int nNewCode;
	int nRet;
	char cBuffer [18];

	// for each program in turn
	while ( strlen ( SoftwareEntry[nIndex].szName ) != 0 ) {

		// Calculate the new code
		nNewCode = CalculateSoftwareCode ( SoftwareEntry[nIndex].szName );


		// Update the structure
		SoftwareEntry[nIndex].nCode = nNewCode;

		// Add in the date
		nNewCode = nNewCode + SoftwareEntry[nIndex].nDate;
	
		// Convert to string
		itoa ( nNewCode, cBuffer, 10 );

		// Write to IML.INI
		nRet = WritePrivateProfileString ( "SOFTWARE",
			SoftwareEntry[nIndex].szName, cBuffer, cImlini );
	
		// If there is an error
		if ( nRet == 0 ) {
			return (FALSE);
		}

		// Move to next program
		++ nIndex;
	}


	if ( nReleases > 7 ) {
		nReleases = 0;
	}

	cBuffer[0] = cAlphaCode [ 24 + nReleases ];
	cBuffer[1] = cAlphaCode [ 16 + nReleases ];
	cBuffer[2] = cAlphaCode [ 8 + nReleases  ];
	cBuffer[3] = 0;

	WritePrivateProfileString( "Hardware", "HWX", cBuffer , cImlini );		


	return (TRUE);
}


/*********************************************************
*** This routine calculates the 35 bit release key.
*** First we calculate a 32 bit word as follows
*** Bits 0 - 14   Lower 15 bits of computer code
*** Bits 15 - 19  Number of Programs 0 - 31
*** Bits 20 - 24  Number of unauthorised  0 - 31
*** Bits 25 - 31  Windmill Date in months since Jan 1999 0 - 128
***
*** We then get the number of releases from IML.INI.
*** We use it to rotate left the 32 bit word with the overflow
*** rotating into the LSB.
*** We convert to 7 characters of alpha code
*** First char   bits 0 - 4      MSB at this end
*** Second char  bits 5 - 9
*** etc to
*** Seventh char bits 30,31 + 3 bits of Releases
***
*** We return code 0000000 if there is a problem
**/


void CalculateKey ( char* cBuffer, int nPrograms, int nInvalidKeys,
				   int nDate, int nComputerCode, int nReleases) 

{
	DWORD dwCode ;
	int nIndex = 0;



	/** An all 0 code means there is something wrong **/ 
	strcpy ( cBuffer, "0000000" );


	/** Reduce to 15 bits **/
	dwCode = nComputerCode & 0x00007FFF ;

	/** Reduce to 5 bits **/
	nPrograms = nPrograms & 0x0000001F ;
	nInvalidKeys = nInvalidKeys & 0x0000001F ;

	/** Add in at 15 to 19 bits **/
	dwCode = dwCode | ( nPrograms << 15 );
	/** Add in at 20 to 24 bits **/
	dwCode = dwCode | ( nInvalidKeys << 20 );


	/** Add in the Windmill Date **/
	dwCode = dwCode | ( nDate << 25 );


	/** Rotate dwCode to the left by the number of releases
	*** Overflow back into LSB.
	**/
	for ( nIndex = 0; nIndex < nReleases; ++nIndex ) {
		if ( dwCode & 0x80000000 ) {
			dwCode = ( dwCode << 1 ) + 1;
		} else {
			dwCode =  dwCode << 1;
		}
	}

	cBuffer [0] = cAlphaCode [  dwCode        & 0x0000001F ];
	cBuffer [1] = cAlphaCode [ (dwCode >> 5)  & 0x0000001F ];
	cBuffer [2] = cAlphaCode [ (dwCode >> 10) & 0x0000001F ];
	cBuffer [3] = cAlphaCode [ (dwCode >> 15) & 0x0000001F ];
	cBuffer [4] = cAlphaCode [ (dwCode >> 20) & 0x0000001F ];
	cBuffer [5] = cAlphaCode [ (dwCode >> 25) & 0x0000001F ];
	cBuffer [6] = cAlphaCode [ (dwCode >> 30) + ( nReleases << 2) ];
	

	return ;


}



/***************************************************************
*** The software section contains
*** ML750=1234
*** etc to
*** Year=2000
*** Month=5
*** Duration=3
**/

int InstallSoftwareFromINF ( char* cFile )

{
	int nIndex = 0;
	int nYear, nMonth, nDuration;
	int nYearIndex;
	int nCurrentDate, nInstallDate;
	char cDate [11];
	char cBuffer [19];
	char			szSysDir [ _MAX_PATH ];
	struct _stat	statBuf;
	int nTime;
	int nRet;
	int nReleases;
	


	/** Fill the software structure with the details from the 
	*** INF file.
	**/
	if ( GetSoftwareDetailsFromINI ( cFile ) == FALSE ) {
		return (FALSE);
	}
	

	nReleases = GetNumberofReleases ();
	
	/** First task is to obtain the Install Date in months
	*** since JAN 99 format. It is stored in keys :
	*** Years  : 1999 etc
	*** Months : 1 to 12 etc
	**/
	
	/** Find years key. This appears as one of the software entries
	*** Since its a value over 2000 we have split it artifically into
	*** code and date
	**/
	while ( stricmp ( SoftwareEntry[nIndex].szName, "Year" ) != 0 ) {
		
		/** Something wrong if we have a NULL string **/
		if ( strlen ( SoftwareEntry[nIndex].szName ) == 0 ) {
			return ( FALSE );
		}

		++nIndex;
	}

	/** Store the year index for later deletion **/
	nYearIndex = nIndex;

	/** Undo the split of code and data **/
	nYear = SoftwareEntry[nIndex].nCode + SoftwareEntry[nIndex].nDate; 
	
	/** Next entry should be month. Value is in date only **/
	if ( stricmp ( SoftwareEntry[nIndex + 1].szName, "Month" ) != 0 ) {
		return (FALSE);
	}
	nMonth = SoftwareEntry[nIndex + 1].nDate;

	/** Next entry should be Duration **/
	if ( stricmp ( SoftwareEntry[nIndex + 2].szName, "Duration" ) != 0 ) {
		return (FALSE);
	}
	/** Value is in date only **/
	nDuration = SoftwareEntry[nIndex + 2].nDate;

	/** Convert to months since Jan 1999 format **/
	nInstallDate = (nYear - 1999) * 12 + nMonth - 1;

	
	/** We now have the install date and duration. We must remove all
	*** trace of them from the .INF file and make entries in
	*** IML.INI for the software.
	**/

	/** Delete the software keys from the .INF **/
	WritePrivateProfileString ( "SOFTWARE", "Year", NULL, cFile );
	WritePrivateProfileString ( "SOFTWARE", "Month",NULL,cFile );
	WritePrivateProfileString ( "SOFTWARE", "Duration", NULL, cFile );
	
	/** Remove year,month,duration from the structure else
	*** they get an entry in IML.INI !
	**/
	strcpy ( SoftwareEntry[nYearIndex].szName, "" );
	strcpy ( SoftwareEntry[nYearIndex + 1].szName, "" );
	strcpy ( SoftwareEntry[nYearIndex + 2].szName, "" );
		
	nIndex = 0;

	/** for each program in turn. Make the INI entry. The code used
	*** contains the correct date but the remainder of the code is
	*** a random number as supplied by the original INF file. This
	*** means that the software is not authorised at this point.
	**/
	while ( strlen ( SoftwareEntry[nIndex].szName ) != 0 ) {

		// Update the structure
		SoftwareEntry[nIndex].nDate = nInstallDate & 0x7F;
	
		// Convert the full code to string. Limit the date to lower 7 bits
		itoa ( SoftwareEntry[nIndex].nCode + ( nInstallDate & 0x7F ) , cBuffer, 10 );

		// Write to IML.INI
		nRet = WritePrivateProfileString ( "SOFTWARE",
			SoftwareEntry[nIndex].szName, cBuffer, cImlini );
	
		// If there is an error
		if ( nRet == 0 ) {
			return (FALSE);
		}

		// Move to next program
		++ nIndex;
	}

	/** Read the current date as "mm/dd/yy" and work out the
	*** current date in our months since JAN 99 format
	**/
	_strdate ( cDate );
	
	/** Work out months and years **/	
	nYear = atoi ( cDate + 6 );
	cDate[2] = 0;
	nMonth = atoi ( cDate ); 
	
	/** Convert the date to months since JAN 99 **/	
	if ( nYear > 40) {
		nCurrentDate = (nYear - 99) * 12 + nMonth - 1 ;
	} else {
		nCurrentDate = ( nYear + 1 ) * 12 + nMonth - 1;
	}

	/** Check that the software is still valid **/
	
	// This is never valid
	if ( nDuration == 0 ) {
		return (FALSE);
	}
	
	
	// CurrentDate less than disc install date !
	if ( nCurrentDate < nInstallDate ) {
		return (FALSE);
	}

	// nDuration = 1 means to the end of the month.
	if  ( nCurrentDate >= ( nInstallDate + nDuration ) ) {
		return ( FALSE );
	}


	/** Read the system directory date. Must be prior to
	*** the current date. This stops people winding back
	*** the date of a new computer.
	**/
	/** Find the system directory **/
	GetSystemDirectory ( szSysDir, _MAX_PATH );

	/** Get its details **/
	_stat ( szSysDir, &statBuf );

	/** Get seconds since 1971 **/
	nTime =  statBuf.st_ctime;


	/** If the current date is less than the system dir date **/
	if ( nTime > time (NULL) ) {
		return (FALSE);
	}

	/** This authorises all programs from the .INF rather than all
	*** programs in IML.INI.
	**/
	nRet = AuthoriseAllPrograms ( nReleases) ;

	/** Write to IML.INI to deauthorize ComIML **/
	WritePrivateProfileString ( "SOFTWARE","ComIML", "25181", cImlini );
		

	return (nRet);

}


