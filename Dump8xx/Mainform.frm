VERSION 5.00
Begin VB.Form Mainform 
   Caption         =   "Dump8xx"
   ClientHeight    =   1935
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4845
   Icon            =   "Mainform.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   1935
   ScaleWidth      =   4845
   StartUpPosition =   3  'Windows Default
   Begin VB.Label Status 
      Height          =   375
      Left            =   1320
      TabIndex        =   5
      Top             =   1200
      Width           =   3375
   End
   Begin VB.Label Label4 
      Caption         =   "Creating"
      Height          =   255
      Left            =   240
      TabIndex        =   4
      Top             =   1200
      Width           =   735
   End
   Begin VB.Label Module 
      Caption         =   "0"
      Height          =   255
      Left            =   3240
      TabIndex        =   3
      Top             =   480
      Width           =   1095
   End
   Begin VB.Label Device 
      Caption         =   "0"
      Height          =   375
      Left            =   1320
      TabIndex        =   2
      Top             =   480
      Width           =   975
   End
   Begin VB.Label Label2 
      Caption         =   "Module"
      Height          =   255
      Left            =   2400
      TabIndex        =   1
      Top             =   480
      Width           =   615
   End
   Begin VB.Label Label1 
      Caption         =   "Device"
      Height          =   255
      Left            =   240
      TabIndex        =   0
      Top             =   480
      Width           =   735
   End
End
Attribute VB_Name = "Mainform"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'Version 1.0 Original
'Version 2.0 Added provision for multiple devices
'            Meant new directory structure
'Version 2.1 Added 826A 3 bytes reading provision

Dim nIndex As Integer
Dim cPath As String



Sub Form_Load()

    Dim cFileName As String
    Dim cDirName As String
    Dim cRet As String
    Dim nDevice As Integer
    Dim nPort As Integer
    Dim nBaud As Integer
    Dim nTele As Integer
    Dim cHWType As String
    Dim nModemReply As Integer
    Dim cBuffer As String
    Dim nTemp As Integer

    'Get the path to Windmill
    cPath = GetWorkingDir()
    cPath = cPath + "\Data"
    cDirName = cPath + "\Dump8xx"
    
    'Does the directory exist
    cRet = Dir(cDirName, vbDirectory)
    
    'If not create it
    If cRet = "" Then
        MkDir (cDirName)
    End If
    
    cFileName = cPath + "\Setup800.log" + Chr$(0)

    Call EnableErrorLog(cFileName, 1)
    
    Show
    
    For nDevice = 0 To 9
        
        Device.Caption = Str(nDevice)
        'See if the device exists and is an IMML800
        'Get its com port and baud rate
        If InitialiseDevice(nDevice) = 0 Then
                        
            'Open the port
            nModemReply = 0
            
            Select Case OpenComPort
                Case 0
                Case 1
                    Unload Mainform
                    Exit Sub
                Case 2
                    Status.Caption = "Dialing the modem"
                    DoEvents
                    nModemReply = ModemDial
            End Select
            
            Select Case nModemReply
                Case 0
                Case 1
                    Status.Caption = "ERROR  Cannot contact the local Modem"
                Case REPLY_BUSY
                    Status.Caption = "ERROR  Modem reply - Line is Busy"
                Case REPLY_NO_CARRIER
                    Status.Caption = "ERROR  Modem reply - No Carrier"
                Case REPLY_NO_DIALTONE
                    Status.Caption = "ERROR  Modem reply - No Dialtone"
                Case REPLY_NO_ANSWER
                    Status.Caption = "ERROR  Modem reply - No Answer"
                Case Else
                    Status.Caption = "ERROR  Making Connection"
            End Select
            
            If nModemReply <> 0 Then
                   Unload Mainform
                   Exit Sub
            End If
                 
            
            cDirName = cPath + "\Dump8xx\Device" + CStr(nDevice)
    
            'Does the directory exist
            cRet = Dir(cDirName, vbDirectory)
    
            'If not create it
            If cRet = "" Then
                MkDir (cDirName)
            End If
                    
        
            'For each module in turn
            For nIndex = 0 To 29
                Module.Caption = CStr(nIndex)
                DoEvents
                
                'Does it exist
                If ReadModuleIDX(nIndex, cHWType) = 0 Then
                    
                    cFileName = cDirName + "\log" + CStr(nIndex) + ".wl"
                    Status.Caption = cFileName
                    DoEvents
                    'Store the data
                    Call WriteLoggerFile(nIndex, cFileName, cHWType)
                    Status.Caption = ""
                End If
            Next nIndex
        End If
    
        Call CloseComPort
        
    Next nDevice
    
    Unload Mainform
    
End Sub


Sub Form_Unload(cancel As Integer)
    Call CloseComPort
End Sub

