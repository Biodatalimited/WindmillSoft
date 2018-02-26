// playfrm.h : interface of the CReplayFrame class
//
/////////////////////////////////////////////////////////////////////////////

class CReplayFrame : public CFrameWnd
{
protected: // create from serialization only
	CReplayFrame();
	DECLARE_DYNCREATE(CReplayFrame)

// Attributes
public:

protected:
	CSplitterWnd m_wndSplitter;
	
// Operations
public:
	void SetStartTime(const char* pszStartTime);
	
// Implementation
public:
	virtual ~CReplayFrame();
#ifdef _DEBUG
	virtual	void AssertValid() const;
	virtual	void Dump(CDumpContext& dc) const;
#endif


protected:	// control bar embedded members
	CStatusBar	m_wndStatusBar;
	CToolBar	m_wndToolBar;
	
	BOOL OnCreateClient(LPCREATESTRUCT, CCreateContext* pContext);	
    BOOL PreCreateWindow(CREATESTRUCT& cs);
    
// Generated message map functions
protected:
	//{{AFX_MSG(CReplayFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
