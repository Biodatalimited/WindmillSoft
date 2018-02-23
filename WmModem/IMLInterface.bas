Attribute VB_Name = "IMLInterface"

Option Explicit
Private Declare Function OpenIMLInterface% Lib "commslib.dll" Alias "_OpenIMLInterface@4" (ByRef nHandle%)
Private Declare Function CloseIMLInterface% Lib "commslib.dll" Alias "_CloseIMLInterface@4" (ByVal nHandle%)
Private Declare Function GetIMLINIPath% Lib "commslib.dll" Alias "_GetIMLINIPath@8" (ByVal szPath As String, ByVal wSize%)
Private Declare Function GetIMLDirPath% Lib "commslib.dll" Alias "_GetIMLDirPath@8" (ByVal szPath As String, ByVal wSize%)

Private Declare Function GetPrivateProfileStringA Lib "kernel32" _
    (ByVal lpAppName As String, ByVal lpKeyName As String, _
    ByVal lpDefault As String, ByVal lpReturnedString As String, _
    ByVal nSize As Long, ByVal lpFileName As String) As Long
Private Declare Function GetPrivateProfileIntA Lib "kernel32" _
    (ByVal lpAppName As String, ByVal lpKeyName As String, _
    ByVal nDefault As Long, ByVal lpFileName As String) As Long
 Private Declare Function WritePrivateProfileStringA Lib "kernel32" _
    (ByVal lpAppName As String, ByVal lpKeyName As String, _
    ByVal lpString As String, ByVal lpFileName As String) As Long

Public Const MAX_NUMS As Integer = 19
Public Const SELECTION As Integer = -1

'Used to start windmill by asking commslib for a handle

Public Sub Dial()
    
    Dim nHandle As Integer
    Dim nRet As Integer
         
    nRet = OpenIMLInterface(nHandle)
    nRet = CloseIMLInterface(nHandle)

    Unload Mainform

End Sub

'Get the path to Windmill via commslib
'Returns the path as a variable length string
'The return does not have \iml.ini

Public Function GetWindmillPath() As String
    
    Dim cFileName As String * 256
    Dim nRet As Long

    nRet = GetIMLINIPath(cFileName, 255)
    
    nRet = InStr(cFileName, ".")
    

    If nRet = 0 Then
        GetWindmillPath = ""
    Else
        'Remove the \ini.
        GetWindmillPath = Left$(cFileName, nRet - 5)
    End If

End Function

'Used by wmModem to save a new number to wmModem.INI

Public Sub SaveNewNumber(cTelephone As String)

    
    Dim cSection As String
    Dim cKey As String
    Dim cFileName As String
    Dim nRet As Long
    Dim nIndex As Integer

    
    cFileName = GetWmModemPath()
    If cFileName = "" Then Exit Sub
     
    cSection = "TELEPHONES"
    
    'Search for the next free slot
    'Limit of 20 numbers
    For nIndex = 1 To 20
        If GetNumberFromINI(nIndex) = "" Then Exit For
    Next nIndex
        
    'Write it
    cKey = "NUM" + CStr(nIndex)
    nRet = WritePrivateProfileStringA(cSection, cKey, cTelephone, cFileName)
    
End Sub

Public Function GetNumberFromINI(nIndex As Integer) As String
    
    Dim cSection As String
    Dim cKey As String
    Dim cFileName As String
    Dim cBuffer As String * 80
    Dim cDefault As String
    Dim nRet As Long
    
    cFileName = GetWmModemPath()
    If cFileName = "" Then
        GetNumberFromINI = ""
        Exit Function
    End If
     
    cSection = "TELEPHONES"
    
    cKey = "NUM" + CStr(nIndex)
       
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFileName)
    If nRet = 0 Then
        GetNumberFromINI = ""
    Else
        GetNumberFromINI = Left$(cBuffer, nRet)
    End If
    
     
End Function

'Reads the selected number from the WmModem INI
'It is under the selected section
'key is num

Public Function GetSelectionFromINI() As String
    
    Dim cSection As String
    Dim cKey As String
    Dim cFileName As String
    Dim cBuffer As String * 80
    Dim cDefault As String
    Dim nRet As Long
    
    cFileName = GetWmModemPath()
    If cFileName = "" Then
        GetSelectionFromINI = ""
        Exit Function
    End If
    
    cSection = "SELECTED"
    cKey = "NUM"
    
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFileName)
    If nRet = 0 Then
        GetSelectionFromINI = ""
    Else
        GetSelectionFromINI = Left$(cBuffer, nRet)
    End If
    
     
End Function

'Saves the passed in number under the key Selected

Public Sub SaveSelectedNumber(cTelephone As String)
    
    Dim cSection As String
    Dim cKey As String
    Dim cFileName As String
    
    cFileName = GetWmModemPath()
    If cFileName = "" Then Exit Sub
     
    cSection = "SELECTED"
    cKey = "NUM"
    Call WritePrivateProfileStringA(cSection, cKey, cTelephone, cFileName)
    
End Sub

'Remove a number from the list in the modem.ini file

Public Sub RemoveNumber(cTelephone As String)

    Dim nIndex As Integer
    Dim cSection As String
    Dim cKey As String
    Dim cFileName As String
    Dim cBuffer As String * 80
    Dim nRet As Long
    
    cFileName = GetWmModemPath()
    If cFileName = "" Then Exit Sub
     
    cSection = "TELEPHONES"
    cBuffer = Chr$(0)
    
    'Find the number and delete it
    For nIndex = 1 To 20
        If GetNumberFromINI(nIndex) = cTelephone Then
            cKey = "NUM" + CStr(nIndex)
            Call WritePrivateProfileStringA(cSection, cKey, "", cFileName)
        End If
    Next nIndex
    
End Sub

'Gets the details of nDevice from iml ini
'returns true if the device is imml800 type
'false otherwise

Public Function GetDeviceFromIMLINI(nDevice As Integer, ByRef nComm As Integer, ByRef nBaud As Integer, ByRef nTele As Integer) As Boolean

    Dim cSection As String
    Dim cKey As String
    Dim cFileName As String
    Dim cBuffer As String * 80
    Dim cDefault As String
    Dim nRet As Long
    
    cFileName = GetWindmillPath()
    cFileName = cFileName + "\iml.ini"
    
    'Read the settings for the passed in device
    cSection = "Device" + CStr(nDevice)
    cKey = "settings"
    cDefault = ""
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFileName)
    
    'is it IMML800?
    If Left$(cBuffer, nRet) <> "IMML800" Then
         GetDeviceFromIMLINI = False
        Exit Function
    End If
    
    'It is so get the parameters
   
    'The com port
    cKey = "M0P0"
    nRet = GetPrivateProfileIntA(cSection, cKey, 255, cFileName)
    If nRet = 255 Then
        GetDeviceFromIMLINI = False
        Exit Function
    Else
        nComm = nRet
    End If
     
     'The baud rate
    cKey = "M0P1"
    nRet = GetPrivateProfileIntA(cSection, cKey, 255, cFileName)
    If nRet = 255 Then
          GetDeviceFromIMLINI = False
        Exit Function
    Else
        nBaud = nRet
    End If
      
    'The Telephone Enable
    cKey = "M0P3"
    nRet = GetPrivateProfileIntA(cSection, cKey, 255, cFileName)
    If nRet = 255 Then
          GetDeviceFromIMLINI = False
        Exit Function
    Else
        nTele = nRet
    End If
  
    GetDeviceFromIMLINI = True

End Function

Public Function GetWmModemPath() As String
    
    Dim cFileName As String
    
    cFileName = GetWindmillPath()
    If cFileName = "" Then
        GetWmModemPath = ""
        Exit Function
    End If
    GetWmModemPath = cFileName + "\wmModem.ini"
 
End Function

Public Function GetWorkingDir() As String
    
    Dim cFileName As String * 256
    Dim nRet As Long
    Dim nIndex As Integer

    nRet = GetIMLDirPath(cFileName, 255)
    

    If nRet = 1 Then
        GetWorkingDir = ""
    Else
        'Remove the \ini.
        nIndex = 1
        While Mid$(cFileName, nIndex, 1) <> Chr$(0)
            nIndex = nIndex + 1
        Wend
        
        GetWorkingDir = Left$(cFileName, nIndex - 1)
    End If
End Function
