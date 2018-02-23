// SecurityDlg.h : header file
//

#if !defined(AFX_SECURITYDLG_H__40DE52A7_D8C3_11D2_82CD_444553540000__INCLUDED_)
#define AFX_SECURITYDLG_H__40DE52A7_D8C3_11D2_82CD_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CSecurityDlg dialog

class CSecurityDlg : public CDialog
{
// Construction
public:
	CString m_Cust;
//	cfile m_File;
	CSecurityDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSecurityDlg)
	enum { IDD = IDD_SECURITY_DIALOG };
	CString	m_ComputerCode;
	CString	m_Date;
	CString	m_Invalid;
	CString	m_Releases;
	CString	m_ReleaseKey;
	CString	m_Programs;
	CString	m_EntryCode;
	CString	m_User_Key;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSecurityDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSecurityDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SECURITYDLG_H__40DE52A7_D8C3_11D2_82CD_444553540000__INCLUDED_)
