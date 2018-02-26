// readdlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReadDlg dialog

class CReadDlg : public CDialog
{
// Construction
public:
	CReadDlg(CString strName, CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CReadDlg)
	enum { IDD = IDD_READDIALOG };
	CString	m_strChanName;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CReadDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
