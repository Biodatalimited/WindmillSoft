// replay.h : main header file for the REPLAY application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CReplayApp:
// See replay.cpp for the implementation of this class
//

class CReplayApp : public CWinApp
{
public:
	CReplayApp();

// Overrides
	virtual BOOL InitInstance();

// Implementation

	//{{AFX_MSG(CReplayApp)
	afx_msg void OnAppAbout();
	afx_msg void OnHelpContents();
	afx_msg void OnHelpHowToUseHelp();
	afx_msg void OnHelpIndex();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
