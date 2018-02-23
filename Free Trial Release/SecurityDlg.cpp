// SecurityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Security.h"
#include "SecurityDlg.h"
#include "Enter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


int UndoKey ( char* cBuffer, int* ptnPrograms, int* ptnInvalidKeys,
				int* ptnDate, int* ptnComputerCode, int* ptnReleases);
void CalculateKey ( char* cBuffer, int nPrograms, int nInvalidKeys,
				   int nDate, int nComputerCode, int nReleases); 

void Alpha2_Alpha ( char* cBuffer );

/////////////////////////////////////////////////////////////////////////////
// CSecurityDlg dialog

CSecurityDlg::CSecurityDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSecurityDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSecurityDlg)
	m_ComputerCode = _T("");
	m_Date = _T("");
	m_Invalid = _T("");
	m_Releases = _T("");
	m_ReleaseKey = _T("");
	m_Programs = _T("");
	m_EntryCode = _T("");
	m_User_Key = _T("");
	m_Cust = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSecurityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSecurityDlg)
	DDX_Text(pDX, IDC_COMPUTER_CODE, m_ComputerCode);
	DDX_Text(pDX, IDC_DATE, m_Date);
	DDX_Text(pDX, IDC_INVALID, m_Invalid);
	DDX_Text(pDX, IDC_RELEASE, m_Releases);
	DDX_Text(pDX, IDC_RELEASE_KEY, m_ReleaseKey);
	DDX_Text(pDX, IDC_INSTALLED_PROGS, m_Programs);
	DDX_Text(pDX, IDC_USER_KEY, m_User_Key);
	DDX_Text(pDX, IDC_CUST_LABEL, m_Cust);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSecurityDlg, CDialog)
	//{{AFX_MSG_MAP(CSecurityDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSecurityDlg message handlers

BOOL CSecurityDlg::OnInitDialog()
{
	
	
	CDialog::OnInitDialog();


	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSecurityDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSecurityDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSecurityDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	
	char cBuffer [20];
	char cTemp [20];
	int nPrograms, nInvalidKeys;
	int nDate, nComputerCode, nReleases, nMonth, nYear;
	int nError = FALSE;
	int nTemp;
	CFile myFile;
	CFileException fileException;
	char szBuffer [256];
	char cKey [] = "7654321";
	char cCustomer [40];


	Enter dlg;
//	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();


	
	dlg.GetEntry (cBuffer, cCustomer);
	
	m_User_Key = cBuffer;
	m_Cust = cCustomer;

	if ( strlen ( cCustomer ) == 0 ) {

		MessageBox ( "No Customer Entry", "Code Entry", MB_OK |
						       MB_ICONEXCLAMATION );
		nError = TRUE;
		return;
	}

	Alpha2_Alpha (cBuffer);


	if ( strlen ( cBuffer ) < 7 ) {
		MessageBox ( "Too Few Characters in Code", "Code Entry", MB_OK |
						       MB_ICONEXCLAMATION );
		nError = TRUE;
		return;
	}

	if ( strlen ( cBuffer ) > 7 ) {
		MessageBox ( "Too Many Characters in Code", "Code Entry", MB_OK |
						       MB_ICONEXCLAMATION );
		nError = TRUE;
		return;
	}

	/** Calculate and display the values in the key **/
	if ( UndoKey ( cBuffer, &nPrograms, &nInvalidKeys,
		&nDate, &nComputerCode, &nReleases ) == FALSE ) {
		
		MessageBox ( "Illegal Characters in Code", "Code Entry", MB_OK |
						       MB_ICONEXCLAMATION );
		nError = TRUE;
		return;
	}
	
	itoa ( nPrograms, cBuffer, 10);
	m_Programs = cBuffer;
		
	itoa ( nInvalidKeys, cBuffer, 10);
	m_Invalid = cBuffer;

	nTemp = nDate + 128;	//Adjust for recent data

	nMonth = nTemp % 12 + 1;
	nYear = nTemp / 12 + 1999;

	itoa ( nMonth, cTemp, 10);
	strcat ( cTemp, " : ");
	itoa ( nYear, cBuffer, 10 );
	strcat ( cTemp, cBuffer );

	m_Date = cTemp;

	itoa ( nComputerCode, cBuffer, 10);
	m_ComputerCode = cBuffer;
	
	itoa ( nReleases, cBuffer, 10);
	m_Releases = cBuffer;
	
    nTemp = nReleases + 1;
	if ( nTemp > 7 ) {
		nTemp = 0;
	}

	/** Calculate the new release key **/
	CalculateKey ( cBuffer, nPrograms, 0, 
		           nDate, nComputerCode, nTemp );

	strcpy ( cTemp, cBuffer + 8);

	m_ReleaseKey = cTemp;
	
	if ( nError == FALSE) {
		UpdateData (FALSE);
	}

	/** Save the data to file **/

	if ( !myFile.Open( "c:\\Licences ComIML.txt", CFile::modeWrite ), &fileException )
	{

	}

	CTime theTime = CTime::GetCurrentTime();
	CString cDateEntry = theTime.Format ("%d %m %y");

	strcpy ( szBuffer, (LPCTSTR) cDateEntry );

	strcat ( szBuffer, "\t");
	strcat ( szBuffer, cTemp );
	strcat ( szBuffer, "\t" );
	strcat ( szBuffer, cCustomer );
	strcat ( szBuffer, "\n\r\n\r" );


	myFile.SeekToEnd( );
	myFile.Write( szBuffer, strlen( szBuffer ) ); 

	myFile.Close();

}







/** Code to convert 5 bit binary to alpha characters **/
const char cAlphaCode [33] = "nhg53bqzc9twmeiayp2fj8u6kxdv74sr";

/** Code used in request Key **/
const char cAlpha2 [33] =	 "RS47VDXK6U8JF2PYALEMWT9CZQB35GHN";


void Alpha2_Alpha ( char* cBuffer )

{
	int nByte;
	int nIndex;

	for ( nByte=0; nByte < 8; ++nByte ) {

		for ( nIndex = 0; nIndex < 32; ++ nIndex ) {
			if ( cAlpha2[nIndex] == cBuffer[nByte] ) {
				cBuffer[nByte] = cAlphaCode [nIndex];
				break;
			}
		}	
	}
}

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
	cBuffer [7] = 0;

	/** Bytes 8 to 15 are null terminated string for licence release **/
	cBuffer [8] = cAlpha2 [  dwCode        & 0x0000001F ];
	cBuffer [9] = cAlpha2 [ (dwCode >> 5)  & 0x0000001F ];
	cBuffer [10] = cAlpha2 [ (dwCode >> 10) & 0x0000001F ];
	cBuffer [11] = cAlpha2 [ (dwCode >> 15) & 0x0000001F ];
	cBuffer [12] = cAlpha2 [ (dwCode >> 20) & 0x0000001F ];
	cBuffer [13] = cAlpha2 [ (dwCode >> 25) & 0x0000001F ];
	cBuffer [14] = cAlpha2 [ (dwCode >> 30) + ( nReleases << 2) ];
	cBuffer [15] = 0;


	return ;


}
/************************************************************
*** This function simply reverses the previous one.
**/

int UndoKey ( char* cBuffer, int* ptnPrograms, int* ptnInvalidKeys,
				int* ptnDate, int* ptnComputerCode, int* ptnReleases)

{
	DWORD dwCode = 0;
	int nIndex;

	for ( nIndex = 0; nIndex < 7; nIndex ++ ) {
		if ( AlphaDecode ( cBuffer [nIndex] ) == -1 ) {
			*ptnPrograms = 0;
			*ptnInvalidKeys = 0;
			*ptnDate = 0;
			*ptnComputerCode = 0;
			*ptnReleases = 0;
			return ( FALSE );
		}
	}

	/** Recover the 32 bit code **/
	dwCode = (DWORD) AlphaDecode ( cBuffer [0] );
	dwCode = dwCode | ( (DWORD) AlphaDecode ( cBuffer [1] ) << 5  );
	dwCode = dwCode | ( (DWORD) AlphaDecode ( cBuffer [2] ) << 10 );
	dwCode = dwCode | ( (DWORD) AlphaDecode ( cBuffer [3] ) << 15 );
	dwCode = dwCode | ( (DWORD) AlphaDecode ( cBuffer [4] ) << 20 );
	dwCode = dwCode | ( (DWORD) AlphaDecode ( cBuffer [5] ) << 25 );
	dwCode = dwCode | ( (DWORD) AlphaDecode ( cBuffer [6] ) << 30 );

	/** Recover the number of releases **/
	*ptnReleases = (AlphaDecode ( cBuffer [6] )) >> 2;

	/** Derotate the 32 bit word **/
	for ( nIndex = 0; nIndex < *ptnReleases; ++nIndex ) {
		if ( dwCode & 0x00000001 ) {
			dwCode = ( dwCode >> 1 ) + 0x80000000;
		} else {
			dwCode =  dwCode >> 1;
		}
	}

	/** Computer code in lower 15 bits **/
	*ptnComputerCode = dwCode & 0x00007FFF;

	/** Number of programs in 15 - 19 **/
	*ptnPrograms = ( dwCode >> 15 ) & 0x0000001F ;


	/** Number of invalid in 20 - 24 **/ 
	*ptnInvalidKeys = ( dwCode >> 20 ) & 0x0000001F ;

	/** Date Code in 25 - 31 **/
	*ptnDate = ( dwCode >> 25 ) & 0x0000007F ;

	return ( TRUE );

}
