//  Create demo channels in OnOpenDocument

//	m_nScanCount = 1000;
//	AddChannel("Time", 0, RGB(255, 255, 255), -1);
//	AddChannel("Temp1", 1, RGB(255, 0, 0), 2);
//	AddChannel("Temp2", 2, RGB(0, 255, 0), 2);
//	AddChannel("Pressure", 3, RGB(0, 0, 255), 3);
//	
//	AddYAxis("D\ne\ng\n \nC");	
//	CObject* pobjChan;
//	m_mapChan.Lookup("Temp1", pobjChan);
//	((CYAxis*)(m_listYAxis.GetHead()))->LinkChannel((CChannel*)pobjChan);
//	m_mapChan.Lookup("Temp2", pobjChan);
//	((CYAxis*)(m_listYAxis.GetHead()))->LinkChannel((CChannel*)pobjChan);	
//	((CYAxis*)(m_listYAxis.GetHead()))->ScaleAxis();
//	
//	AddYAxis("k\nP\na\ns\nc\na\nl");
//	POSITION pos = m_listYAxis.GetHeadPosition();
//	m_listYAxis.GetNext(pos);		// Discard first axis
//	CYAxis* pYAxis = (CYAxis*)m_listYAxis.GetNext(pos);
//	m_mapChan.Lookup("Pressure", pobjChan);
//	pYAxis->LinkChannel((CChannel*)pobjChan);
//	pYAxis->ScaleAxis();	

//	DWORD dwFlags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
//	char szFilter[] = "Logger Files (*.wl)|*.wl|All Files (*.*)|*.*||";
//	CFileDialog dlgFile(TRUE, NULL, NULL, dwFlags, szFilter, NULL);
//	dlgFile.m_ofn.lpstrTitle = "Windmill Replay - Open Logger File";
//	dlgFile.SetWindowPos(NULL, 200, 300, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
//	if (dlgFile.DoModal() == IDOK)
//	{
//		m_strPathName = dlgFile.GetPathName();
//	}
//	else
//	{
//		if (m_strPathName.IsEmpty()) return FALSE;
//	}
	

//  Create data values in AddChannel

//	CTime time = (timStart / 15) * 15;
//	char szValue[20];
//		
//	for (int i = 0; i < 1000; i++)
//	{
//		if (nColumn == 0)
//		{
//			time += 15;
//			strcpy(szValue, time.Format("%H:%M:%S"));
//		}
//		else
//		{
//			double dValue = (float)(10 * (nColumn + ((int)(4096 * (0.9 * sin((float)i/(nColumn * 10)) + ((float)rand() * (float)rand() / (10.0 * RAND_MAX * RAND_MAX)) - 0.05)) / 4096.0F)));
//			_gcvt(dValue, 6, szValue);
//		}
//		pNewChan->AddValue(szValue);
//	}
