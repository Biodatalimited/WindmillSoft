/**
***	LG_HIST.c              ATK-LOGGER
***
***     Andrew Thomson
***
***     This module is responsible for the storage and access of
***     historic logged data....
***
***
***	Associated Files : 	
***
***	Version History :
***	
**/
#define  NOMINMAX
#define  NOKANJI
#define  NOCOMM
#define  STRICT
#define  IML_APPLICATION

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <string.h>

#include "\Windmill 32\Compile Libs\IMLinter.h"   /* ...using the COMMSLIB Dynalink                */
#include "\Windmill 32\Compile Libs\imslib.h"    /* ...using the IMSLIB Dynalink                  */
#include "\Windmill 32\ATKit\ATKit.h"      /* Application Toolkit Header/Prototypes         */
#include "lg_def.h"     /* Manifest Constants and Structure Declarations */
#include "lg_prot.h"    /* Functions Prototypes                          */




extern  CLIENTINFO      clntInfo;


short CreateHistoricBuffer ( VOID )
{
	short           nRow;
	short           nConnection;

	/**     Create the Index which we will use to
	***     access the correct HISTORICDATAROW structure
	***     This will be an array of pointers to HISTORICDATAROW's
	***     We have one structure for every potential row on the
	***     client area.
	**/

	clntInfo.alpHistIndx =  (LPHISTORICDATAROW *) GlobalAllocPtr (
		GMEM_MOVEABLE, (DWORD) clntInfo.lMaxRowsVisible
				* sizeof (LPHISTORICDATAROW *) );

	if ( clntInfo.alpHistIndx == NULL ) {

		return ( ERR_NO_GLOBAL_MEMORY );
	}


	/**     Now Allocate memory for the text strings
	**/
	for ( nRow = 0; nRow < clntInfo.lMaxRowsVisible; nRow++ ) {

		clntInfo.alpHistIndx [ nRow ] = (LPHISTORICDATAROW)
		  GlobalAllocPtr ( GMEM_MOVEABLE, (DWORD)
			               sizeof ( HISTORICDATAROW ) );

		if ( clntInfo.alpHistIndx [ nRow ] == NULL ) {


			/**     No Memory, destroy what we have
			***     created prior to this error
			**/
			DestroyHistoricBuffer ( );
			return ( ERR_NO_GLOBAL_MEMORY );
		}

		/**	Default Fields in Structure
		**/
		clntInfo.alpHistIndx [ nRow ]->szTime [ 0 ] = '\0';

		for ( nConnection = 0; nConnection < MAX_LOGGER_CONNECTIONS;
							nConnection ++ ) {

			/**     Default Data Entries in Table
			**/
			clntInfo.alpHistIndx [ nRow ]
				  ->aszData [ nConnection ] [ 0 ] = '\0';

			clntInfo.alpHistIndx [ nRow ]
				   ->anAlarmState [ nConnection ] =
					ALARM_NORMAL;
		}

	}

	return ( OK );

}



VOID ClearHistoricBuffer ( VOID )
{
	short   nRow;
	short   nConnection;


	for ( nRow = 0; nRow < clntInfo.lMaxRowsVisible; nRow++ ) {

		/**	Default Fields in Structure
		**/
		clntInfo.alpHistIndx [ nRow ]->szTime [ 0 ] = '\0';

		for ( nConnection = 0; nConnection < MAX_LOGGER_CONNECTIONS;
							nConnection ++ ) {

			/**     Default Data Entries in Table
			**/
			clntInfo.alpHistIndx [ nRow ]
				  ->aszData [ nConnection ] [ 0 ] = '\0';

			clntInfo.alpHistIndx [ nRow ]
				   ->anAlarmState [ nConnection ] =
					ALARM_NORMAL;
		}

	}
	return;
}


short DestroyHistoricBuffer ( VOID )
{
	short           nRow;



	if ( clntInfo.alpHistIndx == NULL ) {

		/**     No Buffer Index
		**/
		return ( OK );
	}

	/**    Release memory occupied by the HISTORICDATAROW structures
	**/
	for ( nRow = 0; nRow < clntInfo.lMaxRowsVisible; nRow++ ) {

		if ( clntInfo.alpHistIndx [ nRow ] != NULL ) {

			GlobalFreePtr ( clntInfo.alpHistIndx [ nRow ] );
		}

	}

	/**     Finally destroy the Index itself
	**/
	GlobalFreePtr ( clntInfo.alpHistIndx );
	clntInfo.alpHistIndx = NULL;

	return (OK);

}



VOID ReindexHistoricBuffer ( VOID )
{

	short			nElement;
	short			nLastElement;
	LPHISTORICDATAROW	lpStoredPtr;




	if ( clntInfo.alpHistIndx == NULL ) {

		/**     No Buffer Index
		**/
		return;
	}



	/**     Store the last ptr in the array
	**/
	nLastElement = clntInfo.lMaxRowsVisible - 1;



	lpStoredPtr = clntInfo.alpHistIndx [ nLastElement ];

	/**     Shift all the remaining elements up to fill the space
	***     created in the last element. Fill from the end so we dont
	***     overwrite other valid elements...
	**/
	for ( nElement = nLastElement; nElement > 0; nElement-- ) {

		clntInfo.alpHistIndx [ nElement ] =
				clntInfo.alpHistIndx [ nElement - 1 ];

	}

	/**     The 'Space' has moved to index zero, Put in the stored
	***     pointer to complete the reindexing...
	**/
	clntInfo.alpHistIndx [ 0 ] = lpStoredPtr;


	return;
}
