Attribute VB_Name = "FileCom"
Option Explicit



Const BAUD_1200 As Integer = 1
Const BAUD_2400 As Integer = 2
Const BAUD_4800 As Integer = 3
Const BAUD_9600 As Integer = 4
Const BAUD_19200 As Integer = 5

Declare Function CloseSerialLink% Lib "ml800lib.dll" Alias "_CloseSerialLink@0" ()
Declare Function LongTimeToString% Lib "ml800lib.dll" Alias "_LongTimeToString@8" (ByVal lTime&, ByVal cTime$)
Declare Function OpenSerialLink% Lib "ml800lib.dll" Alias "_OpenSerialLink@8" (ByVal nPort%, ByVal nBaud%)
Declare Function ReadModuleID% Lib "ml800lib.dll" Alias "_ReadModuleID@8" (ByVal nModule%, ByVal cReply$)
Declare Function ReadModuleEpochData% Lib "ml800lib.dll" Alias "_ReadModuleEpochData@16" (ByVal nModule%, ByVal nEpoch%, ByVal nBytes%, cData As Byte)
Declare Function ReadModuleEpochNumber& Lib "ml800lib.dll" Alias "_ReadModuleEpochNumber@4" (ByVal nModule%)
Declare Function EnableErrorLog% Lib "ml800lib.dll" Alias "_EnableErrorLog@8" (ByVal cPath$, ByVal lEnable&)

Private Declare Function GetPrivateProfileStringA Lib "kernel32" _
    (ByVal lpAppName As String, ByVal lpKeyName As String, _
    ByVal lpDefault As String, ByVal lpReturnedString As String, _
    ByVal nSize As Long, ByVal lpFileName As String) As Long
Private Declare Function GetPrivateProfileIntA Lib "kernel32" _
    (ByVal lpAppName As String, ByVal lpKeyName As String, _
    ByVal nDefault As Long, ByVal lpFileName As String) As Long


Dim lBaud As Long
Dim lPort   As Long
Dim nModule As Integer

Public Sub SetPort(nPortNum As Integer)
    lPort = nPortNum
    Call OpenComPort
End Sub
Public Sub SetBaud(nBaudNum As Integer)
    lBaud = nBaudNum
    Call OpenComPort
End Sub

Public Function OpenComPort() As Integer
    Dim nTemp As Integer
    Dim cText As String
    
    OpenComPort = 0
    
    Call CloseSerialLink
    nTemp = OpenSerialLink(lPort, lBaud)
    
    Mainform.ComPort.Caption = CStr(lPort)
    
    If nTemp <> 0 Then
        cText = "Error cannot open COM Port" + vbCr + "  Is IML Device Open ?"
        MsgBox (cText)
        OpenComPort = 1
    End If
    
End Function

Public Function CloseComPort()
    Call CloseSerialLink
End Function

'Look for the specified module by reading its ID code
'Return 0 if found, 1 if not found. Set the nModule global to
'the module number if found

Public Function SearchForModule%(nTarget As Integer, cHWType As String)
    Dim nError As Integer
    Dim cBuffer As String * 100
    Dim nTemp As Integer

    cHWType = ""
    
    nError = ReadModuleID(nTarget, cBuffer)
    If nError <> 0 Then
        SearchForModule = 1
        Exit Function
    End If
    
    nModule = nTarget
    
    nTemp = 1
    While Mid$(cBuffer, nTemp, 1) <> Chr$(0)
        cHWType = cHWType + Mid$(cBuffer, nTemp, 1)
        nTemp = nTemp + 1
    Wend
    
    SearchForModule = 0

End Function

'Get the path to Windmill from the IML.INI

Public Function GetWindmillPath() As String
    
    Dim cSection As String * 80
    Dim cKey As String * 80
    Dim cFileName As String * 80
    Dim cBuffer As String * 80
    Dim cDefault As String * 80
    Dim nRet As Long

    cFileName = "IML.INI"
    cSection = "IMML800"
    cKey = "InstallDir"
    cDefault = ""
    
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFileName)

    If nRet = 0 Then
        GetWindmillPath = ""
    Else
        GetWindmillPath = Left$(cBuffer, nRet)
    End If

End Function

'Look for the specified device in IML.INI.
'If found return 0 and set the baud and port variables
'if not found return 1
'We first look for [DeviceN] with a Settings key of
'"IMML800". Having found this the key we then read
'key M0P0 which is the com port 1 -
'Finally we read key M0P1 which is the baud rate.

Public Function FindDeviceInINI(nDevice As Integer) As Integer

    Dim cSection As String * 80
    Dim cKey As String * 80
    Dim cFileName As String * 80
    Dim cBuffer As String * 80
    Dim cDefault As String * 80
    Dim nRet As Long
    
    FindDeviceInINI = 0 'Start with success
    
    cFileName = "IML.INI"
    
    'section is [Device0] etc
    cSection = "Device" + CStr(nDevice)

    cKey = "Settings"   'Read settings key
    cDefault = ""
    cBuffer = ""
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFileName)

    'No such key
    If nRet = 0 Then
        FindDeviceInINI = 1
        Exit Function
    End If
    
    'Should be IMML800
    If Left$(cBuffer, 7) <> "IMML800" Then
        FindDeviceInINI = 1
        Exit Function
    End If

    
    'Read the com port number
    cKey = "M0P0"
    nRet = GetPrivateProfileIntA(cSection, cKey, 0, cFileName)

    'Com 0 not possible
    If nRet = 0 Then
        FindDeviceInINI = 1
        Exit Function
    End If
    
    'OK we have the port
    lPort = nRet
    
    'Read the baud rate
    cKey = "M0P1"
    nRet = GetPrivateProfileIntA(cSection, cKey, 0, cFileName)
   
    If nRet < 2 Or nRet > 5 Then
        FindDeviceInINI = 1
        Exit Function
    End If
    
    lBaud = nRet
    

End Function
