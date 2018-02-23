Attribute VB_Name = "SerialComms"
Option Explicit

Declare Function CommOpen Lib "IMLComm.dll" Alias "_CommOpen@4" (cComm As Long) As Long
Declare Function CommFlush Lib "IMLComm.dll" Alias "_CommFlush@4" (ByVal hComPort As Long) As Long
Declare Function CommWrite Lib "IMLComm.dll" Alias "_CommWrite@16" (ByVal hComPort As Long, ByVal pBuf As String, ByVal dwSize As Long, dwBytesWritten As Long) As Long
Declare Function CommRead Lib "IMLComm.dll" Alias "_CommRead@16" (ByVal hComPort As Long, ByVal pBuf As String, ByVal dwSize As Long, dwBytesRead As Long) As Long
Declare Function CommClose Lib "IMLComm.dll" Alias "_CommClose@4" (ByVal hComPort As Long) As Long
Declare Function CommLastError Lib "IMLComm.dll" Alias "_CommLastError@0" () As Long
Declare Function CommState Lib "IMLComm.dll" Alias "_CommState@12" (ByVal hComPort As Long, ByVal pBuf As String, dwModemStatus As Long) As Long
Declare Function CommSetState Lib "IMLComm.dll" Alias "_CommSetState@12" (ByVal hComPort As Long, ByVal bDTR As Long, ByVal bRTS As Long) As Long
Declare Function GenerateCRC Lib "IMLComm.dll" Alias "_GenerateCRC@12" (ByVal dwStart As Long, ByVal pBuf As String, ByVal dwSize As Long) As Long

Declare Function DLLParseBuffer Lib "IMLComm.dll" Alias "_DLLParseBuffer@20" (ByVal pBuf As String, ByVal dwBytes As Long, ByVal pszParse As String, ByVal pszReply As String, nChannel As Integer) As Long
Declare Function DLLParseSequence Lib "IMLComm.dll" Alias "_DLLParseSequence@4" (ByVal nParseIndex As Long) As Long


Public Type CommStruct
wPort As Long
wBaud As Long
wBits As Long
wParity As Long
wStop As Long
wHandshake As Long
End Type

Public Type CommCurrent
bCTS As Boolean
bDCD As Boolean
bRI As Boolean
bDSR As Boolean
bBreak As Boolean
bRTS As Boolean
bDTR As Boolean
dwError As Long
End Type

Public hComPort As Long
Public pComm As CommStruct
Public pCommstate As CommCurrent
Public szParse As String * 1024
Dim dwTotalBytes As Long

Public Sub SerialCommsInitialise()

'Initialise the local variables
pComm.wPort = 1
pComm.wBaud = 9600
pComm.wBits = 8
pComm.wParity = 0
pComm.wStop = 1
pComm.wHandshake = 0
hComPort = -1

'We open the port in this state
pCommstate.bRTS = True
pCommstate.bDTR = True

szParse = ""

End Sub


Public Function SerialCommsOpen() As Long

    Dim dwError As Long
    Dim cComm As Long
     
    'Close it if it's already open
    If hComPort <> -1 Then
        CommClose (hComPort)
    End If
     
    hComPort = CommOpen(pComm.wPort)
        
    'If there is an error
    If hComPort = -1 Then
        dwError = CommLastError()
        MsgBox ("Failed to open the COM Port")
    End If
    
    'Zero the bytes read count
    dwTotalBytes = 0
    
    SerialCommsOpen = hComPort
    
End Function

Public Sub SerialCommsClose()
    CommClose (hComPort)
End Sub

Public Sub SerialCommsGetState()

Dim cBuf As String * 100
Dim dwModemStatus As Long



pCommstate.dwError = CommState(hComPort, cBuf, dwModemStatus)


If dwModemStatus And 16 Then
    pCommstate.bCTS = True
Else
    pCommstate.bCTS = False
End If

If dwModemStatus And 32 Then
    pCommstate.bDSR = True
Else
    pCommstate.bDSR = False
End If

If dwModemStatus And 64 Then
    pCommstate.bRI = True
Else
    pCommstate.bRI = False
End If

If dwModemStatus And 128 Then
    pCommstate.bDCD = True
Else
    pCommstate.bDCD = False
End If

End Sub

Public Sub ToggleRTSstate()
    Dim dwError As Long
       
    If pCommstate.bRTS = True Then
        dwError = CommSetState(hComPort, pCommstate.bDTR, 0)
        pCommstate.bRTS = False
    Else
        dwError = CommSetState(hComPort, pCommstate.bDTR, 1)
        pCommstate.bRTS = True
    End If
  
End Sub

Public Sub ToggleDTRstate()
    Dim dwError As Long
       
    If pCommstate.bDTR = True Then
        dwError = CommSetState(hComPort, 0, pCommstate.bRTS)
        pCommstate.bDTR = False
    Else
        dwError = CommSetState(hComPort, 1, pCommstate.bRTS)
        pCommstate.bDTR = True
    End If
  
End Sub


Public Sub SerialCommsSend(cBuf As String, dwSize As Long)

    Dim dwError As Long
    Dim dwBytesWritten As Long
    Dim cCap As String

    dwTotalBytes = 0
    
    dwError = CommWrite(hComPort, cBuf, dwSize, dwBytesWritten)
    If dwBytesWritten = dwSize Then
        CommStatus.TXStatus.Caption = "Transmit Status : All Bytes Sent"
    Else
        cCap = "Transmit Status : " + CStr(dwBytesWritten) + " Bytes Sent. "
        If pComm.wHandshake = 2 Then
            If pCommstate.bCTS = False Then
                cCap = cCap + "CTS false prevents transmission"
            End If
        End If
        CommStatus.TXStatus.Caption = cCap
    End If
    
End Sub

Public Sub SerialCommsRead(cBuf As String, dwSize As Long, dwBytesRead As Long)

    Dim dwError As Long
    Dim cCap As String
    
    dwBytesRead = 0
    dwError = CommRead(hComPort, cBuf, dwSize, dwBytesRead)
    dwTotalBytes = dwTotalBytes + dwBytesRead
    
    SerialCommsGetState
    
    If hComPort = -1 Then
        cCap = "Failed to open COM Port"
        CommStatus.TXStatus = cCap
    Else
        cCap = "Receive Status : " + CStr(dwTotalBytes) + " Bytes Received"
    End If
    
    CommStatus.RXStatus.Caption = cCap

End Sub

'Called repeatedly by the mainform timer to parse the
'reply. It returns the extracted value

Public Sub SerialParseReply(cExtraction As String)

    Dim cOutput As String * 20
    Dim cTemp As String
    Dim cReply As String * MAXROWS
    Dim dwSize As Long
    Dim nChannel As Integer
    Dim nRet As Integer

    'First clear the grid
    cOutput = ""
    Call Parseform.ChannelValue(nChannel, cOutput)

    'Get the reply string from the grid
    Call SerialGrid_GetString(Mainform.GridRec, cTemp, dwSize)
    'Parse it
    cReply = cTemp
    nChannel = -1
    nRet = DLLParseBuffer(cReply, dwSize, szParse, cOutput, nChannel)
    While nRet < 32000
        Call Parseform.ChannelValue(nChannel, cOutput)
        nRet = DLLParseBuffer(cReply, dwSize, szParse, cOutput, nChannel)
    Wend
   

End Sub

'Called from the parse form to set the parse string

Public Sub SerialSetParseString(cString As String)
    szParse = cString
End Sub

'Just calls the DLL function
Public Function SerialParseSequence(dwParseIndex As Long) As Long

SerialParseSequence = DLLParseSequence(dwParseIndex)

End Function
