// replay.cpp : Defines the class behaviors for the application.
//


/**********************************************
*** History
***
*** Version 5.00 Sept 99 produced by 32 bit conversion
*** same as 16 bit but modified to handles seconds from start format
***
*** Version 5.01 Sept 99 produced by adding Help file support !
**/

#include "stdafx.h"
#include "replay.h"

#include "playfrm.h"
#include "chandlg.h"
#include "playdoc.h"
#include "playview.h"
#include "\Windmill 32\Compile Libs\imslib.h"
#include "\Windmill 32\Compile Libs\imlinter.h"
#include <direct.h>
#include <htmlhelp.h>


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern const int nMinRange = 50;
extern const int nAxisWidth = 1200;
extern const int nMaxWinExtent = 30000;
//extern const int nXAxisHeight = 600;

WORD unSerialNum;
	
/////////////////////////////////////////////////////////////////////////////
// CReplayApp

BEGIN_MESSAGE_MAP(CReplayApp, CWinApp)
	//{{AFX_MSG_MAP(CReplayApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_HELP_CONTENTS, OnHelpContents)
	ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
	ON_COMMAND(ID_HELP_HOWTOUSEHELP, OnHelpHowToUseHelp)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplayApp construction

CReplayApp::CReplayApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CReplayApp object

CReplayApp NEAR theApp;

/////////////////////////////////////////////////////////////////////////////
// CReplayApp initialization

BOOL CReplayApp::InitInstance()
{

    char cDir [_MAX_PATH];

	/**
	***     Determine if the user has a valid release copy
	***     of the software. If not put up MessageBox.
	**/
    if ( !IMSClear ( "Replay" ) ) {

		/**	Copy protection has been invalidated
		**/
		MessageBox ( NULL, 
		"        This Program is NOT an Authorised Copy. \r\r    It will however function correctly for this Session. \r\r See Confiml Software Section for Authorisation Details",
					"Replay Security Warning",
					 MB_OK );
						    

		
	}
	
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	SetDialogBkColor();        // set dialog background color to gray
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	AddDocTemplate(new CSingleDocTemplate(IDR_MAINFRAME,
			RUNTIME_CLASS(CReplayDoc),
			RUNTIME_CLASS(CReplayFrame),     // main SDI frame window
			RUNTIME_CLASS(CReplayView)));

	EnableShellOpen();
	RegisterShellFileTypes();

	// create a new (empty) document and main SDI frame window
	OnFileNew();
    	
	
	// Open logger file
	if (m_lpCmdLine[0] == '\0')
	{

		/** Set the working dir **/
		GetIMLDirPath ( cDir, _MAX_PATH );
		strcat ( cDir, "\\Data");
		_chdir ( cDir);
		OnFileOpen();
	}
	else
	{
		OpenDocumentFile(m_lpCmdLine);
	}

	// Quit if file not opened
	CReplayView* pView = (CReplayView*)((CFrameWnd*)m_pMainWnd)->GetActiveView();
	CReplayDoc* pDoc = pView->GetDocument();
	return (pDoc->FileOpened());
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_SerialNo;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
//	m_SerialNo.Format("Serial Number %u", unSerialNum);
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
//	DDX_Text(pDX, IDC_SERIALNO, m_SerialNo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CReplayApp::OnAppAbout()
{

	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}






/////////////////////////////////////////////////////////////////////////////
// CReplayApp commands

void CReplayApp::OnHelpContents()
{
	WinHelp(0L, HELP_CONTENTS);	
}

void CReplayApp::OnHelpHowToUseHelp()
{
	WinHelp(0L, HELP_HELPONHELP);	
}

void CReplayApp::OnHelpIndex()

{
	
	char	szDrive [ _MAX_DRIVE ];
	char	szDir [ _MAX_DIR ];
	char	szFName [ _MAX_FNAME ];
	char	szExt [ _MAX_EXT ];
	char	szPath [ _MAX_PATH ];

	/** Get the full path to the help file **/
	GetModuleFileName ( theApp.m_hInstance, szPath, _MAX_PATH );

	_splitpath ( szPath, szDrive, szDir, szFName, szExt );
	_makepath ( szPath, szDrive, szDir, szFName, ".CHM" );

	
	HtmlHelp ( 0L, szPath, HH_DISPLAY_TOPIC, 0 );

}



