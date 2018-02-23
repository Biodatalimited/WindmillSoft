VERSION 5.00
Begin VB.Form StatusView 
   Caption         =   "Logging Status"
   ClientHeight    =   7905
   ClientLeft      =   165
   ClientTop       =   855
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   7905
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer IMLtimer 
      Enabled         =   0   'False
      Interval        =   1
      Left            =   3720
      Top             =   240
   End
   Begin VB.TextBox StatusBox 
      Height          =   6255
      Left            =   360
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   0
      Top             =   840
      Width           =   3975
   End
   Begin VB.Menu PauseButton 
      Caption         =   "Pause"
   End
   Begin VB.Menu HelpButton 
      Caption         =   "Help"
   End
End
Attribute VB_Name = "StatusView"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Const MAX_LINES = 100

Dim cLines(MAX_LINES) As String
Public bIMLDebug As Boolean
Dim bPause As Boolean
Dim nFirstFull As Integer
Dim nLineCount As Integer
Dim vLastUpdate As Variant


'Status View is used to display the debug for Windmill
'and also for the Run option of comdebug

Private Sub Form_Load()

    'Adjust the sizes

    If bIMLDebug Then
    
        StatusView.Caption = "Windmill Debug Screen"
    
        StatusView.Height = 8500
        StatusView.Width = 6200
        StatusBox.Height = 7000
        StatusBox.Left = 500
        StatusBox.Width = 5000
        StatusBox.Top = 500
        IMLtimer.Enabled = True     'The timer for getting data from Commslib
        
    Else
    
        StatusView.Caption = "Logging Status Screen"
        StatusView.Height = CommLog.Height
        StatusView.Top = CommLog.Top
        StatusView.Left = CommLog.Left + CommLog.Width
        StatusBox.Height = CommLog.Dgrid.Height + 300
        StatusBox.Top = CommLog.Dgrid.Top
    
    End If
   
    bPause = False
   
       
    nLineCount = 0      'No lines yet
    vLastUpdate = 0     'Not updated yet

End Sub


'Called either from the RUN form or the IML Timer to add a new status line
'to the display
'We have a store for 100 lines of data. Throw out old ones if full already

Public Sub WriteStatus(cText As String)

    Dim nIndex As Integer
    Dim cPrint As String
    
    'If the line count buffers are all used
    If nLineCount = UBound(cLines) Then
        'Make space at Ubound(cLines)
        'By shifting all down 1 place
        For nIndex = 0 To UBound(cLines) - 1
            cLines(nIndex) = cLines(nIndex + 1)
        Next nIndex
        
        'Ther is now 1 empty space
        nLineCount = nLineCount - 1
        
    End If
    
    'Where to write this new line
    'NB nLineCount between writes points to the last full line
    'So increment now to get the place to write the new line
    nLineCount = nLineCount + 1
    
    'Convert the non prints to printable
    cPrint = ConvertReply(cText)
    
    'Insert a blank line before the start of parse
    'All in one string
    If Len(cPrint) > 13 Then
        If Left$(cPrint, 13) = "Start Message" Then
            cPrint = vbCr + vbLf + cPrint
        End If
    End If
    
    
    'Add the new text and CRLF chars
    cLines(nLineCount) = cPrint + vbCr + vbLf
      
    
    'Write the data into the text box
    WriteBox

End Sub

'Clear the Text Box and all its stored lines

Public Sub ClearStatus()

    Dim nIndex As Integer
    
    For nIndex = 1 To UBound(cLines)
        cLines(nIndex) = ""
    Next nIndex
    
    nLineCount = 0
    
    StatusBox.Text = ""

End Sub

'Actually write the data into the text box

Private Sub WriteBox()

    Dim nIndex As Integer
    Dim cText As String
    
    'Still collecting during pause but don't update
    If bPause Then Exit Sub
       
    'Update no faster than every 0.5 sec
    If vLastUpdate > Timer Then Exit Sub
              
    'Create the box text string by starting from the most recent entry
    'This is so that we can limit the maximum string length
       
    cText = ""
    For nIndex = 0 To nLineCount
        cText = cLines(nLineCount - nIndex) + cText
        If Len(cText) > 32000 Then
            Exit For
        End If
    Next nIndex
  
        
    StatusBox.Text = cText
    
    'A selection greater than the text length selects the end of text
    'A value greater than 16 bits causes a run time error
    StatusBox.SelStart = 64000
        
    vLastUpdate = Timer + 0.5
    
End Sub

'Convert all non prints into bracketed numbers
'Return the converted string

Function ConvertReply(cIn As String) As String

    Dim nIndex As Integer
    Dim cOut As String
    
    cOut = ""
    For nIndex = 1 To Len(cIn)
        
        Select Case Asc(Mid$(cIn, nIndex, 1))
            Case 32 To 127
                cOut = cOut + Mid$(cIn, nIndex, 1)
            Case 13
                cOut = cOut + "<CR>"
            Case 10
                cOut = cOut + "<LF>"
            Case Else
                cOut = cOut + "<" + CStr(Asc(Mid$(cIn, nIndex, 1))) + ">"
        End Select
    
    Next nIndex

    ConvertReply = cOut

End Function

Private Sub Form_Unload(Cancel As Integer)

    'Reenable the form we started from in IMLdebug mode
    If bIMLDebug Then Startup.Enabled = True

End Sub

Private Sub HelpButton_Click()
    If bIMLDebug Then
        Call DisplayHelp(StatusView.hWnd, 17)
    Else
        Call DisplayHelp(StatusView.hWnd, 18)
    End If
    
End Sub

Private Sub IMLtimer_Timer()

    Dim cRead As String
       
    'Read all available messages from Commslib
    Do
        cRead = ReadDebug()
        If cRead = "" Then Exit Sub
        WriteStatus (cRead)
        DoEvents
        
    Loop
 
End Sub

Private Sub PauseButton_Click()

    If PauseButton.Caption = "Pause" Then
        IMLtimer.Enabled = False
        PauseButton.Caption = "Start"
        bPause = True
    Else
        IMLtimer.Enabled = True
        PauseButton.Caption = "Pause"
        bPause = False
        ClearStatus
    End If

End Sub
