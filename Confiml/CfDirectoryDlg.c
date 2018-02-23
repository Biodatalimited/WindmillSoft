#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#include "CONFIML.H"
#include "\Windmill 32\Compile Libs\imlinter.h"

/** This was added for Windmill 6.0.
*** The user is asked to store a completely bogus file in a working 
*** directory. This allows us to use the standard file save dialog to
*** get a path to the desired working directory. We save the selected
*** directory in the registry and create sub dirs of Setups and Data.
*** We also create the bogus file there.
**/


short DirectoryDlgFn ( HANDLE hInst, HWND hWndParent )

{
	static OPENFILENAME    	ofnDir;  /* for GetSaveFileName Func */


	static char	szFileName [ MAX_PATH_LENGTH + 100 ];
	static char	szDirName  [ MAX_PATH_LENGTH + 1 ];
	char* cFileTitle = "Windmill.dir";

	char	szDrive [ _MAX_DRIVE + 1 ];
	char	szDir   [ _MAX_DIR   + 1 ];
	char	szFName [ _MAX_FNAME + 1 ];
	char	szExt   [ _MAX_EXT   + 1 ];
	char	szFilter[20];
	int     nResult;
	HKEY	myKey;
	char *	szSubKey = "SOFTWARE\\WINDMILL SOFTWARE\\WINDMILL";
	HANDLE  hFile;

	/** Get the current working directory **/
	szDirName [0] = 0;
	GetIMLDirPath ( szDirName, MAX_PATH_LENGTH );

	/** Put up an explanatory message box **/
	strcpy ( szFileName, "Your current default directory for setups and data is :\n\n" );
	strcat ( szFileName, szDirName );
	strcat ( szFileName, "\n\n Select a new one by saving the file Windmill.dir to it." );

	MessageBox( NULL, szFileName, "Default Directory Selection", MB_OK );
		
	/** Our bogus file **/
	strcpy ( szFileName, "Windmill.dir" );

	strcpy ( szFilter, "*.dir *.dir " );
	szFilter [5] = 0;
	szFilter [11] = 0;


	/**     Envoke the Common File Open Dialog
	***     First initialise the OPENFILENAME structure
	***     members.
	**/

	ofnDir.lStructSize       = sizeof ( OPENFILENAME );
	ofnDir.hwndOwner         = hWndParent;
	ofnDir.hInstance         = hInst;
	ofnDir.lpstrFilter       = szFilter;
	ofnDir.lpstrCustomFilter = (LPSTR) NULL;
	ofnDir.nMaxCustFilter    = 0;
	ofnDir.nFilterIndex      = 0L;
	ofnDir.lpstrFile         = (LPSTR) szFileName;
	ofnDir.nMaxFile          = MAX_PATH_LENGTH;
	ofnDir.lpstrFileTitle    = NULL;
	ofnDir.nMaxFileTitle     = 0;
	ofnDir.lpstrInitialDir   = (LPSTR) szDirName;
	ofnDir.lpstrTitle        = "Default Directory Selection";
	ofnDir.Flags             = OFN_PATHMUSTEXIST |
	                              OFN_FILEMUSTEXIST |
	                              OFN_HIDEREADONLY;
	ofnDir.nFileOffset       = 0;
	ofnDir.nFileExtension    = 0;
	ofnDir.lpstrDefExt       = NULL;
	ofnDir.lCustData         = 0L;
	ofnDir.lpfnHook          = NULL;
	ofnDir.lpTemplateName    = NULL;

	nResult = GetSaveFileName ( &ofnDir );

	if ( nResult == FALSE ) {
		return (0);
	}

	/** Remove the current bogus file **/
	strcat ( szDirName, "\\Windmill.def" );
	remove ( szDirName );

	/** Get the working dir only **/
	_splitpath ( szFileName, szDrive, szDir, szFName, szExt );
	_makepath ( szDirName, szDrive, szDir, NULL, NULL );

	/** Remove the trailing \ **/
	szDirName [ strlen ( szDirName ) - 1 ] = 0;

	/** Store the working dir in the registry.**/
	if ( RegOpenKeyEx ( HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_SET_VALUE, &myKey)) {
		return ( 1 );
	}
	RegSetValueEx ( myKey, "WORKINGDIR", 0, REG_SZ, szDirName, strlen(szDirName) );
	RegCloseKey ( myKey );


	/** Make the extra dirs **/
	strcpy ( szFileName, szDirName );
	strcat ( szFileName, "\\Setups" );
	_mkdir ( szFileName );
	strcpy ( szFileName, szDirName );
	strcat ( szFileName, "\\Data" );
	_mkdir ( szFileName );

	/** Create the bogus file **/
	strcpy ( szFileName, szDirName );
	strcat ( szFileName, "\\Windmill.dir" );

	hFile = CreateFile ( szFileName, GENERIC_WRITE, 0, NULL, 
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	
	/** My old car numbers !! **/
	WriteFile ( hFile, "1306 DK - 563 NHW", 17, &nResult,  NULL );

	CloseHandle ( hFile );

	return ( 0 );

}