VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form Timing 
   Caption         =   "Windmill Instrument Timing"
   ClientHeight    =   3675
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4335
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   3675
   ScaleWidth      =   4335
   StartUpPosition =   3  'Windows Default
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   3960
      Top             =   3360
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.Frame Frame1 
      Height          =   3135
      Left            =   240
      TabIndex        =   0
      Top             =   240
      Width           =   3735
      Begin VB.CommandButton OK 
         Caption         =   "OK"
         Height          =   375
         Left            =   2400
         TabIndex        =   11
         Top             =   2520
         Width           =   615
      End
      Begin VB.CommandButton HelpButton 
         Caption         =   "Help"
         Height          =   375
         Left            =   480
         TabIndex        =   10
         Top             =   2520
         Width           =   615
      End
      Begin VB.OptionButton Direct 
         Caption         =   "Option1"
         Height          =   255
         Left            =   720
         TabIndex        =   8
         Top             =   1800
         Width           =   255
      End
      Begin VB.OptionButton Background 
         Caption         =   "Option1"
         Height          =   195
         Left            =   720
         TabIndex        =   6
         Top             =   2040
         Width           =   255
      End
      Begin VB.TextBox PersistBox 
         Height          =   285
         Left            =   240
         TabIndex        =   3
         Text            =   "1000"
         Top             =   1320
         Width           =   855
      End
      Begin VB.TextBox IdleBox 
         Height          =   285
         Left            =   240
         TabIndex        =   2
         Text            =   "1000"
         Top             =   840
         Width           =   855
      End
      Begin VB.TextBox TimeoutBox 
         Height          =   285
         Left            =   240
         TabIndex        =   1
         Text            =   "1000"
         Top             =   360
         Width           =   855
      End
      Begin VB.Label WaitLabel 
         Caption         =   "mSec   Wait Time"
         Height          =   255
         Left            =   1320
         TabIndex        =   12
         Top             =   840
         Width           =   2175
      End
      Begin VB.Label Label5 
         Caption         =   "Read Directly"
         Height          =   255
         Left            =   1200
         TabIndex        =   9
         Top             =   1800
         Width           =   1935
      End
      Begin VB.Label Label4 
         Caption         =   "Read in the Background"
         Height          =   255
         Left            =   1200
         TabIndex        =   7
         Top             =   2040
         Width           =   2055
      End
      Begin VB.Label Label3 
         Caption         =   "mSec   Data Persistence Time"
         Height          =   375
         Left            =   1320
         TabIndex        =   5
         Top             =   1320
         Width           =   2295
      End
      Begin VB.Label Label1 
         Caption         =   "mSec   TimeOut"
         Height          =   255
         Left            =   1320
         TabIndex        =   4
         Top             =   360
         Width           =   1335
      End
   End
End
Attribute VB_Name = "Timing"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit


'These are the variables which the form edits
Public nTimeout As Long
Public nIdleTime As Long
Public nPersistence As Long
Public nReadMode As Long

Private Sub Background_Click()
    WaitLabel.Caption = "mSec   Instrument Idle Time"
End Sub

Private Sub Direct_Click()
    WaitLabel.Caption = "mSec   Wait Time"
End Sub

Private Sub Form_Load()

'Initialise the boxes
TimeoutBox.Text = nTimeout
IdleBox.Text = nIdleTime
PersistBox.Text = nPersistence
If nReadMode = 1 Then
    Background = True
Else
    Direct = True
End If

End Sub


Private Sub Form_Unload(Cancel As Integer)

'Copy the boxes into the variables
nTimeout = Val(TimeoutBox.Text)
nIdleTime = Val(IdleBox.Text)
nPersistence = Val(PersistBox.Text)
If Background = True Then
    WaitLabel.Caption = "Instrument Idle Time"
    nReadMode = 1
Else
    WaitLabel.Caption = "Wait Time"
    nReadMode = 0
End If

End Sub


Private Sub HelpButton_Click()
      
    Call DisplayHelp(Timing.hWnd, 4)

End Sub

Private Sub OK_Click()
    
    nTimeout = Val(TimeoutBox.Text)
    nIdleTime = Val(IdleBox.Text)
    nPersistence = Val(PersistBox.Text)
    If Background = True Then
        nReadMode = 1
    Else
        nReadMode = 0
    End If
    Timing.Visible = False
    
End Sub

Private Sub TimeoutBox_KeyPress(KeyAscii As Integer)

    If TestNumber(KeyAscii) Then
        KeyAscii = 0
    End If

End Sub
Private Sub IdleBox_KeyPress(KeyAscii As Integer)

    If TestNumber(KeyAscii) Then
        KeyAscii = 0
    End If

End Sub

Private Sub PersistBox_KeyPress(KeyAscii As Integer)

    If TestNumber(KeyAscii) Then
        KeyAscii = 0
    End If

End Sub

Private Function TestNumber(nKey As Integer) As Integer

    TestNumber = 1
    If (nKey > 47 And nKey < 58) Or nKey = 8 Then
        TestNumber = 0
    End If
End Function
