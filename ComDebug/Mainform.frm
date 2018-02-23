VERSION 5.00
Object = "{5E9E78A0-531B-11CF-91F6-C2863C385E30}#1.0#0"; "msflxgrd.ocx"
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form Mainform 
   Caption         =   "Windmill comDebug Terminal Screen"
   ClientHeight    =   5040
   ClientLeft      =   165
   ClientTop       =   855
   ClientWidth     =   7980
   FillColor       =   &H00FFFFFF&
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MousePointer    =   3  'I-Beam
   ScaleHeight     =   5040
   ScaleWidth      =   7980
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton RunButton 
      Caption         =   "Run"
      Height          =   375
      Left            =   7080
      TabIndex        =   12
      Top             =   2640
      Width           =   735
   End
   Begin VB.CommandButton CancelButton 
      Caption         =   "Close"
      Height          =   375
      Left            =   7080
      TabIndex        =   11
      Top             =   3480
      Width           =   735
   End
   Begin VB.CommandButton COMStatus 
      Caption         =   "Status"
      Height          =   375
      Left            =   7080
      TabIndex        =   10
      Top             =   600
      Width           =   735
   End
   Begin VB.CommandButton COMSettings 
      Caption         =   "COM"
      Height          =   375
      Left            =   7080
      TabIndex        =   9
      Top             =   1200
      Width           =   735
   End
   Begin VB.CommandButton ParseButton 
      Caption         =   "Parse"
      Height          =   375
      Left            =   7080
      TabIndex        =   8
      Top             =   2040
      Width           =   735
   End
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   6600
      Top             =   4800
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.Timer Timer2 
      Left            =   4920
      Top             =   4560
   End
   Begin VB.CommandButton SendAck 
      Caption         =   "Send"
      Height          =   375
      Left            =   5520
      TabIndex        =   6
      Top             =   4560
      Width           =   735
   End
   Begin MSFlexGridLib.MSFlexGrid GridAck 
      Height          =   4095
      Left            =   4920
      TabIndex        =   5
      Top             =   360
      Width           =   1815
      _ExtentX        =   3201
      _ExtentY        =   7223
      _Version        =   393216
      BackColorSel    =   -2147483643
      AllowBigSelection=   0   'False
      ScrollBars      =   2
   End
   Begin VB.Timer Timer1 
      Interval        =   2000
      Left            =   0
      Top             =   4560
   End
   Begin VB.CommandButton SendButton 
      Caption         =   "Send"
      Height          =   375
      Left            =   720
      TabIndex        =   4
      Top             =   4560
      Width           =   735
   End
   Begin MSFlexGridLib.MSFlexGrid GridRec 
      Height          =   4095
      Left            =   2520
      TabIndex        =   2
      Top             =   360
      Width           =   1815
      _ExtentX        =   3201
      _ExtentY        =   7223
      _Version        =   393216
      BackColorSel    =   -2147483643
      HighLight       =   0
      ScrollBars      =   2
   End
   Begin MSFlexGridLib.MSFlexGrid GridSend 
      Height          =   4095
      Left            =   120
      TabIndex        =   0
      Top             =   360
      Width           =   2055
      _ExtentX        =   3625
      _ExtentY        =   7223
      _Version        =   393216
      BackColorSel    =   -2147483643
      AllowBigSelection=   0   'False
      HighLight       =   2
      ScrollBars      =   2
   End
   Begin VB.Label CommOpenLabel 
      Height          =   255
      Left            =   2520
      TabIndex        =   13
      Top             =   4680
      Width           =   2175
   End
   Begin VB.Label Label4 
      Caption         =   "Acknowledgement"
      Height          =   255
      Left            =   5160
      TabIndex        =   7
      Top             =   120
      Width           =   1575
   End
   Begin VB.Line Line1 
      X1              =   -120
      X2              =   8040
      Y1              =   0
      Y2              =   0
   End
   Begin VB.Label Label2 
      Caption         =   "Reply"
      Height          =   255
      Left            =   3240
      TabIndex        =   3
      Top             =   120
      Width           =   1575
   End
   Begin VB.Label Label1 
      Caption         =   "Prompt "
      Height          =   255
      Left            =   840
      TabIndex        =   1
      Top             =   120
      Width           =   1455
   End
   Begin VB.Menu EditMenu 
      Caption         =   "&Edit"
      Begin VB.Menu Message_Name 
         Caption         =   "Message Name"
      End
      Begin VB.Menu Message_Delete 
         Caption         =   "Message Delete"
      End
      Begin VB.Menu ClearMessage 
         Caption         =   "Clear Grid"
      End
      Begin VB.Menu DeleteRow 
         Caption         =   "Delete Row"
      End
      Begin VB.Menu InsertRow 
         Caption         =   "Insert Row"
      End
   End
   Begin VB.Menu NonPrintMenu 
      Caption         =   "&NonPrint"
      Begin VB.Menu NonPrint 
         Caption         =   "Carriage Return"
         Index           =   0
      End
      Begin VB.Menu NonPrint 
         Caption         =   "Line Feed"
         Index           =   1
      End
      Begin VB.Menu NonPrint 
         Caption         =   "Escape"
         Index           =   2
      End
      Begin VB.Menu NonPrint 
         Caption         =   "Decimal NonPrint"
         Index           =   3
      End
      Begin VB.Menu NonPrint 
         Caption         =   "Hex NonPrint"
         Index           =   4
      End
   End
   Begin VB.Menu Outputs1 
      Caption         =   "&Outputs"
      Begin VB.Menu Value 
         Caption         =   "Value"
      End
      Begin VB.Menu IntegerMenu 
         Caption         =   "Integer"
         Begin VB.Menu IntMSBLSB 
            Caption         =   "MSB-LSB"
         End
         Begin VB.Menu IntLSBMSB 
            Caption         =   "LSB-MSB"
         End
      End
      Begin VB.Menu WordMemu 
         Caption         =   "Word"
         Begin VB.Menu WordMSBLSB 
            Caption         =   "MSB-LSB"
         End
         Begin VB.Menu WordLSBMSB 
            Caption         =   "LSB-MSB"
         End
      End
      Begin VB.Menu LongMenu 
         Caption         =   "Long"
         Begin VB.Menu OutputLong 
            Caption         =   "MSB-LSB         ABCD"
            Index           =   0
         End
         Begin VB.Menu OutputLong 
            Caption         =   "Byte Swap      BACD"
            Index           =   1
         End
         Begin VB.Menu OutputLong 
            Caption         =   "Word Swap    CDAB"
            Index           =   2
         End
         Begin VB.Menu OutputLong 
            Caption         =   "LSB-MSB         DCBA"
            Index           =   3
         End
      End
      Begin VB.Menu Dwordmenu 
         Caption         =   "Dword"
         Begin VB.Menu OutputDword 
            Caption         =   "MSB-LSB         ABCD"
            Index           =   0
         End
         Begin VB.Menu OutputDword 
            Caption         =   "Byte Swap      BADC"
            Index           =   1
         End
         Begin VB.Menu OutputDword 
            Caption         =   "Word Swap    CDAB"
            Index           =   2
         End
         Begin VB.Menu OutputDword 
            Caption         =   "LSB-MSB         DCBA"
            Index           =   3
         End
      End
      Begin VB.Menu FloatMenu 
         Caption         =   "Float"
         Begin VB.Menu OutputFloat 
            Caption         =   "MSB-LSB         ABCD"
            Index           =   0
         End
         Begin VB.Menu OutputFloat 
            Caption         =   "Byte Swap      BADC"
            Index           =   1
         End
         Begin VB.Menu OutputFloat 
            Caption         =   "Word Swap    CDAB"
            Index           =   2
         End
         Begin VB.Menu OutputFloat 
            Caption         =   "LSB-MSB         DCBA"
            Index           =   3
         End
      End
      Begin VB.Menu Hex 
         Caption         =   "Hex"
      End
      Begin VB.Menu SignedByte 
         Caption         =   "Signed Byte"
      End
      Begin VB.Menu UnsignedByte 
         Caption         =   "Byte"
      End
      Begin VB.Menu Bit 
         Caption         =   "Bit"
      End
      Begin VB.Menu Custom 
         Caption         =   "Custom"
      End
   End
   Begin VB.Menu Wait 
      Caption         =   "&Wait"
   End
   Begin VB.Menu CRC 
      Caption         =   "C&RC"
   End
   Begin VB.Menu HelpMenu 
      Caption         =   "&Help"
   End
End
Attribute VB_Name = "Mainform"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim nCurrentGrid As MSFlexGrid
Dim nMode As Integer



Private Sub CancelButton_Click()
    Call Unload(Mainform)
End Sub

Private Sub ClearMessage_Click()
    Call GridClear(nCurrentGrid)
End Sub

Private Sub OKButton_Click()
    Call Unload(Mainform)
End Sub

Private Sub COMSettings_Click()
    
    'Stop data read whilst we change the Comms
    Timer1.Enabled = False
    If bSerialComms = True Then
        CommSet.Show 1
    Else
        NetworkSet.Show 1
    End If
    DoEvents
    Timer1.Enabled = True
    
    'The channel is open at this point
End Sub


Private Sub COMStatus_Click()
    CommStatus.Visible = True
End Sub

Private Sub CRC_Click()
    
    'If we are deleting an inserted value
    If Len(nCurrentGrid.TextMatrix(nCurrentGrid.Row, 1)) = 5 Then
        Call ChannelDiag.ChannelFree(Val(nCurrentGrid.TextMatrix(nCurrentGrid.Row, 2)))
    End If
    
    
    nCurrentGrid.TextMatrix(nCurrentGrid.Row, 1) = "CRC "
    nCurrentGrid.TextMatrix(nCurrentGrid.Row, 2) = ""
    nCurrentGrid.TextMatrix(nCurrentGrid.Row, 0) = CStr(nCurrentGrid.Row)
    
    'Add another row if needed
    If nCurrentGrid.Row = nCurrentGrid.Rows - 1 Then
        nCurrentGrid.Rows = nCurrentGrid.Rows + 1
    End If
 
    'Move to next row
    nCurrentGrid.Row = nCurrentGrid.Row + 1
End Sub



Private Sub DeleteRow_Click()

    Dim nRow As Integer
    
    nRow = nCurrentGrid.Row

   'If we are deleting an inserted value
    If Len(nCurrentGrid.TextMatrix(nRow, 1)) = 5 Then
        Call ChannelDiag.ChannelFree(Val(nCurrentGrid.TextMatrix(nRow, 2)))
    End If

    Call GridDeleteRow(nCurrentGrid)
        
End Sub


Private Sub Form_Activate()


    CommOpenLabel.Caption = ""

    If bSerialComms Then
        If hComPort = -1 Then
            CommOpenLabel.Caption = "Communications Not Open"
        End If
    Else
        If nSocket <> 0 Then
            CommOpenLabel.Caption = "Communications Not Open"
        End If
    End If
    
    'Status form only used for serial comms
    COMStatus.Visible = bSerialComms
    
    If nMode = 0 Then
        Mainform.Caption = "ComDebug Terminal Screen"
        Message_Name.Enabled = False
        RunButton.Visible = False
        ParseButton.Visible = False
    Else
        Mainform.Caption = "Message Edit -    " + Mainmenu.cName
        Message_Name.Enabled = True
        RunButton.Visible = True
        ParseButton.Visible = True
    End If
    
    'Can't delete initialisation
    'No point in deleting only message
    Message_Delete.Enabled = True
    If Mainmenu.Message_Count < 2 Or nMode = 0 Then
        Message_Delete.Enabled = False
    End If
    
    DoEvents
    

End Sub

Private Sub HelpMenu_Click()
    Call DisplayHelp(Mainform.hWnd, 5)
End Sub

Private Sub InsertRow_Click()
    Call GridInsertRow(nCurrentGrid)
End Sub

Private Sub Gridsend_KeyPress(nKey As Integer)
    'Handle key presses made in the grid
    Call GridKey_Press(GridSend, nKey)
End Sub

Private Sub Gridsend_LeaveCell()
    Call GridsetLeave(GridSend)
End Sub
Private Sub Gridsend_LostFocus()
    Call GridsetLeave(GridSend)
End Sub
Private Sub Gridsend_EnterCell()
    'Change Current grid and cell color
    Call GridsetEnter(GridSend)
    Set nCurrentGrid = GridSend
End Sub
Private Sub Gridsend_Gotfocus()
    'Change Current grid and cell color
    Call GridsetEnter(GridSend)
    Set nCurrentGrid = GridSend
End Sub

Private Sub Gridsend_MouseDown(button As Integer, _
shift As Integer, X As Single, Y As Single)

    'Right button calls up the Edit menu
    If button = 2 Then
      Mainform.PopupMenu NonPrintMenu
    End If
    
End Sub

Private Sub Form_Load()
    
    Dim nIndex As Integer
       
    'Initialise the grids
    Call GridInitialise(GridRec)
    Call GridInitialise(GridSend)
    Call GridInitialise(GridAck)
    
    'Load the grids
    Call GridLoadString(GridSend, Mainmenu.cPrompt)
    Call GridLoadString(GridAck, Mainmenu.cAck)
        
    Call SerialSetParseString("")

    Mainform.Top = 300
    Mainform.Left = 2000
    Mainform.Height = 5850
    
              
    
End Sub

Private Sub Form_Unload(Cancel As Integer)
  
    Dim szStr As String
  
       'Store the strings. Check them first
     If GridCheck(GridSend) Then
         Cancel = 1
         Exit Sub
     End If
     If GridCheck(GridAck) Then
         Cancel = 1
         Exit Sub
     End If
     
     Call Parseform.ParseGrid_GetString(szStr)
     Mainmenu.cParse = szStr

     'Store the edited strings in the main menu
     Call GridGetString(GridSend, szStr)
     Mainmenu.cPrompt = szStr
     Call GridGetString(GridAck, szStr)
     Mainmenu.cAck = szStr
     Call Mainmenu.MessageSave
  
    'Unload all the sub forms
    If bSerialComms = True Then
        Call Unload(CommSet)
    Else
        Call Unload(NetworkSet)
    End If
    Call Unload(CommStatus)
    Call Unload(Parseform)
    'If its a simple terminal close the whole program
    If nMode = 0 Then
        Call Unload(Startup)
        Call Unload(ChannelDiag)
        Call Unload(Mainmenu)
    Else
     'go back to the main menu
      Mainmenu.Visible = True
    End If
    
    Channel_Close
   
End Sub

Private Sub Message_Delete_Click()

    Dim cText As String
    Dim nReply As Integer

    If ChannelDiag.Channel_Count(Mainmenu.nEditMsg) <> 0 Then
        cText = "Cannot Delete a Message whilst it contains Data Channels"
        Call MsgBox(cText, vbOKOnly, "Message Delete")
        Exit Sub
    End If
    
    cText = "OK to Delete this entire Message"
    nReply = MsgBox(cText, vbOKCancel, "Message Delete")
    
    If nReply = vbCancel Then
        Exit Sub
    End If
    
    Mainmenu.bMsgDelete = True

    Call Unload(Mainform)
 
End Sub

'User wants to change the message name
'Get the new name and store it in Mainmenu.cName
'Update it in the Form caption

Private Sub Message_Name_Click()
    
    Dim cTitle As String
    
    cTitle = Mainmenu.cName
    Mainmenu.cName = InputBox("Enter a New Name for " + cTitle, "New Message", cTitle)
    If Mainmenu.cName = "" Then
        Mainmenu.cName = cTitle
    End If
    Mainform.Caption = "Message Edit -    " + Mainmenu.cName
     
End Sub



Private Sub NonPrint_Click(Index As Integer)

    Dim cCode As String

    Select Case Index
    Case 0
        Call GridWriteNonPrintRow(nCurrentGrid, "013")
    Case 1
        Call GridWriteNonPrintRow(nCurrentGrid, "010")
    Case 2
        Call GridWriteNonPrintRow(nCurrentGrid, "027")
    Case 3
        cCode = DecimalNonPrint_Enter
        Call GridWriteNonPrintRow(nCurrentGrid, cCode)
    Case 4
        Call MsgBox("You can enter Hex values by typing into the Hex Column", vbOKOnly, "Hex Non Print")
    End Select
    
End Sub

Private Sub ParseButton_Click()
    Parseform.Visible = True
End Sub

Private Sub RunButton_Click()
    
   Dim szStr As String
    
    'Stop this form reading
    Timer1.Enabled = False
    CommLog.Visible = True
    
    'Update the stored strings
     Call GridGetString(GridSend, szStr)
     Mainmenu.cPrompt = szStr
     Call GridGetString(GridAck, szStr)
     Mainmenu.cAck = szStr
     
    'Update the parse string for latest changes
    Call Parseform.ParseGrid_GetString(szParse)
      
     
       
End Sub

Private Sub SendAck_Click()
    If GridCheck(GridAck) Then
        GridSend.SetFocus
        Exit Sub
    End If

    Call SendPromptString(GridAck, True)
End Sub

Private Sub SendButton_Click()
    
    If GridCheck(GridSend) Then
        GridSend.SetFocus
        Exit Sub
    End If
    
    Call GridInitialise(GridRec)
    Call SendPromptString(GridSend, True)
    
End Sub



Private Sub Timer1_Timer()
    Dim cExtraction As String
    
    Call AppendReplyString(GridRec)
    Call SerialParseReply(cExtraction)

End Sub


Private Sub Gridack_KeyPress(nKey As Integer)
    'Handle key presses made in the grid
    Call GridKey_Press(GridAck, nKey)
End Sub

Private Sub Gridack_Lostfocus()
    Call GridsetLeave(GridAck)
End Sub

Private Sub Gridack_LeaveCell()
        Call GridsetLeave(GridAck)
End Sub
Private Sub Gridack_EnterCell()
    'Change Current grid and cell color
    Call GridsetEnter(GridAck)
    Set nCurrentGrid = GridAck
End Sub
Private Sub Gridack_GotFocus()
    'Change Current grid and cell color
    Call GridsetEnter(GridAck)
    Set nCurrentGrid = GridAck
End Sub

Private Sub Gridack_MouseDown(button As Integer, _
shift As Integer, X As Single, Y As Single)

    'Right button calls up the Edit menu
    If button = 2 Then
      Mainform.PopupMenu NonPrintMenu
    End If
    
End Sub

Public Sub SetMode(nSetmode As Integer)
    nMode = nSetmode
End Sub


Private Sub Timer2_Timer()
    bWait = False
End Sub

'*****************************************
'The selections from the output menu are handled here


Private Sub Value_Click()
    Call GridWriteValue(nCurrentGrid, cValue)
End Sub
Private Sub IntLSBMSB_Click()
    Call GridWriteValue(nCurrentGrid, cILSMS)
End Sub
Private Sub IntMSBLSB_Click()
    Call GridWriteValue(nCurrentGrid, cIMSLS)
End Sub
Private Sub WordLSBMSB_Click()
    Call GridWriteValue(nCurrentGrid, cWLSMS)
End Sub
Private Sub WordMSBLSB_Click()
    Call GridWriteValue(nCurrentGrid, cWMSLS)
End Sub
Private Sub Hex_Click()
    Call GridWriteValue(nCurrentGrid, cHex)
End Sub
Private Sub SignedByte_Click()
    Call GridWriteValue(nCurrentGrid, cSBYTE)
End Sub
Private Sub UnSignedByte_Click()
    Call GridWriteValue(nCurrentGrid, cUBYTE)
End Sub
Private Sub Bit_Click()
    Call GridWriteValue(nCurrentGrid, cBIT)
End Sub

Private Sub OutputDword_Click(Index As Integer)
    
    Select Case Index
    Case 0
        Call GridWriteValue(nCurrentGrid, cDWMSLS)
    Case 1
        Call GridWriteValue(nCurrentGrid, cDWBS)
    Case 2
        Call GridWriteValue(nCurrentGrid, cDWWS)
    Case 3
        Call GridWriteValue(nCurrentGrid, cDWLSMS)
    End Select
    
End Sub
Private Sub OutputLong_Click(Index As Integer)

    Select Case Index
    Case 0
        Call GridWriteValue(nCurrentGrid, cLMSLS)
    Case 1
        Call GridWriteValue(nCurrentGrid, cLBS)
    Case 2
        Call GridWriteValue(nCurrentGrid, cLWS)
    Case 3
        Call GridWriteValue(nCurrentGrid, cLLSMS)
    End Select

End Sub

Private Sub OutputFloat_Click(Index As Integer)

    Select Case Index
    Case 0
        Call GridWriteValue(nCurrentGrid, cFLOATMSLS)
    Case 1
        Call GridWriteValue(nCurrentGrid, cFLOATBS)
    Case 2
        Call GridWriteValue(nCurrentGrid, cFLOATWS)
    Case 3
        Call GridWriteValue(nCurrentGrid, cFLOATLSMS)
    End Select

End Sub

Private Sub Custom_Click()
    Call GridWriteValue(nCurrentGrid, cCustom)
End Sub

Private Sub Wait_Click()
    
    'If we are deleting an inserted value
    If Len(nCurrentGrid.TextMatrix(nCurrentGrid.Row, 1)) = 5 Then
        Call ChannelDiag.ChannelFree(Val(nCurrentGrid.TextMatrix(nCurrentGrid.Row, 2)))
    End If
    
    nCurrentGrid.TextMatrix(nCurrentGrid.Row, 1) = "Wait"
    nCurrentGrid.TextMatrix(nCurrentGrid.Row, 2) = ""
    nCurrentGrid.TextMatrix(nCurrentGrid.Row, 0) = CStr(nCurrentGrid.Row)
    
    'Add another row if needed
    If nCurrentGrid.Row = nCurrentGrid.Rows - 1 Then
        nCurrentGrid.Rows = nCurrentGrid.Rows + 1
    End If
 
    'Move to next row
    nCurrentGrid.Row = nCurrentGrid.Row + 1

End Sub



'Called from channeldiag to chalk up a selection


Public Sub ChannelSelected(nChannel As Integer)

    If nChannel = -1 Then
        nCurrentGrid.TextMatrix(nCurrentGrid.Row, 1) = ""
        Exit Sub
    Else
        nCurrentGrid.TextMatrix(nCurrentGrid.Row, 2) = CStr(nChannel)
    End If
    
        'Add another row if needed
    If nCurrentGrid.Row = nCurrentGrid.Rows - 1 Then
        nCurrentGrid.Rows = nCurrentGrid.Rows + 1
    End If
 
    'Move to next row
    nCurrentGrid.Row = nCurrentGrid.Row + 1
    

End Sub


