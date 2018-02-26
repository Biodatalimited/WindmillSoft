; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CAboutDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "replay.h"
LastPage=0

ClassCount=8
Class1=CChannelDlg
Class2=CReplayDoc
Class3=CReplayFrame
Class4=CReplayView
Class5=CReadDlg
Class6=CReplayApp
Class7=CAboutDlg
Class8=CTextView

ResourceCount=5
Resource1=IDD_ABOUTBOX
Resource2=IDD_READDIALOG
Resource3=IDR_MAINFRAME
Resource4=IDD_CHANDIALOG
Resource5=IDD_COLOUR_DIALOG

[CLS:CChannelDlg]
Type=0
BaseClass=CDialog
HeaderFile=CHANDLG.H
ImplementationFile=CHANDLG.CPP

[CLS:CReplayDoc]
Type=0
BaseClass=CDocument
HeaderFile=PLAYDOC.H
ImplementationFile=Playdoc.cpp

[CLS:CReplayFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=PLAYFRM.H
ImplementationFile=PLAYFRM.CPP

[CLS:CReplayView]
Type=0
BaseClass=CView
HeaderFile=PLAYVIEW.H
ImplementationFile=Playview.cpp

[CLS:CReadDlg]
Type=0
BaseClass=CDialog
HeaderFile=READDLG.H
ImplementationFile=READDLG.CPP

[CLS:CReplayApp]
Type=0
BaseClass=CWinApp
HeaderFile=REPLAY.H
ImplementationFile=Replay.cpp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=Replay.cpp
ImplementationFile=Replay.cpp
LastObject=ID_FILE_OPEN

[CLS:CTextView]
Type=0
BaseClass=CView
HeaderFile=TEXTVIEW.H
ImplementationFile=TEXTVIEW.CPP

[DLG:IDD_CHANDIALOG]
Type=1
Class=CChannelDlg
ControlCount=31
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_CHAN1,combobox,1344339971
Control4=IDC_CHAN2,combobox,1344339971
Control5=IDC_CHAN3,combobox,1344339971
Control6=IDC_CHAN4,combobox,1344339971
Control7=IDC_CHAN5,combobox,1344339971
Control8=IDC_CHAN6,combobox,1344339971
Control9=IDC_CHAN7,combobox,1344339971
Control10=IDC_CHAN8,combobox,1344339971
Control11=IDC_COLOUR1,combobox,1344339971
Control12=IDC_COLOUR2,combobox,1344339971
Control13=IDC_COLOUR3,combobox,1344339971
Control14=IDC_COLOUR4,combobox,1344339971
Control15=IDC_COLOUR5,combobox,1344339971
Control16=IDC_COLOUR6,combobox,1344339971
Control17=IDC_COLOUR7,combobox,1344339971
Control18=IDC_COLOUR8,combobox,1344339971
Control19=IDC_DECPL1,combobox,1344339971
Control20=IDC_DECPL2,combobox,1344339971
Control21=IDC_DECPL3,combobox,1344339971
Control22=IDC_DECPL4,combobox,1344339971
Control23=IDC_DECPL5,combobox,1344339971
Control24=IDC_DECPL6,combobox,1344339971
Control25=IDC_DECPL7,combobox,1344339971
Control26=IDC_DECPL8,combobox,1344339971
Control27=IDC_CHANNAME,static,1342308353
Control28=IDC_COLOUR,static,1342308353
Control29=IDC_PLACES,static,1342308353
Control30=IDC_CLEAR_ALL,button,1342242816
Control31=IDC_DEFAULTS,button,1342242816

[DLG:IDD_READDIALOG]
Type=1
Class=CReadDlg
ControlCount=3
Control1=IDCANCEL,button,1342242816
Control2=IDC_READING,static,1342308353
Control3=IDC_READCHAN,edit,1350633600

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_VERSION,static,1342308352
Control3=IDC_COPYRIGHT,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_OPEN
Command2=ID_FILE_PRINT
Command3=ID_FILE_PRINT_PREVIEW
Command4=ID_FILE_PRINT_SETUP
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
Command8=ID_ZOOM_OUT_X
Command9=ID_ZOOM_OUT_Y
Command10=ID_ZOOM_OUT_BOTH
Command11=ID_ZOOM_OUT
Command12=ID_CHANNELS
Command13=ID_REPLAY_GOTOSTART
Command14=ID_REPLAY_GOTOEND
Command15=ID_REPLAY_FORWARD
Command16=ID_REPLAY_REVERSE
Command17=ID_REPLAY_FASTFORWARD
Command18=ID_REPLAY_FASTREVERSE
Command19=ID_REPLAY_STOP
Command20=ID_APP_ABOUT
Command21=ID_HELP_INDEX
CommandCount=21

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[DLG:IDD_COLOUR_DIALOG]
Type=1
Class=?
ControlCount=9
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_LINE_COLOUR,button,1342177287
Control4=IDC_COLOUR_RED,button,1342177289
Control5=IDC_COLOUR_GREEN,button,1342177289
Control6=IDC_COLOUR_BLUE,button,1342177289
Control7=IDC_COLOUR_CYAN,button,1342177289
Control8=IDC_COLOUR_MAGENTA,button,1342177289
Control9=IDC_COLOUR_YELLOW,button,1342177289

