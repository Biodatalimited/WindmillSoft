/**
***	LG_PROT.h                ATK-LOGGER
***
***     Andrew Thomson
***
***     This is an include file for the ATK Logger application.
***	This module declares all Function Prototypes for non-static
***	functions.
***
***
***
***	Associated Files : 	
***
***	Version History :  4.01 Addition of DDE Module prototypes
***			   4.02 New function added, GetInternationalSettings
**/
#define  NOMINMAX
#define  NOKANJI
#define  NOCOMM
#define  STRICT         /* Use Windows 3.1 Strict Style Coding  	*/


/**     Message Processing Functions
***
**/
LRESULT CALLBACK Logger_WndProc ( HWND hwnd, WORD wMsg,
					   WPARAM wParam, LPARAM lParam );


LRESULT CALLBACK EditBox_WndProc ( HWND hwnd, WORD wMsg,
					   WPARAM wParam, LPARAM lParam );


/**     Message Crackers....
**/
BOOL  Logger_OnCreate   ( HWND hwnd, CREATESTRUCT * lpCreateStruct );
VOID  Logger_OnSize     ( HWND hwnd, long state, long cx, long cy );
VOID  Logger_OnPaint    ( HWND hWnd );
VOID  Logger_OnClose    ( HWND hWnd );
VOID  Logger_OnDestroy  ( HWND hwnd );
VOID  Logger_OnCommand  ( HWND hwnd, long id, HWND hwndCtl, long codeNotify );
VOID  Logger_OnHScroll  ( HWND hwnd, HWND hwndctl, long code, long pos );
DWORD Logger_OnMenuChar ( HWND hwnd, long ch, long flags, HMENU hMenu );
VOID  Logger_OnSetFocus ( HWND hwnd, HWND hwndOldFocus );
VOID  Logger_OnGetMinMaxInfo ( HWND hwnd, MINMAXINFO *lpMinMaxInfo );
VOID  Logger_OnTimer    ( HWND hwnd, long id );
VOID  Logger_OnKeyDown  ( HWND hwnd, long vk, BOOL fDown,
						long cRepeat, long flags );
VOID  Logger_OnKeyUp    ( HWND hwnd,  long vk, BOOL fDown,
						long cRepeat, long flags );	

/**     Miscellanious Functions
**/

VOID  CreatePushButtons ( HWND hWndParent );
VOID  CreateScrollBar   ( HWND hWndParent );


/**     Functions in Module: LG_PAINT
***
**/
short PositionButtonBar ( HDC hDC );
VOID  PositionScrollBar ( VOID );

short UpdateButtonBar ( HWND hwndParent );
short UpdateMenuBar   ( HWND hwndParent );
short UpdateStatusBar ( HWND hwndParent );

short DrawStatusBar ( HDC hDC );
short DepressRect ( HDC hDC, LPRECT lprectShadedArea );
short DrawTimeDataDivision ( HDC hdcClient );
short DrawTestData ( HDC hdcClient, RECT *prectInvalidated );

short PaintLoggedData  ( HDC hdcClient, RECT *prectInvalidated );
short PaintChanLabels  ( HDC hdcClient );
short PaintRealTimeClk ( HDC hdcClient );

/**     Functions in Module: LG_UTILS
***
**/
void GetFontMetrics ( HWND hwndClient, HFONT hFont,
				long *plHeight, long *plWidth );


short AllowSetupConfiguration   ( HWND hwnd, PIMSSETUP pimssSetup );
short PreventSetupConfiguration ( HWND hwnd, PIMSSETUP pimssSetup );
void  TimeStructToText ( struct tm *ptmGMT, char *szTimeText, WORD wTimeFormat );

short ServiceRealTimeClock ( HWND hwndParent );
void  FormatTime ( char *szFormatedText, DWORD dwTime, WORD wTimeFormat );
void  FormatDate ( char *szFormatedText, DWORD dwTime, WORD wTimeFormat );

VOID  SizeControls ( VOID );

short AddConnectionToArray ( char *szConnection, PCHANNELINFO lpSummary,
						  short nNumSummaryChans );

BOOL  InvokeWinHelp ( HWND hWnd, WORD wCommand, DWORD dwData );

void  GetInternationalSettings ( void );



/**     Functions in Module: LG_HIST.c
**/
short CreateHistoricBuffer  ( VOID );
short DestroyHistoricBuffer ( VOID );
VOID  ReindexHistoricBuffer ( VOID );
VOID  ClearHistoricBuffer   ( VOID );


/**     Functions in Module : LG_LOG.c
**/
short   LogData ( HWND hwnd );
BOOL ReadFirstChannelChange ( HWND hwnd );



/**     Functions in Module: LG_DLG.c
**/
BOOL PASCAL Settings_DlgProc ( HWND hdlg, WORD msg, WPARAM wParam,
							    LONG lParam );

BOOL PASCAL DataFile_DlgProc ( HWND hdlg, WORD msg, WPARAM wParam,
							    LONG lParam );

BOOL PASCAL PeriodicFiles_DlgProc ( HWND hdlg, WORD msg, WPARAM wParam,
							     LONG lParam );

BOOL PASCAL About_DlgProc ( HWND hdlg, WORD msg, WPARAM wParam,
                                                            LONG lParam );

BOOL PASCAL FileExists_DlgProc ( HWND hdlg, WORD msg, WPARAM wParam,
                                                            LONG lParam );



/**     Functions in LG_FILE.c
**/
short	CreateDataFile	 ( BOOL bOpenFirstFile );
short   AppendDataToFile ( VOID );
short	CloseDataFile	 ( BOOL bCloseLastFile );
short 	WriteHeader	 ( HFILE hHeaderFile );
void    GetFileExtension ( char* szExt );
time_t  GetPeriodicFileTime ( BOOL bEnd );
BOOL IsPeriodicFileCurrent ( char* cFileName );
void	StripPeriodicPostFix (void);


/**     Functions in LG_MAIN.c
**/
void ExtractCmdLineArg ( HWND hwndMain, LPSTR lpszCmdLine );
VOID UpdateTitleBar ( HWND hwndMain );




/**	Functions in module LG_DDE.c
**/

short PTKInitializeDDEService ( HWND hwndApplication, char *szServiceName );
short PTKTerminateDDEService ( void );
VOID  PTKAdviseDDE ( short nItem );
short LoadDDECallBackProc ( VOID );
short FreeDDECallBackProc ( VOID );
