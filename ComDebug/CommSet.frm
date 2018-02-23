VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form CommSet 
   Caption         =   "COM Port Settings"
   ClientHeight    =   5880
   ClientLeft      =   165
   ClientTop       =   450
   ClientWidth     =   5070
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   5880
   ScaleWidth      =   5070
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton TCPIPButton 
      Caption         =   "TCP/IP"
      Height          =   495
      Left            =   2760
      TabIndex        =   16
      Top             =   5160
      Width           =   735
   End
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   4800
      Top             =   5520
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.CommandButton HelpButton 
      Caption         =   "Help"
      Height          =   495
      Left            =   1680
      TabIndex        =   15
      Top             =   5160
      Width           =   735
   End
   Begin VB.Frame Frame1 
      Caption         =   "Comms Settings"
      Height          =   4695
      Left            =   240
      TabIndex        =   2
      Top             =   240
      Width           =   4575
      Begin VB.ComboBox PortCombo 
         Height          =   315
         Left            =   1440
         Style           =   2  'Dropdown List
         TabIndex        =   14
         Top             =   360
         Width           =   2655
      End
      Begin VB.ComboBox FlowCombo 
         Height          =   315
         Left            =   1440
         Style           =   2  'Dropdown List
         TabIndex        =   12
         Top             =   3960
         Width           =   2655
      End
      Begin VB.ComboBox StopCombo 
         Height          =   315
         Left            =   1440
         Style           =   2  'Dropdown List
         TabIndex        =   10
         Top             =   3240
         Width           =   2655
      End
      Begin VB.ComboBox ParityCombo 
         Height          =   315
         Left            =   1440
         Style           =   2  'Dropdown List
         TabIndex        =   5
         Top             =   2520
         Width           =   2655
      End
      Begin VB.ComboBox BitsCombo 
         Height          =   315
         Left            =   1440
         Style           =   2  'Dropdown List
         TabIndex        =   4
         Top             =   1800
         Width           =   2655
      End
      Begin VB.ComboBox BaudCombo 
         Height          =   315
         Left            =   1440
         Style           =   2  'Dropdown List
         TabIndex        =   3
         Top             =   1080
         Width           =   2655
      End
      Begin VB.Label Label6 
         Caption         =   "Flow Control"
         Height          =   375
         Left            =   240
         TabIndex        =   13
         Top             =   4080
         Width           =   975
      End
      Begin VB.Label Label5 
         Caption         =   "Stop Bits"
         Height          =   495
         Left            =   480
         TabIndex        =   11
         Top             =   3360
         Width           =   855
      End
      Begin VB.Label Label4 
         Caption         =   "Parity"
         Height          =   375
         Left            =   720
         TabIndex        =   9
         Top             =   2640
         Width           =   615
      End
      Begin VB.Label Label3 
         Caption         =   "Data Bits"
         Height          =   375
         Left            =   480
         TabIndex        =   8
         Top             =   1920
         Width           =   735
      End
      Begin VB.Label Label2 
         Caption         =   "Baud Rate"
         Height          =   375
         Left            =   360
         TabIndex        =   7
         Top             =   1200
         Width           =   975
      End
      Begin VB.Label Label1 
         Caption         =   "Comm Port"
         Height          =   375
         Left            =   360
         TabIndex        =   6
         Top             =   480
         Width           =   855
      End
   End
   Begin VB.CommandButton CommSetCancel 
      Caption         =   "Cancel"
      Height          =   495
      Left            =   3840
      TabIndex        =   1
      Top             =   5160
      Width           =   735
   End
   Begin VB.CommandButton CommSetOK 
      Caption         =   "OK"
      Height          =   495
      Left            =   600
      TabIndex        =   0
      Top             =   5160
      Width           =   735
   End
End
Attribute VB_Name = "CommSet"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'This form is used to enter serial comm parameters but also to open the comms
'This is to match the NetworkSet form
'On entry to the form the comms are closed if they are already open.
'They are opened again on exit either at new settings or the old settings are restored.
'If bAutoStart is set then the form starts the comms and exits thereafter

Dim pCommTemp As CommStruct
Dim bOpenOnEntry As Boolean
Dim bOpenFailed As Boolean

Public bAutoStart As Boolean

Private Sub CommSetCancel_Click()

    Call Unload(CommSet)
           
End Sub

Private Sub CommSetOK_Click()

Dim cLocal As String

'Read the ComPort list
cLocal = PortCombo.Text
cLocal = Right$(cLocal, Len(cLocal) - 3)
pComm.wPort = Val(cLocal)

'Read the baud list
cLocal = BaudCombo.Text
pComm.wBaud = Val(cLocal)

'Read the bits list
cLocal = BitsCombo.Text
pComm.wBits = Val(cLocal)

'Read parity box
cLocal = ParityCombo.Text
Select Case cLocal
    Case "None"
        pComm.wParity = 0
    Case "Odd"
        pComm.wParity = 1
    Case "Even"
        pComm.wParity = 2
    Case "Mark"
        pComm.wParity = 3
    Case "Space"
        pComm.wParity = 4
End Select

'read stop bits box
cLocal = StopCombo.Text
pComm.wStop = Val(cLocal)
            
cLocal = FlowCombo.Text

Select Case cLocal
    Case "None"
        pComm.wHandshake = 0
    Case "Hardware"
        pComm.wHandshake = 2
    Case "Xon\Xoff"
        pComm.wHandshake = 1
End Select

'Unload if the open is ok
If Channel_Open <> -1 Then
    bOpenFailed = False
    Call Unload(CommSet)
Else
    bOpenFailed = True
End If

End Sub


Private Sub Form_Activate()

    bSerialComms = True

    'Store the originals
    pCommTemp = pComm

    'Record if it was open on entry
    'Close it if it was
    bOpenOnEntry = False
    If hComPort <> -1 Then
        bOpenOnEntry = True
        Channel_Close
    End If
    
    'Not tried yet. Assume a failure
    bOpenFailed = True
    
    'Load the combo boxes
    Load_Values
    
    'Auto click the OK
    If bAutoStart Then
        bAutoStart = False
        CommSetOK_Click
    End If

End Sub


Private Sub Form_Load()

Dim nIndex As Integer
Dim cString As String


'Initialise COM1 to 24
For nIndex = 1 To 24
    cString = "COM" + CStr(nIndex)
    PortCombo.AddItem cString
Next nIndex

'Initialise Baud List
BaudCombo.AddItem "110"
BaudCombo.AddItem "300"
BaudCombo.AddItem "600"
BaudCombo.AddItem "1200"
BaudCombo.AddItem "2400"
BaudCombo.AddItem "4800"
BaudCombo.AddItem "9600"
BaudCombo.AddItem "19200"
BaudCombo.AddItem "38400"
BaudCombo.AddItem "57600"
BaudCombo.AddItem "115200"


ParityCombo.AddItem "None"
ParityCombo.AddItem "Odd"
ParityCombo.AddItem "Even"
ParityCombo.AddItem "Mark"
ParityCombo.AddItem "Space"


'Initialise Bits List
BitsCombo.AddItem "7"
BitsCombo.AddItem "8"



'Initialise Stop Bits List
StopCombo.AddItem "1"
StopCombo.AddItem "2"

'Initialise FlowControl List
FlowCombo.AddItem "None"
FlowCombo.AddItem "Hardware"
FlowCombo.AddItem "Xon\Xoff"

Load_Values


End Sub


Private Sub Load_Values()

    PortCombo.ListIndex = pComm.wPort - 1
    
    Select Case pComm.wBaud
        Case 110
            BaudCombo.ListIndex = 0
        Case 300
            BaudCombo.ListIndex = 1
        Case 600
            BaudCombo.ListIndex = 2
        Case 1200
            BaudCombo.ListIndex = 3
        Case 2400
            BaudCombo.ListIndex = 4
        Case 4800
            BaudCombo.ListIndex = 5
        Case 9600
            BaudCombo.ListIndex = 6
        Case 19200
            BaudCombo.ListIndex = 7
        Case 38400
            BaudCombo.ListIndex = 8
        Case 57600
            BaudCombo.ListIndex = 9
        Case 115200
            BaudCombo.ListIndex = 10
        
    End Select

    ParityCombo.ListIndex = pComm.wParity
    
    Select Case pComm.wBits
        Case 7
            BitsCombo.ListIndex = 0
        Case 8
            BitsCombo.ListIndex = 1
    End Select
    
    StopCombo.ListIndex = pComm.wStop - 1

    Select Case pComm.wHandshake
        Case 0
            FlowCombo.ListIndex = 0
        Case 1
            FlowCombo.ListIndex = 2
        Case 2
            FlowCombo.ListIndex = 1
    End Select

End Sub


Private Sub Form_Unload(Cancel As Integer)

        'If we can't open it restore the originals
    If bOpenFailed Then
        'If the open failed restore the originals
        If bOpenOnEntry Then
            'Reopen if it was open
            pComm = pCommTemp
            Channel_Open
        Else
            'Open failed but was not open on entry
            Call MsgBox("Communications not Open", vbOKOnly)
        End If
    End If
    
End Sub

Private Sub HelpButton_Click()
    

    Call DisplayHelp(CommSet.hWnd, 3)


End Sub

Private Sub TCPIPButton_Click()
   
    pComm.wPort = 0
    NetworkSet.bAutoStart = False
    Call Unload(CommSet)
    DoEvents
    NetworkSet.Show 1
 
End Sub
