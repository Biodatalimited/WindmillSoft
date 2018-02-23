VERSION 5.00
Begin VB.Form CommSelect 
   Caption         =   "Communication Type Selection"
   ClientHeight    =   2745
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   4680
   ClipControls    =   0   'False
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   ScaleHeight     =   2745
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton SelTCPIP 
      Caption         =   "TCPIP Communications"
      Height          =   615
      Left            =   840
      TabIndex        =   1
      Top             =   1440
      Width           =   2895
   End
   Begin VB.CommandButton SelSerial 
      Caption         =   "Serial Communciations"
      Height          =   615
      Left            =   840
      TabIndex        =   0
      Top             =   360
      Width           =   2895
   End
End
Attribute VB_Name = "CommSelect"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub SelSerial_Click()
    CommSet.bAutoStart = False
    Call Unload(CommSelect)
    DoEvents
    CommSet.Show 1
End Sub

Private Sub SelTCPIP_Click()
    NetworkSet.bAutoStart = False
    Call Unload(CommSelect)
    DoEvents
    NetworkSet.Show 1
    
End Sub
