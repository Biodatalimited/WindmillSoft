// readdlg.cpp : implementation file
//

#include "stdafx.h"
#include "replay.h"
#include "readdlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReadDlg dialog

CReadDlg::CReadDlg(CString strName, CWnd* pParent /*=NULL*/)
	: CDialog(CReadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReadDlg)
	m_strChanName = strName;
	//}}AFX_DATA_INIT

	Create(IDD);
}

void CReadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReadDlg)
	DDX_Text(pDX, IDC_READCHAN, m_strChanName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CReadDlg, CDialog)
	//{{AFX_MSG_MAP(CReadDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReadDlg message handlers
