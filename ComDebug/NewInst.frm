VERSION 5.00
Object = "{5E9E78A0-531B-11CF-91F6-C2863C385E30}#1.0#0"; "MSFLXGRD.OCX"
Begin VB.Form NewInst 
   Caption         =   "Define New Instrument"
   ClientHeight    =   5370
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6585
   LinkTopic       =   "Form1"
   ScaleHeight     =   5370
   ScaleWidth      =   6585
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame Frame1 
      Caption         =   "Instrument Timings"
      Height          =   4215
      Left            =   240
      TabIndex        =   2
      Top             =   360
      Width           =   3375
      Begin VB.TextBox Text3 
         Height          =   285
         Left            =   1800
         TabIndex        =   5
         Text            =   "Text3"
         Top             =   3120
         Width           =   1095
      End
      Begin VB.TextBox Text2 
         Height          =   285
         Left            =   1800
         TabIndex        =   4
         Text            =   "Text2"
         Top             =   1800
         Width           =   1095
      End
      Begin VB.TextBox Text1 
         Height          =   285
         Left            =   1800
         TabIndex        =   3
         Text            =   "Text1"
         Top             =   600
         Width           =   1095
      End
      Begin VB.Label Label7 
         Caption         =   "Data read from the instrument is stored for this period."
         Height          =   495
         Left            =   360
         TabIndex        =   11
         Top             =   3480
         Width           =   2655
      End
      Begin VB.Label Label6 
         Caption         =   "After sending a message the program will wait for at least this period before sending another message"
         Height          =   615
         Left            =   360
         TabIndex        =   10
         Top             =   2160
         Width           =   2655
      End
      Begin VB.Label Label5 
         Caption         =   "After requesting data the program will expect data within this period. "
         Height          =   375
         Left            =   360
         TabIndex        =   9
         Top             =   960
         Width           =   2655
      End
      Begin VB.Label Label4 
         Caption         =   "Idle Time : mSecs"
         Height          =   375
         Left            =   240
         TabIndex        =   8
         Top             =   1800
         Width           =   1455
      End
      Begin VB.Label Label3 
         Caption         =   "Persistence : mSecs"
         Height          =   255
         Left            =   120
         TabIndex        =   7
         Top             =   3120
         Width           =   1455
      End
      Begin VB.Label Label2 
         Caption         =   "Time Out  : mSecs"
         Height          =   255
         Left            =   240
         TabIndex        =   6
         Top             =   600
         Width           =   1335
      End
   End
   Begin MSFlexGridLib.MSFlexGrid MSFlexGrid1 
      Height          =   4575
      Left            =   3840
      TabIndex        =   0
      Top             =   360
      Width           =   1935
      _ExtentX        =   3413
      _ExtentY        =   8070
      _Version        =   393216
   End
   Begin VB.Label Label1 
      Caption         =   "Initialisation Message"
      Height          =   255
      Left            =   4080
      TabIndex        =   1
      Top             =   120
      Width           =   1575
   End
End
Attribute VB_Name = "NewInst"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
