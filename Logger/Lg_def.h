/**
***	LG_DEF.h                ATK-LOGGER
***
***	Andrew Thomson
***
***	This is the include file for the ATK Logger application.
***	This module defines manifest constants and structure
***	declarations.
***
***
***
***	Associated Files : 	
***
***	Version History :
***
***	LOGGER-P-4.00		New Version with Improved Features
***
***				MAX_LOGGER_CONNECTIONS increased
***				from 16 to 100
***				Support for IML Alarms.
***
***	LOGGER-P-4.01		Inclusion of DDE server support
***				(see additional SETTINGS fields)
***
***	LOGGER-P-4.02		Some International support added.
***				(see INTERNATIONAL structure)
***
***	LOGGER-P-4.04		DATE_TIME constant added as new
***				time stamp option.
***				New 'Date' field in HISTORICDATAROW
***				Support for Date in INTERNATIONAL
***				structure.
**/
#define  NOMINMAX
#define  NOKANJI
#define  NOCOMM

/**	Column/String Lengths
**/
#define MAX_TIMESTAMP_CHARS		10
#define MAX_DATESTAMP_CHARS		25
#define MAX_DATA_CHARS			15
#define	DEFAULT_COLUMN_WIDTH		( MAX_DATA_CHARS - 3 )
#define DEFAULT_COLUMN_WIDTH_TEXT	"12"

#define OK				0


/**	'Settings' dialog, manifest constants
**/
#define MIN_COLUMN_WIDTH	5
#define MAX_COLUMN_WIDTH	MAX_DATA_CHARS
#define MAX_TITLE		100



#define MAX_LOGGER_CONNECTIONS	100	 /* maximum 100 connections	*/

/** Number of seconds for various time units
*** Included for readability of code.
**/
#define SECS_PER_MINUTE 	60L
#define SECS_PER_HOUR		( SECS_PER_MINUTE * 60L )
#define SECS_PER_DAY		( SECS_PER_HOUR   * 24L	)
#define SECS_PER_WEEK		( SECS_PER_DAY	  * 7L	)




/**	Structure used to hold one row of Historic Data
***
**/
typedef struct HISTORICDATAROWtag {

	/**	Include two additional columns for
	***	historic time & date data.
        **/
	char	szTime  [ MAX_TIMESTAMP_CHARS  + 1 ];
	char	szDate  [ MAX_DATESTAMP_CHARS	+ 1 ];
	char	aszData [ MAX_LOGGER_CONNECTIONS ] [ MAX_DATA_CHARS + 1 ];
	short	anAlarmState [ MAX_LOGGER_CONNECTIONS ];

} HISTORICDATAROW,  *LPHISTORICDATAROW;


typedef *LPHISTORICINDX; /* Ptr to HISTORICDATAROW ptr (for index) */





typedef struct CLIENTINFOtag {


	long	lHeight;		/* Height of Client Area 	*/
	long	lWidth;			/* Width of Client Area  	*/


	long	lFirstChanVisible;	/* Chan in First Visible col  */
	long	lNumColumnsVisible;	/* Visable data Columns	*/
	long	lColumnWidth;		/* Width of col in ave Chars	*/

	long	lNumRowsVisible;	/* Visible data Rows          */
	long	lMaxRowsVisible;	/* Visible Rows IF MAXIMIZED	*/

	short	nDataFieldWidth;
	short	nDataFieldHeight;



	LPHISTORICDATAROW * alpHistIndx;  /* Ptr to Array of Ptrs   */

	long    lTextHeight;              /* Std Text Height (helv_8) 	*/
	long    lAveTextWidth;            /* Std Text Width  (helv_8)   */

	long    lLargeTextHeight;         /* 'Status Bar' Text Height   */
	long    lLargeTextWidth;          /* 'Status Bar' text Width    */



} CLIENTINFO, *PCLIENTINFO;




typedef struct CONNECTIONStag {

	HANDLE		hIMSLibrary;  		/* Handle to IMSLib         */
	short		nIMLInterface;		/* Handle to IML Interface  */

	IMSSETUP	imsSetup;    		/* IMS Setup structure      */
	BOOL		bIMSSetupLoaded;	/* using an IMS Setup       */

	short		nNumConnected;		/* Actual Number Connected  */
	short		nMaxConnections;	/* Max possible connections */
	WORD		wChanTypes;     	/* Types to connect         */

	LOCALHANDLE	hConnections; 		/* Hdl to ATKChanInfo array */
	PATKCHANINFO	pciConnections; 	/* Ptr to ATKChanInfo array */

} CONNECTIONS, *PCONNECTIONS;



typedef struct SETTINGStag {

	/**	Setup file specification
	**/
	char	    szFileSpec [ _MAX_PATH ];

	/**	General logging settings
	**/
	DWORD	dwScanInterval;
	WORD	wTimeStampFormat;
	DWORD   dwLogDuration;     // Milliseconds
	BOOL    bLogOnAlarm;
	BOOL    bLogOnKeyPress;
	BOOL    bLogOnChange;

	BOOL	bScreenDisplayOn;
	BOOL	bIndicateEmptyScans;
	BOOL	bDigitalAsText;
	WORD	wCharsPerColumn;


	/**	File settings
	**/
	BOOL	bLogDataToDisk;
	WORD	wFileFormat;
	// The base date file name with no path
	char	szDataFile      [ _MAX_FNAME + _MAX_EXT + 1 ];
    /* The directory for data with no final \ */
	char	szDataDirectory [ _MAX_PATH + 1 ];
    // The file currently being written to 
	char    szDataFileCurrent   [ _MAX_FNAME + _MAX_EXT + 1 ];
    // The file + path currently being written to
	char    szDataFileFull  [ _MAX_PATH + _MAX_FNAME + _MAX_EXT + 1 ];

	/**	Periodic files
	**/
	BOOL	bPeriodicFiles;
	WORD	wPeriodType;   /* PERIOD_EVERY_INTERVAL,  etc.. */

	WORD    wPostFixType;

	WORD	wNumIntervalUnits;
	WORD	wIntervalUnit; /* MINUTE_UNITS, HOUR_UNITS etc...	*/


	/**	DDE server support
	**/
	BOOL	bDDEEnabled;
	char	szDDEServiceName [ ATK_MAX_DDE_SERVICE_NAME ];


	/**	Miscellaneous
	**/
	char	szTitle [ MAX_TITLE + 1 ];

} SETTINGS, *PSETTINGS;


typedef struct LOGINFOtag {


	BOOL	bIsLogging;
	BOOL	bIsPaused;
	BOOL	bAutoStarted;
	BOOL    bLogPressed;

	DWORD	dwTimeStarted;              /* In MilliSecs since boot */
	DWORD	dwTimeLastScan;             /* Ditto                   */

	time_t	tmCalenderTimeStarted;      /* Calender time as Secs   */
	time_t	tmCalenderTimeStopped;      /* ditto                   */
	time_t	tmCalenderTimeNextFile;	    /* Start new file at.....   */
	
	char    szFirstChannel [ MAX_VALUE + 1 ];

} LOGINFO, *PLOGINFO;




#define HIST_DATA_COL 1
#define HIST_TIME_COL 0




/**	Structure used to hold International Settings
***	These are read from the WIN.INI [ intl ] section
***	and are configured by the user from Control Panel
**/
typedef struct INTERNATIONALtag {

	char	cDecimalSeparator;
	char	cListSeparator;
	char	cTimeSeparator;
	char	cDateSeparator;
	BOOL	bAmericanDateFormat;	/* TRUE if MM/DD/YY */

} INTLINFO;




/**
***     Structure used to hold 'Real Time Clock'
**/
typedef struct  CLOCKINFOtag {

	time_t	timeNow;              /* Time Now in Secs since Jan 1st...*/
	time_t	timeAtLastUpdate;     /* Time when clock was last painted */

} CLOCKINFO;


typedef struct BUTTONBARtag {

	HWND	hButtonWnd;
	DWORD	dwStyle;
	char	*szText;
	long	lXPos;
	long	lYPos;

} BUTTONBAR;


/**
***     Key Dimensions of Client Area Controls
***     Button and Status Bars...
**/
typedef struct CONTROLStag {

	short	nPushButtonHeight;
	short	nPushButtonWidth;
	short	nEditBoxLength;
	short	nEditBoxHeight;
	short	nEditBoxLabelLength;
	short	nBarBorder;
	short	nSingleBarHeight;
	short	nDoubleBarHeight;
	short	nMinSingleBarWidth;
	short	nStatusBarHeight;
	short	nScrollBarHeight;

} CONTROLS;


/**	Timer IDs
**/
#define BACKGROUND_TIMER	1
#define SAMPLE_TIMER        2



/**     Button Bar, Push Button Identifiers...
***
***
**/
#define B_START         0
#define B_STOP          1
#define B_PAUSE         2
#define B_RESUME        3
#define B_LOG           4





/**     Menu Item Identifiers
***
**/
#define IDM_EXIT        	100
#define IDM_LOADIMS     	101
#define IDM_SAVE_SETUP  	102
#define IDM_RESTORE_SETUP	103
#define IDM_INPUTS			104
#define IDM_SETTINGS		105
#define IDM_DDE 			106
#define IDM_DATAFILE		107
#define IDM_HELP_CONTENTS	108
//#define IDM_HELP_ON_HELP	109
#define IDM_ABOUT			110
#define IDM_FILEOPT			111



/**     Client Area Control IDs
**/
#define IDC_FILEEDIT		200
#define IDC_HSCROLL		201
#define IDC_START		202
#define IDC_STOP		( IDC_START + 1 )
#define IDC_PAUSE		( IDC_START + 2 )
#define IDC_RESUME		( IDC_START + 3 )
#define IDC_LOG         ( IDC_START + 4 )


/**
***     Button Bar & Status Bar Definitions
***     These specify the dimensions of buttons etc.
***     We may have to replace these with dimensions
***     calculated from the font size !
***
**/
#define NUM_BUTTONS             5
#define PUSHBUTTON_HEIGHT       20
#define PUSHBUTTON_LENGTH       60
#define EDITBOX_LENGTH          120
#define EDITBOX_HEIGHT          20
#define EDITBOX_LABEL_LENGTH    80
#define BAR_BORDER              2
#define SINGLEBAR_HEIGHT        (PUSHBUTTON_HEIGHT + ( 2 * BAR_BORDER ) )
#define DOUBLEBAR_HEIGHT        ( ( 2 * PUSHBUTTON_HEIGHT ) + \
                                  ( 4 * BAR_BORDER ) )
#define MIN_SINGLEBAR_WIDTH     ( ( PUSHBUTTON_LENGTH * NUM_BUTTONS ) \
                                 + EDITBOX_LENGTH + EDITBOX_LABEL_LENGTH + 5 )



#define STATUSBAR_HEIGHT	 ( ( 11 * SINGLEBAR_HEIGHT / 11 ) + 1 )
#define SCROLLBAR_HEIGHT	16 /* PUSHBUTTON_HEIGHT*/

/**     File Buffer Size, this should be large enough to hold
***     seventeen fields of ascii data plus terminators
**/
#define MAXIMUM_FIELD_SIZE      25
#define DATA_FILE_BUFFER_SIZE   ( MAXIMUM_FIELD_SIZE * ( MAX_LOGGER_CONNECTIONS + 1 ) )


/**
***     Status Bar Manifest Constants
**/
#define STATUS_BAR_ITEM_LENGTH  25
#define NUM_STATUS_BAR_ITEMS     4

/**     The following manifests identify the time string formats
***     and the data file formats. The constants are also used to
***     load the appriopriate descriptions from the String table
***     to be  used in the 'settings' dialog box.
**/
#define NUM_TIMESTAMP_FORMATS	 5	 /* Number of formats		*/
#define TIMESTAMP_BASE           9000   /* IDs start at BASE ID 	*/

#define HOURS_MINS_SECS		TIMESTAMP_BASE
#define SECS_SINCE_START	( TIMESTAMP_BASE + 1 )
#define SECS_AND_HUNDRETHS	( TIMESTAMP_BASE + 2 )
#define MINUTES_AND_SECS	( TIMESTAMP_BASE + 3 )
#define DATE_TIME		( TIMESTAMP_BASE + 4 ) 


#define NUM_FILE_FORMATS	3
#define FILEFORMAT_BASE 	9005	 /* IDs start at BASE ID	 */

#define TAB_SEPARATED		FILEFORMAT_BASE
#define SPACE_SEPARATED		( FILEFORMAT_BASE + 1 )
#define COMMA_SEPARATED		( FILEFORMAT_BASE + 2 )


/**	Interval Units for SETTINGS.wIntervalUnit
**/
#define NUM_INTERVAL_UNITS	4
#define INTERVAL_UNITS_BASE	9008	/* IDs start at BASE ID 	*/

#define MINUTE_UNITS		( INTERVAL_UNITS_BASE + 0 )
#define HOUR_UNITS		( INTERVAL_UNITS_BASE + 1 )
#define DAY_UNITS		( INTERVAL_UNITS_BASE + 2 )
#define WEEK_UNITS		( INTERVAL_UNITS_BASE + 3 )


/**	Period Types for SETTINGS.wPeriodType
**/
#define NUM_FILE_PERIOD_TYPES	4
#define PERIOD_TYPE_BASE	9012	/* IDs start at base ID 	*/


#define PERIOD_EVERY_HOUR	( PERIOD_TYPE_BASE + 1 )
#define PERIOD_EVERY_DAY	( PERIOD_TYPE_BASE + 2 )
#define PERIOD_EVERY_MONTH	( PERIOD_TYPE_BASE + 3 )
            
/**	Scan Interval Uunits for setting dialog ( not stored ) 
***	used to calculate dwScanInterval
**/            
#define NUM_SCANINTERVAL_UNIT_TYPES 	2
#define SCANINTERVAL_UNITS_BASE		9016
#define SCANINTERVAL_UNITS_SECS		( SCANINTERVAL_UNITS_BASE + 0 )
#define SCANINTERVAL_UNITS_MINS		( SCANINTERVAL_UNITS_BASE + 1 ) 


/** Postfix Types
**/
#define POSTFIX_TYPE_BASE	9020	/* IDs start at base ID 	*/


#define POSTFIX_HH	        ( POSTFIX_TYPE_BASE + 0 )
#define POSTFIX_DDHH    	( POSTFIX_TYPE_BASE + 1 )
#define POSTFIX_MMDDHH	    ( POSTFIX_TYPE_BASE + 2 )
#define POSTFIX_YYMMDDHH	( POSTFIX_TYPE_BASE + 3 )



            

/**     Dialog Box Manifests
**/
#define MAX_EDITBOX_STRLEN         128
#define MAX_COMBOBOX_STRLEN        40


/**     Default file extensions
**/
#define DEF_COMBINED_EXT       "wl"
#define DEF_HDRFILE_EXT        "wh"
#define DEF_DATAFILE_EXT       "wd"



/**     HELP NAME & CONTEXTS, USED WHEN INVOKING HELP APPLICATION
**/
#define HELP_FILE_NAME          "LOGGER"
#define HELP_FILE_EXT           "HLP"

#define HC_SETTINGSHELP		1
#define HC_FILESETTINGSHELP	2
#define HC_PERIODICFILES	3

/**     MANIFEST CONSTANTS USED TO READ\WRITE APPL SETUP FILE.
**/
#define APPL_SETUP_SECTIONNAME     "Application0"
#define MAX_KEY_LENGTH             _MAX_PATH



/**	Manifests constants used by GetMillisecondCount function (LG_UTILS)
**/
#define  TIMER_CTRL	0x43		/* System 8254 control register   */
#define  COUNT_0	0x40		/* System 8254 Counter 0 data reg */



/**     Micellanous Manifest Constants
**/
#define MIN_INTERVAL	20L	  /* Min Scan Interval in Millisecs 	*/
#define MAX_INTERVAL	86400000L /* max Interval 24 Hours 		*/



/**     Logger Application Internal Error Codes
***
**/
#define ERR_INVALID_INTERVAL    	1000
#define ERR_NO_LOCAL_MEMORY     	1001
#define ERR_NO_GLOBAL_MEMORY    	1002
#define ERR_FILE_CREATION       	1003
#define ERR_FILE_WRITE          	1004
#define ERR_INVALID_FILENAME    	1005
#define ERR_CHAN_NOT_IN_SUMMARY 	1006
#define ERR_USER_ABORTED        	1007
#define ERR_INVALID_CMDLINE_ARG 	1008
#define ERR_ALL_CONNECTIONS_MISSING	1009
#define ERR_ALL_CHANS_IN_ERROR		1010
#define ERR_DEMO_VIOLATED		1011
#define ERR_UNABLE_TO_SAVE_SETUP	1012
#define ERR_NON_PERIODIC_FILE_SPEC	1013
#define ERR_RESET_VALUE_TOO_HIGH	1014
#define ERR_RESET_VALUE_TOO_LOW 	1015
#define ERR_TOO_MANY_UNIT_INTERVALS	1016
#define ERR_TOO_FEW_UNIT_INTERVALS	1017
#define ERR_NOT_PROTECT_MODE		1018

#define DDE_ERR_NOT_PROTECT_MODE	1019
#define DDE_ERR_INIT_FAILED		1020
#define DDE_ERR_NO_GLOBAL_MEMORY	1021
#define DDE_ERR_DUPLICATE_SERVICE_NAME	1022
