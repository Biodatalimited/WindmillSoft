VERSION 5.00
Begin VB.Form CommStatus 
   Caption         =   "COM Port Status"
   ClientHeight    =   2295
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6960
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   2295
   ScaleWidth      =   6960
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer TimerStatus 
      Enabled         =   0   'False
      Interval        =   2
      Left            =   120
      Top             =   1680
   End
   Begin VB.Frame Frame1 
      Caption         =   "Communications Status"
      Height          =   855
      Left            =   600
      TabIndex        =   12
      Top             =   1320
      Width           =   6015
      Begin VB.Label RXStatus 
         Caption         =   "Receive Status :"
         Height          =   255
         Left            =   240
         TabIndex        =   14
         Top             =   480
         Width           =   5655
      End
      Begin VB.Label TXStatus 
         Caption         =   "Transmit Status :"
         Height          =   255
         Left            =   240
         TabIndex        =   13
         Top             =   240
         Width           =   5415
      End
   End
   Begin VB.Frame Frame3 
      Caption         =   "COM Output Lines"
      Height          =   1095
      Index           =   0
      Left            =   4080
      TabIndex        =   5
      Top             =   120
      Width           =   2535
      Begin VB.PictureBox DTRred 
         Height          =   135
         Left            =   1440
         Picture         =   "CommStatus.frx":0000
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   26
         Top             =   480
         Width           =   135
      End
      Begin VB.PictureBox DTRgreen 
         Height          =   135
         Left            =   1440
         Picture         =   "CommStatus.frx":0182
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   25
         Top             =   720
         Width           =   135
      End
      Begin VB.PictureBox RTSred 
         Height          =   135
         Left            =   360
         Picture         =   "CommStatus.frx":0304
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   24
         Top             =   480
         Width           =   135
      End
      Begin VB.PictureBox RTSgreen 
         Height          =   135
         Left            =   360
         Picture         =   "CommStatus.frx":0486
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   23
         Top             =   720
         Width           =   135
      End
      Begin VB.CommandButton RTSToggle 
         Height          =   315
         Left            =   720
         TabIndex        =   7
         Top             =   360
         Width           =   495
      End
      Begin VB.CommandButton DTRToggle 
         Height          =   315
         Left            =   1800
         TabIndex        =   6
         Top             =   360
         Width           =   495
      End
      Begin VB.Label Label13 
         Caption         =   "Toggle"
         Height          =   255
         Left            =   1800
         TabIndex        =   28
         Top             =   720
         Width           =   495
      End
      Begin VB.Label Label9 
         Caption         =   "Toggle"
         Height          =   255
         Left            =   720
         TabIndex        =   27
         Top             =   720
         Width           =   495
      End
      Begin VB.Label Label12 
         Caption         =   "DTR"
         Height          =   255
         Left            =   1320
         TabIndex        =   9
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label11 
         Caption         =   "RTS"
         Height          =   255
         Left            =   240
         TabIndex        =   8
         Top             =   240
         Width           =   735
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "COM Input Lines"
      Height          =   1095
      Index           =   0
      Left            =   600
      TabIndex        =   0
      Top             =   120
      Width           =   2655
      Begin VB.PictureBox RIred 
         Height          =   135
         Left            =   2160
         Picture         =   "CommStatus.frx":0608
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   22
         Top             =   480
         Width           =   135
      End
      Begin VB.PictureBox RIgreen 
         Height          =   135
         Left            =   2160
         Picture         =   "CommStatus.frx":078A
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   21
         Top             =   720
         Width           =   135
      End
      Begin VB.PictureBox DSRred 
         Height          =   135
         Left            =   1560
         Picture         =   "CommStatus.frx":090C
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   20
         Top             =   480
         Width           =   135
      End
      Begin VB.PictureBox DSRgreen 
         Height          =   135
         Left            =   1560
         Picture         =   "CommStatus.frx":0A8E
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   19
         Top             =   720
         Width           =   135
      End
      Begin VB.PictureBox DCDred 
         Height          =   135
         Left            =   960
         Picture         =   "CommStatus.frx":0C10
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   18
         Top             =   480
         Width           =   135
      End
      Begin VB.PictureBox DCDgreen 
         Height          =   135
         Left            =   960
         Picture         =   "CommStatus.frx":0D92
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   17
         Top             =   720
         Width           =   135
      End
      Begin VB.PictureBox CTSred 
         Height          =   135
         Left            =   360
         Picture         =   "CommStatus.frx":0F14
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   16
         Top             =   480
         Width           =   135
      End
      Begin VB.PictureBox CTSGreen 
         Height          =   135
         Left            =   360
         Picture         =   "CommStatus.frx":1096
         ScaleHeight     =   75
         ScaleWidth      =   75
         TabIndex        =   15
         Top             =   720
         Width           =   135
      End
      Begin VB.Label Label10 
         Caption         =   "RI"
         Height          =   255
         Left            =   2160
         TabIndex        =   4
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label6 
         Caption         =   "CTS"
         Height          =   255
         Left            =   240
         TabIndex        =   3
         Top             =   240
         Width           =   495
      End
      Begin VB.Label Label7 
         Caption         =   "DCD"
         Height          =   255
         Left            =   840
         TabIndex        =   2
         Top             =   240
         Width           =   495
      End
      Begin VB.Label Label8 
         Caption         =   "DSR"
         Height          =   255
         Left            =   1440
         TabIndex        =   1
         Top             =   240
         Width           =   375
      End
   End
   Begin VB.Label Label3 
      Caption         =   "False"
      Height          =   255
      Left            =   120
      TabIndex        =   11
      Top             =   840
      Width           =   615
   End
   Begin VB.Label Label2 
      Caption         =   "True"
      Height          =   255
      Left            =   120
      TabIndex        =   10
      Top             =   600
      Width           =   615
   End
End
Attribute VB_Name = "CommStatus"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub DTRToggle_Click()
    ToggleDTRstate
End Sub

Private Sub Form_Load()

    UpdateStatus
    TimerStatus.Enabled = True
    
    CommStatus.Left = Mainform.Left
    CommStatus.Top = Mainform.Top + Mainform.Height

End Sub

Private Sub RTSToggle_Click()
    ToggleRTSstate
End Sub

Private Sub TimerStatus_Timer()

    UpdateStatus

End Sub

Private Sub UpdateStatus()

    SerialCommsGetState
     
    If pCommstate.bCTS Then
        CTSred.Visible = True
        CTSGreen.Visible = False
    Else
        CTSred.Visible = False
        CTSGreen.Visible = True
    End If
    
     If pCommstate.bDCD Then
        DCDred.Visible = True
        DCDgreen.Visible = False
    Else
        DCDred.Visible = False
        DCDgreen.Visible = True
    End If
   
    If pCommstate.bDSR Then
        DSRred.Visible = True
        DSRgreen.Visible = False
    Else
        DSRred.Visible = False
        DSRgreen.Visible = True
    End If
       
    If pCommstate.bRI Then
        RIred.Visible = True
        RIgreen.Visible = False
    Else
        RIred.Visible = False
        RIgreen.Visible = True
    End If

    If pCommstate.bDTR Then
        DTRred.Visible = True
        DTRgreen.Visible = False
    Else
        DTRred.Visible = False
        DTRgreen.Visible = True
    End If
    
    If pCommstate.bRTS Then
        RTSred.Visible = True
        RTSgreen.Visible = False
    Else
        RTSred.Visible = False
        RTSgreen.Visible = True
    End If

   
End Sub
