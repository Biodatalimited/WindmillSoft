VERSION 5.00
Object = "{5E9E78A0-531B-11CF-91F6-C2863C385E30}#1.0#0"; "msflxgrd.ocx"
Begin VB.Form ChannelDiag 
   Caption         =   "Channel Details"
   ClientHeight    =   3900
   ClientLeft      =   165
   ClientTop       =   855
   ClientWidth     =   7440
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   3900
   ScaleWidth      =   7440
   StartUpPosition =   3  'Windows Default
   Begin MSFlexGridLib.MSFlexGrid ChGrid 
      Height          =   2895
      Left            =   240
      TabIndex        =   0
      Top             =   240
      Width           =   5175
      _ExtentX        =   9128
      _ExtentY        =   5106
      _Version        =   393216
      Cols            =   6
      BackColorSel    =   -2147483643
      ForeColorSel    =   -2147483640
      AllowBigSelection=   0   'False
      ScrollBars      =   2
      SelectionMode   =   1
   End
   Begin VB.Label Script 
      Alignment       =   2  'Center
      Height          =   255
      Left            =   600
      TabIndex        =   1
      Top             =   3360
      Width           =   5655
   End
   Begin VB.Shape Shape1 
      Height          =   495
      Left            =   360
      Top             =   3240
      Width           =   6135
   End
   Begin VB.Line Line1 
      X1              =   120
      X2              =   6720
      Y1              =   0
      Y2              =   0
   End
   Begin VB.Menu AddChan 
      Caption         =   "Add Channel"
   End
   Begin VB.Menu DelChan 
      Caption         =   "Delete Channel"
   End
End
Attribute VB_Name = "ChannelDiag"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit


'Structure to hold details of each channel
Private Type ChannelStruct

cName As String
cMax As String
cMin As String
nMode As Integer
nMessage As Integer

End Type

Const nMaxChans As Integer = 100
'public variables used during loading and unloading
'of channel data

'All the details for one channel
Public cName As String
Public cMax As String
Public cMin As String
Public nMode As Integer
Public nMessage As Integer
Public nChannel As Integer
Public nChans As Integer

Public nEditMode As Integer

Dim nPreviousCol As Integer
Dim nPreviousRow As Integer
Dim bReturn As Boolean

Dim Channels(nMaxChans - 1) As ChannelStruct
Dim nEditRow As Integer
Dim bNonSelect As Boolean

'Create a new channel. Allocate storage and write its
'number to the grid

Private Sub AddChan_Click()

    Dim cTemp As String
    Dim cText As String
    Dim nError As Integer
    
    'Increase the channel count
    If nChans < nMaxChans Then
        nChans = nChans + 1
    End If
    
    'Write the Channel number
    ChGrid.TextMatrix(nChans, 0) = CStr(nChans - 1)
    ChGrid.TextMatrix(nChans, 2) = "100"
    ChGrid.TextMatrix(nChans, 3) = "0"
    ChGrid.Row = nChans
    ChGrid.Col = 1
    'Scroll to leave one empty row
    If nChans > 10 Then
        ChGrid.TopRow = nChans - 9
    End If
    
    
    'Default the min and max
    Channels(nChans - 1).cMax = "100"
    Channels(nChans - 1).cMin = "0"
     
     nError = 1
     While nError
     
        'Clear it. It may be set after duplicates
        ChGrid.TextMatrix(nChans, 1) = ""
    
        cText = "Input a channel name ( 12 characters max )" + vbCr + vbCr + _
        "Names must contain only a-z, A-Z, 0-9 and _"
            
        'Get a name
        cTemp = InputBox(cText, "Channel Name")
        If Len(cTemp) > 12 Then
            cTemp = Left$(cTemp, 12)
        End If
        
        If TestIMLString(cTemp) Then
            cTemp = "Names must contain only a-z, A-Z, 0-9,  and _ "
            Call MsgBox(cTemp, vbOKOnly, "Name Error")
        Else
            'Must write it in for the duplicate test
            ChGrid.TextMatrix(nChans, 1) = cTemp
            nError = DuplicateChannel
        End If
        
        
    Wend
    
    'Write it in
    Channels(nChans - 1).cName = cTemp


End Sub

'Test a whole string as being a valid IML name

Private Function TestIMLString(cText As String) As Integer

    Dim nIndex As Integer
    Dim cChac As String
    
    TestIMLString = 1

    For nIndex = 1 To Len(cText)
        
        If TestIMLChar(Mid$(cText, nIndex, 1)) Then
            Exit Function
        End If
        
    Next nIndex

    TestIMLString = 0


End Function

'Check the char is valid in an IML name

Private Function TestIMLChar(cChar As String) As Integer

    TestIMLChar = 0

    If (cChar >= "0") And (cChar <= "9") Then Exit Function
    If (cChar >= "a") And (cChar <= "z") Then Exit Function
    If (cChar >= "A") And (cChar <= "Z") Then Exit Function
    If cChar = "_" Then Exit Function
    
    TestIMLChar = 1
    
End Function


Private Sub DelChan_Click()

    Dim nDelChan As Integer
    Dim nChan As Integer
    Dim nRet As Integer
    Dim cTitle As String

    cTitle = "Windmill Channel Deletion"

    If ChGrid.Row = 0 Then
        Exit Sub
    End If

    'The row to delete
    nDelChan = ChGrid.Row - 1
    
    'can't delete if too big
    If nDelChan > nChans - 1 Then
        Exit Sub
    End If
           
    'Can't delete if its allocated
    If Channels(nDelChan).nMode <> 0 Then
        Call MsgBox("Can't delete an allocated channel", , cTitle)
        Exit Sub
    End If
           
    nRet = MsgBox("This action changes the channel numbering and makes existing .IMS Files Invalid" + _
    vbCr + "Do you wish to proceed?", vbOKCancel, cTitle)
           
    If nRet = vbCancel Then
        Exit Sub
    End If
           
    'Delete the channel and move all the others down
    For nChan = nDelChan + 1 To nChans
        Channels(nChan - 1).cName = Channels(nChan).cName
        Channels(nChan - 1).cMax = Channels(nChan).cMax
        Channels(nChan - 1).cMin = Channels(nChan).cMin
        Channels(nChan - 1).nMode = Channels(nChan).nMode
        Channels(nChan - 1).nMessage = Channels(nChan).nMessage
    Next nChan
    
    nChans = nChans - 1

    'look for channels that are allocated and have
    'had their numbers decremented. Decrement the references
    'in the prompt and parse strings
    For nChan = nDelChan To nChans
        If Channels(nChan).nMode <> 0 Then
            nChannel = nChan
            nMode = Channels(nChan).nMode
            nMessage = Channels(nChan).nMessage
            Call Mainmenu.DecrementChannel
        End If
    Next nChan
    
    'Update the screen
    ChannelDetailsShow


End Sub

Private Sub Form_Load()

    Dim cTemp As String
        
    'No channel selection made yet
    bNonSelect = True
    
    nChannel = 0
    
    ChGrid.Rows = nMaxChans + 1
    ChGrid.Cols = 6
    ChGrid.ColWidth(0) = 1200
    ChGrid.ColWidth(1) = 1400
    ChGrid.ColWidth(2) = 1000
    ChGrid.ColWidth(3) = 1000
    ChGrid.ColWidth(4) = 1000
    ChGrid.ColWidth(5) = 1000
    
    ChGrid.Width = 6950
       
    ChGrid.ColAlignment(0) = flexAlignCenterCenter
    ChGrid.ColAlignment(1) = flexAlignLeftTop
    ChGrid.ColAlignment(2) = flexAlignLeftTop
    ChGrid.ColAlignment(3) = flexAlignLeftTop
    ChGrid.ColAlignment(4) = flexAlignCenterCenter
    ChGrid.ColAlignment(5) = flexAlignCenterCenter
    
    nPreviousCol = 1
    nPreviousRow = 1
    bReturn = False

    'Fill the grid
    ChannelDetailsShow
    
End Sub

'Filters the keypress on the basis of which column is
'being entered into. Then adds it to the text.

Private Sub ChGrid_KeyPress(nKey As Integer)

    If nEditMode <> MODE_ADD_UNALLOCATED_CHANNELS Then
        Exit Sub
    End If

    'This is silly
    If ChGrid.Row = 0 Then
        Exit Sub
    End If
    
    'If col 0 does not contain the channel number
    If ChGrid.TextMatrix(ChGrid.Row, 0) <> CStr(ChGrid.Row - 1) Then
        'may be a new channel. But the previous col 0
        'must be OK
        If ChGrid.TextMatrix(ChGrid.Row - 1, 0) = "" Then
            'we are too far down the grid just ignore
            Exit Sub
        End If
    End If

    Select Case ChGrid.Col
        Case 0, 4, 5
            Exit Sub
        Case 1
            If nKey = 8 Then 'backspace
                    If Len(ChGrid.Text) <> 0 Then
                        ChGrid.Text = Left$(ChGrid.Text, Len(ChGrid.Text) - 1)
                    End If
                    Call ChannelDetailChange(ChGrid.Row, ChGrid.Col, ChGrid.Text)
                    Exit Sub
            End If
            
            If TestIMLChar(Chr$(nKey)) Then Exit Sub
                'only  0-9, A-Z, a-z, _ OK
    
            'max 12 chars
            If Len(ChGrid.Text) > 11 Then
                Exit Sub
            End If
            
        Case 2, 3
            
            'Limit Min and Max to numbers
            Select Case nKey
                Case 8
                    If Len(ChGrid.Text) <> 0 Then
                        ChGrid.Text = Left$(ChGrid.Text, Len(ChGrid.Text) - 1)
                    End If
                    Exit Sub
                Case 43, 45
                    If Len(ChGrid.Text) <> 0 Then
                        Exit Sub
                    End If
                Case 46
                    If Len(ChGrid.Text) = 0 Then
                        Exit Sub
                    End If

                Case 48 To 57
                Case Else
                    Exit Sub
            End Select
                       
    End Select

    ChGrid.Text = ChGrid.Text + Chr$(nKey)
    Call ChannelDetailChange(ChGrid.Row, ChGrid.Col, ChGrid.Text)

End Sub




'When the user closes the form we check everything
'for validity. This simply consists of checking that all
'channels have valid unique names. Give a message box
'and refuse to close.

Private Sub Form_Unload(Cancel As Integer)

    Dim nRow As Integer
    Dim cTitle As String
    Dim nTestRow As String
     
    If nEditMode <> MODE_ADD_UNALLOCATED_CHANNELS Then
        Parseform.Enabled = True
        Mainform.Enabled = True
    End If
    
    'Clear the parse action if we fail to select a
    'channel
    If bNonSelect = True Then
        If nEditMode = MODE_READ_VALUE_CHANNEL Then
            Call Parseform.ChannelSelected(-1)
        End If
        If nEditMode = MODE_WRITE_VALUE_CHANNEL Then
            Call Mainform.ChannelSelected(-1)
        End If
    End If
    
    'If no name specified just leave
    If ChGrid.TextMatrix(nEditRow, 1) = "" Then
        Exit Sub
    End If
 
    
    Cancel = DuplicateChannel

End Sub



Private Function DuplicateChannel() As Integer

    Dim nTestRow As Integer
    Dim nRow As Integer
    Dim cText As String
    
    For nTestRow = 1 To nChans
    
        'for each grid row in turn
        For nRow = 1 To nChans
                    
            'Look for a duplicate channel
            If ChGrid.TextMatrix(nRow, 1) = ChGrid.TextMatrix(nTestRow, 1) Then
                'Allowed to have no name
                If ChGrid.TextMatrix(nRow, 1) <> "" Then
                    If nRow <> nTestRow Then
                        'found one so clear the present one
                        cText = ChGrid.TextMatrix(nRow, 1) + " is Duplicated"
                        Call MsgBox(cText, vbOKOnly, "Duplicate Channel Name")
                        DuplicateChannel = 1
                        ChGrid.Row = nRow
                        ChGrid.Col = 1
        
                        'Scroll to leave one empty row
                        If nRow > 10 Then
                            ChGrid.TopRow = nChans - 9
                        End If
                                        
                        Exit Function
                    End If
                End If
            End If
        Next nRow
    Next nTestRow
    
    
    DuplicateChannel = 0

End Function


Public Sub InitialiseChannelStore()
    
    Dim nIndex As Integer
    
    'Clear any existing channels
    For nIndex = 0 To nChans
        Channels(nIndex).cMax = ""
        Channels(nIndex).cMin = ""
        Channels(nIndex).cName = ""
        Channels(nIndex).nMessage = 0
        Channels(nIndex).nMode = 0
    Next nIndex
    
    nChannel = 0
    nChans = 0
    
End Sub

'Write the channel details onto the ChannelDiag form

Public Sub ChannelDetailsShow()

    Dim nIndex As Integer
    Dim cTemp As String
    
    ChGrid.Clear
    ChGrid.TextMatrix(0, 0) = "Channel"
    ChGrid.TextMatrix(0, 1) = "Name"
    ChGrid.TextMatrix(0, 2) = "Max. Value"
    ChGrid.TextMatrix(0, 3) = "Min. Value"
    ChGrid.TextMatrix(0, 4) = "Mode"
    ChGrid.TextMatrix(0, 5) = "Msg"
   
    'Last channel is always empty
    For nIndex = 0 To nChans - 1
        ChGrid.TextMatrix(nIndex + 1, 0) = CStr(nIndex)
        ChGrid.TextMatrix(nIndex + 1, 1) = Channels(nIndex).cName
        ChGrid.TextMatrix(nIndex + 1, 2) = Channels(nIndex).cMax
        ChGrid.TextMatrix(nIndex + 1, 3) = Channels(nIndex).cMin
        Select Case Channels(nIndex).nMode
            Case READTYPE
                cTemp = "Read"
            Case WRITETYPE
                cTemp = "Write"
            Case READBIT
                cTemp = "ReadBit"
            Case WRITEBIT
                cTemp = "WriteBit"
            Case RECNOTYPE
                cTemp = "RecNo"
            Case Else
                cTemp = ""
        End Select
        ChGrid.TextMatrix(nIndex + 1, 4) = cTemp
        'if we have a mode we must have a message
        If cTemp <> "" Then
            Call Mainmenu.GetMessageName(Channels(nIndex).nMessage, cTemp)
        End If
        
        ChGrid.TextMatrix(nIndex + 1, 5) = cTemp
        
    Next nIndex
       
    Select Case nEditMode
        Case MODE_ADD_UNALLOCATED_CHANNELS
        ChGrid.SelectionMode = flexSelectionFree
        AddChan.Enabled = True
        DelChan.Enabled = True
        Script.Caption = "Name, Max and Min Values are all optional.   Mode and Msg are display only."
        
        
        Case Else
        ChGrid.SelectionMode = flexSelectionByRow
        AddChan.Enabled = True
        DelChan.Enabled = False
        Parseform.Enabled = False
        Mainform.Enabled = False
        Script.Caption = "Click on a channel to accept the extracted data"
                
    End Select

    ChGrid.Row = 1
    ChGrid.Col = 1
    ChGrid.CellBackColor = vbGreen

End Sub


'The channels details have changed
'Store the edited values
'If this is a new channel then create an extra one

Public Sub ChannelDetailChange(nRow As Integer, nCol As Integer, cNew As String)

    If nCol = 0 Or nRow = 0 Then
        Exit Sub
    End If
    
         
    Select Case nCol
        Case 1
            Channels(nRow - 1).cName = cNew
        Case 2
            Channels(nRow - 1).cMax = cNew
        Case 3
            Channels(nRow - 1).cMin = cNew
    End Select
    


End Sub



'Prevents entries into the unused part of the grid
'Goto the last full row
'if there are no channels then goto row 1

Private Sub ChGrid_EnterCell()

    If ChGrid.Col > 3 Then
        bReturn = True
        ChGrid.Row = nPreviousRow
        ChGrid.Col = nPreviousCol
        bReturn = False
        Exit Sub
    End If
                

    If ChGrid.TextMatrix(ChGrid.Row, 0) = "" Then
        
        If nChans = 0 Then
            ChGrid.Row = 1
        Else
            ChGrid.Row = nChans
        End If
        
    End If
    
    ChGrid.CellBackColor = vbGreen
  
End Sub

Private Sub ChGrid_LeaveCell()

    ChGrid.CellBackColor = 0

    If bReturn = True Then Exit Sub
    nPreviousRow = ChGrid.Row
    nPreviousCol = ChGrid.Col

End Sub


'Routine called by imdfile to load the array.
'imdfile fills in all these variables then calls this.

Public Sub AddChannel()

    Channels(nChannel).cName = cName
    Channels(nChannel).cMax = cMax
    Channels(nChannel).cMin = cMin
    Channels(nChannel).nMode = nMode
    Channels(nChannel).nMessage = nMessage
    nChannel = nChannel + 1
    nChans = nChans + 1
    
    
End Sub

'Routine called by imdfile to get access to the channel
'details. We load them into the public variables

Public Function LoadChannel(nChan As Integer) As Integer
    
    If nChan < nChans Then
        LoadChannel = True
    Else
        LoadChannel = False
        Exit Function
    End If

     nChannel = nChan
    cName = Channels(nChannel).cName
    cMax = Channels(nChannel).cMax
    cMin = Channels(nChannel).cMin
    nMode = Channels(nChannel).nMode
    nMessage = Channels(nChannel).nMessage

 End Function

' This is called to free a channel from its allocation
' Called when we add an action in parseform and a channel
' is specified at that step

Public Sub ChannelFree(nChannel As Integer)
    
    Channels(nChannel).nMode = 0
    Channels(nChannel).nMessage = 0
    
End Sub

'When we OK a channel selection then if the channel is
'not already allocated we note the new allocation
'and update parseform

Private Sub ChGrid_MouseDown(button As Integer, _
shift As Integer, x As Single, y As Single)
    
    Dim nChan As Integer
    Dim cPrompt As String
    
    If nEditMode = MODE_ADD_UNALLOCATED_CHANNELS Then
        Exit Sub
    End If
    If button <> 1 Then
        Exit Sub
    End If
    
    If ChGrid.TextMatrix(ChGrid.Row, 0) = "" Then
        Call MsgBox("Add Channels to Select")
        Exit Sub
    End If
    
    nChan = ChGrid.Row - 1

    If Channels(ChGrid.Row - 1).nMode = 0 Then
        cPrompt = "Select Channel " + CStr(nChan) + " ?"
        If MsgBox(cPrompt, vbOKCancel, "Channel Selection") = vbCancel Then
            Exit Sub
        End If
        
        Channels(nChan).nMessage = Mainmenu.nEditMsg
        Channels(nChan).nMode = nEditMode
        If nEditMode = MODE_READ_VALUE_CHANNEL _
        Or nEditMode = MODE_READ_BIT_CHANNEL _
        Or nEditMode = MODE_READ_RECNUM_CHANNEL Then
            Call Parseform.ChannelSelected(nChan)
        End If
        If nEditMode = MODE_WRITE_VALUE_CHANNEL Or nEditMode = MODE_WRITE_BIT_CHANNEL Then
            Call Mainform.ChannelSelected(nChan)
        End If
        bNonSelect = False
        Call Unload(ChannelDiag)
    Else
        Call MsgBox("Channel already allocated")
    End If
  

End Sub

'Count the number of channels refered to in the passed in message
'Return the count

Public Function Channel_Count(nMsg As Integer) As Integer

    Dim nIndex As Integer
    Channel_Count = 0
    
    For nIndex = 0 To nChans - 1
        If Channels(nIndex).nMessage = nMsg Then
            Channel_Count = Channel_Count + 1
        End If
    Next nIndex
    
End Function

'When we delete a message then higher message numbers are decremented
'We have already checked that the deleted message has no data channels

Public Sub Channel_Message_Delete(nDelMsg As Integer)

    Dim nIndex As Integer
    
    For nIndex = 0 To nChans - 1
    
        If Channels(nIndex).nMessage > nDelMsg Then
            Channels(nIndex).nMessage = Channels(nIndex).nMessage - 1
        End If
    
    Next nIndex
    
    
End Sub
