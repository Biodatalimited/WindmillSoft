Attribute VB_Name = "SerialGrid"
Option Explicit

Declare Function DLLStringToFloat Lib "IMLComm.dll" Alias "_DLLStringToFloat@8" (ByVal pszString As String, ByVal pBuffer As String) As Long


'Dim bNonPrintMode As Boolean
Public Const cValue As String = "Value"
Public Const cILSMS As String = "IntLM"
Public Const cIMSLS As String = "IntML"
Public Const cWLSMS As String = "WorLM"
Public Const cWMSLS As String = "WorML"
Public Const cLLSMS As String = "LngLM"
Public Const cLBS As String = "LngBS"
Public Const cLWS As String = "LngWS"
Public Const cLMSLS As String = "LngML"
Public Const cDWLSMS As String = "DWrLM"
Public Const cDWBS As String = "DWrBS"
Public Const cDWWS As String = "DWrWS"
Public Const cDWMSLS As String = "DWrML"
Public Const cSBYTE As String = "SByte"
Public Const cUBYTE As String = "Byte "
Public Const cBIT As String = "Bit "
Public Const cFLOATLSMS As String = "FltLM"
Public Const cFLOATBS As String = "FltBS"
Public Const cFLOATWS As String = "FltWS"
Public Const cFLOATMSLS As String = "FltML"
Public Const cHex As String = "Hex  "
Public Const cCustom = "Cus"

Public Const MAXROWS As Integer = 16384
Public bWait As Boolean

'Validates and interprets each key press as it is made in
'a cell

Public Sub GridKey_Press(oGrid As MSFlexGrid, nKey As Integer)

    Dim cCell As String
    Dim bNonPrintchar As Boolean
                
        bNonPrintchar = False
    
        'Can't overwrite channel insertions or waits
        If Len(oGrid.TextMatrix(oGrid.Row, 1)) > 3 Then
            Beep
            Exit Sub
        End If
    
        'If we are entering in the hex column
        If oGrid.Col = 2 Then
            Call Hex_KeyPress(oGrid, nKey)
            Exit Sub
        End If
                        
        
        'Allow printing chars only
        Select Case nKey
            Case 32 To 127
                cCell = Chr$(nKey)
            Case 13
                cCell = "013"
            Case Else
                Exit Sub
        End Select
    
        'Write the string and move to next cell
        Call GridWriteRow(oGrid, cCell)
        oGrid.Row = oGrid.Row + 1
   
End Sub

Public Sub GridInitialise(oGrid As MSFlexGrid)

    Dim nIndex As Integer

    oGrid.Clear
    oGrid.Rows = 20
    oGrid.Cols = 3
    oGrid.ColWidth(0) = 600
    oGrid.ColWidth(1) = 600
    oGrid.ColWidth(2) = 400
    oGrid.Width = 1900
       
    oGrid.ColAlignment(0) = flexAlignCenterCenter
    oGrid.ColAlignment(1) = flexAlignCenterCenter
    oGrid.ColAlignment(2) = flexAlignCenterCenter
 
       
    oGrid.TextMatrix(0, 0) = "Byte"
    oGrid.TextMatrix(0, 1) = "Char"
    oGrid.TextMatrix(0, 2) = "Hex"
          
    oGrid.Row = 1
    oGrid.Col = 1

    oGrid.TopRow = 1

End Sub

'Put the byte number into column 0
Private Sub ByteNumber(oGrid As MSFlexGrid)
    oGrid.TextMatrix(oGrid.Row, 0) = CStr(oGrid.Row)
End Sub

'Used to write in a full row after a string has been entered in an Input box
'This will be a number 0 to 255

Public Sub GridWriteNonPrintRow(oGrid As MSFlexGrid, cCell As String)

    oGrid.Col = 1               'Make sure we are in col 1. Might be in 2
    Call GridWriteRow(oGrid, cCell)
    oGrid.Row = oGrid.Row + 1
 

End Sub


'This routine fills a complete row
'It writes the byte number
'The char and the hex value

Public Sub GridWriteRow(oGrid As MSFlexGrid, cCell As String)
    
    Dim nRow As Integer
    
    'Number this cell.
    Call ByteNumber(oGrid)
    oGrid.Text = cCell
    nRow = oGrid.Row
    
    'This is hex entry
    'Add the decimal equivalent
    If oGrid.Col = 2 Then
        'Fill col 1 with char equivalent
        cCell = HexToDec(cCell)
        'If its printable replace with character version
        If Val(cCell) > 31 And Val(cCell) < 128 Then
            cCell = Chr$(Val(cCell))
        End If
        oGrid.TextMatrix(nRow, 1) = cCell
    Else
        'This is entry in Col 1
        'Depends on NonPrint Mode
        If Len(cCell) = 3 Then
            oGrid.TextMatrix(nRow, 1) = cCell
            'Get the hex equivalent
            cCell = DecToHex(cCell)
        Else
            'Not NonPrint so single char
            Call CharToHex(cCell)
        End If
        'Write the hex col
        oGrid.TextMatrix(nRow, 2) = cCell
    End If

    oGrid.CellBackColor = 0 'Revert to white
       
    'Add new rows as needed
    If nRow > oGrid.Rows - 5 Then
        oGrid.Rows = nRow + 5
    End If
    
    'scroll as needed
    If oGrid.Row - oGrid.TopRow > 12 Then
        oGrid.TopRow = oGrid.Row - 12
    End If
     
     
End Sub




'Called when we change cells or grids. It checks that its OK to do so.
'If we try to enter beyond the end of data. We change row to the next place to write
'Set the call Back Colour to Green



Public Sub GridsetEnter(oGrid As MSFlexGrid)

    Dim nEnd As Integer
    
                    
    'If we try to move beyond the end of data
    'move us back to the next cell to add
    nEnd = FindEndOfData(oGrid)
    If oGrid.Row > nEnd Then
        oGrid.Row = nEnd
    End If
     
    oGrid.CellBackColor = vbGreen
    
End Sub

'On leaving a cell we must complete the row to ensure that all columns tally
'Inserted Channels or Waits require no changes to the row
'If the current cell is empty then clear the whole row
'If it has a value then complete the row
'Always change the Back colour to white

Public Sub GridsetLeave(oGrid As MSFlexGrid)

    
    If CheckRow(oGrid) Then
        Beep
    End If
      
    'Cell color reverts to white
    oGrid.CellBackColor = 0
    
    'Detect inserted channels
    'or Wait instructions
    'Number the row and quit we do not use GridWriteRow for these types
    If Len(oGrid.TextMatrix(oGrid.Row, 1)) > 3 Then
        Call ByteNumber(oGrid)
        Exit Sub
    End If
         
    If oGrid.Text <> "" Then
        Call GridWriteRow(oGrid, oGrid.Text)
    Else
        oGrid.TextMatrix(oGrid.Row, 1) = ""
        oGrid.TextMatrix(oGrid.Row, 2) = ""
    End If
    

End Sub


Public Function CheckRow(oGrid As MSFlexGrid) As Integer

    Dim cError As String


    cError = ""
    
    If oGrid.TextMatrix(oGrid.Row, 0) = "" Then
        CheckRow = 0
        Exit Function
    End If

    Select Case Len(oGrid.TextMatrix(oGrid.Row, 1))
    
    Case 0
        'Its valid to have nothing in Col 1 if Col 2 has a 2 byte hex
        If Len(oGrid.TextMatrix(oGrid.Row, 2)) <> 2 Then
            cError = "The Char Column is empty"
        End If
    Case 1
        If Asc(oGrid.TextMatrix(oGrid.Row, 1)) <> Val(HexToDec(oGrid.TextMatrix(oGrid.Row, 2))) Then
            cError = "The Char and Hex Columns Disagree"
        End If
    Case 2
        cError = "2 chars in Char column"
    Case 3
        If DecToHex(oGrid.TextMatrix(oGrid.Row, 1)) <> oGrid.TextMatrix(oGrid.Row, 2) Then
            cError = "The Char and Hex Columns Disagree"
        End If
    Case 4
        If oGrid.TextMatrix(oGrid.Row, 2) <> "" Then
            cError = "Wait shpuld have no entry in Hex column"
        End If
    Case 5
        'Output channels need a channel number
        If oGrid.TextMatrix(oGrid.Row, 2) = "" Then
            If Left$(oGrid.TextMatrix(oGrid.Row, 1), 3) = "Cus" Then
                'Custom Values larger than 49 need output channels
                If Val(Right$(oGrid.TextMatrix(oGrid.Row, 1), 2)) > 49 Then
                    cError = "Custom Output instruction with no channel"
                End If
            Else
                cError = "Output instruction with no channel"
            End If
        End If
        
    End Select
    
    
    If cError <> "" Then
        Call MsgBox(cError, vbOKOnly, "Error on Row")
        CheckRow = 1
    Else
        CheckRow = 0
    End If
    
    


End Function


Private Sub Hex_KeyPress(oGrid As MSFlexGrid, nKey As Integer)

    Dim cCode As String
    
   
    cCode = oGrid.Text
    
    'Backspace always OK. Simply remove the last
    'char and finish
    If nKey = 8 Then
        Select Case Len(cCode)
            Case 0
                Exit Sub
            Case 1
                cCode = ""
                
            Case 2
                cCode = Left$(cCode, 1)
        End Select
        Call GridWriteRow(oGrid, cCode)

        Exit Sub
    End If
                         
    'CR ends the input if we have 2 chars
    If nKey = 13 Then
        If Len(cCode) <> 2 Then
            Beep
            Exit Sub
        End If
        Call GridWriteRow(oGrid, cCode)
        oGrid.Row = oGrid.Row + 1
        Exit Sub
    End If
    
   
    Select Case Chr$(nKey)
        Case "0" To "9"
        Case "A" To "F"
        Case "a" To "f"
            nKey = nKey - 32 'Convert to upper case
        Case Else
            Beep
            Exit Sub
    End Select
    
    If Len(cCode) > 1 Then
        cCode = Mid$(cCode, 2, 1)
    End If
    
    cCode = cCode + Chr$(nKey)
    Call GridWriteRow(oGrid, cCode)
 
End Sub

'Convert a 2 byte hex string to a 3 byte decimal string

Private Function HexToDec(cHex As String) As String

    Dim nTemp As Integer
    Dim cReply As String
    
    If Len(cHex) <> 2 Then
        cHex = ""
        Exit Function
    End If
    
    nTemp = 16 * HexToInt(Left$(cHex, 1)) + HexToInt(Right$(cHex, 1))
    
    cReply = CStr(nTemp)
    
    Select Case Len(cReply)
        Case 1
            cReply = "00" + cReply
        Case 2
            cReply = "0" + cReply
    End Select
    
    HexToDec = cReply
    
End Function

'Convert a 3 byte decimal string to a 2 byte hex string

Private Function DecToHex(cDecimal As String) As String

    Dim nTemp As Integer
    
    nTemp = Val(cDecimal)
    
    DecToHex = IntToHex(nTemp \ 16) + IntToHex(nTemp - 16 * (nTemp \ 16))
    
End Function

'Convert a hex byte into a value 0 to 15
Private Function HexToInt(cHex As String) As Integer
    
    Select Case cHex
        Case "0" To "9"
            HexToInt = Asc(cHex) - Asc("0")
        Case "A" To "F"
            HexToInt = Asc(cHex) - Asc("A") + 10
        Case Else
            HexToInt = 0
    End Select
 
End Function

'Convert an integer 0-15 to a hex byte
Private Function IntToHex(nValue As Integer) As String

    Select Case nValue
        Case 0 To 9
            IntToHex = Chr$(Asc("0") + nValue)
        Case 10 To 15
            IntToHex = Chr$(Asc("A") + nValue - 10)
    End Select
End Function

'Convert any char to a hex string
Private Sub CharToHex(cChar As String)

    Dim nTemp As Integer
    
    If cChar = "" Then
        nTemp = 0
    Else
        nTemp = Asc(cChar)
    End If
    
    cChar = IntToHex(nTemp \ 16) + IntToHex(nTemp - 16 * (nTemp \ 16))
  
End Sub

Public Function FindEndOfData(oGrid As MSFlexGrid) As Integer

    'This looks for a "" in col 0 of the data.

    Dim nIndex As Integer
    
    'Look for a "" in col 0
    For nIndex = 1 To oGrid.Rows - 1
        If oGrid.TextMatrix(nIndex, 0) = "" Then
            Exit For
        End If
    Next nIndex
     
  
    FindEndOfData = nIndex
    
 End Function



'This routine deletes one row from the grid and moves
'the lower rows up one

Public Sub GridDeleteRow(oGrid As MSFlexGrid)

    Dim nRow As Integer
    
    nRow = oGrid.Row

    'Can't delete an empty row
    If oGrid.TextMatrix(nRow, 0) = "" Then Exit Sub
        
    'Remove the item
    oGrid.RemoveItem (nRow)
    
    Call GridRenumber(oGrid)
   
    oGrid.CellBackColor = vbGreen
    
    'Maintain a minimum of 20 rows
    If oGrid.Rows < 20 Then
        oGrid.Rows = oGrid.Rows + 1
    End If
    

End Sub

'Renumber the steps in Col 0 after an insertion or deletion
'Return the number of the final used row

Public Function GridRenumber(oGrid As MSFlexGrid) As Integer

    Dim nRow As Integer
    
    For nRow = 1 To oGrid.Rows
        If oGrid.TextMatrix(nRow, 0) = "" Then
            Exit For
        End If
        oGrid.TextMatrix(nRow, 0) = CStr(nRow)
    Next nRow
    
    GridRenumber = nRow

End Function


'This routine inserts a row in the grid
'It increases the number of rows if needed
'It renumbers the rows
'It leaves the focus at the empty cell

Public Sub GridInsertRow(oGrid As MSFlexGrid)

    Dim nEnd As Integer
    Dim nIndex As Integer
    Dim nRow As Integer
    
    nRow = oGrid.Row
    oGrid.CellBackColor = 0
    
    If oGrid.TextMatrix(nRow, 0) = "" Then Exit Sub
      
    'Add the row. With a 0 number so renumber works
    Call oGrid.AddItem("0", nRow)
      
    'Renumber it all
    nEnd = GridRenumber(oGrid)
    
    'Add extra rows if needed
    If (oGrid.Rows - nEnd) < 5 Then
        oGrid.Rows = oGrid.Rows + 1
    End If
    

    'including any back color
    oGrid.Col = 1
    oGrid.CellBackColor = vbGreen
       


End Sub




'Send a prompt string in response to a Send click
'If bSend is true really send it else simply prepare the cRunString for the RUN form

Public Function SendPromptString(oGrid As MSFlexGrid, bSend As Boolean) As String
    Dim dwIndex As Long
    Dim dwSize As Long
    Dim cTemp As String
    Dim cMessage As String
    Dim dwCheck As Long
    Dim nLastByte As Integer
    Dim nBitPos As Integer
    Dim nORMask As Integer
    Dim nANDMask As Integer
    Dim cRunString As String
    
    'Initialise the bit masks
    nORMask = 0
    nANDMask = 255
        
    'Initialise the string used for RUN purposes
    cRunString = ""
      
    'First prepare the string
    cMessage = ""
    For dwIndex = 1 To oGrid.Rows - 1
        
        'read the chars in Col 1
        cTemp = oGrid.TextMatrix(dwIndex, 1)
        Select Case Len(cTemp)
            Case 0
                Exit For
            Case 1  'Simple char
                cTemp = ApplyMask(cTemp, nORMask, nANDMask)
                cMessage = cMessage & cTemp
            Case 3  'This is a non print
                cTemp = Chr$(Val(cTemp))
                cTemp = ApplyMask(cTemp, nORMask, nANDMask)
                cMessage = cMessage & cTemp
            Case 4  'Either Wait or CRC
                If cTemp = "Wait" Then
                    dwSize = Len(cMessage)
                    cRunString = cRunString + cMessage + "%WAIT"
                    If bSend Then
                        Call Channel_Send(cMessage, dwSize)
                        DoWait
                    End If
                    cMessage = ""
                End If
                If cTemp = "CRC " Then
                    cMessage = cMessage + DoCRC(cMessage)
                End If
            Case 5  'This is an inserted channel
                'Bit insertions are special. We simply
                'prepare the masks and wait for a normal
                'char or a non print to apply them to
                If Left$(cTemp, 4) = cBIT Then
                    'Final byte of cTemp is bit pos 0 to 7
                    nBitPos = Val(Right$(cTemp, 1))
                    If GetUserValue(cBIT, oGrid.TextMatrix(dwIndex, 2)) = "1" Then
                        'Set the bit
                        nORMask = nORMask Or 2 ^ nBitPos
                    Else
                        'Clear the bit
                        nANDMask = nANDMask And (255 - 2 ^ nBitPos)
                    End If
                Else
                    If Left$(cTemp, 3) = cCustom Then
                        cMessage = cMessage + CustomGetUserValue(cTemp, oGrid.TextMatrix(dwIndex, 2), cMessage)
                    Else
                        cMessage = cMessage + GetUserValue(cTemp, oGrid.TextMatrix(dwIndex, 2))
                    End If
                End If
        End Select
        
     Next dwIndex
     
     cRunString = cRunString + cMessage
     
     dwSize = Len(cMessage)
     If bSend Then Call Channel_Send(cMessage, dwSize)
     
     SendPromptString = cRunString
     
End Function

'This is called to apply the mask created by Bit insertions
'to a sent out char. This can have one or more bits to set or clear
'We return the masked char and reset the bit masks. We
'simply return the passed in char if there is nothing
'to do.

Private Function ApplyMask(cChar As String, nORMask As Integer, nANDMask As Integer) As String

    Dim nTemp As Integer
    
    If nORMask = 0 And nANDMask = 255 Then
        ApplyMask = cChar
        Exit Function
    End If
    
    nTemp = Asc(cChar)
    nTemp = (nTemp Or nORMask) And nANDMask
    ApplyMask = Chr$(nTemp)
    
    nORMask = 0
    nANDMask = 255
    
End Function

'This routine calculates the CRC-16 on the passed in string
'The result is appended to the string as 2 bytes

Private Function DoCRC(cMessage As String) As String

    Dim dwIndex As Long
    Dim dwLsb As Long
    Dim dwMsb As Long
    Dim dwCheck As Long
     
    dwIndex = Len(cMessage)
     
    dwCheck = GenerateCRC(65535, cMessage, dwIndex)
     
    dwMsb = dwCheck \ 256
    dwLsb = dwCheck - dwMsb * 256
     
    DoCRC = Chr$(dwLsb) & Chr$(dwMsb)
     
End Function

'Do the wait defined by idle time when we send out a message

Public Sub DoWait()

    Mainform.Timer2.Interval = Timing.nIdleTime
    Mainform.Timer2.Enabled = True
    bWait = True
    While bWait
        DoEvents
    Wend
    Mainform.Timer2.Enabled = False
   

End Sub

'When we are sending a prompt string out and we find
'an inserted value then we prompt the user for something
'to put into the string
'cType is the type of value required
'cChannel is the data channel number
'We return the string representing the value

Private Function GetUserValue(cType As String, cChannel As String) As String

    Dim cScript As String
    Dim vVal As Variant
    Dim vLSB As Variant
    Dim vMSB As Variant
    Dim vByte As Variant
    Dim vTemp As Variant
    Dim bNeg As Boolean
    Dim cBuffer As String * 10
    Dim nIndex As Integer
    Dim cTemp As String
    Dim cChannelText As String
    Dim vLong(4) As Variant
    
    Call ChannelDiag.LoadChannel(Val(cChannel))
    
    cChannelText = cChannel + "  " + ChannelDiag.cName
    
    'Run the input box until we get a valid value for
    'the specific type
    Do
    
    Select Case cType
        Case cValue
            cScript = "Enter a string max 12 chars for Channel " + cChannelText
            cScript = InputBox(cScript)
            Exit Do
        Case cILSMS, cIMSLS
            cScript = "Enter a number -32768 to +32767 for Channel " + cChannelText
            cScript = InputBox(cScript)
            If Val(cScript) > -32769 And Val(cScript) < 32768 Then
                Exit Do
            End If
        Case cWLSMS, cWMSLS
            cScript = "Enter a number 0 to 65535 for Channel " + cChannelText
            cScript = InputBox(cScript)
            If Val(cScript) > -1 And Val(cScript) < 65536 Then
                Exit Do
            End If
        Case cLLSMS, cLBS, cLWS, cLMSLS
            cScript = "Enter a number -2,147,483,648 to +2,147,483,647 for Channel " + cChannelText
            cScript = InputBox(cScript)
            If Val(cScript) > -2147483649# And Val(cScript) < 2147483648# Then
                Exit Do
            End If
        Case cDWLSMS, cDWBS, cDWWS, cDWMSLS
            cScript = "Enter a number 0 to 4,294,967,295 for Channel " + cChannelText
            cScript = InputBox(cScript)
            If Val(cScript) > -1 And Val(cScript) < 4294967296# Then
                Exit Do
            End If
        Case cSBYTE
            cScript = "Enter a number -128 to + 127 for channel " + cChannelText
            cScript = InputBox(cScript)
            If Val(cScript) > -129 And Val(cScript) < 128 Then
                Exit Do
            End If
        Case cUBYTE
            cScript = "Enter a number 0 to 255 for channel " + cChannelText
            cScript = InputBox(cScript)
            If Val(cScript) > -1 And Val(cScript) < 256 Then
                Exit Do
            End If
        Case cBIT
            cScript = "Enter 0 or 1 for channel " + cChannelText
            cScript = InputBox(cScript)
            If Val(cScript) = 0 Or Val(cScript) = 1 Then
                Exit Do
            End If
         Case cFLOATMSLS, cFLOATBS, cFLOATWS, cFLOATLSMS
            cScript = "Enter a numeric value for channel " + cChannelText
            cScript = InputBox(cScript)
            If cScript = "" Then
                cScript = "0"
            End If
            Exit Do
         Case cHex
            cScript = "Enter Hex chars to define bytes " + cChannelText + vbCr _
            + "Use upper case for A to F" + vbCr _
            + "2 Hex chars for each byte. Max 6 bytes"
            cScript = InputBox(cScript)
            If TestHex(cScript) = 0 Then
                Exit Do
            End If
   
    End Select
    
    Loop
    
    'cScript now contains the output value
    
    'For the binary types of output we must change the
    'entered value into bytes. We calculate the LSB, MSB
    'of an integer and the Byte of a single byte. We only
    'do it for these types because we otherwise get
    'overflows.
    Select Case cType
    
        Case cILSMS, cIMSLS, cWLSMS, cWMSLS, cSBYTE, cUBYTE
        
        vVal = Val(cScript)
        If vVal < 0 Then
            vTemp = vVal + 32768
            vMSB = vTemp \ 256
            vLSB = vTemp - vMSB * 256
            vMSB = vMSB + 128
            vByte = vVal + 256
        Else
            vMSB = vVal \ 256
            vLSB = vVal - vMSB * 256
            vByte = vVal
        End If
        
        Case cLLSMS, cLBS, cLWS, cLMSLS, cDWLSMS, cDWBS, cDWWS, cDWMSLS
        
        vVal = Val(cScript)
        
        'Handle negative numbers
        vTemp = vVal
        If vVal < 0 Then
            bNeg = True
            vTemp = vVal + 2147483648#
        Else
            bNeg = False
        End If
        
        'Handle positives needing MSB set
        If vVal > 2147483647 Then
            bNeg = True
            vTemp = vVal - 2147483648#
        End If
        
        vLong(0) = vTemp - 256 * (vTemp \ 256)
        vTemp = vTemp \ 256
        vLong(1) = vTemp - 256 * (vTemp \ 256)
        vTemp = vTemp \ 256
        vLong(2) = vTemp - 256 * (vTemp \ 256)
        vLong(3) = vTemp \ 256

        If bNeg Then vLong(3) = vLong(3) + 128
        
        Case cFLOATLSMS, cFLOATBS, cFLOATWS, cFLOATMSLS
            Call DLLStringToFloat(cScript, cBuffer)
        
    End Select
    
    'Now create the strings for the binary types
    Select Case cType
    
        Case cILSMS, cWLSMS
            cScript = Chr$(vLSB) + Chr$(vMSB)
        Case cIMSLS, cWMSLS
            cScript = Chr$(vMSB) + Chr$(vLSB)
        Case cLMSLS, cDWMSLS
            cScript = Chr$(vLong(3)) + Chr$(vLong(2)) + Chr$(vLong(1)) + Chr$(vLong(0))
        Case cLBS, cDWBS
            cScript = Chr$(vLong(2)) + Chr$(vLong(3)) + Chr$(vLong(0)) + Chr$(vLong(1))
        Case cLWS, cDWWS
            cScript = Chr$(vLong(1)) + Chr$(vLong(0)) + Chr$(vLong(3)) + Chr$(vLong(2))
        Case cLLSMS, cDWLSMS
            cScript = Chr$(vLong(0)) + Chr$(vLong(1)) + Chr$(vLong(2)) + Chr$(vLong(3))
        Case cSBYTE, cUBYTE
            cScript = Chr$(vByte)
        Case cFLOATMSLS
            cScript = Mid$(cBuffer, 4, 1) + Mid$(cBuffer, 3, 1) _
            + Mid$(cBuffer, 2, 1) + Mid$(cBuffer, 1, 1)
        Case cFLOATBS
            cScript = Mid$(cBuffer, 3, 1) + Mid$(cBuffer, 4, 1) _
            + Mid$(cBuffer, 1, 1) + Mid$(cBuffer, 2, 1)
        Case cFLOATWS
            Call DLLStringToFloat(cScript, cBuffer)
            cScript = Mid$(cBuffer, 2, 1) + Mid$(cBuffer, 1, 1) _
            + Mid$(cBuffer, 4, 1) + Mid$(cBuffer, 3, 1)
        Case cFLOATLSMS
            cScript = Left$(cBuffer, 4)
        Case cHex
            cScript = Hex2Bytes(cScript)
    End Select
    
    GetUserValue = cScript
    
End Function

'Check its a hex string of up to 12 chars. Even number of chars only
'Return 1 if error 0 else

Public Function TestHex(cTest As String) As Integer

    Dim nIndex As Integer
    
    TestHex = 1         'Assume error
    
    If Len(cTest) = 0 Then
        Exit Function
    End If

    If Len(cTest) > 12 Then
        Exit Function
    End If

    'Even number of bytes needed
    If Len(cTest) And 1 Then
        Exit Function
    End If

    For nIndex = 1 To Len(cTest)
    
        Select Case Mid$(cTest, nIndex, 1)
        
        Case "0" To "9"
            TestHex = 1
        Case "A" To "F"
            TestHex = 1
        Case Else
            Exit Function
        End Select
        
    Next nIndex
    
    TestHex = 0

End Function


'The user has entered a series of hex chars
'Convert them to a string of bytes

Public Function Hex2Bytes(cTest As String) As String

    Dim nIndex As Integer
    Dim nTemp As Integer
    Dim nValue As Integer
    
    Hex2Bytes = ""
    
    For nIndex = 1 To Len(cTest) / 2
    
        nTemp = Asc(Mid$(cTest, nIndex * 2 - 1, 1)) - 48
        If nTemp < 10 Then
            nValue = 16 * nTemp
        Else
            nValue = 16 * (nTemp - 7)
        End If
        
        nTemp = Asc(Mid$(cTest, nIndex * 2, 1)) - 48
        If nTemp < 10 Then
            nValue = nValue + nTemp
        Else
            nValue = nValue + (nTemp - 7)
        End If
         
        Hex2Bytes = Hex2Bytes + Chr$(nValue)
    
   
    Next nIndex


End Function

'Read bytes from the comms port and append them to
'the receive grid.
'Fill in both direct and hex columns
'Deal with non prints
'We increase the grid size as required
'We also limit the grid size to stop it going crazy


Public Sub AppendReplyString(oGrid As MSFlexGrid)


    Dim nEnd As Integer
    Dim nIndex As Integer
    Dim cBuf As String * MAXROWS
    Dim dwSize As Long
    Dim dwBytesRead As Long
    Dim cTemp As String
    Dim cHex As String
    
    'Stop it running away on continuous flow
    If oGrid.Rows > MAXROWS Then
        Exit Sub
    End If
     
    'Maximum allowed 2048 bytes
    Call Channel_Read(cBuf, MAXROWS, dwBytesRead)
    
    nEnd = FindEndOfData(oGrid)
    'Point to last full cell
    nEnd = nEnd - 1
        
    'Increase the grid
    If (nEnd + dwBytesRead) > (oGrid.Rows - 5) Then
        oGrid.Rows = nEnd + dwBytesRead + 5
    End If
    
    For nIndex = 1 To dwBytesRead
        'Write the byte number into col 0
        oGrid.TextMatrix(nEnd + nIndex, 0) = CStr(nEnd + nIndex)
        'Write the hex into col 2
        cTemp = Mid$(cBuf, nIndex, 1)
        cHex = cTemp
        Call CharToHex(cHex)
        oGrid.TextMatrix(nEnd + nIndex, 2) = cHex
        'Write the byte into col 1
        Select Case Asc(cTemp)
            Case 32 To 127
            Case Else
                cTemp = CStr(Asc(cTemp))
                If Len(cTemp) = 1 Then
                    cTemp = "00" + cTemp
                End If
                If Len(cTemp) = 2 Then
                    cTemp = "0" + cTemp
                End If
        End Select
        oGrid.TextMatrix(nEnd + nIndex, 1) = cTemp
        
    Next nIndex
    

End Sub

'This routine gets the contents of a grid as a string
'It converts the non printing chars to binary  and returns
'the string size. Used only to extract the received data
'string for parsing

Public Sub SerialGrid_GetString(oGrid As MSFlexGrid, cContents As String, dwSize As Long)

    Dim nEnd As Integer
    Dim cTemp As String
    Dim cExtraction As String
    
    nEnd = FindEndOfData(oGrid)
    cContents = ""
     For dwSize = 1 To nEnd - 1
        cTemp = oGrid.TextMatrix(dwSize, 1)
        If Len(cTemp) = 1 Then
            cContents = cContents & cTemp
        Else
            cContents = cContents + Chr$(Val(cTemp))
        End If
    Next dwSize
    
    
End Sub

'Adds cell back color used during parse stepping

Public Sub SerialGridColor(oGrid As MSFlexGrid, dwSteps As Long)
   
    Dim nIndex As Integer
   
    If oGrid.Rows < oGrid.Row + dwSteps + 1 Then
        Beep
        Exit Sub
    End If
   
    oGrid.Col = 1
    For nIndex = 1 To dwSteps
        oGrid.CellBackColor = vbGreen
        oGrid.Row = oGrid.Row + 1
    Next nIndex

    'Scroll the grid to leave some extra lines visible
    If oGrid.Row > 12 Then
        oGrid.TopRow = oGrid.Row - 12
    End If

End Sub

'Removes all cell back color
'Called by parseform to restart a parse

Public Sub SerialGridClearColor(oGrid As MSFlexGrid)

    Dim nIndex As Integer
    Dim nEnd As Integer
    
    oGrid.Row = 1
    oGrid.Col = 1
    
    While oGrid.CellBackColor <> 0
        oGrid.CellBackColor = 0
        oGrid.Row = oGrid.Row + 1
    Wend
    
    oGrid.Row = 1
       
End Sub

'This checks a grid contents for errors
'It tells the user about them and points to
'the faulty row. It returns true in case of error

Public Function GridCheck(oGrid As MSFlexGrid) As Boolean

    Dim nRow As Integer
    Dim nEnd As Integer
    Dim cTitle As String
    Dim cHex As String
    Dim cChar As String
    
    GridCheck = True
    cTitle = "Message Definition Error"
    nEnd = FindEndOfData(oGrid)
    For nRow = 1 To nEnd - 1
    
        cHex = oGrid.TextMatrix(nRow, 2)
        cChar = oGrid.TextMatrix(nRow, 1)
        Select Case Len(cChar)
            Case 0
                oGrid.Row = nRow
                oGrid.CellBackColor = vbGreen
                Call MsgBox("No Value in Green Cell", , cTitle)
                Exit Function
            Case 1
                Call CharToHex(cChar)
                If cChar <> cHex Then
                    oGrid.Row = nRow
                    oGrid.CellBackColor = vbGreen
                    Call MsgBox("Char and hex columns disagree", , cTitle)
                    Exit Function
                End If
            Case 2, 3
                cChar = DecToHex(cChar)
                If cChar <> cHex Then
                    oGrid.Row = nRow
                    oGrid.CellBackColor = vbGreen
                    Call MsgBox("Char and hex columns disagree", , cTitle)
                    Exit Function
                End If
            Case 4
                If cChar <> "Wait" And cChar <> "CRC " Then
                    oGrid.Row = nRow
                    oGrid.CellBackColor = vbGreen
                    Call MsgBox("Unrecognised Instruction", , cTitle)
                     Exit Function
                End If
            Case 5
                If cHex = "" Then
                    If Left$(cChar, 3) <> cCustom Then
                        oGrid.Row = nRow
                        oGrid.CellBackColor = vbGreen
                        Call MsgBox("Inserted Value has no Channel Number", , cTitle)
                    End If
                End If
             
            Case Else
                oGrid.Row = nRow
                oGrid.CellBackColor = vbGreen
                Call MsgBox("Too many characters", , cTitle)
                Exit Function
        End Select
        
    Next nRow

    GridCheck = False
End Function

'This sub loads the passed in string into the grid.
'This involves checking for non prints and inserted values
'It is called when the form loads to fill the prompt
'and ack grids with an existing message

Public Sub GridLoadString(oGrid As MSFlexGrid, szStr As String)
   
    Dim nIndex As Integer
    Dim nTemp As Integer
    Dim cTemp As String
    Dim cCode As String
    Dim cChannel As String
     
    oGrid.Col = 1
    
     
    'For each char
    For nIndex = 1 To Len(szStr)
        'If it is a \
        If Mid$(szStr, nIndex, 1) = "\" Then
        
            Select Case Mid$(szStr, nIndex + 1, 1)
            Case "\"
                'If it's \\ then just means \
                Call GridWriteRow(oGrid, Mid$(szStr, nIndex, 1))
                nIndex = nIndex + 1
            Case "C"
                'This must be \C(123) non print type
                
                'If its just a leading space then convert it to such
                'Space then appears as space not 032
                If Mid$(szStr, nIndex + 3, 3) = "032" Then
                    'Just a space
                    Call GridWriteRow(oGrid, " ")
                Else
                    Call GridWriteRow(oGrid, Mid$(szStr, nIndex + 3, 3))
                End If
                nIndex = nIndex + 6

            Case "E"
                oGrid.Text = cValue
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "G"
                'Get the type number as if it is a channel number
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
                nTemp = Val(oGrid.TextMatrix(oGrid.Row, 2))
                'Get the real channel number needs an extra leading char
                nIndex = nIndex - 1
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
                
                Select Case nTemp
                Case 0
                    oGrid.Text = cDWMSLS
                Case 1
                    oGrid.Text = cDWBS
                Case 2
                    oGrid.Text = cDWWS
                Case 3
                    oGrid.Text = cDWLSMS
                Case 8
                    oGrid.Text = cLMSLS
                Case 9
                    oGrid.Text = cLBS
                Case 10
                    oGrid.Text = cLWS
                Case 11
                    oGrid.Text = cLLSMS
                Case 16
                    oGrid.Text = cFLOATMSLS
                Case 17
                    oGrid.Text = cFLOATBS
                Case 18
                    oGrid.Text = cFLOATWS
                Case 19
                    oGrid.Text = cFLOATLSMS
                End Select
                
            Case "H"
                oGrid.Text = cHex
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "J"
                oGrid.Text = cILSMS
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "K"
                oGrid.Text = cIMSLS
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "L"
                oGrid.Text = cWLSMS
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "M"
                oGrid.Text = cWMSLS
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "N"
                oGrid.Text = cSBYTE
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "O"
                oGrid.Text = cUBYTE
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "P"
                oGrid.Text = cBIT
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
                'Add the bit number
                oGrid.Text = oGrid.Text + Mid$(szStr, nIndex + 1, 1)
                nIndex = nIndex + 1
            Case "Q"
                oGrid.Text = cFLOATLSMS
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "R"
                oGrid.Text = cFLOATMSLS
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "U"
                oGrid.Text = cLLSMS
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "V"
                oGrid.Text = cLMSLS
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "X"
                oGrid.Text = cDWLSMS
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "Y"
                oGrid.Text = cDWMSLS
                nIndex = nIndex + GridWriteChannelNum(oGrid, Right$(szStr, Len(szStr) - nIndex))
            Case "W"
                oGrid.Text = "Wait"
                oGrid.TextMatrix(oGrid.Row, 0) = CStr(oGrid.Row)
                nIndex = nIndex + 1
            Case "Z"
                oGrid.Text = "CRC "
                oGrid.TextMatrix(oGrid.Row, 0) = CStr(oGrid.Row)
                nIndex = nIndex + 1
            Case "A"
                'Get the Custom Index via the grid
                nIndex = nIndex + CustomLoadString(Right$(szStr, Len(szStr) - nIndex), cCode, cChannel)
                oGrid.Text = cCode
                oGrid.TextMatrix(oGrid.Row, 2) = cChannel
                oGrid.TextMatrix(oGrid.Row, 0) = CStr(oGrid.Row)
            End Select
        Else
            'Just a normal char
            Call GridWriteRow(oGrid, Mid$(szStr, nIndex, 1))
        End If
            
        'arrange to leave 4 spare rows
        If oGrid.Row + 5 > oGrid.Rows Then
            oGrid.Rows = oGrid.Rows + 5
        End If
        
        'go to next row
        oGrid.Row = oGrid.Row + 1
        
    Next nIndex

    oGrid.CellBackColor = 0

End Sub

'A channel number needs to be extracted and put into the grid
'We are writing the grid from the stored string
'The string we are passed should be of the form "E(99)etc"
'extract the channel number and put it in col 2
'Return the number of bytes to advance in order to point
'at the final )

Public Function GridWriteChannelNum(oGrid As MSFlexGrid, szStr As String) As Integer

    Dim nIndex As Integer
    Dim nStart As Integer
    Dim cTemp As String
                                
    oGrid.TextMatrix(oGrid.Row, 0) = CStr(oGrid.Row)
    'Number starts 2 bytes into string
    oGrid.TextMatrix(oGrid.Row, 2) = Val(Right$(szStr, Len(szStr) - 2))
   
    'Find the final ) and return the number of to advance
    'in order to point to it.
    'We know the first byte is not it
    For nIndex = 2 To Len(szStr)
       If Mid$(szStr, nIndex, 1) = ")" Then
            Exit For
        End If
    Next nIndex
        
    GridWriteChannelNum = nIndex

End Function


'Converts the grid contents into a single string for storage
'in the message array and in the data files
'The grid should be checked for errors first

Public Sub GridGetString(oGrid As MSFlexGrid, szStr As String)

    Dim nIndex As Integer
    Dim cTemp As String
    Dim cReply As String
    
    szStr = ""
    For nIndex = 1 To oGrid.Rows - 1
        
        'read the chars in Col 1
        cTemp = oGrid.TextMatrix(nIndex, 1)
        Select Case Len(cTemp)
            Case 0
                Exit For
            Case 1
                szStr = szStr & cTemp
                If cTemp = "\" Then
                    szStr = szStr + "\"
                End If
                
                'Leading spaces are a problem because GetPrivateProfileString ignores them !
                'Express them as a non print instead.
                'Convert them back when reloading the file
                
                If (cTemp = " ") And (nIndex = 1) Then
                    szStr = szStr + "\C(032)"
                End If
                    
            Case 3  'This is a non print
                szStr = szStr & "\C(" & cTemp + ")"
            Case 4  ' must be Wait
                Select Case cTemp
                Case "Wait"
                    szStr = szStr & "\W"
                Case "CRC "
                    szStr = szStr & "\Z"
                End Select
            Case 5  'This is a channel
                Select Case cTemp
                    Case cValue
                        szStr = szStr + "\E("
                    Case cILSMS
                        szStr = szStr + "\J("
                    Case cIMSLS
                        szStr = szStr + "\K("
                    Case cWLSMS
                        szStr = szStr + "\L("
                    Case cWMSLS
                        szStr = szStr + "\M("
                    Case cHex
                        szStr = szStr + "\H("
                    Case cSBYTE
                        szStr = szStr + "\N("
                    Case cUBYTE
                        szStr = szStr + "\O("
                    Case cDWMSLS
                        szStr = szStr + "\G(0)("
                    Case cDWBS
                        szStr = szStr + "\G(1)("
                    Case cDWWS
                        szStr = szStr + "\G(2)("
                    Case cDWLSMS
                        szStr = szStr + "\G(3)("
                    Case cLMSLS
                        szStr = szStr + "\G(8)("
                    Case cLBS
                        szStr = szStr + "\G(9)("
                    Case cLWS
                        szStr = szStr + "\G(10)("
                    Case cLLSMS
                        szStr = szStr + "\G(11)("
                   Case cFLOATMSLS
                        szStr = szStr + "\G(16)("
                    Case cFLOATBS
                        szStr = szStr + "\G(17)("
                    Case cFLOATWS
                        szStr = szStr + "\G(18)("
                    Case cFLOATLSMS
                        szStr = szStr + "\G(19)("
                End Select
               
               'See if its custom
               cReply = CustomGetString(cTemp, oGrid.TextMatrix(nIndex, 2))
               szStr = szStr + cReply
             
               'If its not custom
               If cReply = "" Then
                                               
                     'Bit is special case reads "Bit 7"
                     'Convert to \P(99)7
                     If Left$(cTemp, 4) = cBIT Then
                         szStr = szStr + "\P("
                     End If
                     
                     'This can be null for custom entries
                     If oGrid.TextMatrix(nIndex, 2) <> "" Then
                         'Channel number in brackets
                         szStr = szStr + oGrid.TextMatrix(nIndex, 2)
                         szStr = szStr + ")"
                     End If
                     
                     'Get the bit number
                     If Left$(cTemp, 4) = cBIT Then
                         szStr = szStr + Right$(cTemp, 1)
                     End If
                     
                End If
        
        End Select
        
     Next nIndex
     
End Sub

'Put an inserted write channel marker into the grid
'nValue specifies the channel type
'The appropriate string is put into the grid
'Gather further data if we are dealing with bit values or custom types

Public Sub GridWriteValue(oGrid As MSFlexGrid, cType As String)

    Dim vBit As Variant
    Dim cReply As String

    'If this already has an allocated channel free it
    If Len(oGrid.TextMatrix(oGrid.Row, 1)) = 5 Then
        Call ChannelDiag.ChannelFree(Val(oGrid.TextMatrix(oGrid.Row, 2)))
    End If
    
    'Clear any remaining channel number
    oGrid.TextMatrix(oGrid.Row, 2) = ""
    
    'Select the channel edit mode
    ChannelDiag.nEditMode = MODE_WRITE_VALUE_CHANNEL
    
     
    Select Case cType

        Case cBIT
            'Insert Bit need to know which bit
            vBit = 8
            While vBit < 0 Or vBit > 7
                vBit = InputBox("Enter bit number 0 - 7")
            Wend
            oGrid.TextMatrix(oGrid.Row, 1) = cBIT + CStr(vBit)
            'Overwrite the edit mode
            ChannelDiag.nEditMode = MODE_WRITE_BIT_CHANNEL

        Case cCustom
            cReply = CustomGridEntry("Custom Output")
            
            If cReply = "" Then Exit Sub
            oGrid.TextMatrix(oGrid.Row, 1) = cReply
            'If there is no associated channel
            If (Val(Mid$(cReply, 4, 2)) < CUSTOM_SPLIT) Then
                oGrid.TextMatrix(oGrid.Row, 0) = CStr(oGrid.Row)
                oGrid.Row = oGrid.Row + 1
                Exit Sub
            End If
            
        Case Else
        
        oGrid.TextMatrix(oGrid.Row, 1) = cType
  
    End Select
    
    'Bring up channel diag to select the channel
    
    ChannelDiag.Visible = True
    
    
    'The channeldiag writes in the channel number

End Sub

'This clears a message grid. It first deletes any inserted values.
'Then Initialises again to start with a clean sheet

Public Sub GridClear(oGrid As MSFlexGrid)

    Dim nEnd As Integer
    Dim nRow As Integer
    
    nEnd = FindEndOfData(oGrid)
    
    'Look for inserted values
    For nRow = 1 To nEnd - 1
        'If we are deleting an inserted value we must free it
        If Len(oGrid.TextMatrix(nRow, 1)) = 5 Then
            Call ChannelDiag.ChannelFree(Val(oGrid.TextMatrix(nRow, 2)))
        End If
    Next nRow

    Call GridInitialise(oGrid)
    
    oGrid.SetFocus

End Sub


'Enter a number 0-255
'Check for correctness
'Then return as a 3 byyte string


Public Function DecimalNonPrint_Enter() As String
    
    Dim cCode As String
    Dim cTitle As String
    Dim nIndex As Integer
    
    cTitle = "Decimal Non Printing Characters"
    cCode = InputBox(" Enter Code 0 to 255", "Decimal NonPrint")
    
    If cCode = "" Then Exit Function
    If Len(cCode) > 3 Then Exit Function
    For nIndex = 1 To Len(cCode)
        If Mid$(cCode, nIndex, 1) < "0" Then Exit Function
        If Mid$(cCode, nIndex, 1) > "9" Then Exit Function
    Next nIndex
    If Val(cCode) > 255 Then Exit Function
    
    
    'Expand to be 3 bytes long
    Select Case Len(cCode)
    Case 0
        cCode = ""
    Case 1
        cCode = "00" + cCode
    Case 2
        cCode = "0" + cCode
    Case 3
    End Select
       
    
    DecimalNonPrint_Enter = cCode

End Function
