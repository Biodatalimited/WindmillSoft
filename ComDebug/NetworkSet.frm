VERSION 5.00
Begin VB.Form NetworkSet 
   Caption         =   "TCP IP Settings"
   ClientHeight    =   4935
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   8070
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   4935
   ScaleWidth      =   8070
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton SerialButton 
      Caption         =   "Serial"
      Height          =   615
      Left            =   4920
      TabIndex        =   14
      Top             =   3120
      Width           =   855
   End
   Begin VB.CommandButton Pingpb 
      Caption         =   "Ping"
      Height          =   615
      Left            =   2280
      TabIndex        =   13
      Top             =   3120
      Width           =   855
   End
   Begin VB.Frame Frame2 
      Height          =   735
      Left            =   1440
      TabIndex        =   10
      Top             =   3960
      Width           =   5055
      Begin VB.Label WaitLabel 
         Caption         =   "Connecting to the instrument. Please Wait"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   480
         TabIndex        =   11
         Top             =   240
         Visible         =   0   'False
         Width           =   3975
      End
   End
   Begin VB.CommandButton CancelButton 
      Caption         =   "Cancel"
      Height          =   615
      Left            =   6240
      TabIndex        =   9
      Top             =   3120
      Width           =   855
   End
   Begin VB.CommandButton OKButton 
      Caption         =   "Connect"
      Height          =   615
      Left            =   960
      TabIndex        =   8
      Top             =   3120
      Width           =   855
   End
   Begin VB.CommandButton HrlpButton 
      Caption         =   "Help"
      Height          =   615
      Left            =   3600
      TabIndex        =   7
      Top             =   3120
      Width           =   855
   End
   Begin VB.Frame Frame1 
      Caption         =   "TCP IP Settinhgs"
      Height          =   2535
      Left            =   360
      TabIndex        =   0
      Top             =   240
      Width           =   7335
      Begin VB.TextBox TCPport 
         Height          =   285
         Left            =   4920
         TabIndex        =   5
         Top             =   840
         Width           =   1215
      End
      Begin VB.TextBox NetName 
         Height          =   285
         Left            =   360
         TabIndex        =   4
         Top             =   1920
         Width           =   6495
      End
      Begin VB.TextBox IPAdd 
         Height          =   285
         Left            =   360
         TabIndex        =   3
         Text            =   "xxx.xxx.xxx.xxx"
         Top             =   840
         Width           =   1245
      End
      Begin VB.OptionButton OptionName 
         Caption         =   "Instrument is Identified by Name"
         Height          =   255
         Left            =   360
         TabIndex        =   2
         Top             =   1440
         Width           =   3375
      End
      Begin VB.OptionButton OptionIP 
         Caption         =   "Instrument has fixed IP Address"
         Height          =   255
         Left            =   360
         TabIndex        =   1
         Top             =   480
         Width           =   3135
      End
      Begin VB.Label Label2 
         Caption         =   "IP 255.255.255.255"
         Height          =   255
         Left            =   2160
         TabIndex        =   12
         Top             =   840
         Width           =   1455
      End
      Begin VB.Label Label1 
         Caption         =   "Port Number"
         Height          =   255
         Left            =   4920
         TabIndex        =   6
         Top             =   480
         Width           =   975
      End
   End
End
Attribute VB_Name = "NetworkSet"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'This form is used to enter network comm parameters but also to open the comms
'This is because the comms can take a long time to open and this form tells the user to wait
'On entry to the form the comms are closed if they are already open.
'They are opened again on exit either at new settings or the old settings are restored.
'If bAutoStart is set then the form starts the comms and exits thereafter


Dim cStoredName As String
Dim cStoredIP As String
Dim lStoredPort As Long
Public bAutoStart As Boolean
Dim bOpenOnEntry As Boolean
Dim bOpenFailed As Boolean

'Enable either the IP input box or the name input box

Sub BoxEnable(bEnableIP As Boolean)

    Dim nIndex As Integer
    
    If bEnableIP Then
        IPAdd.Enabled = True
        IPAdd.Text = cIPaddress
        NetName.Enabled = False
        NetName.Text = ""
    Else
        NetName.Enabled = True
        NetName.Text = cIPName
        IPAdd.Enabled = False
        IPAdd.Text = ""
    End If


End Sub

'Restore the original settings on cancel

Private Sub CancelButton_Click()

    Call Unload(NetworkSet)
    
End Sub

'Store the input values
'Close the socket if its open and remember that
'Handle the autostart

Private Sub Form_Activate()

    bSerialComms = False
    
    'Save for use with cancel
    cStoredName = cIPName
    cStoredIP = cIPaddress
    lStoredPort = lPort
    
    'Say the open failed so original will be restored if possible
    bOpenFailed = True
    
    'Close the comms. Remember if it was open on entry
    'Cancel must reopen at the old values if it was
    
    bOpenOnEntry = False
    If nSocket = 0 Then
        bOpenOnEntry = True
        Channel_Close
    End If
    
    If cIPName <> "" Then
        OptionName = True
    Else
        OptionIP = True
    End If
    
    DoEvents

    TCPport.Text = CStr(lPort)
    IPAdd.Text = cIPaddress
    NetName.Text = cIPName
     If bAutoStart = True Then
       Call OKButton_Click
    End If
    bAutoStart = False
    
End Sub

'If the socket cannot be opened restore the original paramters
'If it was open on entry then restore it

Private Sub Form_Unload(Cancel As Integer)
    
    If bOpenFailed Then
        If bOpenOnEntry Then
            cIPaddress = cStoredIP
            lPort = lStoredPort
            cIPName = cStoredName
            Channel_Open
        Else
            Call MsgBox("Communications not open", vbOKOnly)
        End If
    End If

End Sub

Private Sub HrlpButton_Click()
    
    Call DisplayHelp(NetworkSet.hWnd, 7)

End Sub

'Open the comms if we can

Private Sub OKButton_Click()
    Dim nError As Integer

    
    nError = ReadIPAddress
    If OptionIP.Value = True Then
        If nError Then
            Call MsgBox("Invalid IP Address", vbOKOnly, "Network Settings Error")
            Exit Sub
        End If
    End If
    
    cIPName = NetName.Text
    If OptionName.Value = True Then
        If cIPName = "" Then
            Call MsgBox("Invalid Name", vbOKOnly, "Network Settings Error")
            Exit Sub
        End If
    End If
    
    nError = ReadPort
    If nError Then
        Call MsgBox("Invalid Port Number", vbOKOnly, "Network Settings Error")
        Exit Sub
    End If
    
    If OptionName.Value = True Then
        If (Resolve_NetWork_Name(cIPName, cIPaddress)) = -1 Then
            Call MsgBox("Cannot Resolve Name to IP Address", vbOKOnly, "Network Settings Error")
            Exit Sub
        End If
        IPAdd.Text = cIPaddress
    End If
    
    'Open the comms displaying the wait label
    WaitLabel.Visible = True
    DoEvents
    If Channel_Open = 0 Then
        bOpenFailed = False
        NetworkSet.Visible = False
    Else
        bOpenFailed = True
    End If
    WaitLabel.Visible = False
    

        
End Sub

Private Sub OptionIP_Click()
    BoxEnable (True)
End Sub

Private Sub OptionName_Click()
    BoxEnable (False)
End Sub

'Get the IP string from the boxes

Function ReadIPAddress() As Integer


    Dim cText As String
    Dim cDecimal(4) As String
    Dim nByte As Integer
    Dim nIndex As Integer
    
    'Assume Error
    ReadIPAddress = 1
    
    cText = IPAdd.Text
    
    nByte = 0
    cDecimal(0) = ""
    For nIndex = 1 To Len(cText)
    
        Select Case Mid$(cText, nIndex, 1)
            Case "0" To "9"
                cDecimal(nByte) = cDecimal(nByte) + Mid$(cText, nIndex, 1)
            Case "."
                If cDecimal(nByte) = "" Then
                    Exit Function
                End If
                nByte = nByte + 1
                If nByte > 3 Then
                    Exit Function
                End If
                cDecimal(nByte) = ""
            Case Else
                Exit Function
        End Select
                    
    Next nIndex
    
    If nByte <> 3 Then
        Exit Function
    End If
    
    For nByte = 1 To 3
        If Val(cDecimal(nByte)) > 255 Then
            Exit Function
        End If
    Next nByte
       
    cIPaddress = ""
    For nByte = 0 To 3
    
        cIPaddress = cIPaddress + CStr(Val(cDecimal(nByte)))
        If nByte < 3 Then
            cIPaddress = cIPaddress + "."
        End If
    
    Next nByte
    
    IPAdd.Text = cIPaddress
    
    ReadIPAddress = 0
    

End Function



'Return 1 for non numeric chars or port greater than 65535

Function ReadPort() As Integer

    Dim nIndex As Integer
    Dim cText As String
    
    ReadPort = 1
    
    cText = TCPport.Text
    
    For nIndex = 1 To Len(cText)
        If Mid$(cText, nIndex, 1) < "0" Or Mid$(cText, nIndex, 1) > "9" Then Exit Function
    Next nIndex
    
    TCPport.Text = CStr(Val(cText))
    
    If Val(cText) > 65535 Then
        cText = 65535
        Exit Function
    End If
    
    
    lPort = Val(cText)

    ReadPort = 0
        
    
End Function

'Used to cause the form to act as a loading dialog

Private Sub Timer1_Timer()
    
 '   If bAutoStart = True Then
 '       Call OKButton_Click
 '   End If
    bAutoStart = False
    
End Sub

Private Sub Pingpb_Click()

    Dim cComm As String
    Dim vReply As Variant
    Dim nError As Integer
    
    nError = ReadIPAddress
    If OptionIP.Value = True Then
        If nError Then
            Call MsgBox("Invalid IP Address", vbOKOnly, "Network Settings Error")
            Exit Sub
        End If
        cComm = cIPaddress
    End If
    
    cIPName = NetName.Text
    If OptionName.Value = True Then
        If cIPName = "" Then
            Call MsgBox("Invalid Name", vbOKOnly, "Network Settings Error")
            Exit Sub
        End If
        cComm = cIPName
    End If
    
    cComm = "ping " + cComm
    
    vReply = Shell(cComm, vbNormalFocus)

End Sub

Private Sub SerialButton_Click()
   
    CommSet.bAutoStart = False
    Call Unload(NetworkSet)
    DoEvents
    CommSet.Show 1
 
End Sub
