// playdoc.cpp : implementation of the CReplayDoc class
//

#include "stdafx.h"
#include "replay.h"

#include "chandlg.h"
#include "readdlg.h"
#include "playdoc.h"
#include "playfrm.h"
#include "yaxis.h"
#include "channel.h"

#include <math.h>
#include <ctype.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

const int nMaxLineLen = 2000;
const int nMaxLines = 16000;

const long lMaxChanRec = 2000L;
const int nChanRecLen = 75;

char szLine[nMaxLineLen];


/////////////////////////////////////////////////////////////////////////////
// CReplayDoc

IMPLEMENT_DYNCREATE(CReplayDoc, CDocument)

BEGIN_MESSAGE_MAP(CReplayDoc, CDocument)
	//{{AFX_MSG_MAP(CReplayDoc)
	ON_COMMAND(ID_CHANNELS, OnChannels)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplayDoc construction/destruction

CReplayDoc::CReplayDoc()
{
	// TODO: add one-time construction code here
	m_hCoords = NULL;
	m_strPathName.Empty();
	m_strCaption.Empty();
	m_strPathName = "HHElp";
}

CReplayDoc::~CReplayDoc()
{
}

void CReplayDoc::SetTitle(const char* pszTitle)
{
	m_strTitle = m_strCaption;
	UpdateFrameCounts();        // will cause name change in views
}

void CReplayDoc::DeleteContents()
{
	if (m_hCoords != NULL)
	{
		GlobalUnlock(m_hCoords);
		GlobalFree(m_hCoords);
	}
	
	POSITION pos = m_listChan.GetHeadPosition();
	while (pos != NULL)
	{
		delete (CChannel*)m_listChan.GetNext(pos);
	}
	
	pos = m_listYAxis.GetHeadPosition();
	while (pos != NULL)
	{
		delete (CYAxis*)m_listYAxis.GetNext(pos);
	}
	
	m_astrNames.RemoveAll();
	m_astrUnits.RemoveAll();
	m_listChan.RemoveAll();
	m_mapChan.RemoveAll();
	m_listYAxis.RemoveAll();
	
	m_nCursor = -1;		
}

BOOL CReplayDoc::OnOpenDocument(const char* pszPathName)
{
	DeleteContents();
	
	if (!m_ioFile.Open(pszPathName, CFile::modeRead | CFile::shareDenyNone | CFile::typeText)) return FALSE;
	m_strPathName = pszPathName;
	
	/** Get start time from file. This is on the second line.
	*** The whole line is written to a panel on the screen.
	**/
	m_ioFile.ReadString(szLine, nMaxLineLen);
	m_ioFile.ReadString(szLine, nMaxLineLen);
	int i = strlen(szLine);
	while ((i > 0) && (iscntrl(szLine[--i])))
	{
		szLine[i] = 0;		// Remove trailing control characters
	}
	((CReplayFrame*)(AfxGetApp()->m_pMainWnd))->SetStartTime(szLine);
	
	// Set File Pointer to channel name header line in file
	FindChanNames();
		
	// Get channel names from file. Add them to a string array
	char* pszField;
	i = 0;
	while (strlen(pszField = GetField(i++)) != 0)
	{
		m_astrNames.Add(pszField);
	}
	
	// Get units names from file. Add them to a string array
	m_ioFile.ReadString(szLine, nMaxLineLen);
	for (i = 0; i < m_astrNames.GetSize(); i++)
	{
		m_astrUnits.Add(GetField(i));
	}
	
	// Get "Time" data from file
	int nDecPlaces;
	char* cToken;
	m_ioFile.ReadString(szLine, nMaxLineLen);
	pszField = GetField(0);
	// Find first :
	cToken = strchr (pszField, ':');
	if (cToken != NULL)
	{
		// Point to after first :	
		++cToken;
		// If there is a second :
		if ( strchr ( cToken, ':' ) != NULL ) {
			// This is HH:MM:SS
			nDecPlaces = -2;
		} else {
			// This is MM:SS
			nDecPlaces = -3;
		}
	}
	else if (strchr(pszField, '.') != NULL)
	{
		// This is sec.hundredths from start
		nDecPlaces = 2;
	}
	else
	{
		// This is seconds from start
		nDecPlaces = 0;
	}
	
	// Time is regarded as another channel 
	AddChannel("Time", 0, RGB(0, 0, 0), nDecPlaces);
	
	m_ioFile.Close();
	m_dlgChan.SetChannels(&m_astrNames, &m_astrUnits);
    SelectChannels();
    
	CreateYAxes();

	m_nScanCount = ((CChannel*)m_listChan.GetHead())->GetLength();
	
	m_hCoords = GlobalAlloc(GMEM_MOVEABLE, (DWORD)m_nScanCount * sizeof(CPoint));
	if (m_hCoords == NULL)
	{
		FatalAppExit(0, "Not enough memory");
	}
	m_pCoords = (CPoint FAR*)GlobalLock(m_hCoords);
	if (m_pCoords == NULL)
	{
		FatalAppExit(0, "Cannot lock memory");
	}

	SetModifiedFlag(FALSE);
	SetPathName(pszPathName);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CReplayDoc serialization

void CReplayDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


/////////////////////////////////////////////////////////////////////////////
// CReplayDoc diagnostics

#ifdef _DEBUG
void CReplayDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CReplayDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CReplayDoc commands

void CReplayDoc::OnChannels()
{
	SelectChannels();
	CreateYAxes();
	UpdateAllViews(NULL);
}

void CReplayDoc::SelectChannels()
{
	// Display "Select Channels" dialog box
	if (m_dlgChan.DoModal() != IDOK) return;
	
	DeleteDataChannels();
	DeleteYAxes();
	
	m_ioFile.Open(m_strPathName, CFile::modeRead | CFile::shareDenyNone | CFile::typeText);
	
	// Get channel data from file
	int i;
	
	i = m_dlgChan.m_nChan1;
	if (i > 0) AddChannel(m_astrNames[i], i, GetColour(m_dlgChan.m_nColour1), m_dlgChan.m_nDecPl1 - 1);
	
	i = m_dlgChan.m_nChan2;
	if (i > 0) AddChannel(m_astrNames[i], i, GetColour(m_dlgChan.m_nColour2), m_dlgChan.m_nDecPl2 - 1);
	
	i = m_dlgChan.m_nChan3;
	if (i > 0) AddChannel(m_astrNames[i], i, GetColour(m_dlgChan.m_nColour3), m_dlgChan.m_nDecPl3 - 1);
	
	i = m_dlgChan.m_nChan4;
	if (i > 0) AddChannel(m_astrNames[i], i, GetColour(m_dlgChan.m_nColour4), m_dlgChan.m_nDecPl4 - 1);
	
	i = m_dlgChan.m_nChan5;
	if (i > 0) AddChannel(m_astrNames[i], i, GetColour(m_dlgChan.m_nColour5), m_dlgChan.m_nDecPl5 - 1);
	
	i = m_dlgChan.m_nChan6;
	if (i > 0) AddChannel(m_astrNames[i], i, GetColour(m_dlgChan.m_nColour6), m_dlgChan.m_nDecPl6 - 1);
	
	i = m_dlgChan.m_nChan7;
	if (i > 0) AddChannel(m_astrNames[i], i, GetColour(m_dlgChan.m_nColour7), m_dlgChan.m_nDecPl7 - 1);
	
	i = m_dlgChan.m_nChan8;
	if (i > 0) AddChannel(m_astrNames[i], i, GetColour(m_dlgChan.m_nColour8), m_dlgChan.m_nDecPl8 - 1);
	
	m_ioFile.Close();
}

void CReplayDoc::DeleteYAxes()
{
	POSITION pos = m_listChan.GetHeadPosition();

	while (pos != NULL)
	{
		((CChannel*)m_listChan.GetNext(pos))->SetYAxis(NULL);
	}
	
	pos = m_listYAxis.GetHeadPosition();
	while (pos != NULL)
	{
		delete (CYAxis*)m_listYAxis.GetNext(pos);
	}
	
	m_listYAxis.RemoveAll();
}

void CReplayDoc:: DeleteDataChannels(void)
{
	POSITION pos = m_listChan.GetHeadPosition();
	CChannel* pTime = (CChannel*)m_listChan.GetNext(pos);	// Skip over "Time" channel

	while (pos != NULL)
	{
		delete (CChannel*)m_listChan.GetNext(pos);
	}
	
	m_listChan.RemoveAll();
	m_mapChan.RemoveAll();
	
	// Restore "Time" channel
	m_listChan.AddTail(pTime);
	m_mapChan.SetAt(m_astrNames[0], pTime);
}

/*************************************************************
*** Read from the file all the data for a channel. 
**/

void CReplayDoc::AddChannel(CString strName, UINT nColumn, COLORREF dwColour, int nDecPlaces)
{
	CReadDlg dlgRead(strName);
	
	// Create the new channel
	CChannel* pNewChan = new CChannel(strName, nColumn, dwColour, nDecPlaces);
	m_listChan.AddTail(pNewChan);
	m_mapChan.SetAt(strName, pNewChan);
	
	// Find header line
	FindChanNames();
	
	// Skip over units line
	m_ioFile.ReadString(szLine, nMaxLineLen);
	
	// Get channel data
	int nLines = 0;
	while ((m_ioFile.ReadString(szLine, nMaxLineLen) != NULL)
				&& (strlen(GetField(0)) != 0) && (nLines++ < nMaxLines))
	{
		pNewChan->AddValue(GetField(nColumn));
	}
	
	dlgRead.DestroyWindow();
}

void CReplayDoc::CreateYAxes(void)
{
	POSITION pos = m_listChan.GetHeadPosition();
	CChannel* pChan = (CChannel*)m_listChan.GetNext(pos);
	
	while (pos != NULL)
	{
		pChan = (CChannel*)m_listChan.GetNext(pos);
		if (pChan->GetYAxis() == NULL)
		{
			// Create axis for this channel
			CString strUnits =
				(pChan->GetDecPlaces() >= 0) ? m_astrUnits[pChan->GetColumn()] : "";
			CYAxis* pYAxis = new CYAxis(strUnits);
			m_listYAxis.AddTail(pYAxis);
			pYAxis->LinkChannel(pChan);
			POSITION p = pos;
			while (p != NULL)
			{
				// Link other channels with same units to this axis
				pChan = (CChannel*)m_listChan.GetNext(p);
				if (((pChan->GetDecPlaces() >= 0) && (m_astrUnits[pChan->GetColumn()] == strUnits))
					|| ((pChan->GetDecPlaces() < 0) && (strUnits == "")))
				{
					pYAxis->LinkChannel(pChan);
				}
			}
			pYAxis->ScaleAxis();
		}
	}
}

void CReplayDoc::SetXAxis(CString strName)
{
	CObject* pobjChan;	
	m_mapChan.Lookup(strName, pobjChan);
	CChannel* pChan = (CChannel*)pobjChan;
	
	int FAR* pnCoords = pChan->GetCoords();
	
	for (int i = 0; i < m_nScanCount; i++)
	{
		m_pCoords[i].x = pnCoords[i];
	}
}
	
void CReplayDoc::SetCursor(int nScan)
{
	m_nCursor = nScan;
}

/****************************************************
*** Start at the top of the file and look for the line with channel
*** names. This starts with either 'Time' or 'Date'. 
*** If the format includes Date then set m_nColOffset to 1
*** else set m_nColOffset to 0. Return leaving the file pointer at
*** the desired line
**/ 

void CReplayDoc::FindChanNames()
{
	m_ioFile.SeekToBegin();
	
	while (m_ioFile.ReadString(szLine, nMaxLineLen) != NULL)
	{
		CString strChan(szLine, 4);
		
		if (strChan == "Time")
		{
			m_nColOffset = 0;
			return;
		}
		else if (strChan == "Date")
		{
			m_nColOffset = 1;
			return;
		}
	}

	FatalAppExit(0, "Invalid File Format - Channel names not found");
}

char* CReplayDoc::GetField(int nField)
{
	int i = 0;
	
//	for (int j = 0; j < nField; j++)
//	{
//		// Skip over spaces/tabs/nulls
//		while ((szLine[i] == ' ') || (szLine[i] == '\t') || (szLine[i] == '\0')) i++;
//		// Skip over field
//		while ((szLine[i] != ' ') && (szLine[i] != '\t')
//				&& (szLine[i] != '\0') && (szLine[i] != '\n')) i++;
//	}
//	
//	// Skip over spaces/tabs/nulls
//	while ((szLine[i] == ' ') || (szLine[i] == '\t') || (szLine[i] == '\0')) i++;
//	
//	// Mark end of field
//	j = i;
//	while ((szLine[j] != ' ') && (szLine[j] != '\t')
//			&& (szLine[j] != '\0') && (szLine[j] != '\n')) j++;
//	if (szLine[j] == '\n') szLine[j + 1] = '\n';
//	szLine[j] = '\0';
	
	for (int j = 0; j < (nField + m_nColOffset); j++)
	{
		// Skip over field
		while ((szLine[i] != '\t') && (szLine[i] != '\0') && (szLine[i] != '\n')) i++;
		// Skip over end of field marker
		i++;
	}
	
	// Mark end of field
	j = i;
	while ((szLine[j] != '\t') && (szLine[j] != '\0') && (szLine[j] != '\n')) j++;
	if (szLine[j] == '\n') szLine[j + 1] = '\n';
	szLine[j] = '\0';
	
	return (&szLine[i]);
}

COLORREF CReplayDoc::GetColour(int nIndex)
{
	switch (nIndex)
	{
		case 0: return RGB(255, 0, 0);		// Red
		case 1: return RGB(0, 255, 0);		// Green
		case 2: return RGB(0, 0, 255);		// Blue
		case 3: return RGB(0, 255, 255);	// Cyan
		case 4: return RGB(255, 0, 255);	// Magenta
		case 5: return RGB(255, 255, 0);	// Yellow
		
		case 'R': return RGB(255, 0, 0);	// Red
		case 'G': return RGB(0, 255, 0);	// Green
		case 'B': return RGB(0, 0, 255);	// Blue
		case 'C': return RGB(0, 255, 255);	// Cyan
		case 'M': return RGB(255, 0, 255);	// Magenta
		case 'Y': return RGB(255, 255, 0);	// Yellow
		
		case 'r': return RGB(255, 0, 0);	// Red
		case 'g': return RGB(0, 255, 0);	// Green
		case 'b': return RGB(0, 0, 255);	// Blue
		case 'c': return RGB(0, 255, 255);	// Cyan
		case 'm': return RGB(255, 0, 255);	// Magenta
		case 'y': return RGB(255, 255, 0);	// Yellow
	}
	return RGB(255, 255, 255);
}

CString CReplayDoc::ColourName(COLORREF dwColour)
{
	switch (dwColour)
	{
		case RGB(255, 0, 0):	return "Red";
		case RGB(0, 255, 0):	return "Green";
		case RGB(0, 0, 255):	return "Blue";
		case RGB(0, 255, 255):	return "Cyan";
		case RGB(255, 0, 255):	return "Magenta";
		case RGB(255, 255, 0):	return "Yellow";
	}
	return "";
}

		