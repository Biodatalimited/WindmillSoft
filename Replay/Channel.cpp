// channel.cpp : Implementation of CChannel class
//

#include "stdafx.h"
#include "resource.h"

#include "channel.h"
#include "yaxis.h"
#include "chandlg.h"
#include "colordlg.h"
#include "playdoc.h"
#include "playview.h"
#include "round.h"

#include <math.h>
#include <limits.h>

// Time stamps are stored as a displacement from 1st Jan 1970
CTime CChannel::m_TimeOrigin = CTime(1970, 1, 1, 0, 0, 0);

CChannel::CChannel(CString strName, UINT nColumn, COLORREF dwColour, int nDecPlaces)
	: m_nMaxLength(16000L)
{
	m_strName = strName;
	m_nColumn = nColumn;
	m_dwColour = dwColour;
	m_nDecPlaces = nDecPlaces;
	m_flResolution = (nDecPlaces >= 0) ? (float)pow(10.0, -nDecPlaces) : 1.0F;
	m_pYAxis = NULL;
	m_nLength = 0;
	m_lMinVal = LONG_MAX;
	m_lMaxVal = LONG_MIN;
	m_lPrevious = 0L;
	m_lDaySecs = 0L;
	m_strValA.Empty();
	m_strValB.Empty();
	
	m_hYcoords = GlobalAlloc(GMEM_MOVEABLE, (DWORD)m_nMaxLength * sizeof(int));
	if (m_hYcoords == NULL)
	{
		FatalAppExit(0, "Not enough memory");
	}
	m_pnYcoords = (int FAR*)GlobalLock(m_hYcoords);
	if (m_pnYcoords == NULL)
	{
		FatalAppExit(0, "Cannot lock memory");
	}
	m_hData = GlobalAlloc(GMEM_MOVEABLE, (DWORD)m_nMaxLength * sizeof(long));
	if (m_hData == NULL)
	{
		FatalAppExit(0, "Not enough memory");
	}
	m_plData = (long FAR*)GlobalLock(m_hData);
	if (m_plData == NULL)
	{
		FatalAppExit(0, "Cannot lock memory");
	}
}

CChannel::~CChannel()
{
	GlobalUnlock(m_hData);
	GlobalFree(m_hData);
	GlobalUnlock(m_hYcoords);
	GlobalFree(m_hYcoords);
}

void CChannel::SetYAxis(CYAxis* pYAxis)
{
	m_pYAxis = pYAxis;
}

void CChannel::ScaleChannel(float flScale, long lFloor, float flResolution)
{
//	long lFloor = m_pYAxis->GetFloor();
//	float flResolution = m_pYAxis->GetResolution();
//	float flScale = m_pYAxis->GetScale();
	
	long lFactor = MakeLong(m_flResolution / flResolution);
	
	for (int i = 0; i < m_nLength; i++)
	{
		if (m_plData[i] == LONG_MIN)
		{
			m_pnYcoords[i] = 0;
		}
		else
		{
			m_pnYcoords[i] = MakeInt(((m_plData[i] * lFactor) - lFloor) * flScale);
		}
	}
}
	
long CChannel::TimeStampToSeconds(char* pszTime)
{
	int nYear = TwoFigsToInt(pszTime + 5);
	//////////////////////////////////////
//	if ((*(pszTime + 5) == '0') || (*(pszTime + 6) == '9')) return 0;
	//////////////////////////////////////
	int nMonth = MonthToInt(pszTime + 2);
	int nDay = TwoFigsToInt(pszTime);
	int nHour = TwoFigsToInt(pszTime + 8);
	int nMin = TwoFigsToInt(pszTime + 11);
	int nSec = TwoFigsToInt(pszTime + 14);
		
	nYear += (nYear >= 70) ? 1900 : 2000;		// Range of CTime is 1970 - 2038

	CTimeSpan ts = CTime(nYear, nMonth, nDay, nHour, nMin, nSec) - m_TimeOrigin;
	return ts.GetTotalSeconds();

}


/****************************************************
*** Adds a value to a channel array.Special arrangments for
*** time stamp.
**/

void CChannel::AddValue(char* pszValue)
{
#ifdef DARAD
	if (pszValue[1] == ' ')
#else
	if ((pszValue[0] == 'E') &&
		(pszValue[1] == 'r') &&
		(pszValue[2] == 'r') &&
		(pszValue[3] == 'o') &&
		(pszValue[4] == 'r') &&
		(pszValue[5] == ' '))
#endif
	{
		m_plData[m_nLength++] = LONG_MIN;
		return;
	}
	
	// Convert the passed string to this value
	long lValue;
	
	if (m_nDecPlaces >= 0)
	{
		// Decimal format
		lValue = MakeLong(float(atof(pszValue) / m_flResolution));
	}
	else if (m_nDecPlaces == -1)
	{
		// Digital channel
		if (m_strValA.IsEmpty()) m_strValA = pszValue;
		if (strcmp(pszValue, m_strValA) == 0) lValue = 0L;
		else
		{
			if (m_strValB.IsEmpty()) m_strValB = pszValue;
			if (strcmp(pszValue, m_strValB) == 0) lValue = 1L;
			else
			{
				// FatalAppExit(0, CString(pszValue) + " is not a valid digital state");
				m_plData[m_nLength++] = LONG_MIN;
				return;
			}
		}
	}
	else if (m_nDecPlaces == -2)
	{
		// HH:MM:SS format
		if ((strlen(pszValue) != 8) || (pszValue[2] != ':') || (pszValue[5] != ':'))
		{
			FatalAppExit(0, CString(pszValue) + " is not a valid time stamp");
		}
		int nHH = TwoFigsToInt(pszValue);
		int nMM = TwoFigsToInt(pszValue + 3);
		int nSS = TwoFigsToInt(pszValue + 6);
		lValue = (nHH * 3600L) + (nMM * 60L) + nSS;
		if (lValue < m_lPrevious)
		{
			// Time stamp has passed midnight
			m_lDaySecs += 86400L;
		}
		m_lPrevious = lValue;
		lValue += m_lDaySecs;
	}
	else if (m_nDecPlaces == - 3)
	{
		// MM:SS format
		
		// Find the colon
		char* cToken;
		int nDigits;

		cToken = strchr ( pszValue, ':' );
		
		if ( cToken == NULL )
		{
			FatalAppExit(0, CString(pszValue) + " is not a valid time stamp");
		}
		
		// Get the number of digits in the minutes
		nDigits = strlen (pszValue) - strlen(cToken);

		int nHH = 0;
		int nMM = 0;
		int nSS = TwoFigsToInt(pszValue + nDigits + 1);
		int nIndex;

		// Convert minutes to an integer
		for ( nIndex = 0; nIndex < nDigits ; ++nIndex ) {
			nMM = 10 * nMM + ( *(pszValue + nIndex) - '0' )	;
		}

		
		lValue = (nHH * 3600L) + (nMM * 60L) + nSS;
		if (lValue < m_lPrevious)
		{
			// Time stamp has passed midnight
			m_lDaySecs += 86400L;
		}
		m_lPrevious = lValue;
		lValue += m_lDaySecs;

	}
	else
	{
		// DARAD format
		lValue = TimeStampToSeconds(pszValue);
	}

	m_plData[m_nLength++] = lValue;
		
	if (lValue < m_lMinVal) m_lMinVal = lValue;
	if (lValue > m_lMaxVal) m_lMaxVal = lValue;
}


CString CChannel::GetValue(int nSample)
{
	if (m_plData[nSample] == LONG_MIN) return ("******");
	
	if (m_nDecPlaces >= 0)
	{
		// Decimal format
		char szBuff[20];
		
		sprintf(szBuff, "%.*li", (m_nDecPlaces + 1), m_plData[nSample]);
	
		if (m_nDecPlaces > 0)
		{
			int nLast = strlen(szBuff);
			for (int i = nLast; i >= (nLast - m_nDecPlaces); i--) szBuff[i + 1] = szBuff[i];
	    	szBuff[i + 1] = '.';
		}

		return (szBuff);
	}
	else if (m_nDecPlaces == -1)
	{
		// Digital channel
		return ((m_plData[nSample] == 0L) ? m_strValA : m_strValB);
	}
	else if (m_nDecPlaces == -2)
	{
		// DayDDD HH:MM:SS format
		CTimeSpan ts = m_plData[nSample];
		
		if (m_lMaxVal < 86400L)
		{
			return (ts.Format("%H:%M:%S"));
		}
		else
		{
			return (ts.Format("Day%D %H:%M:%S"));
		}
	}
	else if (m_nDecPlaces == -3)
	{
		// DayDDD HH:MM:SS format
		CTimeSpan ts = m_plData[nSample];
				
		return (ts.Format("%M:%S"));
					
	}
	else
	{
		// DARAD format
		CTime TimeStamp = m_TimeOrigin + CTimeSpan(m_plData[nSample]);
		return TimeStamp.Format("%d %b %y %H:%M:%S");
	}
}

void CChannel::DrawKey(CDC* pDC, CPoint* pKey)
{
	CPen NewPen;
	NewPen.CreatePen(PS_SOLID, 0, m_dwColour);
	CPen* pOldPen = pDC->SelectObject(&NewPen);
	
	pDC->Polyline(pKey, 6);
	
	pDC->SelectObject(pOldPen);
}

void CChannel::DrawTrend(CDC* pDC, CReplayView* pView)
{
	CPoint FAR* pCoords = pView->GetDocument()->GetCoords();
	
	int nILeft = pView->GetILeft();
	int nIRight = pView->GetIRight();
		
	for (int i = nILeft; i <= nIRight; i++)
	{
		pCoords[i].y = m_pnYcoords[i];
	}
	
	COLORREF crColour = pDC->GetNearestColor(m_dwColour);
	if (pDC->IsPrinting())
	{
		if (crColour == RGB(255, 255, 255)) crColour = RGB(0, 0, 0);
	}
	else
	{
		if (crColour == RGB(0, 0, 0)) crColour = RGB(255, 255, 255);
	}
	
	CPen NewPen;
	NewPen.CreatePen(PS_SOLID, 0, crColour);
	CPen* pOldPen = pDC->SelectObject(&NewPen);
	
	pDC->SetWindowOrg(pView->GetLogXLeft(), m_pYAxis->GetLogYBottom());
	pDC->SetWindowExt((pView->GetLogXRight() - pView->GetLogXLeft()),
						(m_pYAxis->GetLogYTop() - m_pYAxis->GetLogYBottom()));
	pDC->Polyline((pCoords + nILeft), (nIRight - nILeft + 1));
	
	pDC->SelectObject(pOldPen);
}

