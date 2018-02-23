/**	CONFIML.H	Nigel Roberts.		27 June, 1991
***
*** IML Configuration program CONFIML.
***
*** Main Include file.
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




/** Constants used in the program.
**/


#define VERSION 		"ConfIML 7.01"




#define MAX_DEVICES				30

#define MAX_QUESTION_LENGTH		80
#define MAX_ANSWER_LENGTH		20
#define MAX_DESC_LENGTH 		80
#define MAX_NAME_LENGTH 		80
#define MAX_FIELD_LENGTH		30
#define MAX_DEFAULT_LENGTH		30
#define MAX_PATH_LENGTH 		80
#define MAX_EXENAME				12

#define MAX_MODULES				18

#define MAX_QUESTIONS			10

#define MAX_OPTIONS				20
#define MAX_OPTION_LENGTH		20

#define QUESTION_OPTION 		1
#define QUESTION_ANSWER 		2


#define DEV_FREE				0
#define DEV_330X				1
#define DEV_WINAPP				2

#define DEV_X_WINAPP			3 /* extended winapp - dll setup */




#define NOT_KNOWN				-1


#define FAIL_INITIALISE 		100
#define FAIL_READ_HARDWARE		110
#define FAIL_READ_IML			120
#define FAIL_WRITE_IML			130
#define IDS_REMOVE				140
#define IDS_EXIT				150
#define IDS_RESTART_WINDOWS		160

#define NO_MEMORY				170
#define FAIL_INSTALL_SOFTWARE   171
#define FAIL_READ_REGISTRY      172



#define MOD_FREE		     '\0'
#define MOD_INUSE		     'I'


#define UNIT_MODULE			0
#define UNIT_DEVICE			1





/** Data structure definitions.
**/

typedef struct QuestionTag {
	short		nType;		/* Type of question */
	short		nOptions;	/* Number of answer options */
	char		szQn [ MAX_QUESTION_LENGTH ];	/* The question */
	char		szAns [ MAX_ANSWER_LENGTH ];	/* The answer */
	char		szField [ MAX_FIELD_LENGTH ];	/* the IML.INI field*/
	char		szDefault [ MAX_DEFAULT_LENGTH ];  /*the default ans*/
	char		aszOpt [ MAX_OPTIONS ][ MAX_OPTION_LENGTH ];
	char		aszOptAns [ MAX_OPTIONS ][ MAX_OPTION_LENGTH ];
	short		nCurrentOpt;

} QUEST, * PQUEST, * LPQUEST ;



typedef struct	UserDeviceTag {

	char		szDesc [ MAX_DESC_LENGTH ];
	short		nQns;
	HANDLE		haqnQn;
	char		szExeName [ MAX_PATH_LENGTH ];
	char		szName [ MAX_NAME_LENGTH ];
	short		nDeviceNo;
	short		nModuleNo;
	short		nType;

	char		szHWName [ MAX_EXENAME ];

} USER_DEV, * PUSER_DEV,  * LPUSER_DEV;





typedef struct HWDeviceTag {

	short		nUnit;

	short		nDeviceType;
	char		szName [ MAX_NAME_LENGTH ];
	char		szDesc [ MAX_DESC_LENGTH ];
	char		szInstallDir [ MAX_PATH_LENGTH ];
	char		szExeName [ MAX_EXENAME ];
	short		nModules;
	HANDLE		hamodModule;

	HINSTANCE	hInstDLL;


} HWDEV, * PHWDEV, * LPHWDEV;



typedef struct IMLDeviceTag {
	char	acModule [ MAX_MODULES ];
	short	nType;
	char	szExeName [ MAX_EXENAME ];

	char		szHWName [ MAX_EXENAME ];

} IMLDEV, * PIMLDEV;



typedef struct GeneralTag {
	HANDLE		hahwHardware;
	HANDLE		haudUserDev;
	short		nUserDevices; //The number of lines in the initial display dialog
	short		nIMLDevices;  //The number of IML devices
	short		nHWDevices;   //Number of potential device types. ie size of ADD dialog
	char		szSetupPath [ MAX_PATH_LENGTH ];
	char		szInstallDir [ MAX_PATH_LENGTH ];
	short		bChangedConfig;
	HANDLE		hInst;
	IMLDEV		aimlDev [ MAX_DEVICES ];
	short		nSetDevice;

	short		bAutoAdd;  //Indicates start up with a command line argument

	short		bAddingDevice;

	HCURSOR 	hHourGlass;

} GENERAL_INFO;





/** Function prototypes.
**/


/** cfmain.c
**/

//int WINAPI WinMain ( HANDLE hInst, HANDLE hPrevInst, LPSTR lpszCmdLine,
//							      int nCmdShow );

void CenterDialog ( HWND hDlg );



/** CFINIINF.C
**/
short	ReadIMLDevices ( void );
short	GetHardWareDetails ( LPSTR lpszCommandLine );
short	WriteIMLINI ( void );



/** CFUTIL.C
**/
char *	Detokenise ( char * lpszWithTokens, char *szTokensReplaced, short
								nMod );

char *	Despace ( char * szStr );
short	AddModuleToHWDevice ( PHWDEV phwDev, PUSER_DEV pudDev );
short	AddQuestionToUserDevice ( PUSER_DEV pudDev, LPQUEST pquQuest);
short	AddHardwareToList ( PHWDEV phwDev );
short	AddUserDeviceToList ( PUSER_DEV pudDev );
short	GetHWStructForUserDev ( PUSER_DEV pudDev, PHWDEV phwDev );
short	RemoveUserDeviceFromList ( short nDev );
short	CopyUserDevice ( PUSER_DEV pudTo, PUSER_DEV pudFrom );
short	GetDeviceAndModuleNumbers ( PUSER_DEV pudDev );
short	IsNewHardware ( PHWDEV phwDev );


void HandleError ( short nIDMess );




/** CFDEVDLG.C
**/

BOOL WINAPI SelectDeviceDlgFn ( HWND hDlg, WORD wMessage, WPARAM wParam,
							LONG lParam );

int InvokeWinHelp ( HWND hDlg, WORD wCommand );




/** CFADDDLG.C
**/

BOOL WINAPI AddDeviceDlgFn ( HWND hDlg, WORD wMessage, WPARAM wParam,
							LONG lParam );






/** CFSETDLG.C
**/

BOOL WINAPI SettingsDlgFn ( HWND hDlg, WORD wMessage, WORD wParam,
							LONG lParam );

HANDLE	CreateSettingsDlg ( short nLevel );


/** Copyprot.c **/
BOOL WINAPI SoftwareDlgFn ( HWND hDlg, WORD wMessage, WPARAM wParam,
							LONG lParam );

BOOL WINAPI ReleaseDlgFn ( HWND hDlg, WORD wMessage, WPARAM wParam,
							LONG lParam );

short DirectoryDlgFn ( HANDLE hInst, HWND hWndParent );
