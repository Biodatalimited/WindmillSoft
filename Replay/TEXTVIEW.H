// textview.h : interface of the CTextView class
//

/////////////////////////////////////////////////////////////////////////////

class CTextView : public CView
{
	DECLARE_DYNCREATE(CTextView)
protected:
	CTextView();			// protected constructor used by dynamic creation

// Attributes
public:
    CReplayDoc* GetDocument();
    
protected:
	int m_nFirstScan;
	int m_nFirstChan;
	int m_nNumLines;
	int m_nNumCols;
	int m_nLineHeight;
	
// Operations
public:

// Implementation
public:
	virtual ~CTextView();
	void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual	void OnDraw(CDC* pDC);		// overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG	// debug version in textview.cpp
inline CReplayDoc* CTextView::GetDocument()
   { return (CReplayDoc*) m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
