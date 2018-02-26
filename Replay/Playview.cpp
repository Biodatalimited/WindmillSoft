// playview.cpp : implementation of the CReplayView class
//

#include "stdafx.h"

#include "resource.h"

#include <afxpriv.h>

#include "yaxis.h"
#include "channel.h"
#include "playview.h"
#include "colordlg.h"
#include "chandlg.h"
#include "playdoc.h"
#include "round.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

enum {ZoomDrag, CursorDrag};
enum {TimerId = 1};

/////////////////////////////////////////////////////////////////////////////
// CReplayView

IMPLEMENT_DYNCREATE(CReplayView, CView)

BEGIN_MESSAGE_MAP(CReplayView, CView)
	//{{AFX_MSG_MAP(CReplayView)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_COMMAND(ID_ZOOM_OUT, OnZoomOut)
	ON_COMMAND(ID_ZOOM_OUT_BOTH, OnZoomOutBoth)
	ON_COMMAND(ID_ZOOM_OUT_X, OnZoomOutX)
	ON_COMMAND(ID_ZOOM_OUT_Y, OnZoomOutY)
	ON_COMMAND(ID_REPLAY_GOTOSTART, OnReplayGotoStart)
	ON_COMMAND(ID_REPLAY_GOTOEND, OnReplayGotoEnd)
	ON_COMMAND(ID_REPLAY_FORWARD, OnReplayForward)
	ON_COMMAND(ID_REPLAY_REVERSE, OnReplayReverse)
	ON_COMMAND(ID_REPLAY_FASTFORWARD, OnReplayFastForward)
	ON_COMMAND(ID_REPLAY_FASTREVERSE, OnReplayFastReverse)
	ON_COMMAND(ID_REPLAY_STOP, OnReplayStop)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplayView construction/destruction

CReplayView::CReplayView()
{
   	m_rgnClip.CreateRectRgn(0, 0, 0, 0);
}

CReplayView::~CReplayView()
{
	m_rgnClip.DeleteObject();
}

/////////////////////////////////////////////////////////////////////////////
// CReplayView drawing

void CReplayView::OnInitialUpdate()
{
	extern const int nMaxWinExtent;
	
	CReplayDoc* pDoc = GetDocument();
	
    POSITION pos = pDoc->GetFirstChanPos();
    if (pos == NULL) return;					// File not yet opened
    CChannel* pChan = pDoc->GetNextChan(pos);	// First channel is Time
    
    m_flResolution = pChan->GetResolution();
    
    m_lXFloor = MakeLong(pChan->GetMinVal() / m_flResolution);
    m_lXCeiling = MakeLong(pChan->GetMaxVal() / m_flResolution);
    if (pChan->GetDecPlaces() >= 0)
    {
    	Round(m_lXFloor, m_lXCeiling);
    }
    else
    {
    	RoundTime(m_lXFloor, m_lXCeiling);
    }
    
    m_flScaleFactor = (float)nMaxWinExtent / (float)(m_lXCeiling - m_lXFloor);
	pChan->ScaleChannel(m_flScaleFactor, m_lXFloor, m_flResolution);

    m_lXLeft = m_lXFloor;
    m_lXRight = m_lXCeiling;
    m_nXLeft = 0;
    m_nXRight = nMaxWinExtent;
	m_nILeft = 0;
	m_nIRight = pDoc->GetScanCount() - 1;
	
	SetScrollRange(SB_HORZ, 0, nMaxWinExtent, FALSE);
	SetScrollPos(SB_HORZ, 0, TRUE);
	SetScrollRange(SB_VERT, 0, nMaxWinExtent, FALSE);
	SetScrollPos(SB_VERT, nMaxWinExtent, TRUE);
    
	pDoc->SetXAxis("Time");
	
//	for (pos = pDoc->GetFirstYAxisPos(); pos != NULL;)
//	{
//		CYAxis* pYAxis = pDoc->GetNextYAxis(pos);
//		pYAxis->ScaleAxis;
//	}
	
   	m_nLeftMargin = 0;
   	m_nRightMargin = 600;
   	m_nTopMargin = 200;
   	m_nBottomMargin = 50;
   	
   	m_nDrawAxes = TRUE;
   	
    CView::OnInitialUpdate();
}
    
void CReplayView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CReplayDoc* pDoc = GetDocument();
	
	int nCursor = pDoc->GetCursor();
	if ((nCursor >= 0) && !((nCursor > m_nILeft) && (nCursor < m_nIRight)))
	{
		// Cursor is not in view - scroll to cursor position
    	POSITION pos = pDoc->GetFirstChanPos();
    	CChannel* pChan = pDoc->GetNextChan(pos);	// First channel is Time
		int nPos = pChan->GetCoords()[nCursor];
		
		nPos = max((nPos - ((m_nXRight - m_nXLeft) / 2)), (0));
		
		OnHScroll(SB_THUMBPOSITION, nPos, NULL);
	}
	
	Invalidate();
}

void CReplayView::OnDraw(CDC* pDC)
{
	extern const int nAxisWidth;
	
	CReplayDoc* pDoc = GetDocument();
	
	if ((pDoc->GetFirstChanPos()) == NULL) return;	// File not yet opened
	
	COLORREF Colour;
	CRect rectClientDP;
	if (pDC->IsPrinting())
	{
		Colour = RGB(0, 0, 0);
		rectClientDP.SetRect(0, 0, pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
	}
	else
	{
		Colour = RGB(255, 255, 255);
		GetClientRect(rectClientDP);
	}
	
	// Convert client rectangle to logical coordinates
	CRect rectClient(rectClientDP);
	pDC->SetMapMode(MM_TWIPS);
	pDC->SetViewportOrg(0, rectClient.Height());
	pDC->SetWindowOrg(0, 0);
	pDC->DPtoLP(rectClient);
	
	// Set pen, font, etc.
	
	CPen NewPen;
	NewPen.CreatePen(PS_SOLID, 0, Colour);
	CPen* pOldPen = pDC->SelectObject(&NewPen);
	
	pDC->SetTextColor(Colour);
	pDC->SetBkMode(TRANSPARENT);
	
	CFont NewFont;
	NewFont.CreateFont(0, 							// Height
					   0,							// Width
					   0,							// Escapement
					   0,							// Orientation
					   FW_DONTCARE,					// Weight
					   0,							// Italic
					   0,							// Underline
					   0,							// Strikeout
					   ANSI_CHARSET,				// Character Set
					   OUT_DEFAULT_PRECIS,			// Output Precision
					   CLIP_DEFAULT_PRECIS,			// Clipping Precision
					   PROOF_QUALITY,				// Quality
					   DEFAULT_PITCH | FF_DONTCARE,	// Pitch and Family
					   "System");					// Face Name
					   
//	CFont* pOldFont = pDC->SelectObject(&NewFont);
	
	// Find height of a line of text
	TEXTMETRIC tmFont;
	pDC->GetTextMetrics(&tmFont);
	m_nXAxisHeight = (2 * tmFont.tmHeight) + tmFont.tmExternalLeading + 100;

	// Calculate position of trend graph rectangle
	m_rectGraph.SetRect(m_nLeftMargin + (pDoc->GetAxisCount() * nAxisWidth),
						rectClient.top - m_nTopMargin,
						rectClient.right - m_nRightMargin,
						m_nBottomMargin + m_nXAxisHeight);
						
	pDC->LPtoDP(m_rectGraph);
	
	m_rectGraph.left += 1;
	
	// Calculate position of X-axis rectangle
	m_rectXAxis.SetRect(0, m_nBottomMargin + m_nXAxisHeight - 100, rectClient.right, m_nBottomMargin);
	pDC->LPtoDP(m_rectXAxis);
	
	// Set up coordinate system for trend graphs						
		
	pDC->SetMapMode(MM_ANISOTROPIC);
	pDC->SetViewportOrg(m_rectGraph.left, m_rectGraph.bottom);
	pDC->SetViewportExt(m_rectGraph.Width(), -m_rectGraph.Height());
							
	// Clip drawing to graph area
	SetClipRect(pDC, m_rectGraph);
	
	// Draw trends
	POSITION pos = pDoc->GetFirstChanPos();
	CChannel* pChan = pDoc->GetNextChan(pos);
	while (pos != NULL)
	{
		pChan = pDoc->GetNextChan(pos);
		pChan->DrawTrend(pDC, this);	
	}
	
//	pDC->SelectClipRgn(NULL);
	SetClipRect(pDC, rectClientDP);

	// Draw cursor
	int nCursor = pDoc->GetCursor();
	if ((nCursor > m_nILeft) && (nCursor < m_nIRight))
	{
		pDC->SetWindowOrg(m_nXLeft, 0);
		pDC->SetWindowExt((m_nXRight - m_nXLeft), 1);
		m_rectCursor.left = (*(pDoc->GetCoords() + nCursor)).x;
		m_rectCursor.right = m_rectCursor.left;
		m_rectCursor.top = 1;
		m_rectCursor.bottom = 0;
		pDC->LPtoDP(&m_rectCursor);
		m_rectCursor.right = m_rectCursor.left + 1;
		pDC->SetMapMode(MM_TEXT);
		pDC->SetWindowOrg(m_rectGraph.left, m_rectGraph.bottom);
		pDC->InvertRect(m_rectCursor);
	}
	else
	{
		m_rectCursor.SetRectEmpty();
	}		
	
//	// Draw frame round trends
//	pDC->SetWindowOrg(0, 0);
//	pDC->SetWindowExt(1, 1);
//	static CPoint aptTrendFrame[4] = { CPoint(0, 1), CPoint(1, 1),
//									   CPoint(1, 0), CPoint(0, 0) };

//	CPen NewPen;
//	NewPen.CreatePen(PS_SOLID, 0, RGB(64, 64, 64));
//	CPen* pOldPen = pDC->SelectObject(&NewPen);
//	pDC->Polyline(aptTrendFrame, 4);
//	pDC->SelectObject(pOldPen);
	
if (m_nDrawAxes == TRUE)
{	
	// Draw Y-axes
	pDC->SetMapMode(MM_TWIPS);
	pDC->SetViewportOrg(0, 0);
	pDC->SetWindowOrg(0, rectClient.top);
	CRect rectFrame;
	pos = pDoc->GetFirstYAxisPos();
	for ( int i = 0; pos != NULL; i++)
	{
		CYAxis* pYAxis = pDoc->GetNextYAxis(pos);
		rectFrame.SetRect(m_nLeftMargin + (i * nAxisWidth),
						  rectClient.top - m_nTopMargin,
						  m_nLeftMargin + ((i + 1) * nAxisWidth),
						  m_nBottomMargin + m_nXAxisHeight);
		pYAxis->DrawAxis(pDC, rectFrame);
	}
	
	// Draw X-axis
	rectFrame.SetRect(rectFrame.right, rectFrame.bottom,
					  (rectClient.right - m_nRightMargin), m_nBottomMargin);
					  
	// Draw horizontal axis
	pDC->MoveTo(rectFrame.left, rectFrame.top);
	pDC->LineTo(rectFrame.right, rectFrame.top);
	
	// Draw tick marks
	CRect rectLabel;
	CString strValue;
	CTime tm;
	CTimeSpan ts;
	
	long lWidth = m_lXRight - m_lXLeft;
	pos = pDoc->GetFirstChanPos();
	CChannel* pTime = pDoc->GetNextChan(pos);
	// 5 or 6 Ticks
	int nTicks = ((pTime->GetDecPlaces() >= 0) || (lWidth > 691200L)) ? 5 : 6;
	
	for (i = 0; i <= nTicks; i++)
	{
		int j = rectFrame.left + (int)(((long)i * (rectFrame.right - rectFrame.left)) / nTicks);
		pDC->MoveTo(j, (rectFrame.top - 100));
		pDC->LineTo(j, rectFrame.top);
		
		rectLabel.SetRect(j - 500, rectFrame.top - 100, j + 500, rectFrame.bottom);
				
		long lTickVal = m_lXLeft + (i * (lWidth / nTicks));
		
		// Use to prepare string values
		char szValue[20];

		if (nTicks == 5)
		{
			if (pTime->GetDecPlaces() >= 0)
			{
								
				sprintf(szValue, "%g", lTickVal * m_flResolution);
//				j = strlen(szValue) - 1;
//				if (szValue[j] == '.') szValue[j] = 0;
				strValue = szValue;
				m_strUnits = "Seconds";
			}
			else if (pTime->GetDecPlaces() == -2)
			{
				ts = lTickVal;
				strValue = ts.Format("%D");			
				m_strUnits = "Days";
			}
			else if ( pTime->GetDecPlaces() == -3 )
			{
				
				sprintf ( szValue, "%i%s%i", 
					(int) (lTickVal / 60) , ":", (int) ( lTickVal % 60 ));
				strValue = szValue;
				m_strUnits = "Time (mm:ss)";
			}

			else
			{
				// DARAD format
				tm = CChannel::m_TimeOrigin + CTimeSpan(lTickVal);
				strValue = tm.Format("%d %b %y");
				m_strUnits = "Date";
			}
		}
		else if (lWidth > 600L)
		{
			if (pTime->GetDecPlaces() == -2)
			{
				ts = lTickVal;
				strValue = ts.Format("%H:%M");
				m_strUnits = "Time (hh:mm)";
			}
			else if ( pTime->GetDecPlaces() == -3 )
			{
				sprintf ( szValue, "%i%s%i", 
					   (int)(lTickVal / 60), ":", (int)(lTickVal % 60) );
				strValue = szValue;
				m_strUnits = "Time (mm:ss)";
			}

			else
			{
				// DARAD format
				tm = CChannel::m_TimeOrigin + CTimeSpan(lTickVal);
				strValue = tm.Format("%H:%M");
				m_strUnits = "Time (hh:mm)";
			}
		}
		else
		{
			if (pTime->GetDecPlaces() == -2)
			{
				ts = lTickVal;
				strValue = ts.Format("%H:%M:%S");
				m_strUnits = "Time (hh:mm:ss)";
			}
			else if ( pTime->GetDecPlaces() == -3 )
			{
				sprintf ( szValue, "%i%s%i", 
					(int) (lTickVal / 60) , ":", (int) ( lTickVal % 60 ));
				strValue = szValue;
				m_strUnits = "Time (mm:ss)";
			}
			else
			{
				// DARAD format
				tm = CChannel::m_TimeOrigin + CTimeSpan(lTickVal);
				strValue = tm.Format("%H:%M:%S");
				m_strUnits = "Time (hh:mm:ss)";
			}
		}
		
		if ((nTicks == 5) || ((i % 2) == 0))
		{
			pDC->DrawText(strValue, -1, rectLabel, DT_TOP | DT_CENTER | DT_SINGLELINE | DT_NOCLIP);
		}
	}
	
	// Label Axis
	pDC->DrawText(m_strUnits, -1, rectFrame, DT_BOTTOM | DT_CENTER | DT_SINGLELINE);

	if ((nTicks == 6) && (m_lXCeiling > 86400L))
	{
		if (pTime->GetDecPlaces() == -2)
		{
			ts = m_lXLeft;
			strValue = ts.Format("Day %D");
		}
		else
		{
			// DARAD format
			tm = CChannel::m_TimeOrigin + CTimeSpan(m_lXLeft);
			strValue = tm.Format("%d %b");
		}
		rectLabel.SetRect(rectFrame.left - 500, rectFrame.top, rectFrame.left + 1000, rectFrame.bottom);
		pDC->DrawText(strValue, -1, rectLabel, DT_BOTTOM | DT_LEFT | DT_SINGLELINE);
		
		if (pTime->GetDecPlaces() == -2)
		{
			ts = m_lXRight;
			strValue = ts.Format("Day %D");
		}
		else
		{
			// DARAD format
			tm = CChannel::m_TimeOrigin + CTimeSpan(m_lXRight);
			strValue = tm.Format("%d %b");
		}
		rectLabel.SetRect(rectFrame.right - 1000, rectFrame.top, rectFrame.right + 500, rectFrame.bottom);
		pDC->DrawText(strValue, -1, rectLabel, DT_BOTTOM | DT_RIGHT | DT_SINGLELINE);
	}                                                               
}
	pDC->SelectObject(pOldPen);
//	pDC->SelectObject(pOldFont);
}

/////////////////////////////////////////////////////////////////////////////
// CReplayView printing

BOOL CReplayView::OnPreparePrinting(CPrintInfo* pInfo)
{
	pInfo->SetMaxPage(1);
	pInfo->m_nNumPreviewPages = 1;
	
	return DoPreparePrinting(pInfo);
}

void CReplayView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: add extra initialization before printing
}

void CReplayView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}




/////////////////////////////////////////////////////////////////////////////
// CReplayView diagnostics

#ifdef _DEBUG
void CReplayView::AssertValid() const
{
	CView::AssertValid();
}

void CReplayView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CReplayDoc* CReplayView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CReplayDoc)));
	return (CReplayDoc*) m_pDocument;
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CReplayView message handlers

BOOL CReplayView::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClip;
	pDC->GetClipBox(rectClip);
	
    CBrush brushBkgnd;
    brushBkgnd.CreateSolidBrush(RGB(64, 64, 64));
    
    pDC->FillRect(rectClip, &brushBkgnd);
    
	return TRUE;
}

void CReplayView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rectCursorVicinity = m_rectCursor;
	rectCursorVicinity.InflateRect(2, 0);
	
	if (rectCursorVicinity.PtInRect(point))
	{
		SetCapture();
		m_nDragType = CursorDrag;
	}
}

void CReplayView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture() != this) return;
	
	ReleaseCapture();
	
	CClientDC dc(this);
	
	// Remove any existing cursor from screen
	dc.InvertRect(m_rectCursor);
	
	if (m_rectGraph.PtInRect(point))
	{
		// Find nearest sample to mouse position
		dc.SetMapMode(MM_ANISOTROPIC);
		dc.SetViewportOrg(m_rectGraph.left, m_rectGraph.bottom);
		dc.SetViewportExt(m_rectGraph.Width(), -m_rectGraph.Height());
		dc.SetWindowOrg(m_nXLeft, 0);
		dc.SetWindowExt((m_nXRight - m_nXLeft), 1);
		dc.DPtoLP(&point);
	
		CPoint FAR* pointCoords = GetDocument()->GetCoords();
	    int nScanCount = GetDocument()->GetScanCount();
	    
		int i = m_nILeft + 2;
		while ((i < (nScanCount - 1)) && (pointCoords[i].x < point.x)) i++;
		if ((point.x - pointCoords[i - 1].x) < (pointCoords[i].x - point.x)) i--;
	
		// Draw cursor
		GetDocument()->SetCursor(i);
		m_rectCursor.left = pointCoords[i].x;
		m_rectCursor.right = m_rectCursor.left;
		m_rectCursor.top = 1;
		m_rectCursor.bottom = 0;
		dc.LPtoDP(&m_rectCursor);
		m_rectCursor.right = m_rectCursor.left + 1;
		dc.SetMapMode(MM_TEXT);
		dc.SetWindowOrg(m_rectGraph.left, m_rectGraph.bottom);
		dc.InvertRect(m_rectCursor);
	}
	else
	{
		// Remove cursor
		GetDocument()->SetCursor(-1);
		m_rectCursor.SetRectEmpty();
	}
	
	GetDocument()->UpdateAllViews(this);
}

void CReplayView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
//	CRect rectCursorVicinity = m_rectCursor;
//	rectCursorVicinity.InflateRect(2, 0);
//	
//	if (rectCursorVicinity.PtInRect(point))
//	{
//		// Double click on existing cursor - remove it
//		CClientDC dc(this);
//		dc.InvertRect(m_rectCursor);
//		
//		GetDocument()->SetCursor(-1);
//		m_rectCursor.SetRectEmpty();
//	}
//	else if (m_rectGraph.PtInRect(point))

	if (m_rectGraph.PtInRect(point))
	{
		// New cursor will be created by OnLButtonUp
		SetCapture();
	}	
}

void CReplayView::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_rectGraph.PtInRect(point))
	{
		SetCapture();
		m_rectMouse.TopLeft() = point;
		m_rectMouse.BottomRight() = point;
		m_nDragType = ZoomDrag;
	}
}

void CReplayView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture() != this) return;

	ReleaseCapture();
	
	if ((m_rectMouse.Height() == 0) || (m_rectMouse.Width() == 0)) return;
	
	// Arrange corners so that left<right and top<bottom
	
	int x1 = min(m_rectMouse.left, m_rectMouse.right);
	int x2 = max(m_rectMouse.left, m_rectMouse.right);
	int y1 = min(m_rectMouse.top, m_rectMouse.bottom);
	int y2 = max(m_rectMouse.top, m_rectMouse.bottom);
	
	m_rectMouse.left = x1;
	m_rectMouse.right = x2;
	m_rectMouse.top = y1;
	m_rectMouse.bottom = y2;
	
	CClientDC dc(this);
	
	dc.SetMapMode(MM_ANISOTROPIC);
	dc.SetViewportOrg(m_rectGraph.left, m_rectGraph.bottom);
	dc.SetViewportExt(m_rectGraph.Width(), -m_rectGraph.Height());
	
	CReplayDoc* pDoc = GetDocument();
	CRect rectZoom = m_rectMouse;
	CYAxis* pYAxis = pDoc->GetLastYAxis();
	if (strlen(pYAxis->m_szUnits) == 0) pYAxis = pDoc->GetFirstYAxis();
	dc.SetWindowOrg(m_nXLeft, pYAxis->GetLogYBottom());
	dc.SetWindowExt((m_nXRight - m_nXLeft),
					(pYAxis->GetLogYTop() - pYAxis->GetLogYBottom()));
	dc.DPtoLP(rectZoom);
	
	Zoom(rectZoom);	
}

void CReplayView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (GetCapture() != this) return;
	
	// If necessary, adjust point to lie within trend graph area
	point.x = max(point.x, m_rectGraph.left);
	point.x = min(point.x, m_rectGraph.right);
	point.y = max(point.y, m_rectGraph.top);
	point.y = min(point.y, m_rectGraph.bottom);
	
	CClientDC dc(this);
	
	switch (m_nDragType)
	{
		case ZoomDrag:
			dc.InvertRect(m_rectMouse);
			m_rectMouse.BottomRight() = point;
			dc.InvertRect(m_rectMouse);
			break;
			
		case CursorDrag:
			dc.InvertRect(m_rectCursor);
			m_rectCursor.left = point.x;
			m_rectCursor.right = m_rectCursor.left + 1;
			dc.InvertRect(m_rectCursor);
			break;
	}
}

void CReplayView::OnZoomOutBoth()
{
	OnZoomOutX();
	OnZoomOutY();
}

void CReplayView::OnZoomOutX()
{
	extern const int nMaxWinExtent;
	CRect rectZoom;
	
	long lWidth = (m_nXRight - m_nXLeft);
	
	rectZoom.left = (int)max((m_nXLeft - (lWidth / 2)), 0L);
	rectZoom.right = (int)min((rectZoom.left + (lWidth * 2)), (long)nMaxWinExtent);

	CReplayDoc* pDoc = GetDocument();
	CYAxis* pYAxis = pDoc->GetLastYAxis();
	if (strlen(pYAxis->m_szUnits) == 0) pYAxis = pDoc->GetFirstYAxis();
	rectZoom.bottom = pYAxis->GetLogYBottom();
	rectZoom.top = pYAxis->GetLogYTop();
	
	m_nILeft = 0;
	m_nIRight = pDoc->GetScanCount() - 1;;
	
	Zoom(rectZoom);
}

void CReplayView::OnZoomOutY()
{
	extern const int nMaxWinExtent;
	CRect rectZoom;
	
	CReplayDoc* pDoc = GetDocument();
	CYAxis* pYAxis = pDoc->GetLastYAxis();
	if (strlen(pYAxis->m_szUnits) == 0) pYAxis = pDoc->GetFirstYAxis();
	
	long lHeight = pYAxis->GetLogYTop() - pYAxis->GetLogYBottom();
	
	rectZoom.bottom = (int)max((pYAxis->GetLogYBottom() - (lHeight / 2)), 0L);
	rectZoom.top = (int)min((rectZoom.bottom + (lHeight * 2)), (long)nMaxWinExtent);
	rectZoom.left = m_nXLeft;
	rectZoom.right = m_nXRight;
	
	Zoom(rectZoom);
}

void CReplayView::OnZoomOut()
{
	extern const int nMaxWinExtent;
	
	CRect rectZoom;
	CReplayDoc* pDoc = GetDocument();
		
	rectZoom.left = 0;
	rectZoom.right = nMaxWinExtent;
	rectZoom.bottom = 0;
	rectZoom.top = nMaxWinExtent;
	
	m_nILeft = 0;
	m_nIRight = pDoc->GetScanCount() - 1;
	
	Zoom(rectZoom);
}

void CReplayView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CReplayDoc* pDoc = GetDocument();
    POSITION pos = pDoc->GetFirstChanPos();
    CChannel* pTime = pDoc->GetNextChan(pos);	// First channel is Time
    
    long lWidth = (m_lXRight - m_lXLeft);
	long lStep = ((pTime->GetDecPlaces() >= 0) || (lWidth > 691200L)) ? (lWidth / 10L) : (lWidth / 6L);
    
	switch (nSBCode)
	{
		case SB_LEFT:
			m_lXLeft = min((MakeLong(pTime->GetMinVal() / m_flResolution)), (m_lXCeiling - lWidth));
			m_lXLeft = (m_lXLeft >= 0) ? (m_lXLeft / lStep) * lStep
									   : -(((-m_lXLeft - 1) / lStep) + 1) * lStep;
//			m_lXLeft = m_lXFloor;
			m_nILeft = 0;
			m_nIRight = pDoc->GetScanCount() - 1;
			break;
			
		case SB_RIGHT:
			m_lXLeft = min((MakeLong(pTime->GetMaxVal() / m_flResolution)), (m_lXCeiling - lWidth));
			m_lXLeft = (m_lXLeft >= 0) ? (m_lXLeft / lStep) * lStep
									   : -(((-m_lXLeft - 1) / lStep) + 1) * lStep;
//			m_lXLeft = m_lXCeiling - lWidth;
			m_nILeft = 0;
			m_nIRight = pDoc->GetScanCount() - 1;
			break;
			
		case SB_LINELEFT:
			m_lXLeft = max((m_lXLeft - lStep), m_lXFloor);
			m_nILeft = 0;
			break;
			
		case SB_LINERIGHT:
			m_lXLeft = min((m_lXLeft + lStep), (m_lXCeiling - lWidth));
			m_nIRight = pDoc->GetScanCount() - 1;
			break;
			
		case SB_PAGELEFT:
			m_lXLeft = max((m_lXLeft - lWidth), m_lXFloor);
			m_nILeft = 0;
			break;
			
		case SB_PAGERIGHT:
			m_lXLeft = min((m_lXLeft + lWidth), (m_lXCeiling - lWidth));
			m_nIRight = pDoc->GetScanCount() - 1;
			break;
			
		case SB_THUMBPOSITION:
			m_lXLeft = min((MakeLong(nPos / m_flScaleFactor) + m_lXFloor), (m_lXCeiling - lWidth));
			m_lXLeft = (m_lXLeft >= 0) ? (m_lXLeft / lStep) * lStep
									   : -(((-m_lXLeft - 1) / lStep) + 1) * lStep;
			m_nILeft = 0;
			m_nIRight = pDoc->GetScanCount() - 1;
			break;
			
		default:
			return;
	}
    
    m_lXRight = m_lXLeft + lWidth;
    
//	m_lXLeft = MakeLong(pChan->GetValue(m_nILeft) / m_flResolution);
//	m_lXRight = MakeLong(pChan->GetValue(m_nIRight) / m_flResolution);
//	Round(m_lXLeft, m_lXRight);
	m_nXLeft = MakeInt((m_lXLeft - m_lXFloor) * m_flScaleFactor);
	m_nXRight = MakeInt((m_lXRight - m_lXFloor) * m_flScaleFactor);
	
	int FAR* pCoords = pTime->GetCoords();
		
	while ((m_nILeft < (m_nIRight - 1)) && (pCoords[m_nILeft + 1] < m_nXLeft)) m_nILeft++;
	while ((m_nIRight > (m_nILeft + 1)) && (pCoords[m_nIRight - 1] > m_nXRight)) m_nIRight--;

	SetScrollPos(SB_HORZ, m_nXLeft, TRUE);

	Invalidate();			
}

void CReplayView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	extern const int nMaxWinExtent;
	
	CReplayDoc* pDoc = GetDocument();
	CRect rectScroll;
	
	for (POSITION pos = pDoc->GetFirstYAxisPos(); pos != NULL;)
	{
		CYAxis* pYAxis = pDoc->GetNextYAxis(pos);
		int nBottom = pYAxis->GetLogYBottom();
		int nHeight = pYAxis->GetLogYTop() - nBottom;
		
		switch (nSBCode)
		{
			case SB_BOTTOM:
				rectScroll.bottom = 0;
				break;
				
			case SB_TOP:
				rectScroll.bottom = nMaxWinExtent - nHeight;
				break;
				
			case SB_LINEDOWN:
				rectScroll.bottom = max((nBottom - (nHeight / 10)), 0);
				break;
				
			case SB_LINEUP:
				rectScroll.bottom = min((nBottom + (nHeight / 10)),
										nMaxWinExtent - nHeight);
				break;
				
			case SB_PAGEDOWN:
				rectScroll.bottom = max((nBottom - nHeight), 0);
				break;
				
			case SB_PAGEUP:
				rectScroll.bottom = min((nBottom + nHeight),
										nMaxWinExtent - nHeight);
				break;
				
			default:
				return;
		}
				
		rectScroll.top = rectScroll.bottom + nHeight;		
		pYAxis->SetAxis(rectScroll, FALSE);
	}

	SetScrollPos(SB_VERT, (nMaxWinExtent - rectScroll.bottom), TRUE);

	Invalidate();			
}

void CReplayView::Zoom(CRect rectZoom)
{
	extern const int nMaxWinExtent;
	extern const int nMinRange;
	
	CReplayDoc* pDoc = GetDocument();
	
	for (POSITION pos = pDoc->GetFirstYAxisPos(); pos != NULL;)
	{
		CYAxis* pYAxis = pDoc->GetNextYAxis(pos);
		pYAxis->SetAxis(rectZoom, TRUE);
	}
	
	if (rectZoom.Width() < nMinRange)
	{
		rectZoom.right = min((rectZoom.left + nMinRange), nMaxWinExtent);
		rectZoom.left = rectZoom.right - nMinRange;
	}
	
    pos = pDoc->GetFirstChanPos();
    CChannel* pTime = pDoc->GetNextChan(pos);	// First channel is Time

	m_lXLeft = MakeLong(rectZoom.left / m_flScaleFactor) + m_lXFloor;
	m_lXRight = MakeLong(rectZoom.right / m_flScaleFactor) + m_lXFloor;
	if (pTime->GetDecPlaces() >= 0)
	{
		Round(m_lXLeft, m_lXRight);
	}
	else
	{
		RoundTime(m_lXLeft, m_lXRight);
	}
	m_nXLeft = MakeInt((m_lXLeft - m_lXFloor) * m_flScaleFactor);
	m_nXRight = MakeInt((m_lXRight - m_lXFloor) * m_flScaleFactor);
	
	int FAR* pCoords = pTime->GetCoords();
		
	while ((m_nILeft < (m_nIRight - 1)) && (pCoords[m_nILeft + 1] < m_nXLeft)) m_nILeft++;
	while ((m_nIRight > (m_nILeft + 1)) && (pCoords[m_nIRight - 1] > m_nXRight)) m_nIRight--;
	
	SetScrollPos(SB_HORZ, m_nXLeft, TRUE);
	SetScrollPos(SB_VERT, (nMaxWinExtent - rectZoom.bottom), TRUE);
		
	Invalidate();
}

void CReplayView::OnReplayGotoStart()
{
	OnHScroll(SB_LEFT, 0, NULL);	
}

void CReplayView::OnReplayGotoEnd()
{
	OnHScroll(SB_RIGHT, 0, NULL);	
}

void CReplayView::OnReplayForward()
{
	if ((m_lDX = (m_lXRight - m_lXLeft) / 50L) == 0L) m_lDX = 1L;
	StartPlay(1);
}

void CReplayView::OnReplayReverse()
{
	if ((m_lDX = (m_lXRight - m_lXLeft) / 50L) == 0L) m_lDX = 1L;
	m_lDX = -m_lDX;
	StartPlay(1);
}

void CReplayView::OnReplayFastForward()
{
	if ((m_lDX = (m_lXRight - m_lXLeft) / 25L) == 0L) m_lDX = 1L;
	StartPlay(1);
}

void CReplayView::OnReplayFastReverse()
{
	if ((m_lDX = (m_lXRight - m_lXLeft) / 25L) == 0L) m_lDX = 1L;
	m_lDX = -m_lDX;
	StartPlay(1);
}

void CReplayView::OnReplayStop()
{
	KillTimer(TimerId);
	m_nDrawAxes = TRUE;
	OnHScroll(SB_THUMBPOSITION, m_nXLeft, NULL);
}

void CReplayView::OnTimer(UINT nIDEvent)
{
	Play();
	
	CView::OnTimer(nIDEvent);
}

void CReplayView::StartPlay(int nInterval)
{
	if (SetTimer(TimerId, 50, NULL) == 0)
	{
		MessageBox("Cannot start replay because no timers are available", "Replay");
		return;
	}
	
	m_nDrawAxes = FALSE;
	InvalidateRect(m_rectXAxis);
}

void CReplayView::Play()
{
	extern const int nMaxWinExtent;
	
//	char szScrolls[20];
//	nScrolls++;
//	GetParentFrame()->SetWindowText(itoa(nScrolls, szScrolls, 10));
//	Invalidate();
//	return;
    
	CClientDC dc(this);
	
	dc.SetMapMode(MM_ANISOTROPIC);
	dc.SetViewportOrg(m_rectGraph.left, m_rectGraph.bottom);
	dc.SetViewportExt(m_rectGraph.Width(), -m_rectGraph.Height());
	
	CReplayDoc* pDoc = GetDocument();
	CYAxis* pYAxis = pDoc->GetLastYAxis();
	if (strlen(pYAxis->m_szUnits) == 0) pYAxis = pDoc->GetFirstYAxis();
	dc.SetWindowOrg(m_nXLeft, pYAxis->GetLogYBottom());
	dc.SetWindowExt((m_nXRight - m_nXLeft),
					(pYAxis->GetLogYTop() - pYAxis->GetLogYBottom()));
	
//	CPoint ptDX = CPoint((m_rectGraph.left + m_nDX), m_rectGraph.bottom);
//	dc.DPtoLP(&ptDX);
	int nLogDX = MakeInt(m_lDX * m_flScaleFactor);
	CPoint pt = CPoint(m_nXLeft + nLogDX, pYAxis->GetLogYBottom());
	dc.LPtoDP(&pt);
	int nDX = pt.x - m_rectGraph.left;
		
	long lWidth = m_lXRight - m_lXLeft;
//	int nLogDX = m_nXLeft - ptDX.x;
//	long lEngDX = MakeLong(nLogDX / m_flScaleFactor);
//	m_lXLeft = max((m_lXLeft + lEngDX), (m_lXFloor));
	m_lXLeft = max((m_lXLeft + m_lDX), (m_lXFloor));
	m_lXLeft = min((m_lXLeft), (m_lXCeiling - lWidth));
	m_lXRight = m_lXLeft + lWidth;
	m_nXLeft = MakeInt((m_lXLeft - m_lXFloor) * m_flScaleFactor);
	m_nXRight = MakeInt((m_lXRight - m_lXFloor) * m_flScaleFactor);
	
    POSITION pos = pDoc->GetFirstChanPos();
    CChannel* pChan = pDoc->GetNextChan(pos);	// First channel is Time
	int FAR* pCoords = pChan->GetCoords();
		
	if (m_lDX > 0)
	{
		while ((m_nILeft < (pDoc->GetScanCount() - 2))
				&& (pCoords[m_nILeft + 1] < m_nXLeft)) m_nILeft++;
		while ((m_nIRight < (pDoc->GetScanCount() - 1))
				&& (pCoords[m_nIRight] < m_nXRight)) m_nIRight++;
	}
	else
	{
		while ((m_nILeft > 0) && (pCoords[m_nILeft] > m_nXLeft)) m_nILeft--;
		while ((m_nIRight > 1) && (pCoords[m_nIRight - 1] > m_nXRight)) m_nIRight--;
	}
	
	if ((m_nXLeft == 0) || (m_nXRight == nMaxWinExtent))
	{
		OnReplayStop();
		return;
	}
	
//	CRect rectUpdate;
//	m_rgnClip.SetRectRgn(0, 0, 0, 0);
//	dc.ScrollDC(-nDX, 0, rectLogGraph, rectLogGraph, &m_rgnClip, NULL);
	
//	dc.LPtoDP(rectUpdate);	
//	InvalidateRect(rectUpdate);
//	InvalidateRgn(&m_rgnClip);

	ScrollWindow(-nDX, 0, m_rectGraph, m_rectGraph);
//	if (pDoc->m_nPlayState == SLOW) InvalidateRect(m_rectXAxis);
	UpdateWindow();
		
	SetScrollPos(SB_HORZ, m_nXLeft, TRUE);
}

void CReplayView::SetClipRect(CDC* pDC, CRect rectClip)
{
	// Adjust rectangle if Print Preview
	if (pDC->IsKindOf(RUNTIME_CLASS(CPreviewDC)))
	{
		((CPreviewDC*)pDC)->PrinterDPtoScreenDP(&rectClip.TopLeft());
		((CPreviewDC*)pDC)->PrinterDPtoScreenDP(&rectClip.BottomRight());
		
		CPoint ptOrg = pDC->SetViewportOrg(0, 0);
		CPoint ptTemp;
		::GetViewportOrgEx(pDC->m_hDC, &ptTemp);
		rectClip += ptTemp;
		pDC->SetViewportOrg(ptOrg);
	}
	
	m_rgnClip.SetRectRgn(rectClip);
	pDC->SelectClipRgn(&m_rgnClip);
}


