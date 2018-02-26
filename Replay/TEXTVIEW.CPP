// textview.cpp : implementation of the CTextView class
//

#include "stdafx.h"
#include "replay.h"
#include "channel.h"
#include "chandlg.h"
#include "colordlg.h"
#include "playdoc.h"
#include "textview.h"

#include <stdlib.h>
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

int nScanX;
int nTimeX;
int nKeyHeight;
int nColWidth;

/////////////////////////////////////////////////////////////////////////////
// CTextView

IMPLEMENT_DYNCREATE(CTextView, CView)

CTextView::CTextView()
{
}

CTextView::~CTextView()
{
}


BEGIN_MESSAGE_MAP(CTextView, CView)
	//{{AFX_MSG_MAP(CTextView)
	ON_WM_ERASEBKGND()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextView drawing

void CTextView::OnInitialUpdate()
{
	m_nFirstChan = 1;
	m_nFirstScan = 0;
    
    CView::OnInitialUpdate();
}

void CTextView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
    CReplayDoc* pDoc = GetDocument();
    
	if ((pDoc->GetFirstChanPos()) == NULL) return;	// File not yet opened

    if (pDoc->GetCursor() >= 0)
    {
    	m_nFirstScan = max(pDoc->GetCursor() - ((m_nNumLines - 1) / 2), 0);
    }
    
	SetScrollRange(SB_VERT, 0, (pDoc->GetScanCount() - 1), FALSE);
	SetScrollPos(SB_VERT, m_nFirstScan, TRUE);
	SetScrollRange(SB_HORZ, 1, (pDoc->GetChanCount() - 1), FALSE);
	SetScrollPos(SB_HORZ, m_nFirstChan, TRUE);

	Invalidate();
}

void CTextView::OnDraw(CDC* pDC)
{
	CReplayDoc* pDoc = GetDocument();

	if ((pDoc->GetFirstChanPos()) == NULL) return;	// File not yet opened

	// Prepare device context
	pDC->SetMapMode(MM_TEXT);
	pDC->SetViewportOrg(0, 0);
	pDC->SetWindowOrg(0, 0);
	pDC->SetTextColor(RGB(255, 255, 255));
	pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextAlign(TA_RIGHT);
    
	// Select font
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
	m_nLineHeight = tmFont.tmHeight + tmFont.tmExternalLeading;
	nKeyHeight = m_nLineHeight;
	
	// Find widths of Scan/Time/Data columns
	nColWidth = tmFont.tmAveCharWidth * 13;
	nScanX = tmFont.tmAveCharWidth * 6;
	POSITION pos = pDoc->GetFirstChanPos();
	CChannel* pChan = pDoc->GetNextChan(pos);
	nTimeX = nScanX + tmFont.tmAveCharWidth * ((pChan->GetMaxVal() < 86400.0F) ? 10 : 17);

#ifdef DARAD
	nTimeX += tmFont.tmAveCharWidth * 5;
#endif
	
	// Calculate number of lines of data to display
	CRect rectClient;
	GetClientRect(rectClient);
    m_nNumLines = (rectClient.bottom - m_nLineHeight - nKeyHeight) / m_nLineHeight;
    m_nFirstScan = min((m_nFirstScan), (max((pDoc->GetScanCount() - m_nNumLines), (0))));
    m_nNumLines = min((m_nNumLines), (pDoc->GetScanCount() - m_nFirstScan));
    
    // Calculate number of columns to display
    m_nNumCols = (rectClient.right - nTimeX) / nColWidth;
    
	// Set up coords for channel "Keys"
	CPoint aptKey[6];
	int nHScale = -(nColWidth / 12);
	int nVScale = nKeyHeight / 4;
	
	aptKey[0] = CPoint(0 * nHScale, 1 * nVScale);
	aptKey[1] = CPoint(2 * nHScale, 1 * nVScale);
	aptKey[2] = CPoint(3 * nHScale, 0 * nVScale);
	aptKey[3] = CPoint(4 * nHScale, 2 * nVScale);
	aptKey[4] = CPoint(5 * nHScale, 1 * nVScale);
	aptKey[5] = CPoint(7 * nHScale, 1 * nVScale);
	
    // Display "Scan" column
    pDC->SetViewportOrg(0, 0);
    pDC->TextOut(nScanX, 0, "Scan", 4);
	pDC->SetViewportOrg(0, m_nLineHeight + nKeyHeight);
	int i, y;
	char szBuff[10];
	for (i = 0, y = 0; i < m_nNumLines; i++, y += m_nLineHeight)
	{
		_itoa((m_nFirstScan + i), szBuff, 10);
		pDC->TextOut(nScanX, y, szBuff, strlen(szBuff));
	}

	// Display "Time" column
    pDC->SetViewportOrg(0, 0);
    pDC->TextOut(nTimeX, 0, "Time", 4);
	pDC->SetViewportOrg(0, m_nLineHeight + nKeyHeight);
	for (i = 0, y = 0; i < m_nNumLines; i++, y += m_nLineHeight)
	{
		pDC->TextOut(nTimeX, y, pChan->GetValue(m_nFirstScan + i));
	}
	
    // Skip over undisplayed channels
    for (i = 1; i < m_nFirstChan; i++) pChan = pDoc->GetNextChan(pos);
    
    // Display data columns
    int nCols = 0;
    while ((pos != NULL) && (nCols < m_nNumCols))
    {
    	pChan = pDoc->GetNextChan(pos);
    	pDC->SetViewportOrg((nTimeX + ((nCols + 1) * nColWidth)), 0);
    	pDC->TextOut(0, 0, pChan->GetName());
		pDC->SetViewportOrg((nTimeX + ((nCols + 1) * nColWidth)), m_nLineHeight);

		pChan->DrawKey(pDC, aptKey);

		pDC->SetViewportOrg((nTimeX + ((nCols + 1) * nColWidth)), m_nLineHeight + nKeyHeight);
		for (i = 0, y = 0; i < m_nNumLines; i++, y += m_nLineHeight)
		{
			pDC->TextOut(0, y, pChan->GetValue(m_nFirstScan + i));
		}
	    nCols++;
	}
	
//	pDC->SelectObject(pOldFont);
	
	// Invert line containing cursor
	int nCursor = pDoc->GetCursor();
	if ((m_nFirstScan <= nCursor) && (nCursor < (m_nFirstScan + m_nNumLines)))
	{
		pDC->SetViewportOrg(0, m_nLineHeight + nKeyHeight);
		CRect rectCursor = rectClient;
		rectCursor.top = (nCursor - m_nFirstScan) * m_nLineHeight;
		rectCursor.bottom = rectCursor.top + m_nLineHeight;
		pDC->InvertRect(rectCursor);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTextView diagnostics

#ifdef _DEBUG
void CTextView::AssertValid() const
{
	CView::AssertValid();
}

void CTextView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CReplayDoc* CTextView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CReplayDoc)));
	return (CReplayDoc*) m_pDocument;
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextView message handlers


BOOL CTextView::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClip;
	pDC->GetClipBox(rectClip);
	
    CBrush brushBkgnd;
    brushBkgnd.CreateSolidBrush(RGB(64, 64, 64));
    
    pDC->FillRect(rectClip, &brushBkgnd);
    
	return TRUE;
}

void CTextView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CReplayDoc* pDoc = GetDocument();
	
	int nMaxFirstScan = max((pDoc->GetScanCount() - m_nNumLines), 0);
	
	switch (nSBCode)
	{
		case SB_BOTTOM:
			m_nFirstScan = nMaxFirstScan;
			break;
				
		case SB_TOP:
			m_nFirstScan = 0;
			break;
				
		case SB_LINEDOWN:
			m_nFirstScan = min((m_nFirstScan + 1), nMaxFirstScan);
			break;
				
		case SB_LINEUP:
			m_nFirstScan = max((m_nFirstScan - 1), 0);
			break;
				
		case SB_PAGEDOWN:
			m_nFirstScan = min((m_nFirstScan + m_nNumLines), nMaxFirstScan);
			break;
				
		case SB_PAGEUP:
			m_nFirstScan = max((m_nFirstScan - m_nNumLines), 0);
			break;
				
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			m_nFirstScan = min((int)nPos, nMaxFirstScan);
			break;
			
		default:
			return;
	}

	SetScrollPos(SB_VERT, m_nFirstScan, TRUE);

	Invalidate();			
}

void CTextView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CReplayDoc* pDoc = GetDocument();
	
	int nMaxFirstChan = pDoc->GetChanCount() - 1;
	
	switch (nSBCode)
	{
		case SB_LEFT:
			m_nFirstChan = 1;
			break;
			
		case SB_RIGHT:
			m_nFirstChan = nMaxFirstChan;
			break;
			
		case SB_LINELEFT:
			m_nFirstChan = max((m_nFirstChan - 1), 1);
			break;
			
		case SB_LINERIGHT:
			m_nFirstChan = min((m_nFirstChan + 1), nMaxFirstChan);
			break;
			
		case SB_PAGELEFT:
			m_nFirstChan = max((m_nFirstChan - m_nNumCols), 1);
			break;
			
		case SB_PAGERIGHT:
			m_nFirstChan = min((m_nFirstChan + m_nNumCols), nMaxFirstChan);
			break;
			
		default:
			return;
	}
    
	SetScrollPos(SB_HORZ, m_nFirstChan, TRUE);

	Invalidate();			
}

void CTextView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// Find line containing mouse pointer
	int nLine = (point.y - (m_nLineHeight + nKeyHeight)) / m_nLineHeight;
	
	if ((nLine >= 0) && (nLine < m_nNumLines))
	{
		// Set cursor position
		GetDocument()->SetCursor(m_nFirstScan + nLine);
		GetDocument()->UpdateAllViews(this);
		Invalidate();
	}
}

