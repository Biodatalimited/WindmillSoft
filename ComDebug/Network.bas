Attribute VB_Name = "Network"


Declare Sub Network_Initialise Lib "IMLComm.dll" Alias "_Network_Initialise@0" ()
Declare Function Network_Socket_Open Lib "IMLComm.dll" Alias "_Network_Socket_Open@8" (ByVal ptAddress As String, ByVal dwPort As Long) As Long
Declare Sub Network_Close Lib "IMLComm.dll" Alias "_Network_Close@4" (ByVal nSocket As Long)
Declare Function Network_Send Lib "IMLComm.dll" Alias "_Network_Send@12" (ByVal nSocket As Long, ByVal ptBuffer As String, ByVal ulSize As Long) As Long
Declare Function Network_Reply Lib "IMLComm.dll" Alias "_Network_Reply@12" (ByVal nSocket As Long, ByVal ptBuffer As String, ByVal dwSize As Long) As Long
Declare Function Network_Name2IP Lib "IMLComm.dll" Alias "_Network_Name2IP@12" (ByVal cName As String, ByVal cIPaddress As String, ByRef lSize As Long) As Long
Declare Function ReadDebugString Lib "Commslib.dll" Alias "_ReadDebugString@4" (ByVal pBuf As String) As Long
Declare Sub WriteDebugString Lib "Commslib.dll" Alias "_WriteDebugString@4" (ByVal pBuf As String)

Public bSerialComms As Boolean
Public cIPaddress As String
Public cIPName As String
Public lPort As Long
Public nSocket As Long


Public Sub NetworkComms_Initialise()

    bSerialComms = False
    cIPaddress = ""
    cIPName = ""
    lPort = 0
    nSocket = -1

End Sub


Public Function Channel_Open() As Long

    Dim vTime As Variant

    If bSerialComms = True Then
        Channel_Open = SerialCommsOpen
    Else
        Network_Initialise
        nSocket = Network_Socket_Open(cIPaddress, lPort)
        Channel_Open = nSocket
    End If

End Function

Public Sub Channel_Close()

    If bSerialComms = True Then
        Call SerialCommsClose
    Else
        Call Network_Close(nSocket)
        nSocket = -1
    End If

End Sub

Public Sub Channel_Send(cBuf As String, dwSize As Long)

    Dim ulReply As Long
    Dim nIndex As Integer

   
    If dwSize = 0 Then Exit Sub
   

        If bSerialComms = True Then
            Call SerialCommsSend(cBuf, dwSize)
        Else
            ulReply = Network_Send(nSocket, cBuf, dwSize)
        End If
        


End Sub

Public Sub Channel_Read(cBuf As String, dwSize As Long, dwBytesRead As Long)

    Dim lRet As Long
    
    If bSerialComms = True Then
        Call SerialCommsRead(cBuf, dwSize, dwBytesRead)
    Else
        dwBytesRead = Network_Reply(nSocket, cBuf, MAXROWS)
    End If
    
    If dwBytesRead < 0 Then dwBytesRead = 0


End Sub

Public Function Resolve_NetWork_Name(cName As String, cAddress As String) As Integer

    Dim cTempAdd As String * 20
    Dim lSize As Long
    Dim nError As Integer
    
    
    Network_Initialise
    
    nError = Network_Name2IP(cName, cTempAdd, lSize)
    
    If nError = 0 Then
         cAddress = Left$(cTempAdd, lSize)
    End If

    Resolve_NetWork_Name = nError
    

End Function


Public Function ReadDebug() As String

    Dim cRead As String * 256
    Dim nReadBytes As Long
        
    nReadBytes = ReadDebugString(cRead)
   
    ReadDebug = Left$(cRead, nReadBytes)

End Function


Public Sub WriteDebug()

    Dim cRead As String * 256
    Dim nReadBytes As Long
    
    For nReadBytes = 0 To 11
        cRead = "???" + CStr(nReadBytes) + Chr$(0)
        Call WriteDebugString(cRead)
    Next nReadBytes

End Sub

