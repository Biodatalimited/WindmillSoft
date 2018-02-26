// playview.h : interface of the CReplayView class
//
/////////////////////////////////////////////////////////////////////////////

class CReplayDoc;		// Forward declaration

class CReplayView : public CView
{
protected: // create from serialization only
	CReplayView();
	DECLARE_DYNCREATE(CReplayView)

// Attributes
public:
	CReplayDoc* GetDocument();
	
protected:
	int m_nXLeft;
	int m_nXRight;
	int m_nILeft;
	int m_nIRight;
	int m_nLeftMargin;
	int m_nRightMargin;
	int m_nTopMargin;
	int m_nBottomMargin;
	int m_nDragType;
	int m_nDrawAxes;
	int m_nXAxisHeight;
	long m_lDX;
	long m_lXFloor;
	long m_lXCeiling;
	long m_lXLeft;
	long m_lXRight;
	float m_flResolution;
	float m_flScaleFactor;
	CString m_strUnits;
	CRgn m_rgnClip;
	CRect m_rectGraph;
	CRect m_rectXAxis;
	CRect m_rectMouse;
	CRect m_rectCursor;
	
// Operations
public:
	int GetILeft() { return m_nILeft; }
	int GetIRight() { return m_nIRight; }
	int GetLogXLeft() { return m_nXLeft; }
	int GetLogXRight() { return m_nXRight; }

// Implementation
public:
	virtual ~CReplayView();
	void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	void Zoom(CRect rectZoom);
	void SetClipRect(CDC* pDC, CRect rectClip);
	void StartPlay(int nInterval);
	void Play();
	
	// Printing support
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Generated message map functions
protected:
	//{{AFX_MSG(CReplayView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnZoomOut();
	afx_msg void OnZoomOutBoth();
	afx_msg void OnZoomOutX();
	afx_msg void OnZoomOutY();
	afx_msg void OnReplayGotoStart();
	afx_msg void OnReplayGotoEnd();
	afx_msg void OnReplayForward();
	afx_msg void OnReplayReverse();
	afx_msg void OnReplayFastForward();
	afx_msg void OnReplayFastReverse();
	afx_msg void OnReplayStop();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG	// debug version in playview.cpp
inline CReplayDoc* CReplayView::GetDocument()
   { return (CReplayDoc*) m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
