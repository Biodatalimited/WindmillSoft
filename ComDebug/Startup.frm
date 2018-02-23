VERSION 5.00
Begin VB.Form Startup 
   Caption         =   "Windmill comDebug Serial Instrument Set Up Utility"
   ClientHeight    =   5400
   ClientLeft      =   2130
   ClientTop       =   2070
   ClientWidth     =   6585
   FillColor       =   &H00C0C0C0&
   FillStyle       =   2  'Horizontal Line
   Icon            =   "Startup.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   5400
   ScaleWidth      =   6585
   Begin VB.Frame Frame7 
      Height          =   615
      Left            =   2400
      TabIndex        =   21
      Top             =   3000
      Width           =   3975
      Begin VB.CommandButton WindmillDebug 
         Height          =   255
         Left            =   240
         TabIndex        =   22
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label11 
         Caption         =   "Windmill Debug Screen"
         Height          =   255
         Left            =   840
         TabIndex        =   23
         Top             =   240
         Width           =   2055
      End
   End
   Begin VB.Frame Frame6 
      Height          =   615
      Left            =   2400
      TabIndex        =   18
      Top             =   4440
      Width           =   3975
      Begin VB.CommandButton ExitButton 
         Height          =   255
         Left            =   240
         TabIndex        =   19
         Top             =   240
         Width           =   375
      End
      Begin VB.Label label9 
         Caption         =   "Close"
         Height          =   255
         Left            =   840
         TabIndex        =   20
         Top             =   240
         Width           =   1215
      End
   End
   Begin VB.Frame Frame5 
      Height          =   615
      Left            =   2400
      TabIndex        =   12
      Top             =   3720
      Width           =   3975
      Begin VB.CommandButton HelpOverview 
         Height          =   255
         Left            =   240
         TabIndex        =   13
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label5 
         Caption         =   "HELP Overview"
         Height          =   255
         Left            =   840
         TabIndex        =   14
         Top             =   240
         Width           =   2655
      End
   End
   Begin VB.Frame Frame4 
      Height          =   615
      Left            =   2400
      TabIndex        =   9
      Top             =   2280
      Width           =   3975
      Begin VB.CommandButton DeviceEdit 
         Height          =   255
         Left            =   240
         TabIndex        =   10
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label4 
         Caption         =   "Edit Windmill Device"
         Height          =   255
         Left            =   840
         TabIndex        =   11
         Top             =   240
         Width           =   2775
      End
   End
   Begin VB.Frame Frame3 
      Height          =   615
      Left            =   2400
      TabIndex        =   6
      Top             =   1560
      Width           =   3975
      Begin VB.CommandButton LoadButton 
         Height          =   255
         Left            =   240
         TabIndex        =   7
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label3 
         Caption         =   "Edit an existing Windmill Instrument File"
         Height          =   255
         Left            =   840
         TabIndex        =   8
         Top             =   240
         Width           =   2895
      End
   End
   Begin VB.Frame Frame2 
      Height          =   615
      Left            =   2400
      TabIndex        =   3
      Top             =   840
      Width           =   3975
      Begin VB.CommandButton NewButton 
         Height          =   255
         Left            =   240
         TabIndex        =   4
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label2 
         Caption         =   "Create a new Windmill Instrument File"
         Height          =   255
         Left            =   840
         TabIndex        =   5
         Top             =   240
         Width           =   2895
      End
   End
   Begin VB.Frame Frame1 
      Height          =   615
      Left            =   2400
      TabIndex        =   0
      Top             =   120
      Width           =   3975
      Begin VB.CommandButton TermButton 
         Height          =   255
         Left            =   240
         TabIndex        =   1
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label1 
         Caption         =   "Use the program as a Terminal utility"
         Height          =   255
         Left            =   840
         TabIndex        =   2
         Top             =   240
         Width           =   2895
      End
   End
   Begin VB.Label Label8 
      Caption         =   "comDebug"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000000FF&
      Height          =   375
      Left            =   360
      TabIndex        =   17
      Top             =   3360
      Width           =   1575
   End
   Begin VB.Shape Shape1 
      Height          =   3375
      Left            =   120
      Top             =   600
      Width           =   2055
   End
   Begin VB.Label Label7 
      Caption         =   "Software"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000000FF&
      Height          =   375
      Left            =   720
      TabIndex        =   16
      Top             =   2880
      Width           =   1215
   End
   Begin VB.Label Label6 
      Caption         =   "Windmill"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000000FF&
      Height          =   375
      Left            =   360
      TabIndex        =   15
      Top             =   2520
      Width           =   1215
   End
   Begin VB.Image Image1 
      Height          =   1755
      Left            =   480
      Picture         =   "Startup.frx":044A
      Stretch         =   -1  'True
      Top             =   720
      Width           =   1455
   End
End
Attribute VB_Name = "Startup"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit


Private Sub DeviceEdit_Click()
    
    IMLEdit.Visible = True
    Startup.Visible = False
    
End Sub

Private Sub ExitButton_Click()
    Call Unload(Startup)
End Sub

Private Sub HelpOverview_Click()

    Call DisplayHelp(Startup.hWnd, 1)
    
End Sub

'Create a new instrument file
Private Sub NewButton_Click()
        
    'default the comm parameters
    SerialCommsInitialise
    NetworkComms_Initialise
               
    'Select the comms type
    CommSelect.Show 1
           
    Startup.Visible = False
       
    'Indicate new file
    Mainmenu.SetMode (MODE_NEW_FILE)
    
    Mainmenu.Visible = True

    'Set the mode to full IML
    Mainform.SetMode (1)
End Sub

'Use as a simple terminal
Private Sub TermButton_Click()
  
    'Select the comms type
    CommSelect.Show 1
    
    Startup.Visible = False
    'Set the mode to simple terminal
    Mainform.SetMode (0)
    
    Mainform.Visible = True

End Sub

Private Sub LoadButton_Click()
    

    'If the load fails the Mainmenu unloads and an error occurs when we try
    'to make it visible
    On Error GoTo LoadExit
    
    'Indicate existing file
    Mainmenu.SetMode (MODE_LOAD_FILE)
    
    'Now bring up the main menu
    Mainmenu.Visible = True
    Startup.Visible = False
    
    'Set the mode to full IML
    Mainform.SetMode (1)
    
    WriteDebug
        
LoadExit:
        
End Sub

Private Sub Form_Load()
    SerialCommsInitialise
    NetworkComms_Initialise
    ReadIMLPaths
End Sub

Private Sub Form_Unload(Cancel As Integer)

Call Unload(IMLEdit)

End Sub

Private Sub WindmillDebug_Click()

      StatusView.bIMLDebug = True
      StatusView.Visible = True
      Startup.Enabled = False

End Sub
