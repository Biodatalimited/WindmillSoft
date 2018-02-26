// playfrm.cpp : implementation of the CReplayFrame class
//

#include "stdafx.h"
#include "replay.h"

#include "playfrm.h"
#include "playview.h"
#include "textview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReplayFrame

IMPLEMENT_DYNCREATE(CReplayFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CReplayFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CReplayFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// arrays of IDs used to initialize control bars

// toolbar buttons - IDs are command buttons
static UINT BASED_CODE buttons[] =
{
	// same order as in the bitmap 'toolbar.bmp'
	ID_ZOOM_OUT_X,
	ID_ZOOM_OUT_Y,
	ID_ZOOM_OUT_BOTH,
		ID_SEPARATOR,
	ID_ZOOM_OUT,
		ID_SEPARATOR,
	ID_REPLAY_GOTOSTART,
	ID_REPLAY_FASTREVERSE,
	ID_REPLAY_REVERSE,
	ID_REPLAY_STOP,
	ID_REPLAY_FORWARD,
	ID_REPLAY_FASTFORWARD,
	ID_REPLAY_GOTOEND
};

static UINT BASED_CODE indicators[] =
{
	ID_SEPARATOR,			// status line indicator
//	ID_INDICATOR_CAPS,
//	ID_INDICATOR_NUM,
//	ID_INDICATOR_SCRL,
	ID_SEPARATOR
};

/////////////////////////////////////////////////////////////////////////////
// CReplayFrame construction/destruction

CReplayFrame::CReplayFrame()
{
	// TODO: add member initialization code here
}

CReplayFrame::~CReplayFrame()
{
}

BOOL CReplayFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.x = 0;
	cs.y = 0;
	cs.cx = 800;
	cs.cy = 480;
	
	return CWnd::PreCreateWindow(cs);
}

int CReplayFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadBitmap(IDR_MAINFRAME) ||
		!m_wndToolBar.SetButtons(buttons,
		  sizeof(buttons)/sizeof(UINT)))
	{
		TRACE("Failed to create toolbar\n");
		return -1;		// fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE("Failed to create status bar\n");
		return -1;		// fail to create
	}
    
    UINT unID, unStyle;
    int nWidth;
    
    // Add standard border to "Prompt" pane
    m_wndStatusBar.GetPaneInfo(0, unID, unStyle, nWidth);
    m_wndStatusBar.SetPaneInfo(0, unID, unStyle & (~SBPS_NOBORDERS), nWidth);
    
    // Default "Start Time" pane width is 1/4 screen.
    // Increase this to 1/2 screen.
    m_wndStatusBar.GetPaneInfo(1, unID, unStyle, nWidth);
    m_wndStatusBar.SetPaneInfo(1, unID, unStyle, nWidth * 2);
    
	return 0;
}

BOOL CReplayFrame::OnCreateClient(LPCREATESTRUCT, CCreateContext* pContext)
{

/*********************************************************************/
//	// default create client will create a view if asked for it
//	if (pContext != NULL)
//	{
//		// try to create view object from RuntimeClass
//		if (pContext->m_pNewViewClass != NULL)
//		{
//			CWnd* pView = (CWnd*)pContext->m_pNewViewClass->CreateObject();
//				// NOTE: can be a CWnd with PostNcDestroy self cleanup
//
//			if (pView == NULL)
//			{
//				TRACE1("Warning: Dynamic create of view type %Fs failed\n",
//					pContext->m_pNewViewClass->m_lpszClassName);
//				return FALSE;
//			}
//			ASSERT(pView->IsKindOf(RUNTIME_CLASS(CWnd)));
//			// Views are always created with a border !
//			const char* pWndClass = AfxRegisterWndClass(CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW,
//								0, (HBRUSH)GetStockObject(BLACK_BRUSH), 0);
//			if (!pView->Create(pWndClass, NULL,
//				AFX_WS_DEFAULT_VIEW | WS_HSCROLL | WS_VSCROLL,
//				CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST, pContext))
//			{
//				TRACE0("Warning: couldn't create view for frame\n");
//				return FALSE;       // can't continue without a view
//			}
//		}
//	}
/***************************************************************************/
	TRACE1("OnCreateClient: View Class is %Fs\n",
		pContext->m_pNewViewClass->m_lpszClassName);
		
    // create splitter window with 2 rows, 1 column
	if (!m_wndSplitter.CreateStatic(this, 2, 1))
	{
		TRACE("Failed to Create Static Splitter\n");
		return FALSE;
	}
    
	// add the first splitter pane - the graphics view in row 0
	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CReplayView),
								  CSize(500, 300), pContext))
	{
		TRACE("Failed to create graphics pane\n");
		return FALSE;
	}

	// add the second splitter pane - the text view in row 1
	if (!m_wndSplitter.CreateView(1, 0, RUNTIME_CLASS(CTextView),
								  CSize(500, 150), pContext))
	{
		TRACE("Failed to create text pane\n");
		return FALSE;
	}

	// activate the input view
	SetActiveView((CView*)m_wndSplitter.GetPane(0, 0));

	return TRUE;
}


void CReplayFrame::SetStartTime(const char* pszStartTime)
{
	m_wndStatusBar.SetPaneText(1, pszStartTime, TRUE);
}


/////////////////////////////////////////////////////////////////////////////
// CReplayFrame diagnostics

#ifdef _DEBUG
void CReplayFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CReplayFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CReplayFrame message handlers

