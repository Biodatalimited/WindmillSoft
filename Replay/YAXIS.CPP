// yaxis.cpp : Implementation of class CYAxis
//

#include "stdafx.h"

#include "yaxis.h"
#include "channel.h"
#include "round.h"

#include <float.h>
#include <math.h>

CYAxis::CYAxis(CString strName)
{
	int i = 0;
	int j = 0;
	
	while ((i < strName.GetLength()) && (i < MAXUNITSLEN))
	{
		m_szUnits[j++] = strName[i++];
		m_szUnits[j++] = '\n';
	}
    m_szUnits[j] = '\0';
    
	m_flMinVal = FLT_MAX;
	m_flMaxVal = -FLT_MAX;
	m_flResolution = 1.0F;
	m_nDecPlaces = 0;
}

CYAxis::~CYAxis()
{
	m_listChan.RemoveAll();
}

void CYAxis::ScaleAxis()
{
	extern const int nMaxWinExtent;
	
	if (strlen(m_szUnits) == 0)
	{
		// Digital axis
		m_lYFloor = 0;
		m_lYCeiling = 2 * m_listChan.GetCount() + 1;
	}
	else
	{
		m_lYFloor = MakeLong(m_flMinVal / m_flResolution);
		m_lYCeiling = MakeLong(m_flMaxVal / m_flResolution);
		Round(m_lYFloor, m_lYCeiling);
	}

	m_flScaleFactor = (float)nMaxWinExtent / (float)(m_lYCeiling - m_lYFloor);
	
	int i = 0;
	for (POSITION pos = m_listChan.GetHeadPosition(); pos != NULL; i++)
	{
		CChannel* pChan = (CChannel*)m_listChan.GetNext(pos);
		long lFloor = (strlen(m_szUnits) == 0) ? -(2 * i + 1) : m_lYFloor;
		pChan->ScaleChannel(m_flScaleFactor, lFloor, m_flResolution);
	}
	
	m_lYBottom = m_lYFloor;
	m_lYTop = m_lYCeiling;
	m_nYBottom = 0;
	m_nYTop = nMaxWinExtent;
	
}

void CYAxis::LinkChannel(CChannel* pChan)
{
	m_listChan.AddTail(pChan);
	
	pChan->SetYAxis(this);
	
	if (pChan->GetResolution() < m_flResolution)
	{
		m_flResolution = pChan->GetResolution();
		m_nDecPlaces = MakeInt((float)log10(1.0 / m_flResolution));
	}
	if (pChan->GetMinVal() < m_flMinVal)
	{
		m_flMinVal = pChan->GetMinVal();
	}
	if (pChan->GetMaxVal() > m_flMaxVal)
	{
		m_flMaxVal = pChan->GetMaxVal();
	}
}

void CYAxis::DrawAxis(CDC* pDC, CRect& rectFrame)
{
	extern const int nAxisWidth;
	
	// Set pen, font, etc.
	
//	CPen NewPen;
//	NewPen.CreatePen(PS_SOLID, 0, RGB(255, 255, 255));
//	CPen* pOldPen = pDC->SelectObject(&NewPen);
	
//	CFont NewFont;
//	NewFont.CreateFont(180, 						// Height
//					   0,							// Width
//					   0,							// Escapement
//					   0,							// Orientation
//					   FW_DONTCARE,					// Weight
//					   0,							// Italic
//					   0,							// Underline
//					   0,							// Strikeout
//					   ANSI_CHARSET,				// Character Set
//					   OUT_DEFAULT_PRECIS,			// Output Precision
//					   CLIP_DEFAULT_PRECIS,			// Clipping Precision
//					   PROOF_QUALITY,				// Quality
//					   FIXED_PITCH | FF_DONTCARE,	// Pitch and Family
//					   "Fixedsys");					// Face Name
//					   
//	CFont* pOldFont = pDC->SelectObject(&NewFont);
	
//	pDC->SetTextColor(RGB(255, 255, 255));
//	pDC->SetBkMode(TRANSPARENT);
	
	// Draw vertical axis
	pDC->MoveTo(rectFrame.right, rectFrame.bottom);
	pDC->LineTo(rectFrame.right, rectFrame.top);
	
	
	int i, j;
	CRect rectLabel = rectFrame;
    rectLabel.right -= 100;
    
	if (strlen(m_szUnits) == 0)
	{
		// Digital axis
		pDC->MoveTo((rectFrame.right - 100), rectFrame.bottom);
		pDC->LineTo(rectFrame.right, rectFrame.bottom);
		pDC->MoveTo((rectFrame.right - 100), rectFrame.top);
		pDC->LineTo(rectFrame.right, rectFrame.top);
		
		for (i = 0; i < m_listChan.GetCount(); i++)
		{
			CChannel* pChan = (CChannel*)m_listChan.GetAt(m_listChan.FindIndex(i));

			j = rectFrame.bottom + (int)(((2 * (long)i + 1) * (rectFrame.top - rectFrame.bottom)) / (2 * m_listChan.GetCount() + 1));
			pDC->MoveTo((rectFrame.right - 100), j);
			pDC->LineTo(rectFrame.right, j);
			rectLabel.top = j + 200; 
			rectLabel.bottom = j - 200;
			pDC->DrawText(pChan->m_strValA, -1, rectLabel, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

			j = rectFrame.bottom + (int)(((2 * (long)i + 2) * (rectFrame.top - rectFrame.bottom)) / (2 * m_listChan.GetCount() + 1));
			pDC->MoveTo((rectFrame.right - 100), j);
			pDC->LineTo(rectFrame.right, j);
			rectLabel.top = j + 200; 
			rectLabel.bottom = j - 200;
			pDC->DrawText(pChan->m_strValB, -1, rectLabel, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}
	}
	else
	{			
		// Draw tick marks
		char szValue[20];
	
		for (i = 0; i <= 5; i++)
		{
			j = rectFrame.bottom + (int)(((long)i * (rectFrame.top - rectFrame.bottom)) / 5);
			pDC->MoveTo((rectFrame.right - 100), j);
			pDC->LineTo(rectFrame.right, j);
		
			rectLabel.SetRect(rectFrame.left, j + 200, rectFrame.right - 100, j - 200);
						  		
			sprintf(szValue, "%g", (m_lYBottom + (i * ((m_lYTop - m_lYBottom) / 5))) * m_flResolution);
//		j = strlen(szValue) - 1;
//		if (szValue[j] == '.') szValue[j] = 0;
			pDC->DrawText(szValue, -1, rectLabel, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}

		// Label axis
	
		int nSpace = (rectFrame.top - rectFrame.bottom - ((strlen(m_szUnits) / 2) * 240)) / 2;
		rectLabel.SetRect(rectFrame.left + 200, rectFrame.top - nSpace,
						  rectFrame.left + 500, rectFrame.bottom);
		pDC->DrawText(m_szUnits, -1, rectLabel, DT_LEFT);
	}
	
//	pDC->SelectObject(pOldPen);
//	pDC->SelectObject(pOldFont);
}	

void CYAxis::SetAxis(CRect rectLimits, BOOL bRound)
{
	extern const int nMaxWinExtent;
	extern const int nMinRange;
	
	if (strlen(m_szUnits) == 0) return;		// Digital axis
	
	if ((rectLimits.top - rectLimits.bottom) < nMinRange)
	{
		rectLimits.top = min((rectLimits.bottom + nMinRange), nMaxWinExtent);
		rectLimits.bottom = rectLimits.top - nMinRange;
	}
	
	m_lYBottom = MakeLong(rectLimits.bottom / m_flScaleFactor) + m_lYFloor;
	m_lYTop = MakeLong(rectLimits.top / m_flScaleFactor) + m_lYFloor;
	if (bRound)
	{
		Round(m_lYBottom, m_lYTop);
	}
	m_nYBottom = EngToLog(m_lYBottom);
	m_nYTop = EngToLog(m_lYTop);
}

/*********************************************************************
void CYAxis::ZoomOut()
{
	extern const int nMaxWinExtent;
	
	m_flYBottom = m_flYFloor;
	m_flYTop = m_flYCeiling;
	m_nYBottom = 0;
	m_nYTop = nMaxWinExtent;
}
*********************************************************************/
		
int CYAxis::EngToLog(long lValue)
{
	return MakeInt((lValue - m_lYFloor) * m_flScaleFactor);
}

int CYAxis::EngToLog(long lValue1, long lValue2)
{
	return MakeInt((lValue2 - lValue1) * m_flScaleFactor);
}
	

