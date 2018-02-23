VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form Mainmenu 
   Caption         =   "Windmill comDebug  Main Menu"
   ClientHeight    =   5565
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6540
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   5565
   ScaleWidth      =   6540
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame Frame7 
      Height          =   1335
      Left            =   120
      TabIndex        =   24
      Top             =   3840
      Width           =   2055
      Begin VB.CommandButton CancelButton 
         Height          =   255
         Left            =   240
         TabIndex        =   27
         Top             =   960
         Width           =   375
      End
      Begin VB.CommandButton OKButton 
         Height          =   255
         Left            =   240
         TabIndex        =   25
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label12 
         Caption         =   "Cancel"
         Height          =   255
         Left            =   840
         TabIndex        =   28
         Top             =   960
         Width           =   735
      End
      Begin VB.Label Label11 
         Caption         =   "OK"
         Height          =   255
         Left            =   840
         TabIndex        =   26
         Top             =   240
         Width           =   495
      End
   End
   Begin VB.Frame Frame6 
      Height          =   615
      Left            =   2400
      TabIndex        =   21
      Top             =   4560
      Width           =   3855
      Begin VB.CommandButton HelpButton 
         Height          =   255
         Left            =   240
         TabIndex        =   22
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label10 
         Caption         =   "Help"
         Height          =   255
         Left            =   960
         TabIndex        =   23
         Top             =   240
         Width           =   1095
      End
   End
   Begin VB.Frame Frame3 
      Height          =   615
      Left            =   2400
      TabIndex        =   18
      Top             =   3840
      Width           =   3855
      Begin VB.CommandButton Save 
         Height          =   255
         Left            =   240
         TabIndex        =   19
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label9 
         Caption         =   "Save  .IMD File"
         Height          =   255
         Left            =   960
         TabIndex        =   20
         Top             =   240
         Width           =   1935
      End
   End
   Begin VB.Frame Frame4 
      Height          =   615
      Left            =   2400
      TabIndex        =   10
      Top             =   720
      Width           =   3855
      Begin VB.CommandButton ChanButton 
         Height          =   255
         Left            =   240
         TabIndex        =   11
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label4 
         Caption         =   "Edit the data channel details"
         Height          =   255
         Left            =   960
         TabIndex        =   12
         Top             =   240
         Width           =   2295
      End
   End
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   6000
      Top             =   5280
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.Frame Frame5 
      Height          =   615
      Left            =   2400
      TabIndex        =   6
      Top             =   1440
      Width           =   3855
      Begin VB.CommandButton EditTimings 
         Height          =   255
         Left            =   240
         TabIndex        =   7
         Top             =   240
         Width           =   375
      End
      Begin VB.Label Label5 
         Caption         =   "Edit the Instrument Timings"
         Height          =   255
         Left            =   960
         TabIndex        =   8
         Top             =   240
         Width           =   2415
      End
   End
   Begin VB.Frame Frame2 
      Height          =   1575
      Left            =   2400
      TabIndex        =   3
      Top             =   2160
      Width           =   3855
      Begin VB.CommandButton Initialise 
         Height          =   255
         Left            =   240
         TabIndex        =   29
         Top             =   240
         Width           =   375
      End
      Begin VB.CommandButton MessageAdd 
         Height          =   255
         Left            =   240
         TabIndex        =   16
         Top             =   1200
         Width           =   375
      End
      Begin VB.ComboBox MsgList 
         Height          =   315
         Left            =   1560
         Style           =   2  'Dropdown List
         TabIndex        =   9
         Top             =   720
         Width           =   1935
      End
      Begin VB.CommandButton EditMsg 
         Height          =   255
         Left            =   240
         TabIndex        =   4
         Top             =   720
         Width           =   375
      End
      Begin VB.Label Label13 
         Caption         =   "Initialisation Message"
         Height          =   255
         Left            =   960
         TabIndex        =   30
         Top             =   240
         Width           =   2055
      End
      Begin VB.Label Label3 
         Caption         =   "Add a new Message"
         Height          =   255
         Left            =   960
         TabIndex        =   17
         Top             =   1200
         Width           =   1935
      End
      Begin VB.Label Label2 
         Caption         =   "Edit "
         Height          =   255
         Left            =   960
         TabIndex        =   5
         Top             =   720
         Width           =   375
      End
   End
   Begin VB.Frame Frame1 
      Height          =   615
      Left            =   2400
      TabIndex        =   0
      Top             =   0
      Width           =   3855
      Begin VB.CommandButton CommButton 
         Height          =   255
         Left            =   240
         TabIndex        =   1
         Top             =   240
         Width           =   375
      End
      Begin VB.Label CommEditLabel 
         Caption         =   "Edit COM Port Settings"
         Height          =   255
         Left            =   960
         TabIndex        =   2
         Top             =   240
         Width           =   2415
      End
   End
   Begin VB.Image Image1 
      Height          =   1530
      Left            =   480
      Picture         =   "Mainmenu.frx":0000
      Stretch         =   -1  'True
      Top             =   360
      Width           =   1335
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
      Left            =   240
      TabIndex        =   15
      Top             =   2040
      Width           =   1215
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
      Left            =   600
      TabIndex        =   14
      Top             =   2400
      Width           =   1455
   End
   Begin VB.Shape Shape1 
      Height          =   3135
      Left            =   120
      Top             =   120
      Width           =   2055
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
      Height          =   495
      Left            =   360
      TabIndex        =   13
      Top             =   2760
      Width           =   1575
   End
End
Attribute VB_Name = "Mainmenu"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Type Message
    cName As String
    cPrompt As String
    cParse As String
    cAck As String
End Type

Public cName As String
Public cPrompt As String
Public cParse As String
Public cAck As String
Public nEditMsg As Integer
Public bMsgDelete As Boolean

Dim bEdited As Boolean
Dim Messages() As Message
Dim nMode As Integer
Dim nCancel As Integer

Const CANCEL_FALSE = 0
Const CANCEL_CLICK = 1
Const CANCEL_FAIL_LOAD = 2


' Just leave the form if cancel is pressed
Private Sub CancelButton_Click()
    nCancel = CANCEL_CLICK
    Call Unload(Mainmenu)
    
End Sub

'The channel edit dialog
Private Sub ChanButton_Click()
    
    ChannelDiag.nEditMode = MODE_ADD_UNALLOCATED_CHANNELS
    ChannelDiag.Visible = True
    bEdited = True
End Sub

'Edit the Comms parameters
Private Sub CommButton_Click()
    If bSerialComms Then
        CommSet.Show 1
    Else
        NetworkSet.Show 1
    End If
    bEdited = True
    
    
End Sub

'Edit a selected message

Private Sub EditMsg_Click()
    
    LoadMessage (MsgList.ListIndex + 1)
    
    'A New File has a default Message_1 Name
    If (nEditMsg = 1) And (nMode = MODE_NEW_FILE) Then
        'Prompt the user for a new name
        cName = InputBox("Enter a Name for the First Message", "First Message", "Message_1")
        'Update the name in the memory and the list
        If cName = "" Then Exit Sub
        Messages(1).cName = cName
        nMode = MODE_NEW_EDIT
        Fill_Message_List (1)
    End If
    
    If bSerialComms Then
        CommSet.bAutoStart = True
        CommSet.Show 1
    Else
        NetworkSet.bAutoStart = True
        NetworkSet.Show 1
    End If
       
    Mainform.Visible = True
    Mainmenu.Visible = False
    bEdited = True
End Sub

Private Sub EditTimings_Click()
    Timing.Visible = True
    bEdited = True
End Sub

Private Sub Form_Activate()

    If bMsgDelete Then
        MessageDelete (nEditMsg)
    End If
    
    bMsgDelete = False

End Sub

Private Sub Form_Initialize()
    'We always have a blank message at the end
    ReDim Messages(0)
End Sub

Private Sub Form_Load()
    
    Dim nMessage As Integer
    Dim cMsg As String
    Dim cTitle As String
    Dim cFile As String
    
    nEditMsg = 0
    nCancel = CANCEL_FALSE
    
    bEdited = False
    bMsgDelete = False
        
    
   ' Get the filename to load
    If nMode = MODE_LOAD_FILE Then
        CommonDialog1.Flags = cdlOFNHideReadOnly
        CommonDialog1.Filter = "Instrument Files .imd | *.imd"
        CommonDialog1.DialogTitle = "Open Instrument File"
        CommonDialog1.InitDir = cIMLDirPath
        CommonDialog1.ShowOpen
    
        'Null file name sends us back and unloads the mainmenu
        If CommonDialog1.FileName = "" Then
            nCancel = CANCEL_FAIL_LOAD
            Unload Mainmenu
            Exit Sub
        End If
        
        'Check that the file exists
        cFile = Dir(CommonDialog1.FileName)
        If cFile = "" Then
             'If we failed to open the file go back
            cTitle = "File Open Error"
            cMsg = "Failed to Find the File"
            Call MsgBox(cMsg, , cTitle)
            nCancel = CANCEL_FAIL_LOAD
            Unload Mainmenu
            Exit Sub
        End If
        
        'Load the file
        If LoadIMDFile(CommonDialog1.FileName) = False Then
            
            'If we failed to open the file go back
            cTitle = "File Open Error"
            cMsg = "Failed to Load the File"
            Call MsgBox(cMsg, , cTitle)
            nCancel = CANCEL_FAIL_LOAD
            Unload Mainmenu
            Exit Sub
            
        End If
   
    End If
   
    
    If nMode = MODE_NEW_FILE Then
        Messages(0).cName = "Initialisation"
        Messages(0).cAck = ""
        Messages(0).cParse = ""
        Messages(0).cPrompt = ""
        ReDim Preserve Messages(1)
        Messages(1).cName = "Message_1"
        Messages(1).cAck = ""
        Messages(1).cParse = ""
        Messages(1).cPrompt = ""
        ReDim Preserve Messages(2)
        
        ChannelDiag.InitialiseChannelStore
        Timing.nTimeout = 1000
        Timing.nPersistence = 1000
        Timing.nIdleTime = 0
        Timing.nReadMode = 0
        
    End If
       
    'Create the list with channel 1 name selected
    Call Fill_Message_List(1)
    
    If bSerialComms = True Then
        CommEditLabel.Caption = "Edit COM Port Settings"
    Else
        CommEditLabel.Caption = "Edit Network Settings"
    End If
      
End Sub

Private Sub Form_Unload(Cancel As Integer)
     
    'Cancel after failing to load
    If nCancel = CANCEL_FAIL_LOAD Then
        Startup.Visible = True
        Exit Sub
    End If
     
    If (nCancel <> CANCEL_CLICK) And bEdited Then
        Select Case MsgBox("Save changes to .IMD file", vbYesNoCancel)
            
        Case vbYes
            If FileSave = False Then
                Cancel = 1
                Exit Sub
            End If
        Case vbCancel
            Cancel = 1
            Exit Sub
        End Select
    End If
    
    'Unload all the files
    Call Unload(ChannelDiag)
    Call Unload(Mainform)
    Call Unload(CommSet)
    Call Unload(NetworkSet)
    Call Unload(CommStatus)
    Call Unload(Parseform)
    Call Unload(Timing)
    
    'Close the comms
    Call Channel_Close

    Startup.Visible = True
    
End Sub


Private Sub HelpButton_Click()
    Call DisplayHelp(Mainmenu.hWnd, 2)
End Sub

'Load the initialisation message

Private Sub Initialise_Click()

    Dim cText As String
    Dim vReply As Variant
    
    cText = "This is the Initialisation Message." + vbCr
    cText = cText + "It is sent only once when ComIML loads" + vbCr
    cText = cText + "It might, for example, be used to set an instrument operating mode" + vbCr
    cText = cText + "For normal purposes use Message_1 , _2  etc." + vbCr + vbCr
    cText = cText + "Do you want an Initialisation Message ?"
    
    vReply = MsgBox(cText, vbYesNo, "Initialisation Message")
    If vReply = vbNo Then
        Exit Sub
    End If
    
    LoadMessage (0)
    
    If bSerialComms Then
        CommSet.bAutoStart = True
        CommSet.Show 1
    Else
        NetworkSet.bAutoStart = True
        NetworkSet.Show 1
    End If
    
    Mainform.Visible = True
    Mainmenu.Visible = False
    bEdited = True
    
End Sub

Private Sub MessageAdd_Click()

    Dim cTitle As String
    
    cTitle = InputBox("Enter a Name for the New Message", "New Message", "Message_" + CStr(UBound(Messages)))
    
    
    'Start the message edit
    Messages(UBound(Messages)).cName = cTitle
    nEditMsg = UBound(Messages)
    'Add the new message to the list
    MsgList.AddItem Messages(nEditMsg).cName
    MsgList.Text = Messages(nEditMsg).cName
    'Create an extra message
    ReDim Preserve Messages(UBound(Messages) + 1)
    
    'Initialise the strings
'    LoadMessage (nEditMsg)
    
    'Launch the editor
 '   Mainform.Visible = True
    
    bEdited = True
    
    EditMsg_Click
    
End Sub

Sub MessageDelete(nDelMsg As Integer)

    Dim nIndex As Integer
    
    'Copy the messages down 1
    For nIndex = nDelMsg To UBound(Messages) - 1
        Messages(nIndex) = Messages(nIndex + 1)
    Next nIndex

    'Remove the final message
    ReDim Preserve Messages(UBound(Messages) - 1)
    
    'Refill the list
    Fill_Message_List (1)
    
    ChannelDiag.Channel_Message_Delete (nDelMsg)
    

End Sub



Public Sub SetMode(nSetmode As Integer)
    nMode = nSetmode
End Sub


'Routine called by imdfile to load the array
Public Sub AddMessage()
    ReDim Preserve Messages(nEditMsg + 1)
    Messages(nEditMsg).cName = cName
    Messages(nEditMsg).cPrompt = cPrompt
    Messages(nEditMsg).cParse = cParse
    Messages(nEditMsg).cAck = cAck
    nEditMsg = nEditMsg + 1
End Sub

Public Function LoadMessage(nMessage As Integer) As Integer
    
    If nMessage < UBound(Messages) Then
        LoadMessage = True
    Else
        LoadMessage = False
        Exit Function
    End If

    nEditMsg = nMessage
    cPrompt = Messages(nEditMsg).cPrompt
    cParse = Messages(nEditMsg).cParse
    cAck = Messages(nEditMsg).cAck
    cName = Messages(nEditMsg).cName

    
End Function

Public Sub GetMessageName(nMessage As Integer, cBuffer As String)
    cBuffer = Messages(nMessage).cName
End Sub

Public Sub MessageSave()
    Messages(nEditMsg).cPrompt = cPrompt
    Messages(nEditMsg).cParse = cParse
    Messages(nEditMsg).cAck = cAck
    
    'Redo the message list if the name changes
    If Messages(nEditMsg).cName <> cName Then
        Messages(nEditMsg).cName = cName
        Fill_Message_List (nEditMsg)
    End If

End Sub

Public Sub DecrementChannel()

    Dim nChan As Integer
    
    'Store the channel
    nChan = ChannelDiag.nChannel
    
    'Load the message to edit
    LoadMessage (ChannelDiag.nMessage)
    
    'Mode 1 means that channels are in parse string
    If ChannelDiag.nMode = 1 Then
        cParse = DecrementString(cParse)
    Else
        'Mode 2 means that channels are in other strings
        cPrompt = DecrementString(cPrompt)
        cAck = DecrementString(cAck)
    End If
        
    MessageSave
    
End Sub

'Search through the string for channel references
'If they are to the specified channel + 1 then
'adjust them to the channel

Private Function DecrementString(cString As String) As String

    Dim nIndex As Integer
    Dim cNew As String
    Dim cTemp As String
    Dim cChannel As String
    Dim bChannel As Boolean
    
    bChannel = False
    
    'Copy the first 2 bytes into the new string
    cNew = Left$(cString, 2)
    
    'Channel numbers occur as \X(99)
    'Copy the string char by char to cNew
    'When we find ( with \ 2 before then
    'Copy number to cChannel
    'If cChannel is specified channel + 1 then decrement
    'Finally replace cString with new version

    For nIndex = 3 To Len(cString)
        
        'Current char
        cTemp = Mid$(cString, nIndex, 1)
        
        'if we aren't extracting a channel
        If bChannel = False Then
            cNew = cNew + cTemp
        Else
            'get the channel string
            cChannel = cChannel + cTemp
        End If
        
        Select Case cTemp
        
        Case "("
            'if a bracket occurs 2 chars after a \
            'this is a channel
            If Mid$(cString, nIndex - 2, 1) = "\" Then
                bChannel = True
                cChannel = ""
            End If
        Case ")"
            If bChannel = True Then
                If Val(cChannel) = ChannelDiag.nChannel + 1 Then
                    cNew = cNew + CStr(ChannelDiag.nChannel) + ")"
                Else
                    cNew = cNew + cChannel
                End If
                bChannel = False
            End If
        End Select
    Next nIndex
        
    DecrementString = cNew
        
End Function

'The dialog will default to the default dir for a first save
'If this is an edit of an existing file then we suggest it as
'the store place

Private Function FileSave() As Boolean
        
    Dim nIndex As Integer
    
        
    FileSave = True
    
    CommonDialog1.CancelError = True
    On Error GoTo errorhandler
    ' Get the filename to load
    CommonDialog1.Flags = cdlOFNHideReadOnly + cdlOFNNoChangeDir
    CommonDialog1.Filter = "Instrument Files .imd | *.imd"
    CommonDialog1.DialogTitle = "Save Instrument File"
    If CommonDialog1.FileName = "" Then
        'Go for default dir
        CommonDialog1.InitDir = cIMLDirPath
    Else
        'Split the filename into dir and file
        nIndex = Len(CommonDialog1.FileName)
        
        'look for final \
        For nIndex = 1 To Len(CommonDialog1.FileName)
            If Mid$(CommonDialog1.FileName, Len(CommonDialog1.FileName) - nIndex, 1) = "\" Then
                Exit For
            End If
        Next nIndex
    
        'OK we can split it
        If nIndex < Len(CommonDialog1.FileName) Then
            'We have the module name
            CommonDialog1.InitDir = Left$(CommonDialog1.FileName, Len(CommonDialog1.FileName) - nIndex - 1)
            CommonDialog1.FileName = Right$(CommonDialog1.FileName, nIndex)
        End If
    End If
    CommonDialog1.ShowSave
    
    
    Call SaveIMDFile(CommonDialog1.FileName)
    Call SaveNamesFile
    Exit Function
    
errorhandler:
    FileSave = False
    
End Function

'In response to OK prompt for File Save.
'Remain in menu in response to cancel or a failed
'file save

Private Sub OKButton_Click()
    Call Unload(Mainmenu)
End Sub

Private Sub Save_Click()
    FileSave
    bEdited = False
End Sub

'Create the message list
'nSelect is the message number to show as selected

Private Sub Fill_Message_List(nSelect As Integer)

    Dim nMessage As Integer
    
    MsgList.Clear
    
    'Fill the message list
    nMessage = 1
    While nMessage < UBound(Messages)
        MsgList.AddItem Messages(nMessage).cName
        nMessage = nMessage + 1
    Wend
    MsgList.ListIndex = nSelect - 1
    
End Sub

Public Function Message_Count() As Integer

       Message_Count = UBound(Messages) - 1
       
End Function
