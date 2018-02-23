VERSION 5.00
Begin VB.Form ChannelSel 
   Caption         =   "Channel Selection"
   ClientHeight    =   3285
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   2865
   LinkTopic       =   "Form1"
   ScaleHeight     =   3285
   ScaleWidth      =   2865
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton ChannelNew 
      Caption         =   "New"
      Height          =   495
      Left            =   480
      TabIndex        =   2
      Top             =   2640
      Width           =   735
   End
   Begin VB.CommandButton ChannelOK 
      Caption         =   "OK"
      Height          =   495
      Left            =   1680
      TabIndex        =   1
      Top             =   2640
      Width           =   735
   End
   Begin VB.ListBox List1 
      Height          =   2205
      Left            =   360
      TabIndex        =   0
      Top             =   120
      Width           =   2055
   End
End
Attribute VB_Name = "ChannelSel"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub ChannelNew_Click()
    
    AddNewChannel
    
End Sub

