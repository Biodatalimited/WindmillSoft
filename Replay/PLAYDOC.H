// playdoc.h : interface of the CReplayDoc class
//
/////////////////////////////////////////////////////////////////////////////

// Forward declaration of data structure classes
class CChannel;
class CYAxis;

class CReplayDoc : public CDocument
{
protected: // create from serialization only
	CReplayDoc();
	DECLARE_DYNCREATE(CReplayDoc)

// Attributes
public:

protected:
	int m_nScanCount;
	int m_nCursor;
	int m_nColOffset;
	CObList m_listChan;		// Each element is a CChannel
	CMapStringToOb m_mapChan;
	CObList m_listYAxis;
	HGLOBAL m_hCoords;
	CPoint FAR* m_pCoords;
	CString m_strPathName;
	CString m_strCaption;
	CString m_strStartTime;
	CStringArray m_astrNames;
	CStringArray m_astrUnits;
	CStdioFile m_ioFile;
	CChannelDlg m_dlgChan;
	
public:

// Operations
public:
	BOOL FileOpened() { return !m_strPathName.IsEmpty(); }
	int GetScanCount() { return m_nScanCount; }
	int GetChanCount() { return m_listChan.GetCount(); }
	int GetAxisCount() { return m_listYAxis.GetCount(); }
	void SetCursor(int nScan);
	int GetCursor() { return m_nCursor; }
	CPoint FAR* GetCoords() { return m_pCoords; }
	void AddChannel(CString strName, UINT nColumn, COLORREF dwColour, int nDecPlaces);
	void RemoveChannel();
	void FindChanNames();
	char* GetField(int nField);
	COLORREF GetColour(int nIndex);
	CString ColourName(COLORREF dwColour);
	POSITION GetFirstChanPos() { return m_listChan.GetHeadPosition(); }
	CChannel* GetNextChan(POSITION& pos) { return (CChannel*)m_listChan.GetNext(pos); }
//	void AddYAxis(CString strUnits);
//	void RemoveYAxis();
	void CreateYAxes(void);
	void DeleteYAxes(void);
	POSITION GetFirstYAxisPos() { return m_listYAxis.GetHeadPosition(); }
	CYAxis* GetNextYAxis(POSITION& pos) { return (CYAxis*)m_listYAxis.GetNext(pos); }
	CYAxis* GetFirstYAxis() { return (CYAxis*) m_listYAxis.GetHead(); }
	CYAxis* GetLastYAxis() { return (CYAxis*) m_listYAxis.GetTail(); }
	void SetXAxis(CString strName);
    void SelectChannels();
    void DeleteDataChannels();
    BOOL ValidDirName(const char* pszPathName);
    
// Implementation
public:
	virtual ~CReplayDoc();
	virtual void Serialize(CArchive& ar);	// overridden for document i/o
#ifdef _DEBUG
	virtual	void AssertValid() const;
	virtual	void Dump(CDumpContext& dc) const;
#endif
protected:
    void DeleteContents();
	void SetTitle(const char* pszTitle);
	virtual	BOOL OnOpenDocument(const char* pszPathName);
    
// Generated message map functions
protected:
	//{{AFX_MSG(CReplayDoc)
	afx_msg void OnChannels();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

		