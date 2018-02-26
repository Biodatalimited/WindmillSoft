// yaxis.h : Interface of the CYAxis class
//

#define MAXUNITSLEN 12

// Forward declarations
class CChannel;
class CReplayView;

class CYAxis : public CObject
{
public:
	CYAxis(CString strName);
	~CYAxis();
	
// Attributes
public:
	char m_szUnits[MAXUNITSLEN * 2 + 1];

protected:
	int m_nDecPlaces;
	int m_nYBottom;
	int m_nYTop;
	long m_lYFloor;
	long m_lYCeiling;
	long m_lYBottom;
	long m_lYTop;
	float m_flMinVal;
	float m_flMaxVal;
	float m_flResolution;
	float m_flScaleFactor;
	CObList m_listChan;
	
// Operations
public:
	int GetLogYBottom() { return m_nYBottom; }
	int GetLogYTop() { return m_nYTop; }
	long GetFloor() { return m_lYFloor; }
	int GetDecPlaces() { return m_nDecPlaces; }
	float GetResolution() { return m_flResolution; }
	float GetScale() { return m_flScaleFactor; }
	void LinkChannel(CChannel* pChan);
	void UnlinkChannel(CChannel* pChan);
	void ScaleAxis();
	int EngToLog(long lValue);
	int EngToLog(long lValue1, long lValue2);
	void DrawAxis(CDC* pDC, CRect& rectFrame);
	void SetAxis(CRect rectLimits, BOOL bRound);
};
