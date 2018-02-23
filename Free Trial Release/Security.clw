; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=Enter
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Security.h"

ClassCount=3
Class1=CSecurityApp
Class2=CSecurityDlg

ResourceCount=4
Resource2=IDD_SECURITY_DIALOG
Resource3=IDD_SECURITY_DIALOG (English (U.S.))
Resource1=IDR_MAINFRAME
Class3=Enter
Resource4=IDD_ENTER_DIALOG

[CLS:CSecurityApp]
Type=0
HeaderFile=Security.h
ImplementationFile=Security.cpp
Filter=N

[CLS:CSecurityDlg]
Type=0
HeaderFile=SecurityDlg.h
ImplementationFile=SecurityDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_USER_KEY



[DLG:IDD_SECURITY_DIALOG]
Type=1
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Class=CSecurityDlg

[DLG:IDD_SECURITY_DIALOG (English (U.S.))]
Type=1
Class=CSecurityDlg
ControlCount=18
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_COMPUTER_CODE,static,1342308352
Control9=IDC_DATE,static,1342308352
Control10=IDC_INSTALLED_PROGS,static,1342308352
Control11=IDC_INVALID,static,1342308352
Control12=IDC_RELEASE,static,1342308352
Control13=IDC_BUTTON1,button,1342242816
Control14=IDC_STATIC,static,1342308352
Control15=IDC_RELEASE_KEY,static,1342308352
Control16=IDC_USER_KEY,static,1342308352
Control17=IDC_STATIC,static,1342308352
Control18=IDC_CUST_LABEL,static,1342308352

[DLG:IDD_ENTER_DIALOG]
Type=1
Class=Enter
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDC_CODE_ENTER,edit,1350631552
Control3=IDC_STATIC,static,1342308352
Control4=IDC_CUSTOMER,edit,1350631552
Control5=IDC_STATIC,static,1342308352

[CLS:Enter]
Type=0
HeaderFile=Enter.h
ImplementationFile=Enter.cpp
BaseClass=CDialog
Filter=D
LastObject=Enter
VirtualFilter=dWC

