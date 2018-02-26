// channel.h : Interface of the CChannel class
//

// Forward declarations
class CReplayView;
class CYAxis;

class CChannel : public CObject
{
public:
	CChannel(CString strName, UINT nColumn, COLORREF dwColour, int nDecPlaces);
	~CChannel();
		
// Attributes
public:
	CString m_strValA;
	CString m_strValB;
	static CTime m_TimeOrigin;
	
protected:
	const int m_nMaxLength;
	int m_nLength;
	CString m_strName;
	UINT m_nColumn;
	COLORREF m_dwColour;
	CYAxis* m_pYAxis;
	HGLOBAL m_hYcoords;
	HGLOBAL m_hData;
	int FAR* m_pnYcoords;
	long FAR* m_plData;
	int m_nDecPlaces;
	float m_flResolution;
	long m_lMinVal;
	long m_lMaxVal;
	long m_lPrevious;
	long m_lDaySecs;

// Operations
public:
	static long TimeStampToSeconds(char* pszTime);
	CString& GetName() { return m_strName; }
	UINT GetColumn() { return m_nColumn; }
	COLORREF GetColour() { return m_dwColour; }
	int GetLength() { return m_nLength; }
	float GetResolution() { return m_flResolution; }
	int GetDecPlaces() { return m_nDecPlaces; }
	float GetMinVal() { return (m_lMinVal * m_flResolution); }
	float GetMaxVal() { return (m_lMaxVal * m_flResolution); }
	int FAR* GetCoords() { return m_pnYcoords; }
	void SetYAxis(CYAxis* pYAxis);
	CYAxis* GetYAxis() { return m_pYAxis; }
	void AddValue(char* pszValue);
	CString GetValue(int nSample);
	long GetRawValue(int nSample) { return m_plData[nSample]; }
	void ScaleChannel(float flScale, long lFloor, float flResolution);
	void DrawTrend(CDC* pDC, CReplayView* pView);
	void DrawKey(CDC* pDC, CPoint* pKey);
};

