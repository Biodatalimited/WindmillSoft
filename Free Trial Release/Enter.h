#if !defined(AFX_ENTER_H__98DF3261_DE06_11D2_82CD_444553540000__INCLUDED_)
#define AFX_ENTER_H__98DF3261_DE06_11D2_82CD_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Enter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Enter dialog

class Enter : public CDialog
{
// Construction
public:
	CString m_Customer;
	 void GetEntry( char* cBuffer, char* cCust);
	Enter(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(Enter)
	enum { IDD = IDD_ENTER_DIALOG };
	CString	m_UserCode;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Enter)
	public:
	virtual int DoModal();
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Enter)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENTER_H__98DF3261_DE06_11D2_82CD_444553540000__INCLUDED_)
