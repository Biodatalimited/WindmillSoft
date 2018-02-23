VERSION 5.00
Object = "{5E9E78A0-531B-11CF-91F6-C2863C385E30}#1.0#0"; "msflxgrd.ocx"
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form CommLog 
   Caption         =   "Windmill comDebug Display Screen"
   ClientHeight    =   7740
   ClientLeft      =   165
   ClientTop       =   855
   ClientWidth     =   7980
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   7740
   ScaleWidth      =   7980
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton StopButton 
      Caption         =   "Stop"
      Height          =   495
      Left            =   3240
      TabIndex        =   5
      Top             =   6840
      Width           =   1455
   End
   Begin VB.CommandButton StartButton 
      Caption         =   "Start"
      Height          =   495
      Left            =   1320
      TabIndex        =   4
      Top             =   6840
      Width           =   1455
   End
   Begin VB.CommandButton CloseButton 
      Caption         =   "Close"
      Height          =   495
      Left            =   5160
      TabIndex        =   3
      Top             =   6840
      Width           =   1455
   End
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   7200
      Top             =   6960
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.Timer BackTimer 
      Enabled         =   0   'False
      Left            =   9480
      Top             =   6120
   End
   Begin VB.Timer SampleTimer 
      Enabled         =   0   'False
      Interval        =   10
      Left            =   9480
      Top             =   5280
   End
   Begin MSFlexGridLib.MSFlexGrid Dgrid 
      Height          =   6015
      Left            =   360
      TabIndex        =   0
      Top             =   600
      Width           =   7260
      _ExtentX        =   12806
      _ExtentY        =   10610
      _Version        =   393216
   End
   Begin VB.Label SampleRate 
      Caption         =   "Sample Rate Secs : "
      Height          =   255
      Left            =   5160
      TabIndex        =   2
      Top             =   120
      Width           =   2655
   End
   Begin VB.Label FileName 
      Caption         =   "FileName :"
      Height          =   255
      Left            =   360
      TabIndex        =   1
      Top             =   120
      Width           =   4455
   End
   Begin VB.Menu menuFile 
      Caption         =   "File"
      Begin VB.Menu menuDataFile 
         Caption         =   "Data File"
      End
      Begin VB.Menu menuNoFile 
         Caption         =   "Do not log to file"
      End
   End
   Begin VB.Menu menuSampleRate 
      Caption         =   "Sample Rate"
   End
   Begin VB.Menu StatusDisplay 
      Caption         =   "Status Display"
      Begin VB.Menu HideStatus 
         Caption         =   "Hide"
      End
      Begin VB.Menu StandardStatus 
         Caption         =   "Standard Status"
      End
      Begin VB.Menu FullStatus 
         Caption         =   "Full Status"
      End
   End
End
Attribute VB_Name = "CommLog"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'Structure to hold details of each channel
Private Type ChannelStruct
cName As String
nCol As Integer
cValue As String
End Type

Const SM_REST = 0
Const SM_START = 1
Const SM_PROMPT = 2
Const SM_READ_START = 3
Const SM_READ = 4
Const SM_ACK = 5
Const SM_END = 6

Const PROMPT_END = 0
Const PROMPT_SENT = 1
Const PROMPT_WAIT = 2

Dim nReadState As Integer
Dim vReadEnd As Variant
Dim cReadReply As String
Dim dwReadSize As Long
Dim nPromptStep As Integer

Dim sChan() As ChannelStruct

Dim vEpochTime As Variant       'The logger epoch in seconds
Dim nGridRow As Integer         'The next row to write into
Dim vPersist As Variant         'Time at which persistence ends
Dim vStartTime As Variant       'The time of the 0 sample
Dim cFilename As Variant        'File to write to
Dim vSampleRate As Variant      'Seconds
Dim bRunning As Boolean         'We are collecting data
Dim bBusy As Boolean            'We are busy taking a reading. Stop reentrants
Dim bContinuous As Boolean      'Continuous flow flag
Dim bStop As Boolean            'Come to a halt
Dim cPromptStrings() As String  'Set of prompt strings
Dim cAckStrings() As String     'Set of Ack strings
Dim bStandardStatus As Boolean
Dim bExit As Boolean

Private Sub CloseButton_Click()
    Call Unload(CommLog)
End Sub


Private Sub Form_Load()

    Dim nIndex As Integer

    'Hide them for now
    Mainform.Visible = False
    Parseform.Visible = False
    
    'Same size and position as mainform
    CommLog.Height = Mainform.Height + Parseform.Height
    CommLog.Width = Mainform.Width
    CommLog.Left = Mainform.Left
    CommLog.Top = Mainform.Top
    
    'Put the fixed values in the sChan structure
    PrepareChannels
    
    'Prepare the strings to send they may contain output values or waits
    Call PreparePromptStrings(SendPromptString(Mainform.GridSend, False), cPromptStrings())
    Call PreparePromptStrings(SendPromptString(Mainform.GridAck, False), cAckStrings())

    
    'Initialise the grid
    ClearDgrid
    
    cFilename = ""
    FileName.Caption = "File Name : Not logging to file"
    
    vSampleRate = 1
    DisplaySampleRate
    
    bRunning = False
    bBusy = False
    bStop = False
    bStandardStatus = False
    bExit = False
    nReadState = SM_REST
    vReadEnd = 0
    
    bContinuous = False
    If Mainmenu.cPrompt = "" Then
        bContinuous = True
    End If
    
    StartButton.Enabled = True
    StopButton.Enabled = False
    
    'Start the Timer. This will cause background sampling if heeded
    'Never stop it
    SampleTimer.Enabled = True
    
End Sub


'Work out which channels are in this message
'Collect their names
'Allocate them a column
'Col = 0 means not in this message
'Give them IML style numbers if not named

Private Sub PrepareChannels()

    Dim nIndex As Integer
    Dim nCol As Integer
    

   'Dimension the channel array
    ReDim cChannelValue(ChannelDiag.nChans)
    ReDim sChan(ChannelDiag.nChans)
    
    'Load the channel details 1 by 1
    nCol = 1
    For nIndex = 0 To ChannelDiag.nChans - 1
        Call ChannelDiag.LoadChannel(nIndex)
        
        'Is this channel in this message
        If ChannelDiag.nMessage = Mainmenu.nEditMsg Then
            If ChannelDiag.cName <> "" Then
                sChan(nIndex).cName = ChannelDiag.cName             'Use its allocated name
            Else
                'Convert numbers to iml format
                If nIndex < 10 Then                                 'Convert to IML style
                    sChan(nIndex).cName = "0000" + CStr(nIndex)
                Else
                    sChan(nIndex).cName = "000" + CStr(nIndex)
                End If
            End If
            
            'Don't display write channels
            If ChannelDiag.nMode = WRITETYPE Then
                sChan(nIndex).nCol = 0                              'Col 0 means don't display
            Else
                sChan(nIndex).nCol = nCol                           'give it the next col
                nCol = nCol + 1
            End If
        Else
            sChan(nIndex).nCol = 0                                  'Not in this message
        End If
    Next nIndex


End Sub

'If we are running then stop sampling. Return
'If we are not running then Unload

Private Sub Form_Unload(Cancel As Integer)

    'Back to the mainform if we are running
    If bRunning Then
        DoEvents
        bStop = True
        bExit = True
        Cancel = 1
        Exit Sub
    End If

    Call Unload(StatusView)

    Mainform.Visible = True
    'Start the mainform doing its regular reading again
    Mainform.Timer1.Enabled = True

End Sub

'Get a file name to save the data to
Private Sub menudataFile_Click()

    Dim cText As String
    Dim nIndex As Integer

    CommonDialog1.Flags = cdlOFNHideReadOnly
    CommonDialog1.Filter = "Logger File .wl | *.wl"
    CommonDialog1.DialogTitle = "Select Data File"
    CommonDialog1.InitDir = cIMLDataPath
    CommonDialog1.ShowOpen
        
    cFilename = CommonDialog1.FileName
    
    If cFilename = "" Then
        FileName.Caption = "File Name : Not logging to file"
    Else
        cText = cFilename
        If Len(cFilename) > 50 Then
            cText = Right$(cFilename, 50)
            For nIndex = Len(cFilename) - 50 To Len(cFilename)
                If Mid$(cFilename, nIndex, 1) = "\" Then
                    cText = Right$(cFilename, Len(cFilename) - nIndex)
                    Exit For
                End If
            Next nIndex
            
            cText = "..." + cText
        End If
            
    
        FileName.Caption = "File Name : " + cText
    End If
    
End Sub

'Don't log to file

Private Sub menuNoFile_Click()
    cFilename = ""
    FileName.Caption = "File Name : Not logging to file"
End Sub

'Start the sampling to screen
'It may already be background sampling

Private Sub StartButton_Click()

    Dim nIndex As Integer
    Dim cText As String
    Dim cTempFile As String
       

    'Clear all Status
    StatusView.ClearStatus
        
    ClearDgrid
    menuFile.Enabled = False
    menuSampleRate.Enabled = False
    DoEvents
         
    
    'If we are logging to file
    If cFilename <> "" Then
        Open cFilename For Output As #1
        'Put the start time in the file
        Print #1, ""
        cText = "File opened at " + Format(Time, "Long Time") + " " + Format(Date, "ddd d mmm yyyy")
        Print #1, cText
        Print #1, ""
        Print #1, ""
        
        'Prepare the channel names line
        cText = "Time"
        For nIndex = 0 To ChannelDiag.nChans - 1
            If sChan(nIndex).nCol <> 0 Then
                cText = cText + vbTab + sChan(nIndex).cName
            End If
        Next nIndex
        
        Print #1, cText
        Print #1, "Secs"
        
    End If
    
    'Initialise things
    vStartTime = Timer
    vEpochTime = 0
    bRunning = True
    
    'Start a reading in Direct Mode
    If Timing.nReadMode = 0 Then
        vPersist = 0
        nReadState = SM_START
    End If
    
    'Set Button states
    StartButton.Enabled = False
    StopButton.Enabled = True
 

End Sub


Sub EndSampling()
    
    Dim nIndex As Integer
    Dim cText As String
    Dim cTempFile As String

   If cFilename <> "" Then
        Close #1
        
        'Insert the close time in the file
        cTempFile = Left$(cFilename, Len(cFilename) - 3) + "tmp"
        Name cFilename As cTempFile
        
        Open cTempFile For Input As #1
        Open cFilename For Output As #2
        
        Input #1, cText
        Print #2, cText
        Input #1, cText
        Print #2, cText
      
        cText = "File closed at " + Format(Time, "Long Time") + " " + Format(Date, "ddd d mmm yyyy")
        Print #2, cText
        
        While Not EOF(1)
            Input #1, cText
            Print #2, cText
        Wend
                    
        Close #2
        Close #1
        
        Kill cTempFile
    End If
    

    menuFile.Enabled = True
    menuSampleRate.Enabled = True

    nReadState = SM_REST
    bRunning = False
    bStop = False
    
    StartButton.Enabled = True
    StopButton.Enabled = False

    DoEvents
    
    If bExit Then Call Unload(CommLog)

End Sub


Private Sub menuSampleRate_Click()

    Dim vReply As Variant
       
    vReply = InputBox("Enter the Time between samples in seconds", "Sample Rate", vSampleRate)

    vSampleRate = Val(vReply)
    
    Call DisplaySampleRate
    
End Sub

'This is potentially called every 10 msec
'Check current time against next sample time
'If its time for a reading. Then check the persistence
'If its expired in Background its a timeout
'If its expired in direcr mode then do a reading


Private Sub SampleTimer_Timer()
    
    
    Dim nChannel As Integer
    
    
    'Step the readings as required
    'Will cause continuous reading in background
    'Step as needed in direct
    ReadChannels_Sequencer
     
    'Come to a halt if the stop flag is set
    If bStop Then
        EndSampling
        Exit Sub
    End If
            
    'Go no further if not sampling
    If bRunning = False Then Exit Sub
    
           
    'Is it time to read. Check every 10 msec
    If (vEpochTime + vStartTime) > Timer Then
        'Not yet time
        Exit Sub
    End If
       
    'Go no further if we are already doing a direct reading
    'This point may be reached several times before the reading is available
    If Timing.nReadMode = 0 And nReadState <> SM_REST Then
        Exit Sub
    End If
    
    'Is it more than one Sample period over in which case we have missed readings
    'Fill in all missed readings
    If (vEpochTime + vStartTime + 0.5) < Timer Then
        'Missed a reading so increment to next time and make a missed entry
        While Timer > (vEpochTime + vStartTime)
            MissedEntry
        Wend
        Exit Sub
    End If

             
    'Has persistence expired
    If Timer > vPersist Then
        'Yes so can we read
        If Timing.nReadMode = 0 Then
            nReadState = SM_START           'Start the direct reading
        Else
            'Persistence has expired in background mode
            For nChannel = 0 To UBound(sChan) - 1
                sChan(nChannel).cValue = "TimeOut"
            Next nChannel
        End If
    Else
        Call WriteStatus("Persistence Read of Stored Values", False)
        WriteEpoch
    End If
    
    

End Sub

'The mainform grid has provided a single string with output values included plus inserted
'%WAIT where a wait is needed
'Sort this out into a series of strings to send
'The end of strings is indicated by ""

Sub PreparePromptStrings(cPrompt As String, cTarget() As String)

    Dim nCount As Integer
    
    
    ReDim cTarget(0)
        
    'Remove chars one by one from cPrompt
    'if it starts with %WAIT terminate the current string
    'create an extra string with %WAIT in it
    'create an extra string for more text
    
    nCount = 0
    Do While Len(cPrompt) <> 0
        
        If Left$(cPrompt, 5) = "%WAIT" Then
        
            'If the currently forming string has anything in it we need a new string
            'It may not have if this is the very first string or we have just dealt with a %WAIT
            If cTarget(nCount) <> "" Then
                ReDim Preserve cTarget(UBound(cTarget) + 1)     'Add the extra string for %WAIT
                nCount = nCount + 1
            End If
                
            cTarget(nCount) = "%WAIT"
            cPrompt = Right$(cPrompt, Len(cPrompt) - 5)     'Remove %WAIT from cPrompt
            ReDim Preserve cTarget(UBound(cTarget) + 1)     'Add the new string for more chars
            nCount = nCount + 1
        Else
            cTarget(nCount) = cTarget(nCount) + Left$(cPrompt, 1)
            cPrompt = Right$(cPrompt, Len(cPrompt) - 1)
        End If
    Loop
    
    If cTarget(nCount) <> "" Then
        ReDim Preserve cTarget(UBound(cTarget) + 1)     'Add the new string for "" fence
    End If
        
End Sub


'Write the last collected data into the display
'Scroll it if needed

Private Sub WriteEpoch()

    Dim nChannel As Integer
    Dim vTemp As Variant
    Dim cText As String

    'Scroll if needed
    If nGridRow = Dgrid.Rows Then
        For nGridRow = 1 To Dgrid.Rows - 2
            For nChannel = 0 To Dgrid.Cols - 1
                Dgrid.TextMatrix(nGridRow, nChannel) = Dgrid.TextMatrix(nGridRow + 1, nChannel)
            Next nChannel
        Next nGridRow
        nGridRow = Dgrid.Rows - 1
    End If
    
    'Write the values
    WriteTimeStamp
    cText = Dgrid.TextMatrix(nGridRow, 0)
          
    For nChannel = 0 To UBound(sChan) - 1
        If sChan(nChannel).nCol <> 0 Then
            Dgrid.TextMatrix(nGridRow, sChan(nChannel).nCol) = sChan(nChannel).cValue
            cText = cText + Chr$(9) + sChan(nChannel).cValue
        End If
    Next nChannel
   
    'Write them into file if needed
    If cFilename <> "" Then
        Print #1, cText
    End If
   
    
    'Increment the row if needed
    If nGridRow < Dgrid.Rows Then
        nGridRow = nGridRow + 1
    End If
    
End Sub

'Write the missed entry into the display
'Scroll it if needed

Private Sub MissedEntry()

    Dim nChannel As Integer
    Dim vTemp As Variant
    Dim cText As String

    'Scroll if needed
    If nGridRow = Dgrid.Rows Then
        For nGridRow = 1 To Dgrid.Rows - 2
            For nChannel = 0 To Dgrid.Cols - 1
                Dgrid.TextMatrix(nGridRow, nChannel) = Dgrid.TextMatrix(nGridRow + 1, nChannel)
            Next nChannel
        Next nGridRow
        nGridRow = Dgrid.Rows - 1
    End If
    
    'Write the values
    WriteTimeStamp
    cText = Dgrid.TextMatrix(nGridRow, 0)
          
    For nChannel = 0 To UBound(sChan) - 1
        If sChan(nChannel).nCol <> 0 Then
            Dgrid.TextMatrix(nGridRow, sChan(nChannel).nCol) = "------"
            cText = cText + Chr$(9) + "------"
        End If
    Next nChannel
   
    'Write them into file if needed
    If cFilename <> "" Then
        Print #1, cText
    End If
      
    'Increment the row if needed
    If nGridRow < Dgrid.Rows Then
        nGridRow = nGridRow + 1
    End If
    
End Sub

'Set up the grid with channel captions
'All readings cleared

Private Sub ClearDgrid()

    Dim nRow As Integer
    Dim nCol As Integer
    Dim nIndex As Integer
    
    Dgrid.Cols = ChannelDiag.nChans + 1
    If Dgrid.Cols < 5 Then
        Dgrid.Cols = 5
    End If
        
    For nCol = 0 To Dgrid.Cols - 1
        Dgrid.ColWidth(nCol) = 1400
        Dgrid.ColAlignment(nCol) = flexAlignRightTop
    Next nCol
    Dgrid.Width = 200 + 5 * Dgrid.ColWidth(0)
        
    Dgrid.Rows = 24
    Dgrid.Height = 500 + Dgrid.Rows * Dgrid.CellHeight
    
    If Dgrid.Cols > 5 Then
        Dgrid.Height = Dgrid.Height + 200
    End If
    
    Dgrid.TextMatrix(0, 0) = "Time  MM:SS:hh"
        
    For nIndex = 0 To ChannelDiag.nChans
        If sChan(nIndex).nCol <> 0 Then
            Dgrid.TextMatrix(0, sChan(nIndex).nCol) = sChan(nIndex).cName
        End If
    Next nIndex
     
    For nRow = 1 To Dgrid.Rows - 1
        For nCol = 0 To Dgrid.Cols - 1
            Dgrid.TextMatrix(nRow, nCol) = ""
        Next nCol
    Next nRow
    
    nGridRow = 1
    
End Sub

'Create the Timestamp as 00:00:00 to suit replay
'Write it to the grid
'Increment vEpochTime to the next one

Private Sub WriteTimeStamp()

    Dim vTime As Variant
    Dim cText As String
    Dim lMin As Long
    Dim lSec As Long
    Dim lHun As Long
    
    
    vTime = vEpochTime * 1000
    
    lMin = Int(vTime \ 60000)
    vTime = vTime Mod 60000
    lSec = Int(vTime \ 1000)
    lHun = vTime Mod 1000
    lHun = lHun \ 10

    cText = ""
    If lMin < 10 Then
        cText = "0"
    End If
    cText = cText + CStr(lMin) + ":"
    
    If lSec < 10 Then
        cText = cText + "0"
    End If
    cText = cText + CStr(lSec) + ":"
    
    If lHun < 10 Then
        cText = cText + "0"
    End If
    cText = cText + CStr(lHun)
     
    Dgrid.TextMatrix(nGridRow, 0) = cText
     
     'Move to next epoch
    vEpochTime = vEpochTime + vSampleRate
            
       
End Sub


Sub DisplaySampleRate()

    Dim vRate As Variant
       
    SampleRate.Caption = " Sample Rate secs : " + Format(vSampleRate, "###.##")

End Sub


Sub WriteStatus(cMsg As String, bFull As Boolean)

    Dim vTime As Variant
    Dim cText As String
    Dim lMin As Long
    Dim lSec As Long
    Dim lHun As Long
    
    If (bStandardStatus = True) And (bFull = True) Then Exit Sub
    
    
    vTime = (Timer - vStartTime) * 1000
    
    lMin = Int(vTime \ 60000)
    vTime = vTime Mod 60000
    lSec = Int(vTime \ 1000)
    lHun = vTime Mod 1000
    lHun = lHun \ 10

    cText = ""
    If lMin < 10 Then
        cText = "0"
    End If
    cText = cText + CStr(lMin) + ":"
    
    If lSec < 10 Then
        cText = cText + "0"
    End If
    cText = cText + CStr(lSec) + ":"
    
    If lHun < 10 Then
        cText = cText + "0"
    End If
    cText = cText + CStr(lHun)
    
    cText = cText + "    " + cMsg
    
    Call StatusView.WriteStatus(cText)
    

End Sub

Private Sub StandardStatus_Click()

    StatusView.bIMLDebug = False
    bStandardStatus = True
    StatusView.Show
    FullStatus.Checked = False
    StandardStatus.Checked = True
    
End Sub

Private Sub FullStatus_Click()

    StatusView.bIMLDebug = False
    bStandardStatus = False
    StatusView.Show
    FullStatus.Checked = True
    StandardStatus.Checked = False
    
End Sub

Private Sub HideStatus_Click()

    bStandardStatus = True
    Call Unload(StatusView)
    FullStatus.Checked = False
    StandardStatus.Checked = False
    
End Sub

Private Sub StopButton_Click()

    bStop = True
    
End Sub



'Read the channels by sending prompts etc

Private Sub ReadChannels_Sequencer()

    Dim cBuf As String * MAXROWS
    Dim cOutput As String * 20
    Dim dwBytesRead As Long
    Dim nRet As Long
    Dim nChannel As Integer
    Dim cChannelRead() As String
    Dim nIndex As Integer
    Dim vTemp As Variant


    'Prevent reentrants
    If bBusy = True Then Exit Sub
   
   'Set the flag to say we are busy
    bBusy = True
     
    Select Case nReadState
    
        Case SM_REST
            'Rest state nothing doing
            
            'The Idle Time in background mode
            If Timing.nReadMode = 1 Then
                If vReadEnd < Timer Then
                    nReadState = SM_START
                End If
            End If
                        
        Case SM_START
                        
            If Timing.nReadMode = 1 Then
                Call WriteStatus("Background Reading Started", False)
            Else
                Call WriteStatus("Direct Reading Started", False)
            End If
            
             'Don't flush buffers for continuous
            If bContinuous = False Then
                'Flush out any remaining bytes
                Call Channel_Read(cBuf, MAXROWS, dwBytesRead)
                If dwBytesRead > 0 Then
                    Call WriteStatus("Flushed : " + Left$(cBuf, dwBytesRead), False)
                End If
            End If
           
           'Start the following prompt from step 0
            nPromptStep = 0
            cReadReply = ""
            dwReadSize = 0
            vReadEnd = 0
            nReadState = SM_PROMPT
            
        Case SM_PROMPT
             
            If SequencePromptStrings(cPromptStrings(), False) Then
                nReadState = SM_READ_START
            End If
               
        Case SM_READ_START
       
            'Set the timeout for reading
            vReadEnd = Timing.nTimeout
            vReadEnd = Timer + vReadEnd / 1000
            vTemp = Timer
            nReadState = SM_READ
            
        Case SM_READ
        
            'Start with all readings at Timeout
            ReDim cChannelRead(ChannelDiag.nChans)
            For nChannel = 0 To (ChannelDiag.nChans) - 1
                cChannelRead(nChannel) = "TimeOut"
            Next nChannel
 
   
            nRet = 0
            'Get the Reply
            If vReadEnd > Timer Then
                 
                'Get data from the input port
                Call Channel_Read(cBuf, MAXROWS, dwBytesRead)
                If dwBytesRead > 0 Then
                    Call WriteStatus("Reply : " + Left$(cBuf, dwBytesRead), True)
                End If

                'If we have read extra bytes
                If dwBytesRead > 0 Then
                    cReadReply = cReadReply + Left$(cBuf, dwBytesRead)
                    dwReadSize = dwReadSize + dwBytesRead
                
                    cBuf = cReadReply
                    nChannel = -1
                    nRet = DLLParseBuffer(cReadReply, dwReadSize, szParse, cOutput, nChannel)
                    While nRet < 32000
                        cChannelRead(nChannel) = cOutput
                        For nIndex = 1 To Len(cOutput)
                            If Mid$(cOutput, nIndex, 1) = Chr$(0) Then
                                cChannelRead(nChannel) = Left$(cOutput, nIndex - 1)
                                Exit For
                            End If
                        Next nIndex
                
                        Call WriteStatus("Parse Channel " + CStr(nChannel) + "  " + cChannelRead(nChannel), True)
        
                        nRet = DLLParseBuffer(cReadReply, dwReadSize, szParse, cOutput, nChannel)
                    Wend
                End If
                
                If nRet = 32002 Then
                    Call WriteStatus("Full Reply : " + cReadReply, False)
                    If bStandardStatus = False Then
                        Parseform.ParseDisplay (cReadReply)
                    End If
                
                    'Update the main store of readings. May still be Time out
                    For nChannel = 0 To UBound(cChannelRead) - 1
                        sChan(nChannel).cValue = cChannelRead(nChannel)
                    Next nChannel
                    
                    'Expiry time of persistence
                    vPersist = Timing.nPersistence
                    vPersist = Timer + vPersist / 1000
                    
                    vReadEnd = 0
                    nPromptStep = 0
                    nReadState = SM_ACK
                    
                End If
                
            Else
                Call WriteStatus("Time Out Value Exceeded", False)
                If Timing.nReadMode = 0 Then
                    Call WriteEpoch             'Will show missed reading
                End If
                
                vReadEnd = Timing.nIdleTime
                vReadEnd = Timer + vReadEnd / 1000
                nReadState = SM_REST

            End If
            
                            
        Case SM_ACK
        
            If SequencePromptStrings(cAckStrings(), True) Then
                nReadState = SM_END
            End If
            
        Case SM_END
        
            Call WriteStatus("Read OK", False)
            
            If Timing.nReadMode = 0 Then
                Call WriteEpoch             'Write the data into the grid
            End If
            
            'Initialise for background use
            vReadEnd = Timing.nIdleTime
            vReadEnd = Timer + vReadEnd / 1000
            nReadState = SM_REST
 
    End Select
      
    bBusy = False

End Sub


'Send the strings stored in the passed in Array
'Implement Waits if required
'Return 1 when finished

Function SequencePromptStrings(cTarget() As String, bAck As Boolean) As Integer

    
    SequencePromptStrings = 0
    
    'Have we finished
    If cTarget(nPromptStep) = "" Then
        SequencePromptStrings = 1
        Exit Function
    End If
    
        
    'Is it a wait
    If cTarget(nPromptStep) = "%WAIT" Then
        'Is it the start of wait
        If vReadEnd = 0 Then
            vReadEnd = Timing.nIdleTime
            vReadEnd = Timer + vReadEnd / 1000
            Call WriteStatus("WAIT", True)
         Else
            'Is it the end of wait
            If vReadEnd < Timer Then
                nPromptStep = nPromptStep + 1
                vReadEnd = 0
            End If
        End If
        Exit Function
    End If
    
    'Send the string
    Call Channel_Send(cTarget(nPromptStep), Len(cTarget(nPromptStep)))
    If bAck Then
        Call WriteStatus("Acknowledge Sent : " + cTarget(nPromptStep), True)
    Else
        Call WriteStatus("Prompt Sent : " + cTarget(nPromptStep), True)
    End If
    
    nPromptStep = nPromptStep + 1
        
End Function

