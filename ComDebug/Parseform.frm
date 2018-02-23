VERSION 5.00
Object = "{5E9E78A0-531B-11CF-91F6-C2863C385E30}#1.0#0"; "msflxgrd.ocx"
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form Parseform 
   Caption         =   "Windmill Reply Parser"
   ClientHeight    =   1920
   ClientLeft      =   165
   ClientTop       =   855
   ClientWidth     =   6945
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   1920
   ScaleWidth      =   6945
   StartUpPosition =   3  'Windows Default
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   6480
      Top             =   1440
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin MSFlexGridLib.MSFlexGrid ParseGrid 
      Height          =   1575
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   2775
      _ExtentX        =   4895
      _ExtentY        =   2778
      _Version        =   393216
      BackColorSel    =   -2147483643
      AllowBigSelection=   0   'False
      ScrollBars      =   2
   End
   Begin VB.Line Line1 
      X1              =   120
      X2              =   7560
      Y1              =   0
      Y2              =   0
   End
   Begin VB.Menu EditSteps 
      Caption         =   "&Edit Steps"
      Begin VB.Menu DeleteStep 
         Caption         =   "Delete Step"
      End
      Begin VB.Menu InsertStep 
         Caption         =   "Insert Step"
      End
   End
   Begin VB.Menu NonPrintMenu 
      Caption         =   "&NonPrint"
      Begin VB.Menu NonPrint 
         Caption         =   "NonPrint Decimal"
         Index           =   0
      End
      Begin VB.Menu NonPrint 
         Caption         =   "-"
         Index           =   1
      End
      Begin VB.Menu NonPrint 
         Caption         =   "Carriage Return"
         Index           =   2
      End
      Begin VB.Menu NonPrint 
         Caption         =   "Line Feed"
         Index           =   3
      End
      Begin VB.Menu NonPrint 
         Caption         =   "Escape"
         Index           =   4
      End
   End
   Begin VB.Menu AddActionMenu 
      Caption         =   "&Add Action"
      Begin VB.Menu AddAction 
         Caption         =   "Search"
         Index           =   0
      End
      Begin VB.Menu AddAction 
         Caption         =   "Ignore N Bytes"
         Index           =   1
      End
      Begin VB.Menu AddAction 
         Caption         =   "Ignore Until"
         Index           =   2
      End
      Begin VB.Menu AddAction 
         Caption         =   "Custom Action"
         Index           =   3
      End
   End
   Begin VB.Menu ExtractMenu 
      Caption         =   "E&xtract"
      Begin VB.Menu ExtractN 
         Caption         =   "Extract N Bytes"
      End
      Begin VB.Menu ExtractUntil 
         Caption         =   "Extract Until"
      End
      Begin VB.Menu ExtractIntegerMenu 
         Caption         =   "Extract Integer"
         Begin VB.Menu ExtractIntegerMSB 
            Caption         =   "MSB-LSB"
         End
         Begin VB.Menu ExtractIntegerLSB 
            Caption         =   "LSB-MSB"
         End
      End
      Begin VB.Menu ExtractWordMenu 
         Caption         =   "Extract Word"
         Begin VB.Menu ExtractWMSBLSB 
            Caption         =   "MSB-LSB"
         End
         Begin VB.Menu ExtractWLSBMSB 
            Caption         =   "LSB-MSB"
         End
      End
      Begin VB.Menu ExtractLongMenu 
         Caption         =   "Extract Long"
         Begin VB.Menu ExtractLong 
            Caption         =   "MSB-LSB        ABCD"
            Index           =   0
         End
         Begin VB.Menu ExtractLong 
            Caption         =   "Byte Swap     BADC"
            Index           =   1
         End
         Begin VB.Menu ExtractLong 
            Caption         =   "Word Swap    CDAB"
            Index           =   2
         End
         Begin VB.Menu ExtractLong 
            Caption         =   "LSB-MSB         DCBA"
            Index           =   3
         End
      End
      Begin VB.Menu ExtractDwordMenu 
         Caption         =   "Extract Dword"
         Begin VB.Menu ExtractDword 
            Caption         =   "MSB-LSB         ABCD"
            Index           =   0
         End
         Begin VB.Menu ExtractDword 
            Caption         =   "Byte Swap      BADC"
            Index           =   1
         End
         Begin VB.Menu ExtractDword 
            Caption         =   "Word Swap    CDAB"
            Index           =   2
            WindowList      =   -1  'True
         End
         Begin VB.Menu ExtractDword 
            Caption         =   "LSB-MSB         DCBA"
            Index           =   3
         End
      End
      Begin VB.Menu ExtractMenuFloat 
         Caption         =   "Extract Float"
         Begin VB.Menu ExtractFloat 
            Caption         =   "MSB-LSB        ABCD"
            Index           =   0
         End
         Begin VB.Menu ExtractFloat 
            Caption         =   "Byte Swap     BADC"
            Index           =   1
         End
         Begin VB.Menu ExtractFloat 
            Caption         =   "Word Swap   CDAB"
            Index           =   2
         End
         Begin VB.Menu ExtractFloat 
            Caption         =   "LSB-MSB        DCBA"
            Index           =   3
         End
      End
      Begin VB.Menu ExtractHex 
         Caption         =   "Extract N Bytes as Hex"
      End
      Begin VB.Menu ExtractSignByte 
         Caption         =   "Extract Signed Byte"
      End
      Begin VB.Menu ExtractByte 
         Caption         =   "Extract Byte"
      End
      Begin VB.Menu ExtractBit 
         Caption         =   "Extract Bit"
      End
      Begin VB.Menu ExtractRecNo 
         Caption         =   "Extract Record Number"
      End
      Begin VB.Menu ExtractCustom 
         Caption         =   "Extract Custom"
      End
   End
   Begin VB.Menu ParseStep 
      Caption         =   "&Step"
   End
   Begin VB.Menu HelpMenu 
      Caption         =   "&Help"
   End
End
Attribute VB_Name = "Parseform"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
 Option Explicit
 Dim nPreviousRow As Integer
 Dim nPreviousCol As Integer
 Dim dwParseSequence As Long
 Dim cParseRowError As String
 Dim bReturn As Boolean
 
 
 'Strings which appear in Column 1 of the parse grid
 Const cSearch As String = "Search"
 Const cIgnoreN As String = "Ignore N Bytes"
 Const cIgnoreU As String = "Ignore Until"
 Const cExtractN As String = "Extract N Bytes"
 Const cExtractU As String = "Extract Until"
 Const cExtractILSBMSB As String = "Integer LSB-MSB"
 Const cExtractIMSBLSB As String = "Integer MSB-LSB"
 Const cExtractWLSBMSB As String = "Word LSB-MSB"
 Const cExtractWMSBLSB As String = "Word MSB-LSB"
 Const cExtractLMSBLSB As String = "Long MSB-LSB"
 Const cExtractLByteSwap As String = "Long Byte Swap"
 Const cExtractLWordSwap As String = "Long Word Swap"
 Const cExtractLLSBMSB As String = "Long LSB-MSB"
 Const cExtractDWLSBMSB As String = "DWord LSB-MSB"
 Const cExtractDWByteSwap As String = "DWord Byte Swap"
 Const cExtractDWWordSwap As String = "DWord Word Swap"
 Const cExtractDWMSBLSB As String = "DWord MSB-LSB"
 Const cExtractHex As String = "N Bytes as Hex"
 Const cExtractSignByte As String = "Signed Byte"
 Const cExtractByte As String = "Extract Byte"
 Const cExtractBit As String = "Extract Bit"
 Const cExtractFloatMSBLSB As String = "Float MSB-LSB"
 Const cExtractFloatByteSwap As String = "Float Byte Swap"
 Const cExtractFloatWordSwap As String = "Float Word Swap"
 Const cExtractFloatLSBMSB As String = "Float LSB-MSB"
 Const cExtractRecNo As String = "Record Number"




Private Sub AddAction_Click(Index As Integer)

    Dim cText As String

    Select Case Index
        Case 0
            Call ParseAddActionString(cSearch, False, False)
        Case 1
            Call ParseAddActionString(cIgnoreN, False, False)
        Case 2
            Call ParseAddActionString(cIgnoreU, False, False)
        Case 3
            cText = CustomGridEntry("Custom Action")
            If cText = "" Then Exit Sub
            Call ParseAddActionString(cText, False, True)
    End Select
End Sub

'Launch the channeldiag form in Display only mode

Private Sub DeleteStep_Click()
    
    Dim nRow As Integer
    Dim nEnd As Integer
    Dim nIndex As Integer
    
    
    'The current row
    nRow = ParseGrid.Row

    'Delete the channel allocation if there is one
    If ParseGrid.TextMatrix(nRow, 3) <> "" Then
        ChannelDiag.ChannelFree (Val(ParseGrid.TextMatrix(nRow, 3)))
    End If
    
    'Delete the row
    Call GridDeleteRow(ParseGrid)

    
End Sub


Private Sub HelpMenu_Click()
    
    Call DisplayHelp(Parseform.hWnd, 6)

End Sub

Private Sub InsertStep_Click()
   Call GridInsertRow(ParseGrid)
End Sub



Private Sub MenuDelete_Click()
    If ParseGrid.Col = 2 Then
        ParseGrid.Text = ParseGrid.Text + "\C(255)"
    End If
End Sub


'Get the entry of a non print code expressed as 0 to 255

Private Sub NonPrintDecimal_Entry()
    
    Dim cMessage As Variant
    Dim cPrompt As String
    Dim cTitle As String
    
    cPrompt = "Enter a Non Printing character as a decimal code" + vbCr + "0 to 255"
    cTitle = "Decimal Non Printing Characters"

    cMessage = DecimalNonPrint_Enter()
    
    If cMessage < 0 Or cMessage > 255 Then
        Beep
        Exit Sub
    End If
    
    If ParseGrid.Col <> 2 Then
        Beep
        Exit Sub
    End If
    
    'Only certain action can take nonprints
    Select Case ParseGrid.TextMatrix(ParseGrid.Row, 1)
        Case cSearch, cIgnoreU, cExtractU
        
        Case Else
            Beep
            Exit Sub
    End Select

    cPrompt = CStr(cMessage)
    Select Case Len(cPrompt)
        Case 0
            Exit Sub
        Case 1
            cPrompt = "\C(00" + cPrompt + ")"
        Case 2
            cPrompt = "\C(0" + cPrompt + ")"
        Case 3
            cPrompt = "\C(" + cPrompt + ")"
        Case Else
            Beep
            Exit Sub
    End Select
    
    ParseGrid.Text = ParseGrid.Text + cPrompt
    
End Sub

Private Sub NonPrint_Click(Index As Integer)
    
    Dim cMessage As Variant
    Dim cPrompt As String
    Dim cTitle As String
    
       
    If ParseGrid.Col = 2 Then
        Select Case Index
            Case 0
                NonPrintDecimal_Entry
            Case 2
                ParseGrid.Text = ParseGrid.Text + "\C(013)"
            Case 3
                ParseGrid.Text = ParseGrid.Text + "\C(010)"
            Case 4
                ParseGrid.Text = ParseGrid.Text + "\C(027)"
        End Select
        
    End If
End Sub



Private Sub ExtractLong_Click(Index As Integer)

    Select Case Index
    Case 0
        Call ParseAddActionString(cExtractLMSBLSB, True, True)
    Case 1
        Call ParseAddActionString(cExtractLByteSwap, True, True)
    Case 2
        Call ParseAddActionString(cExtractLWordSwap, True, True)
    Case 3
        Call ParseAddActionString(cExtractLLSBMSB, True, True)
    End Select
    
End Sub

Private Sub ExtractDword_Click(Index As Integer)

    Select Case Index
    Case 0
        Call ParseAddActionString(cExtractDWMSBLSB, True, True)
    Case 1
        Call ParseAddActionString(cExtractDWByteSwap, True, True)
    Case 2
        Call ParseAddActionString(cExtractDWWordSwap, True, True)
    Case 3
        Call ParseAddActionString(cExtractDWLSBMSB, True, True)
    End Select
    
End Sub

Private Sub ExtractFloat_Click(Index As Integer)

    Select Case Index
    Case 0
        Call ParseAddActionString(cExtractFloatMSBLSB, True, True)
    Case 1
        Call ParseAddActionString(cExtractFloatByteSwap, True, True)
    Case 2
        Call ParseAddActionString(cExtractFloatWordSwap, True, True)
    Case 3
        Call ParseAddActionString(cExtractFloatLSBMSB, True, True)
    End Select
    
End Sub

Private Sub ExtractIntegerLSB_Click()
    Call ParseAddActionString(cExtractILSBMSB, True, True)
End Sub
Private Sub ExtractIntegerMSB_Click()
    Call ParseAddActionString(cExtractIMSBLSB, True, True)
End Sub

Private Sub ExtractN_Click()
    Call ParseAddActionString(cExtractN, True, False)
End Sub
Private Sub ExtractUntil_Click()
    Call ParseAddActionString(cExtractU, True, False)
End Sub
Private Sub ExtractWLSBMSB_Click()
    Call ParseAddActionString(cExtractWLSBMSB, True, True)
End Sub
Private Sub ExtractWMSBLSB_Click()
    Call ParseAddActionString(cExtractWMSBLSB, True, True)
End Sub

Private Sub ExtractHex_Click()
    Call ParseAddActionString(cExtractHex, True, False)
End Sub
Private Sub ExtractSignByte_Click()
    Call ParseAddActionString(cExtractSignByte, True, True)
End Sub
Private Sub ExtractByte_Click()
    Call ParseAddActionString(cExtractByte, True, True)
End Sub
Private Sub ExtractBit_Click()
    Call ParseAddActionString(cExtractBit, True, False)
End Sub

Private Sub ExtractRecNo_Click()
    Call ParseAddActionString(cExtractRecNo, True, True)
End Sub
Private Sub ExtractCustom_Click()
    Dim cText As String
    cText = CustomGridEntry("Custom Extraction")
    If cText = "" Then Exit Sub
    Call ParseAddActionString(cText, True, True)
End Sub
Private Sub Form_Load()

    Dim nIndex As Integer
    
    
    Parseform.Width = Mainform.Width
    
    ParseGrid.Rows = 20
    ParseGrid.Cols = 6
    ParseGrid.ColWidth(0) = 600
    ParseGrid.ColWidth(1) = 1500
    ParseGrid.ColWidth(2) = 2000
    ParseGrid.ColWidth(3) = 500
    ParseGrid.ColWidth(4) = 1400
    ParseGrid.ColWidth(5) = 1400
    ParseGrid.Width = 7700
       
    ParseGrid.ColAlignment(0) = flexAlignCenterCenter
    ParseGrid.ColAlignment(1) = flexAlignCenterCenter
    ParseGrid.ColAlignment(2) = flexAlignLeftTop
    ParseGrid.ColAlignment(3) = flexAlignCenterCenter
    ParseGrid.ColAlignment(4) = flexAlignCenterCenter
    ParseGrid.ColAlignment(5) = flexAlignCenterCenter

       
    ParseGrid.TextMatrix(0, 0) = "Step"
    ParseGrid.TextMatrix(0, 1) = "Action"
    ParseGrid.TextMatrix(0, 2) = "Parameters"
    ParseGrid.TextMatrix(0, 3) = "Chan"
    ParseGrid.TextMatrix(0, 4) = "Name"
    ParseGrid.TextMatrix(0, 5) = "Value"
         
         
    'Start at row 1 col 1
    ParseGrid.Row = 1
    ParseGrid.Col = 1
    ParseGrid.CellBackColor = vbGreen

    'Clear the text
    For nIndex = 1 To 19
        ParseGrid.TextMatrix(nIndex, 0) = ""
        ParseGrid.TextMatrix(nIndex, 1) = ""
        ParseGrid.TextMatrix(nIndex, 2) = ""
    Next nIndex

    nPreviousRow = 0
    nPreviousCol = 0
    dwParseSequence = 0
    bReturn = False
    
    Parseform.Left = Mainform.Left
    Parseform.Top = Mainform.Top + Mainform.Height
    
    'Load the grid
    ParseGrid_LoadString
    
End Sub

Private Sub Form_Unload(Cancel As Integer)
   
   Dim szStr As String
   
   'Save the string into the mainmenu local copy
   Call ParseGrid_GetString(szStr)
   Mainmenu.cParse = szStr
      
   Call SerialGridClearColor(Mainform.GridRec)

End Sub



Private Sub ParseStep_Click()

    Dim dwParseStep As Long
    Dim nIndex As Integer
    Dim nEnd As Integer
    Dim nRow As Integer
    Dim szParse As String
    
    'If we reach the end of the parse then
    'Clear all colours and return to the parse start
    If ParseGrid.TextMatrix(dwParseSequence + 1, 1) = "" Then
        
        dwParseSequence = 0
           
        'Clear the parse grid color
        nEnd = FindEndOfData(ParseGrid)
        For nRow = 1 To nEnd
            ParseGrid.Row = nRow
            ParseGrid.CellBackColor = 0
        Next nRow
        
        'Return to the top row and colour it
        ParseGrid.TopRow = 1  'Make it visible
        ParseGrid.Row = 1
        ParseGrid.CellBackColor = vbGreen
        
        'Clear the rec grid colour and go back to top
        Mainform.GridRec.Row = 1
        Mainform.GridRec.TopRow = 1
        Call SerialGridClearColor(Mainform.GridRec)
        Beep
        Exit Sub
    End If
    
    'dwParseStep is the number of bytes of the reply
    'to advance
    dwParseStep = SerialParseSequence(dwParseSequence)
        
    'Increment the parse sequence
    dwParseSequence = dwParseSequence + 1
    'Move to next row of parse grid
   
    ParseGrid.Row = dwParseSequence
    'Colour it green
    ParseGrid.Col = 1
    ParseGrid.CellBackColor = vbGreen
    
    'Scroll the grid to keep the next inst in view
    If dwParseSequence > 4 Then
        ParseGrid.TopRow = dwParseSequence - 3
    End If
   
    Call SerialGridColor(Mainform.GridRec, dwParseStep)
 
    
End Sub


Private Sub ParseGrid_Entercell()

    Dim nEnd As Integer
    Dim szParse As String
    Dim cError As String
    
    
    'Can only enter cols 1 and 2
    If ParseGrid.Col > 2 Then
        bReturn = True
        ParseGrid.Row = nPreviousRow
        ParseGrid.Col = nPreviousCol
        bReturn = False
        Exit Sub
    End If
    
     'If we are juist changing cols
     'Give no error messages
    If ParseGrid.Row = nPreviousRow Then
        ParseGrid.CellBackColor = vbGreen
        Exit Sub
    End If
    
       
    If cParseRowError <> "" Then
        Call MsgBox(cParseRowError, vbOKOnly, "Parse Row Error")
        'Inhibit the grid leave function whilst we change
        bReturn = True
        ParseGrid.Row = nPreviousRow
        ParseGrid.Col = nPreviousCol
        bReturn = False
        cParseRowError = ""
       Exit Sub
    End If
    

      
   'First refresh the parse string in case its been edited
    Call ParseGrid_GetString(szParse)
    Call SerialSetParseString(szParse)
     
    nEnd = FindEndOfData(ParseGrid)
    
    'Can't enter cells beyond the end
    'Send it to the end col 1 since this is a blank line
    If ParseGrid.Row > nEnd Then
        ParseGrid.Row = nEnd
        ParseGrid.Col = 1
    End If
    
    'If this row has an empty action enter that column
    If ParseGrid.TextMatrix(ParseGrid.Row, 1) = "" Then
        ParseGrid.Col = 1
    End If
    
    ParseGrid.CellBackColor = vbGreen
    
End Sub


Private Sub ParseGrid_LeaveCell()

    
    DoEvents
    
    'We are leaving so remove the backcolor
    ParseGrid.CellBackColor = 0
    
    'Go no further if we are returning
    If bReturn Then Exit Sub
    
    'Remember the cell we are leaving
    nPreviousRow = ParseGrid.Row
    nPreviousCol = ParseGrid.Col
       
    'Test for Error
    cParseRowError = ParseGridRowError(ParseGrid.Row)



End Sub

'These are the key presses made in the grid. Check them for
'validity before writing them to the screen. What is
'valid depends on the contents of column 1

Private Sub ParseGrid_Keypress(nKey As Integer)
    Dim nLen As Integer
    Dim nNonPrintChars As Integer
    Dim cTemp As String

        
    'Can only type into col 2
    If ParseGrid.Col <> 2 Then
        Exit Sub
    End If

    'We must have something in the action column in order
    'to enter parameters or channels
    If ParseGrid.TextMatrix(ParseGrid.Row, 1) = "" Then
        Exit Sub
    End If
    
    'Since we are writing something the parse step will
    'become invalid. Clear it
    'Clear the rec grid colour and go back to top
    Mainform.GridRec.Row = 1
    Mainform.GridRec.TopRow = 1
    Call SerialGridClearColor(Mainform.GridRec)
    dwParseSequence = 0
     
    nLen = Len(ParseGrid.Text)
    'This is backspace. Remove the last char
    If nKey = 8 Then
        If nLen = 0 Then
            Exit Sub
        End If
        If nLen > 1 Then
            'could be \\
            If Right$(ParseGrid.Text, 2) = "\\" Then
                ParseGrid.Text = Left$(ParseGrid.Text, nLen - 2)
                Exit Sub
            End If
        End If
        If nLen > 6 Then
            'could be non print \C(xxx)
            If Mid$(ParseGrid.Text, nLen - 6, 2) = "\C" Then
                ParseGrid.Text = Left$(ParseGrid.Text, nLen - 7)
                Exit Sub
            End If
        End If
        
        ParseGrid.Text = Left$(ParseGrid.Text, Len(ParseGrid.Text) - 1)
        Exit Sub
    End If
    
    Select Case ParseGrid.TextMatrix(ParseGrid.Row, 1)
        'All printing chars OK
        Case cSearch, cIgnoreU, cExtractU
            
            If nKey < 32 Or nKey > 127 Then
                Exit Sub
            End If
           
            If nKey = 32 Then
                ParseGrid.Text = ParseGrid.Text + "\C(032)"
                Exit Sub
            End If
            
        'Numbers only OK
        Case cIgnoreN
            If Chr$(nKey) < "0" Or Chr$(nKey) > "9" Then
                Exit Sub
            End If
        Case cExtractN
            If Chr$(nKey) < "0" Or Chr$(nKey) > "9" Then
                Exit Sub
            End If
            cTemp = ParseGrid.Text + Chr$(nKey)
            If Val(cTemp) > 12 Then
                cTemp = "Windmill values are limited to maximum 12 characters"
                Call MsgBox(cTemp, vbOKOnly, "Maximum Value Size")
                Exit Sub
            End If
        Case cExtractBit
            'Only 1 char allowed must be 0-7
            If Len(ParseGrid.TextMatrix(ParseGrid.Row, 2)) <> 0 Then
                Exit Sub
            End If
            If Chr$(nKey) < "0" Or Chr$(nKey) > "7" Then
                Exit Sub
            End If
        Case cExtractHex
            If Chr$(nKey) < "0" Or Chr$(nKey) > "9" Then
                Exit Sub
            End If
            cTemp = ParseGrid.Text + Chr$(nKey)
            If Val(cTemp) > 6 Then
                cTemp = "Windmill values are limited to maximum 12 characters" + vbCr _
                + "12 Hex chars are produced from 6 bytes"
                
                Call MsgBox(cTemp, vbOKOnly, "Maximum Value Size")
                Exit Sub
            End If
            
        
        Case Else
            Exit Sub
    End Select
    
    'Add char to the line
    If Chr$(nKey) = "\" Then
        ParseGrid.Text = ParseGrid.Text + "\\"
    Else
        ParseGrid.Text = ParseGrid.Text + Chr$(nKey)
    End If

End Sub



'Read the commands from the grid and convert them to a string

Public Sub ParseGrid_GetString(szParse As String)

    Dim nIndex As Integer
    Dim nEnd As Integer
    Dim szInst As String
    Dim szParam As String
    Dim szChan As String
    
    nEnd = FindEndOfData(ParseGrid)
        
    szParse = ""
    'For each row of the grid
    For nIndex = 1 To nEnd
    
        szParam = ParseGrid.TextMatrix(nIndex, 2)
        szInst = ""
        szChan = "(" + ParseGrid.TextMatrix(nIndex, 3) + ")"
        'Interpret the command column
        Select Case ParseGrid.TextMatrix(nIndex, 1)
        
            Case cSearch
                szInst = "\S" + Chr$(34) + szParam + Chr$(34)
            Case cIgnoreN
                szInst = "\I" + szParam
            Case cIgnoreU
                szInst = "\I" + Chr$(34) + szParam + Chr$(34)
            Case cExtractN
                szInst = "\E" + szChan + szParam
            Case cExtractU
                szInst = "\E" + szChan + Chr$(34) + szParam + Chr$(34)
            Case cExtractILSBMSB
                szInst = "\J" + szChan
            Case cExtractIMSBLSB
                szInst = "\K" + szChan
            Case cExtractWLSBMSB
                szInst = "\L" + szChan
            Case cExtractWMSBLSB
                szInst = "\M" + szChan
            Case cExtractSignByte
                szInst = "\N" + szChan
            Case cExtractHex
                szInst = "\H" + szChan + szParam
            Case cExtractByte
                szInst = "\O" + szChan
            Case cExtractBit
                szInst = "\P" + szChan + szParam
            Case cExtractDWMSBLSB
                szInst = "\G(0)" + szChan
            Case cExtractDWByteSwap
                szInst = "\G(1)" + szChan
            Case cExtractDWWordSwap
                szInst = "\G(2)" + szChan
            Case cExtractDWLSBMSB
                szInst = "\G(3)" + szChan
            Case cExtractLMSBLSB
                szInst = "\G(8)" + szChan
            Case cExtractLByteSwap
                szInst = "\G(9)" + szChan
            Case cExtractLWordSwap
                szInst = "\G(10)" + szChan
            Case cExtractLLSBMSB
                szInst = "\G(11)" + szChan
            Case cExtractFloatMSBLSB
                szInst = "\G(16)" + szChan
            Case cExtractFloatByteSwap
                szInst = "\G(17)" + szChan
            Case cExtractFloatWordSwap
                szInst = "\G(18)" + szChan
            Case cExtractFloatLSBMSB
                szInst = "\G(19)" + szChan
            Case cExtractRecNo
                szInst = "\T" + szChan
        
        End Select
        
        If Left$(ParseGrid.TextMatrix(nIndex, 1), 3) = cCustom Then
            szInst = CustomGetString(ParseGrid.TextMatrix(nIndex, 1), ParseGrid.TextMatrix(nIndex, 3))
        End If
    
        szParse = szParse + szInst
    
    Next nIndex


End Sub

'Takes the parsestring and loads it into the grid
'This routine splits the string into rows and then
'calls ParseGrid_WriteRow

Private Sub ParseGrid_LoadString()

    Dim nRow As Integer
    Dim nIndex As Integer
    Dim nStart As Integer
    Dim cInst As String
    Dim bLiteral As Boolean
    
    'Start the parsing action
    Call SerialSetParseString(Mainmenu.cParse)
    
    If Left$(Mainmenu.cParse, 1) <> "\" Then
        Exit Sub
    End If
    
    nRow = 1
    nIndex = 2 'pointer into the string
    nStart = 2 'The start of the row string
    bLiteral = False
    'for each char of the string
    'Start at second char. First must be a \
    While nIndex < Len(Mainmenu.cParse) + 1
        'Look for the \ for instruction start
        Select Case Mid$(Mainmenu.cParse, nIndex, 1)
            Case "\"
                'If this is outside literal it is an
                'instruction start
                If bLiteral = False Then
                    'Call writerow with the row string we have found
                    Call ParseGrid_WriteRow(nRow, Mid$(Mainmenu.cParse, nStart, nIndex - nStart))
                    nStart = nIndex + 1
                    nRow = nRow + 1
                End If
            Case Chr$(34)  'Toggle literal flag
                ' \ can occur inside literals. So keep this flag
                If bLiteral Then
                    bLiteral = False
                Else
                    bLiteral = True
                End If
        End Select
        nIndex = nIndex + 1
    Wend
    
    'The final instruction ends at the end of the line
    Call ParseGrid_WriteRow(nRow, Mid$(Mainmenu.cParse, nStart, Len(Mainmenu.cParse) - nStart + 1))

End Sub

'This is passed a string containing the data to be
'written to the specified row.
'It decodes the string into Action, Parameter, Channel
'and writes them to the grid.

Private Sub ParseGrid_WriteRow(nRow As Integer, cInst As String)

    Dim nStep As Integer
    Dim cCode As String
    Dim cChannel As String

    'This gives us always at least 4 spare rows
    'Rows run from 0 so .Rows = max row + 1
    If nRow + 5 > ParseGrid.Rows Then
       ParseGrid.Rows = nRow + 5
    End If
    
    'The instruction number into channel 0
    ParseGrid.TextMatrix(nRow, 0) = CStr(nRow)
    
    Select Case Left$(cInst, 1)
    
            Case "A" 'This is custom
                Call CustomLoadString(cInst, cCode, cChannel)
                ParseGrid.TextMatrix(nRow, 1) = cCode
                ParseGrid.TextMatrix(nRow, 3) = cChannel
                If cChannel <> "" Then
                    ParseGrid.TextMatrix(nRow, 4) = GetChannelName(Val(cChannel))
                End If
            Case "S" 'This is search
                ParseGrid.TextMatrix(nRow, 1) = cSearch
                'String must be S"abc"
                ParseGrid.TextMatrix(nRow, 2) = Mid$(cInst, 3, Len(cInst) - 3)
            Case "G"
                Call GLoadString(cInst, nRow)
            Case "J" 'Must be J(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractILSBMSB
                Call ShowChannelNumber(nRow, cInst)
            Case "K" 'Must be K(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractIMSBLSB
                Call ShowChannelNumber(nRow, cInst)
            Case "L" 'Must be L(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractWLSBMSB
                Call ShowChannelNumber(nRow, cInst)
            Case "M" 'Must be M(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractWMSBLSB
                Call ShowChannelNumber(nRow, cInst)
            Case "N" 'Must be N(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractSignByte
                Call ShowChannelNumber(nRow, cInst)
            Case "O" 'Must be O(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractByte
                Call ShowChannelNumber(nRow, cInst)
            Case "P" 'Must be P(0)7
                ParseGrid.TextMatrix(nRow, 1) = cExtractBit
                Call ShowChannelNumber(nRow, cInst)
                If Right$(cInst, 1) <> ")" Then  'Check we have a bit num
                    ParseGrid.TextMatrix(nRow, 2) = Right$(cInst, 1)
                End If
            Case "Q" 'Must be Q(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractFloatLSBMSB
                Call ShowChannelNumber(nRow, cInst)
            Case "R" 'Must be R(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractFloatMSBLSB
                Call ShowChannelNumber(nRow, cInst)
            Case "T" 'Must be T(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractRecNo
                Call ShowChannelNumber(nRow, cInst)
            Case "I" 'I"abc" or I123
                'Is the second char "
                If Mid$(cInst, 2, 1) = Chr$(34) Then
                    'String must be I"abc"
                    ParseGrid.TextMatrix(nRow, 1) = cIgnoreU
                    ParseGrid.TextMatrix(nRow, 2) = Mid$(cInst, 3, Len(cInst) - 3)
                Else
                    'String must be I123
                    ParseGrid.TextMatrix(nRow, 1) = cIgnoreN
                    ParseGrid.TextMatrix(nRow, 2) = Right$(cInst, Len(cInst) - 1)
                End If
            Case "E" 'E(0)"abc" or E(0)123
                'Get the channel number
                nStep = ShowChannelNumber(nRow, cInst)
                If Mid$(cInst, nStep, 1) = Chr$(34) Then
                    'String must be E(0)"abc". Get the rest of the string minus the final "
                    nStep = nStep + 1 'Point beyond the "
                    ParseGrid.TextMatrix(nRow, 1) = cExtractU
                    ParseGrid.TextMatrix(nRow, 2) = Mid$(cInst, nStep, Len(cInst) - nStep)
                Else
                    'String must be E(0)123. Get the rest of the string
                    ParseGrid.TextMatrix(nRow, 1) = cExtractN
                    ParseGrid.TextMatrix(nRow, 2) = Right$(cInst, Len(cInst) - nStep + 1)
                End If
            Case "H"
                nStep = ShowChannelNumber(nRow, cInst)
                ParseGrid.TextMatrix(nRow, 1) = cExtractHex
                'Number of bytes to Extract
                ParseGrid.TextMatrix(nRow, 2) = Right$(cInst, Len(cInst) - nStep + 1)
            Case "U" 'Must be U(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractLLSBMSB
                Call ShowChannelNumber(nRow, cInst)
            Case "V" 'Must be V(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractLMSBLSB
                Call ShowChannelNumber(nRow, cInst)
            Case "X" 'Must be X(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractDWLSBMSB
                Call ShowChannelNumber(nRow, cInst)
            Case "Y" 'Must be Y(0)
                ParseGrid.TextMatrix(nRow, 1) = cExtractDWMSBLSB
                Call ShowChannelNumber(nRow, cInst)
            
    End Select

End Sub

'The string in cBuffer starts X(00). Extract the channel number
'Return the number of bytes to move on the pointer to the next valid char

Private Function GLoadString(cBuffer As String, nRow As Integer) As Integer
    
    Dim nIndex As Integer
    Dim nType As Integer
    Dim nStart As Integer
        
    GLoadString = 0
    
    
    If Mid$(cBuffer, 2, 1) <> "(" Then
        Exit Function
    End If
    
    For nIndex = 3 To Len(cBuffer)
        If Mid$(cBuffer, nIndex, 1) = ")" Then
            Exit For
        End If
    Next nIndex
    
    If nIndex = Len(cBuffer) Then
        Exit Function
    End If

    nType = Val(Mid$(cBuffer, 3, nIndex - 3))
    
    Select Case nType
    
    Case 0
        ParseGrid.TextMatrix(nRow, 1) = cExtractDWMSBLSB
    Case 1
        ParseGrid.TextMatrix(nRow, 1) = cExtractDWByteSwap
    Case 2
        ParseGrid.TextMatrix(nRow, 1) = cExtractDWWordSwap
    Case 3
        ParseGrid.TextMatrix(nRow, 1) = cExtractDWLSBMSB
    Case 8
        ParseGrid.TextMatrix(nRow, 1) = cExtractLMSBLSB
    Case 9
        ParseGrid.TextMatrix(nRow, 1) = cExtractLByteSwap
    Case 10
        ParseGrid.TextMatrix(nRow, 1) = cExtractLWordSwap
    Case 11
        ParseGrid.TextMatrix(nRow, 1) = cExtractLLSBMSB
    Case 16
        ParseGrid.TextMatrix(nRow, 1) = cExtractFloatMSBLSB
    Case 17
        ParseGrid.TextMatrix(nRow, 1) = cExtractFloatByteSwap
    Case 18
        ParseGrid.TextMatrix(nRow, 1) = cExtractFloatWordSwap
    Case 19
        ParseGrid.TextMatrix(nRow, 1) = cExtractFloatLSBMSB
    End Select
    
    nStart = nIndex + 1
    
    If Mid$(cBuffer, nStart, 1) <> "(" Then
        Exit Function
    End If
    
    nStart = nStart + 1
    
    For nIndex = nStart To Len(cBuffer)
        If Mid$(cBuffer, nIndex, 1) = ")" Then
            Exit For
        End If
    Next nIndex
    
    ParseGrid.TextMatrix(nRow, 3) = Mid$(cBuffer, nStart, nIndex - nStart)
    ParseGrid.TextMatrix(nRow, 4) = GetChannelName(Val(ParseGrid.TextMatrix(nRow, 3)))
   

End Function

'Get the channel number from the parse string and put it
'into the grid

Private Function ShowChannelNumber(nRow As Integer, cBuffer As String) As Integer

    Dim nIndex As Integer
    
    If Mid$(cBuffer, 2, 1) <> "(" Then
        ShowChannelNumber = 0
        Exit Function
    End If
    
    For nIndex = 3 To Len(cBuffer)
        If Mid$(cBuffer, nIndex, 1) = ")" Then
            Exit For
        End If
    Next nIndex
    
    ParseGrid.TextMatrix(nRow, 3) = Mid$(cBuffer, 3, nIndex - 3)
    ParseGrid.TextMatrix(nRow, 4) = GetChannelName(Val(ParseGrid.TextMatrix(nRow, 3)))
    
    'Jump to next valid char
    ShowChannelNumber = nIndex + 1

End Function

'This function writes the action string cAction into
'the grid Col 1. If bAdvance is set then leave the focus
'in Col 1.This is used for extract instructions that
'need no parameters. Else move to Col 2

Private Sub ParseAddActionString(cAction As String, bExtract As Boolean, bAdvance As Boolean)

    'Since we are adding an action the parse step will
    'become invalid. Clear it
    'Clear the rec grid colour and go back to top
    Mainform.GridRec.Row = 1
    Mainform.GridRec.TopRow = 1
    Call SerialGridClearColor(Mainform.GridRec)
    dwParseSequence = 0
    
    'Make sure that we have at least 4 spare rows
    If ParseGrid.Row + 5 > ParseGrid.Rows Then
        ParseGrid.Rows = ParseGrid.Row + 5
    End If
     
    'Number the row first
    ParseGrid.TextMatrix(ParseGrid.Row, 0) = ParseGrid.Row
    ParseGrid.TextMatrix(ParseGrid.Row, 1) = cAction
    ParseGrid.TextMatrix(ParseGrid.Row, 2) = ""
    ParseGrid.TextMatrix(ParseGrid.Row, 4) = ""
    If bAdvance = False Then
        'Move to col  2
        ParseGrid.Col = 2
    End If
    
    If ParseGrid.TextMatrix(ParseGrid.Row, 3) <> "" Then
        Call ChannelDiag.ChannelFree(Val(ParseGrid.TextMatrix(ParseGrid.Row, 3)))
        ParseGrid.TextMatrix(ParseGrid.Row, 3) = ""
    End If
        
    If bExtract Then
        'Select the channel
        Select Case cAction
        Case cExtractBit
            ChannelDiag.nEditMode = MODE_READ_BIT_CHANNEL  'Bit extraction mode
        Case cExtractRecNo
            ChannelDiag.nEditMode = MODE_READ_RECNUM_CHANNEL  'Record number
        Case Else
            ChannelDiag.nEditMode = MODE_READ_VALUE_CHANNEL  'Analogue Read
        End Select
        ChannelDiag.Visible = True
    End If
    
    
 
End Sub

'Called from channeldiag to chalk up a selection into the matrix
'If no selection was made then nChannel is -1. Delete the row number

Public Sub ChannelSelected(nChannel As Integer)

    If nChannel = -1 Then
         ParseGrid.TextMatrix(ParseGrid.Row, 1) = ""
    Else
        ParseGrid.TextMatrix(ParseGrid.Row, 3) = CStr(nChannel)
        ParseGrid.TextMatrix(ParseGrid.Row, 4) = GetChannelName(nChannel)
    End If
    
    'If we are in Col 1 then move to next row. Else we are in Col 2
    'and must enter parameters first.
    If ParseGrid.Col = 1 Then
        ParseGrid.Row = ParseGrid.Row + 1
    End If
    
End Sub

'Given the channel number return its name
Public Function GetChannelName(nChannel As Integer) As String

    ChannelDiag.LoadChannel (nChannel)
    GetChannelName = ChannelDiag.cName

End Function


'This function looks for errors in the current row.
'It returns true if there is an error
'Any text entered in the grid is bound to be correct
'Since the keypress routine checks it

Private Function ParseGridRowError(nRow As Integer) As String

    
    Dim cReply As String
       
       
    ParseGridRowError = ""
     
    'Check for a numbered row with no action
    If ParseGrid.TextMatrix(nRow, 1) = "" Then
        'There is no action
        If ParseGrid.TextMatrix(nRow, 0) <> "" Then
            'But there is a row number
            ParseGridRowError = "No Action in the Row"
            nPreviousCol = 1
        End If
        Exit Function
    End If
    
    'Test for an action and suitable parameters
    cReply = ParseCheckAction(nRow)
    
    'Switch on the basis of the result
    Select Case cReply
    Case ""             'Not an action. Must be an extraction
    
    Case "OK"           'Good Action
        ParseGridRowError = ""
        Exit Function
     Case Else
        ParseGridRowError = cReply
        nPreviousCol = 2  'Its an error send it to the error column
        Exit Function
    End Select
    
    'Its an extraction
    
    'These must have channel numbers
    If Len(ParseGrid.TextMatrix(ParseGrid.Row, 3)) = 0 Then
        ParseGridRowError = "Missing Channnel Number"
        nPreviousCol = 1
        Exit Function
    End If

    
    'Check those extractions that need col 2 parameters
           
    If ParseGrid.TextMatrix(nRow, 2) = "" Then
    
        Select Case ParseGrid.TextMatrix(nRow, 1)
        Case cExtractN
            ParseGridRowError = "Enter a count of bytes to Extract"
        Case cExtractU
            ParseGridRowError = "Enter a string to Extract Until"
        Case cExtractBit
            ParseGridRowError = "Enter a bit number 0-7 to Extract"
        Case cExtractHex
            ParseGridRowError = "Enter a Number of Bytes to Extract in Hex Form"
        End Select
        
        If ParseGridRowError <> "" Then
            nPreviousCol = 2
        End If
                    
    End If
    
    
End Function


'Test if this is an Action
'If not return ""
'If it is return OK if it has suitable parameters
'Return an error string if these are missing

Private Function ParseCheckAction(nRow As Integer) As String

    
    Select Case ParseGrid.TextMatrix(nRow, 1)
    
    Case cSearch, cIgnoreN, cIgnoreU
        ParseCheckAction = "OK"
    Case Else
        ParseCheckAction = ""
    End Select
   
    If ParseGrid.TextMatrix(nRow, 2) = "" Then
    
        Select Case ParseGrid.TextMatrix(nRow, 1)
            Case cSearch
                ParseCheckAction = "Enter a string to search for"
            Case cIgnoreU
                ParseCheckAction = "Enter characters to Ignore Until"
            Case cIgnoreN
                ParseCheckAction = "Enter a count of bytes to Ignore"
        End Select
    End If
            

End Function


'Right mouse button brings up add action

Private Sub ParseGrid_MouseDown(button As Integer, _
shift As Integer, x As Single, y As Single)

    'Right button calls up the AddAction menu
    If button = 2 Then
        Select Case ParseGrid.Col
            Case 1
                Parseform.PopupMenu AddActionMenu
            Case 3
                ChannelDiag.Show
        End Select
    End If
    
End Sub

'Update a channel value in the ParseGrid
'nChannel is the one to update. cValue is the new value to show
'a ""  cValue clears all values

Public Sub ChannelValue(nChannel As Integer, cValue As String)

    Dim nRow As Integer
    
    'Clear all
    If cValue = "" Then
       For nRow = 1 To ParseGrid.Rows - 1
            ParseGrid.TextMatrix(nRow, 5) = ""
       Next nRow
       Exit Sub
    End If

    'Search for the channel and show the value
    For nRow = 1 To ParseGrid.Rows - 1
        If ParseGrid.TextMatrix(nRow, 3) = CStr(nChannel) Then
            ParseGrid.TextMatrix(nRow, 5) = cValue
            Exit Sub
        End If
    Next nRow

End Sub

'Called after a reply message has been received in the RUN screen to display the parse results

Public Sub ParseDisplay(cReply As String)

    Dim cLine As String
    Dim nRow As Integer
    Dim nEnd As Integer
    Dim dwParseStep As Long
    Dim dwReplyStart As Long
    
    StatusView.WriteStatus ("Full Parse")
    nEnd = FindEndOfData(ParseGrid)
    dwParseSequence = 0
    dwReplyStart = 1

    For nRow = 1 To nEnd - 1
       
        'dwParseStep is the number of bytes of the reply
        'to advance
        dwParseStep = SerialParseSequence(dwParseSequence)
        cLine = ParseGrid.TextMatrix(nRow, 1) + "   " + ParseGrid.TextMatrix(nRow, 2) + "    " + Mid$(cReply, dwReplyStart, dwParseStep)
        StatusView.WriteStatus (cLine)
        dwReplyStart = dwReplyStart + dwParseStep
        dwParseSequence = dwParseSequence + 1
            
    Next nRow
        
    
End Sub
