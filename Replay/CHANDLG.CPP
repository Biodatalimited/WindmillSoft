// chandlg.cpp : implementation file
//

#include "stdafx.h"
#include "replay.h"
#include "chandlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChannelDlg dialog

CChannelDlg::CChannelDlg()
	: CDialog(CChannelDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CChannelDlg)
	m_nColour1 = -1;
	m_nColour2 = -1;
	m_nColour3 = -1;
	m_nColour4 = -1;
	m_nColour5 = -1;
	m_nColour6 = -1;
	m_nColour7 = -1;
	m_nColour8 = -1;
	m_nDecPl1 = -1;
	m_nDecPl2 = -1;
	m_nDecPl3 = -1;
	m_nDecPl4 = -1;
	m_nDecPl5 = -1;
	m_nDecPl6 = -1;
	m_nDecPl7 = -1;
	m_nDecPl8 = -1;
	m_nChan1 = -1;
	m_nChan2 = -1;
	m_nChan3 = -1;
	m_nChan4 = -1;
	m_nChan5 = -1;
	m_nChan6 = -1;
	m_nChan7 = -1;
	m_nChan8 = -1;
	//}}AFX_DATA_INIT
}

void CChannelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChannelDlg)
	DDX_Control(pDX, IDC_COLOUR8, m_cmbColour8);
	DDX_Control(pDX, IDC_COLOUR7, m_cmbColour7);
	DDX_Control(pDX, IDC_COLOUR6, m_cmbColour6);
	DDX_Control(pDX, IDC_COLOUR5, m_cmbColour5);
	DDX_Control(pDX, IDC_COLOUR4, m_cmbColour4);
	DDX_Control(pDX, IDC_COLOUR3, m_cmbColour3);
	DDX_Control(pDX, IDC_COLOUR2, m_cmbColour2);
	DDX_Control(pDX, IDC_COLOUR1, m_cmbColour1);
	DDX_Control(pDX, IDC_DECPL8, m_cmbDecPl8);
	DDX_Control(pDX, IDC_DECPL7, m_cmbDecPl7);
	DDX_Control(pDX, IDC_DECPL6, m_cmbDecPl6);
	DDX_Control(pDX, IDC_DECPL5, m_cmbDecPl5);
	DDX_Control(pDX, IDC_DECPL4, m_cmbDecPl4);
	DDX_Control(pDX, IDC_DECPL3, m_cmbDecPl3);
	DDX_Control(pDX, IDC_DECPL2, m_cmbDecPl2);
	DDX_Control(pDX, IDC_DECPL1, m_cmbDecPl1);
	DDX_Control(pDX, IDC_CHAN8, m_cmbChan8);
	DDX_Control(pDX, IDC_CHAN7, m_cmbChan7);
	DDX_Control(pDX, IDC_CHAN6, m_cmbChan6);
	DDX_Control(pDX, IDC_CHAN5, m_cmbChan5);
	DDX_Control(pDX, IDC_CHAN4, m_cmbChan4);
	DDX_Control(pDX, IDC_CHAN3, m_cmbChan3);
	DDX_Control(pDX, IDC_CHAN2, m_cmbChan2);
	DDX_Control(pDX, IDC_CHAN1, m_cmbChan1);
	DDX_CBIndex(pDX, IDC_COLOUR1, m_nColour1);
	DDX_CBIndex(pDX, IDC_COLOUR2, m_nColour2);
	DDX_CBIndex(pDX, IDC_COLOUR3, m_nColour3);
	DDX_CBIndex(pDX, IDC_COLOUR4, m_nColour4);
	DDX_CBIndex(pDX, IDC_COLOUR5, m_nColour5);
	DDX_CBIndex(pDX, IDC_COLOUR6, m_nColour6);
	DDX_CBIndex(pDX, IDC_COLOUR7, m_nColour7);
	DDX_CBIndex(pDX, IDC_COLOUR8, m_nColour8);
	DDX_CBIndex(pDX, IDC_DECPL1, m_nDecPl1);
	DDX_CBIndex(pDX, IDC_DECPL2, m_nDecPl2);
	DDX_CBIndex(pDX, IDC_DECPL3, m_nDecPl3);
	DDX_CBIndex(pDX, IDC_DECPL4, m_nDecPl4);
	DDX_CBIndex(pDX, IDC_DECPL5, m_nDecPl5);
	DDX_CBIndex(pDX, IDC_DECPL6, m_nDecPl6);
	DDX_CBIndex(pDX, IDC_DECPL7, m_nDecPl7);
	DDX_CBIndex(pDX, IDC_DECPL8, m_nDecPl8);
	DDX_CBIndex(pDX, IDC_CHAN1, m_nChan1);
	DDX_CBIndex(pDX, IDC_CHAN2, m_nChan2);
	DDX_CBIndex(pDX, IDC_CHAN3, m_nChan3);
	DDX_CBIndex(pDX, IDC_CHAN4, m_nChan4);
	DDX_CBIndex(pDX, IDC_CHAN5, m_nChan5);
	DDX_CBIndex(pDX, IDC_CHAN6, m_nChan6);
	DDX_CBIndex(pDX, IDC_CHAN7, m_nChan7);
	DDX_CBIndex(pDX, IDC_CHAN8, m_nChan8);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CChannelDlg, CDialog)
	//{{AFX_MSG_MAP(CChannelDlg)
	ON_CBN_SELCHANGE(IDC_CHAN1, OnSelchangeChan1)
	ON_CBN_SELCHANGE(IDC_CHAN2, OnSelchangeChan2)
	ON_CBN_SELCHANGE(IDC_CHAN3, OnSelchangeChan3)
	ON_CBN_SELCHANGE(IDC_CHAN4, OnSelchangeChan4)
	ON_CBN_SELCHANGE(IDC_CHAN5, OnSelchangeChan5)
	ON_CBN_SELCHANGE(IDC_CHAN6, OnSelchangeChan6)
	ON_CBN_SELCHANGE(IDC_CHAN7, OnSelchangeChan7)
	ON_CBN_SELCHANGE(IDC_CHAN8, OnSelchangeChan8)
	ON_BN_CLICKED(IDC_CLEAR_ALL, OnClickedClearAll)
	ON_BN_CLICKED(IDC_DEFAULTS, OnClickedDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChannelDlg message handlers

BOOL CChannelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	int i;
	
	m_cmbChan1.ResetContent();
	m_cmbChan1.AddString("(None)");
	for (i = 1; i < m_astrNames->GetSize(); i++) m_cmbChan1.AddString((*m_astrNames)[i]);
	
	m_cmbChan2.ResetContent();
	m_cmbChan2.AddString("(None)");
	for (i = 1; i < m_astrNames->GetSize(); i++) m_cmbChan2.AddString((*m_astrNames)[i]);
	
	m_cmbChan3.ResetContent();
	m_cmbChan3.AddString("(None)");
	for (i = 1; i < m_astrNames->GetSize(); i++) m_cmbChan3.AddString((*m_astrNames)[i]);
	
	m_cmbChan4.ResetContent();
	m_cmbChan4.AddString("(None)");
	for (i = 1; i < m_astrNames->GetSize(); i++) m_cmbChan4.AddString((*m_astrNames)[i]);
	
	m_cmbChan5.ResetContent();
	m_cmbChan5.AddString("(None)");
	for (i = 1; i < m_astrNames->GetSize(); i++) m_cmbChan5.AddString((*m_astrNames)[i]);
	
	m_cmbChan6.ResetContent();
	m_cmbChan6.AddString("(None)");
	for (i = 1; i < m_astrNames->GetSize(); i++) m_cmbChan6.AddString((*m_astrNames)[i]);
	
	m_cmbChan7.ResetContent();
	m_cmbChan7.AddString("(None)");
	for (i = 1; i < m_astrNames->GetSize(); i++) m_cmbChan7.AddString((*m_astrNames)[i]);
	
	m_cmbChan8.ResetContent();
	m_cmbChan8.AddString("(None)");
	for (i = 1; i < m_astrNames->GetSize(); i++) m_cmbChan8.AddString((*m_astrNames)[i]);
	
	if (m_nChan1 == -1)
	{
		// Initialise with default settings
		OnClickedDefaults();
	}
	else
	{
		m_cmbChan1.SetCurSel(m_nChan1);
		m_cmbChan2.SetCurSel(m_nChan2);
		m_cmbChan3.SetCurSel(m_nChan3);
		m_cmbChan4.SetCurSel(m_nChan4);
		m_cmbChan5.SetCurSel(m_nChan5);
		m_cmbChan6.SetCurSel(m_nChan6);
		m_cmbChan7.SetCurSel(m_nChan7);
		m_cmbChan8.SetCurSel(m_nChan8);
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CChannelDlg::SetChannels(CStringArray* astrNames, CStringArray* astrUnits)
{
	m_astrNames = astrNames;
	m_astrUnits = astrUnits;

	if (m_nChan1 >= m_astrNames->GetSize())
	{
		m_nChan1 = 0;
		m_nDecPl1 = 0;
	}
	if (m_nChan2 >= m_astrNames->GetSize())
	{
		m_nChan2 = 0;
		m_nDecPl2 = 0;
	}
	if (m_nChan3 >= m_astrNames->GetSize())
	{
		m_nChan3 = 0;
		m_nDecPl3 = 0;
	}
	if (m_nChan4 >= m_astrNames->GetSize())
	{
		m_nChan4 = 0;
		m_nDecPl4 = 0;
	}
	if (m_nChan5 >= m_astrNames->GetSize())
	{
		m_nChan5 = 0;
		m_nDecPl5 = 0;
	}
	if (m_nChan6 >= m_astrNames->GetSize())
	{
		m_nChan6 = 0;
		m_nDecPl6 = 0;
	}
	if (m_nChan7 >= m_astrNames->GetSize())
	{
		m_nChan7 = 0;
		m_nDecPl7 = 0;
	}
	if (m_nChan8 >= m_astrNames->GetSize())
	{
		m_nChan8 = 0;
		m_nDecPl8 = 0;
	}
}



void CChannelDlg::OnClickedClearAll()
{
	m_cmbChan1.SetCurSel(0);
	OnSelchangeChan1();
	m_cmbChan2.SetCurSel(0);
	OnSelchangeChan2();
	m_cmbChan3.SetCurSel(0);
	OnSelchangeChan3();
	m_cmbChan4.SetCurSel(0);
	OnSelchangeChan4();
	m_cmbChan5.SetCurSel(0);
	OnSelchangeChan5();
	m_cmbChan6.SetCurSel(0);
	OnSelchangeChan6();
	m_cmbChan7.SetCurSel(0);
	OnSelchangeChan7();
	m_cmbChan8.SetCurSel(0);
	OnSelchangeChan8();
}

void CChannelDlg::OnClickedDefaults()
{
	m_cmbColour1.SetCurSel(0);
	m_cmbColour2.SetCurSel(1);
	m_cmbColour3.SetCurSel(2);
	m_cmbColour4.SetCurSel(3);
	m_cmbColour5.SetCurSel(4);
	m_cmbColour6.SetCurSel(5);
	m_cmbColour7.SetCurSel(0);
	m_cmbColour8.SetCurSel(1);
	
	int i = 0;
	
	if (++i < m_astrNames->GetSize())
	{
		m_cmbChan1.SetCurSel(i);
	}
	else
	{
		m_cmbChan1.SetCurSel(0);
	}
	OnSelchangeChan1();
	
	if (++i < m_astrNames->GetSize())
	{
		m_cmbChan2.SetCurSel(i);
	}
	else
	{
		m_cmbChan2.SetCurSel(0);
	}
	OnSelchangeChan2();
	
	if (++i < m_astrNames->GetSize())
	{
		m_cmbChan3.SetCurSel(i);
	}
	else
	{
		m_cmbChan3.SetCurSel(0);
	}
	OnSelchangeChan3();
	
	if (++i < m_astrNames->GetSize())
	{
		m_cmbChan4.SetCurSel(i);
	}
	else
	{
		m_cmbChan4.SetCurSel(0);
	}
	OnSelchangeChan4();
	
	if (++i < m_astrNames->GetSize())
	{
		m_cmbChan5.SetCurSel(i);
	}
	else
	{
		m_cmbChan5.SetCurSel(0);
	}
	OnSelchangeChan5();
	
	if (++i < m_astrNames->GetSize())
	{
		m_cmbChan6.SetCurSel(i);
	}
	else
	{
		m_cmbChan6.SetCurSel(0);
	}
	OnSelchangeChan6();
	
	if (++i < m_astrNames->GetSize())
	{
		m_cmbChan7.SetCurSel(i);
	}
	else
	{
		m_cmbChan7.SetCurSel(0);
	}
	OnSelchangeChan7();
	
	if (++i < m_astrNames->GetSize())
	{
		m_cmbChan8.SetCurSel(i);
	}
	else
	{
		m_cmbChan8.SetCurSel(0);
	}
	OnSelchangeChan8();
}


void CChannelDlg::OnSelchangeChan1()
{
	int i = m_cmbChan1.GetCurSel();
	int j = ((i == 0) || ((*m_astrUnits)[i].IsEmpty())) ? 0 : 3;	// Digital / Analogue 2 d.p.
	m_cmbDecPl1.SetCurSel(j);	
}

void CChannelDlg::OnSelchangeChan2()
{
	int i = m_cmbChan2.GetCurSel();
	int j = ((i == 0) || ((*m_astrUnits)[i].IsEmpty())) ? 0 : 3;	// Digital / Analogue 2 d.p.
	m_cmbDecPl2.SetCurSel(j);	
}

void CChannelDlg::OnSelchangeChan3()
{
	int i = m_cmbChan3.GetCurSel();
	int j = ((i == 0) || ((*m_astrUnits)[i].IsEmpty())) ? 0 : 3;	// Digital / Analogue 2 d.p.
	m_cmbDecPl3.SetCurSel(j);	
}

void CChannelDlg::OnSelchangeChan4()
{
	int i = m_cmbChan4.GetCurSel();
	int j = ((i == 0) || ((*m_astrUnits)[i].IsEmpty())) ? 0 : 3;	// Digital / Analogue 2 d.p.
	m_cmbDecPl4.SetCurSel(j);	
}

void CChannelDlg::OnSelchangeChan5()
{
	int i = m_cmbChan5.GetCurSel();
	int j = ((i == 0) || ((*m_astrUnits)[i].IsEmpty())) ? 0 : 3;	// Digital / Analogue 2 d.p.
	m_cmbDecPl5.SetCurSel(j);	
}

void CChannelDlg::OnSelchangeChan6()
{
	int i = m_cmbChan6.GetCurSel();
	int j = ((i == 0) || ((*m_astrUnits)[i].IsEmpty())) ? 0 : 3;	// Digital / Analogue 2 d.p.
	m_cmbDecPl6.SetCurSel(j);	
}

void CChannelDlg::OnSelchangeChan7()
{
	int i = m_cmbChan7.GetCurSel();
	int j = ((i == 0) || ((*m_astrUnits)[i].IsEmpty())) ? 0 : 3;	// Digital / Analogue 2 d.p.
	m_cmbDecPl7.SetCurSel(j);	
}

void CChannelDlg::OnSelchangeChan8()
{
	int i = m_cmbChan8.GetCurSel();
	int j = ((i == 0) || ((*m_astrUnits)[i].IsEmpty())) ? 0 : 3;	// Digital / Analogue 2 d.p.
	m_cmbDecPl8.SetCurSel(j);	
}
