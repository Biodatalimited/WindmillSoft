// Enter.cpp : implementation file
//

#include "stdafx.h"
#include "Security.h"
#include "Enter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Enter dialog


Enter::Enter(CWnd* pParent /*=NULL*/)
	: CDialog(Enter::IDD, pParent)
{
	//{{AFX_DATA_INIT(Enter)
	m_UserCode = _T("");
	m_Customer = _T("");
	//}}AFX_DATA_INIT
}


void Enter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Enter)
	DDX_Text(pDX, IDC_CODE_ENTER, m_UserCode);
	DDX_Text(pDX, IDC_CUSTOMER, m_Customer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Enter, CDialog)
	//{{AFX_MSG_MAP(Enter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Enter message handlers

void Enter::OnOK() 
{
	// TODO: Add extra validation here
	
	UpdateData ( TRUE);
	
	CDialog::OnOK();
}

int Enter::DoModal() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDialog::DoModal();
}

void Enter::GetEntry( char* cCode, char* cCust )
{
	strcpy ( cCode, m_UserCode );
	strcpy ( cCust, m_Customer );
}

BOOL Enter::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CDialog::Create(IDD, pParentWnd);
}
