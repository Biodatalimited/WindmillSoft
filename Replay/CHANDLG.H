// chandlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChannelDlg dialog

class CChannelDlg : public CDialog
{
// Construction
public:
	CChannelDlg();	// standard constructor

// Dialog Data
	//{{AFX_DATA(CChannelDlg)
	enum { IDD = IDD_CHANDIALOG };
	CComboBox	m_cmbColour8;
	CComboBox	m_cmbColour7;
	CComboBox	m_cmbColour6;
	CComboBox	m_cmbColour5;
	CComboBox	m_cmbColour4;
	CComboBox	m_cmbColour3;
	CComboBox	m_cmbColour2;
	CComboBox	m_cmbColour1;
	CComboBox	m_cmbDecPl8;
	CComboBox	m_cmbDecPl7;
	CComboBox	m_cmbDecPl6;
	CComboBox	m_cmbDecPl5;
	CComboBox	m_cmbDecPl4;
	CComboBox	m_cmbDecPl3;
	CComboBox	m_cmbDecPl2;
	CComboBox	m_cmbDecPl1;
	CComboBox	m_cmbChan8;
	CComboBox	m_cmbChan7;
	CComboBox	m_cmbChan6;
	CComboBox	m_cmbChan5;
	CComboBox	m_cmbChan4;
	CComboBox	m_cmbChan3;
	CComboBox	m_cmbChan2;
	CComboBox	m_cmbChan1;
	int		m_nColour1;
	int		m_nColour2;
	int		m_nColour3;
	int		m_nColour4;
	int		m_nColour5;
	int		m_nColour6;
	int		m_nColour7;
	int		m_nColour8;
	int		m_nDecPl1;
	int		m_nDecPl2;
	int		m_nDecPl3;
	int		m_nDecPl4;
	int		m_nDecPl5;
	int		m_nDecPl6;
	int		m_nDecPl7;
	int		m_nDecPl8;
	int		m_nChan1;
	int		m_nChan2;
	int		m_nChan3;
	int		m_nChan4;
	int		m_nChan5;
	int		m_nChan6;
	int		m_nChan7;
	int		m_nChan8;
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CChannelDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeChan1();
	afx_msg void OnSelchangeChan2();
	afx_msg void OnSelchangeChan3();
	afx_msg void OnSelchangeChan4();
	afx_msg void OnSelchangeChan5();
	afx_msg void OnSelchangeChan6();
	afx_msg void OnSelchangeChan7();
	afx_msg void OnSelchangeChan8();
	afx_msg void OnClickedClearAll();
	afx_msg void OnClickedDefaults();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CStringArray* m_astrNames;
	CStringArray* m_astrUnits;
	
public:
	void SetChannels(CStringArray* astrNames, CStringArray* astrUnits);
};
