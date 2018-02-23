Attribute VB_Name = "AidFile"
Option Explicit

Private Declare Function HtmlHelp Lib "hhctrl.ocx" Alias "HtmlHelpA" _
    (ByVal hwndCaller As Long, ByVal pszFile As String, _
    ByVal ulCommand As Long, ByVal ulContext As Long) As Long
    


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
Private Declare Function GetIMLINIPath Lib "commslib.dll" _
    Alias "_GetIMLINIPath@8" _
    (ByVal cIniPath As String, ByVal dwSize As Long) As Long
 
Private Declare Function GetIMLDirPath Lib "commslib.dll" _
    Alias "_GetIMLDirPath@8" _
    (ByVal cDirPath As String, ByVal dwSize As Long) As Long

Public Const UNUSED As Integer = 0
Public Const READTYPE As Integer = 1
Public Const WRITETYPE As Integer = 2
Public Const READBIT   As Integer = 3
Public Const WRITEBIT As Integer = 4
Public Const RECNOTYPE As Integer = 5


 Public cIMLINIPath As String
 Public cIMLDirPath As String
 Public cIMLDataPath As String
 Dim nIMLDevice As Integer

Public Sub DisplayHelp(myHwnd As Long, ulContext As Long)

    Dim dwReply As Long
    Dim cFilePath As String
    
    cFilePath = App.Path + "\comdebug.chm"
    

    dwReply = HtmlHelp(myHwnd, cFilePath, 15, ulContext)


End Sub



'Read the paths to iml.ini and the default working dir
'Store them in the public variables
'These are NULL if we cannot read

Public Sub ReadIMLPaths()

    Dim nIndex As Integer
    Dim cBuffer As String * 256
    
    If GetIMLINIPath(cBuffer, 256) = 1 Then
        cIMLINIPath = ""
    Else
        nIndex = 1
        While Mid$(cBuffer, nIndex, 1) <> Chr$(0)
            nIndex = nIndex + 1
        Wend
        cIMLINIPath = Left$(cBuffer, nIndex - 1)
    End If
    
    If GetIMLDirPath(cBuffer, 256) = 1 Then
        cIMLDirPath = ""
    Else
        nIndex = 1
        While Mid$(cBuffer, nIndex, 1) <> Chr$(0)
            nIndex = nIndex + 1
        Wend
        cIMLDirPath = Left$(cBuffer, nIndex - 1)
        cIMLDataPath = cIMLDirPath + "\Data"
        cIMLDirPath = cIMLDirPath + "\Setups"

    End If
    
End Sub

Public Function GetIMSChannels(cFilename As String, nModule As Integer) As String

Dim cSection As String * 80     'INI File section
Dim cKey As String * 80         'Key within Section
Dim cDefault As String * 80     'Default to return if no entry
Dim cBuffer As String * 2048    'Data String to read
Dim nSize As Long
Dim nRet As Long
Dim nChannel As Integer
Dim nTemp As Integer
Dim nMessage As Integer
Dim cReply As String
Dim cAddress As String
Dim cName As String
Dim cModFixed As String

'Create the device + module number which is the same for all channels

cModFixed = CStr(nIMLDevice)
If nModule < 10 Then
    cModFixed = cModFixed + "0" + CStr(nModule)
Else
    cModFixed = cModFixed + CStr(nModule)
End If


'for each channel in turn
nChannel = 0
cReply = ""
Do
    'section is [Chan0]
    cSection = "Chan" + CStr(nChannel)
    
    'Check that the channel exists
    cKey = "Message"    'Read the message number
    nTemp = GetPrivateProfileIntA(cSection, cKey, -1, cFilename)
    If nTemp = -1 Then
        Exit Do
    End If
    
    'Prepare the channel number
    If nChannel < 10 Then
        cAddress = cModFixed + "0" + CStr(nChannel) + " "
    Else
        cAddress = cModFixed + CStr(nChannel) + " "
    End If
    
    cKey = "Name"   'Read the channel name
    cDefault = Chr$(0)
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
    'Prepare the channel name
    cName = "N=" + Left$(cBuffer, nRet)
                
    cKey = "Max"    'Read the maximum value
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
       
    cKey = "Min"    'Read the minimum value
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
   
   
    cKey = "Mode"   'Read the mode
    nTemp = GetPrivateProfileIntA(cSection, cKey, 0, cFilename)
    Select Case nTemp
        Case UNUSED
            cReply = cReply + "V " + cAddress + cName + " E=0 M=A O=0 S=1 U=" + vbCrLf
        Case READTYPE
            cReply = cReply + "I " + cAddress + cName + " E=1 M=A O=0 S=1 U=" + vbCrLf
        Case WRITETYPE
            cReply = cReply + "V " + cAddress + cName + " E=1 M=A O=0 S=1 U=" + vbCrLf
        Case READBIT
            cReply = cReply + "B " + cAddress + cName + " E=1 M=D R=01 U=" + vbCrLf
        Case WRITEBIT
            cReply = cReply + "T " + cAddress + cName + " E=1 M=D R=01 U=" + vbCrLf
        Case RECNOTYPE
            cReply = cReply + "V " + cAddress + cName + " E=1 M=A O=0 S=1 U=" + vbCrLf
    End Select
     
    nChannel = nChannel + 1
    
Loop

GetIMSChannels = cReply

End Function
    
Public Function LoadIMDFile(cFilename As String) As Boolean

Dim cSection As String * 80     'INI File section
Dim cKey As String * 80         'Key within Section
Dim cDefault As String * 80     'Default to return if no entry
Dim cBuffer As String * 2048    'Data String to read
Dim nSize As Long
Dim nRet As Long
Dim nChannel As Integer
Dim nTemp As Integer
Dim nMessage As Integer

ChannelDiag.InitialiseChannelStore

'Read the comms parameters from the imd file
cSection = "COMMUNICATIONS"

cKey = "Port"   'First the comm port
pComm.wPort = GetPrivateProfileIntA(cSection, cKey, 0, cFilename)

'If the port is the default value 0 then there is a file
'problem. Let the load go ahead to get the default
'values but return false.

cKey = "Baud"       'Now the baud rate default 9600
pComm.wBaud = GetPrivateProfileIntA(cSection, cKey, 9600, cFilename)

cKey = "Bits"       'Bits in the data format
pComm.wBits = GetPrivateProfileIntA(cSection, cKey, 8, cFilename)

cKey = "Parity"     'The parity
pComm.wParity = GetPrivateProfileIntA(cSection, cKey, 0, cFilename)

cKey = "Stopbits"   'The number of stop bits
pComm.wStop = GetPrivateProfileIntA(cSection, cKey, 2, cFilename)

cKey = "Handshake"  'Handshake type
pComm.wHandshake = GetPrivateProfileIntA(cSection, cKey, 0, cFilename)

cDefault = ""
cKey = "IPAddress"  'The IP Address
nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
cIPaddress = Left$(cBuffer, nRet)

cKey = "IPport"     'The IP port number
nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
lPort = Val(Left$(cBuffer, nRet))

cDefault = ""
cKey = "IPName"  'The IP Name
nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
cIPName = Left$(cBuffer, nRet)


bSerialComms = True
LoadIMDFile = True
If pComm.wPort = 0 Then
    bSerialComms = False
    If cIPaddress = "0.0.0.0" Then
        If cIPName = "" Then
            LoadIMDFile = False
        End If
    End If
End If



'Read the module parameters
cSection = "MODULE"
cKey = "Timeout"
nRet = GetPrivateProfileIntA(cSection, cKey, 1000, cFilename)
Timing.nTimeout = nRet

cKey = "Idletime"
nRet = GetPrivateProfileIntA(cSection, cKey, 0, cFilename)
Timing.nIdleTime = nRet

cKey = "Persistence"
nRet = GetPrivateProfileIntA(cSection, cKey, 1000, cFilename)
Timing.nPersistence = nRet

cKey = "ReadMode"
nRet = GetPrivateProfileIntA(cSection, cKey, 0, cFilename)
Timing.nReadMode = nRet


'for each message in turn
nMessage = 0
Do
    'section is [Message0]
    cSection = "Message" + CStr(nMessage)

    cKey = "Name"   'Read the channel name
    cDefault = ""
    cBuffer = ""
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
    If nRet = 0 Then   '0 bytes means end of channels
        Exit Do
    End If
    Mainmenu.cName = Left$(cBuffer, nRet)

    cKey = "Prompt"    'Read the prompt string
    cBuffer = ""
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 2048, cFilename)
    Mainmenu.cPrompt = Left$(cBuffer, nRet)
   
    cKey = "Parse"    'Read the parse string
    cBuffer = ""
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 2048, cFilename)
    Mainmenu.cParse = Left$(cBuffer, nRet)
   
    cKey = "Ack"  'Read the ack string
    cBuffer = ""
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 2048, cFilename)
    Mainmenu.cAck = Left$(cBuffer, nRet)
     
    Call Mainmenu.AddMessage
     
    nMessage = nMessage + 1 'Move to next message
Loop


'for each channel in turn
nChannel = 0
Do
    'section is [Chan0]
    cSection = "Chan" + CStr(nChannel)
    
    cKey = "Message"    'Read the message number
    nTemp = GetPrivateProfileIntA(cSection, cKey, -1, cFilename)
    
    If nTemp = -1 Then
        Exit Do
    End If
    
    ChannelDiag.nMessage = nTemp



    cKey = "Name"   'Read the channel name
    cDefault = Chr$(0)
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
    ChannelDiag.cName = Left$(cBuffer, nRet)

    cKey = "Max"    'Read the maximum value
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
    ChannelDiag.cMax = Left$(cBuffer, nRet)
   
    cKey = "Min"    'Read the minimum value
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
    ChannelDiag.cMin = Left$(cBuffer, nRet)
   
   
    cKey = "Mode"   'Read the mode
    nTemp = GetPrivateProfileIntA(cSection, cKey, 0, cFilename)
    ChannelDiag.nMode = nTemp
     
  
    Call ChannelDiag.AddChannel
    nChannel = nChannel + 1
    
Loop


End Function

'Writes out the IMD file. Early versions used fixed length strings
'which could cause very large files ( lots of spaces ) but not always!

Public Sub SaveIMDFile(cFilename As String)

Dim cSection As String
Dim cKey As String
Dim cDefault As String
Dim cBuffer As String
Dim cValue As String
Dim nSize As Long
Dim nRet As Long
Dim nChannel As Integer
Dim nMessage As Integer

'Save the comms parameters in the imd file
cSection = "COMMUNICATIONS"

'Store the port as 0 if we are doing networrks
If bSerialComms Then
    cValue = CStr(pComm.wPort)
Else
    cValue = "0"
End If

cKey = "Port"
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cValue = CStr(pComm.wBaud)
cKey = "Baud"
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cValue = CStr(pComm.wBits)
cKey = "Bits"
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cValue = CStr(pComm.wParity)
cKey = "Parity"
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cValue = CStr(pComm.wStop)
cKey = "Stopbits"
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cValue = CStr(pComm.wHandshake)
cKey = "Handshake"
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cValue = cIPaddress
cKey = "IPAddress"
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cValue = CStr(lPort)
cKey = "IPport"
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cValue = cIPName
cKey = "IPName"
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

'Write the module parameters
cSection = "MODULE"
cKey = "Timeout"
cValue = CStr(Timing.nTimeout)
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cKey = "Idletime"
cValue = CStr(Timing.nIdleTime)
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cKey = "Persistence"
cValue = CStr(Timing.nPersistence)
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cKey = "ReadMode"
cValue = CStr(Timing.nReadMode)
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)

cKey = "Timeout"
cValue = CStr(Timing.nTimeout)
Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)


'Store the message data
'[Message0]
'Name=abc
'Prompt=abc
'Parse=abc
'Ack=abc
nMessage = 0
While Mainmenu.LoadMessage(nMessage)

    cSection = "Message" + CStr(nMessage)
    
    cKey = "Name"
    cValue = Mainmenu.cName
    Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)
    
    cValue = Mainmenu.cPrompt
    cKey = "Prompt"
    Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)
    
    cValue = Mainmenu.cParse
    cKey = "Parse"
    Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)
    
    cValue = Mainmenu.cAck
    cKey = "Ack"
    Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)
     
    nMessage = nMessage + 1
Wend

'If we have deleted some messages we need to clear them
'from the file.
Do
    
    'section is [Message0]
    cSection = "Message" + CStr(nMessage)

    cKey = "Name"   'Read the channel name
    cDefault = ""
    cBuffer = ""
    nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
    If nRet = 0 Then   '0 bytes means end of messages
        Exit Do
    End If

    cSection = "Message" + CStr(nMessage)
    Call WritePrivateProfileStringA(cSection, vbNullString, vbNullString, cFilename)
    nMessage = nMessage + 1

Loop

'Store the channel data
'[Chan0]
'Name=abc
'Max=00
'Min=00
'Units=abc
'Mode=0
'Message=0
nChannel = 0
While ChannelDiag.LoadChannel(nChannel)

    cSection = "Chan" + CStr(nChannel)
    
    cKey = "Name"
    cValue = ChannelDiag.cName
    Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)
    

    cKey = "Max"
    cValue = ChannelDiag.cMax
    Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)
    
    cKey = "Min"
    cValue = ChannelDiag.cMin
    Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)
        
    cValue = CStr(ChannelDiag.nMode)
    cKey = "Mode"
    Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)
    
    cValue = CStr(ChannelDiag.nMessage)
    cKey = "Message"
    Call WritePrivateProfileStringA(cSection, cKey, cValue, cFilename)
    nChannel = nChannel + 1
Wend

'If we have deleted some channels we need to clear them
'from the file. nFileChannels is the number in the file
Do
    cSection = "Chan" + CStr(nChannel)
    cKey = "Message"    'Read the message number
    nRet = GetPrivateProfileIntA(cSection, cKey, -1, cFilename)
    
    If nRet = -1 Then
        Exit Do
    End If
   
    cSection = "Chan" + CStr(nChannel)
    Call WritePrivateProfileStringA(cSection, vbNullString, vbNullString, cFilename)
    nChannel = nChannel + 1
Loop

End Sub


'Used to read the modules of an IML device from IMl.INI
'It gets the module name and description
'It returns 0 if it fails 1 if it succeeds


Public Function GetIMLModuleName(cModName As String, cDesc As String, cIMD As String, nModnum As Integer) As Integer


Dim cSection As String
Dim cKey As String
Dim cBuffer As String * 80
Dim cDefault As String * 2
Dim dwRet As Long

'In IML.INI we have
' [COMIML]
' Mod0 = xxxx
' Mod1 = yyyy   etc
'Modules run from 0 upwards with no gaps

cSection = "COMIML"
cKey = "Mod" + CStr(nModnum)
cDefault = ""

dwRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cIMLINIPath)

If dwRet = 0 Then
    cModName = ""
    GetIMLModuleName = 0
    Exit Function
End If
    
cModName = Left$(cBuffer, dwRet)

'Each module has a section
'  [COMIML.xxxx]
'  desc=Short Description
'  IMDFile=full path

cSection = cSection + "." + cModName
cKey = "Desc"

'Get the description
dwRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cIMLINIPath)
If dwRet = 0 Then
    cDesc = ""
    GetIMLModuleName = 0
    Exit Function
End If
cDesc = Left$(cBuffer, dwRet)

'Get the file path
cKey = "IMDFile"
dwRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cIMLINIPath)
If dwRet = 0 Then
    cDesc = ""
    GetIMLModuleName = 0
    Exit Function
End If
    
cIMD = Left$(cBuffer, dwRet)

GetIMLModuleName = 1

End Function

'In the IML.INI file each hardware type has an entry which is used for
'creating the add new hardware list.
'When an IML device is created this data is used to create a [DeviceN]
'section.
'Both of these entries include a list of the modules within the device
'In addition each module ( IMD file in this case ) has its own section
'In order to add or delete a module we must edit both of the first 2
'sections and in the case of a completely new module create its own
'section.



Public Sub WriteIMLModuleName(cModule As String, cDesc As String, cIMD As String, nModnum As Integer)


Dim cSection As String      'File Section Name
Dim cKey As String          'Section Key Name
Dim nIndex As Integer
Dim nDevice As Integer      'IML device number
Dim dwRet As Integer
Dim cBuffer As String * 80
Dim cDefault As String * 2
Dim cIMLDesc As String

'In IML.INI we may have an existing device entry of the form
' [Device0]
' Settings=COMIML
' We need to find the device number

nDevice = 0         'Look for Device 0 first
cKey = "Settings"
dwRet = 1
cDefault = ""

While dwRet > 0

    cSection = "Device" + CStr(nDevice)
    dwRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cIMLINIPath)
    
    'no Comiml device found so we can't edit it
    'set nDevice to -1 to indicate this
    If dwRet = 0 Then
        nDevice = -1
    Else
        'Check if the device is COMIML. If it is then set dwRet to 0
        'To exit the loop
        'If not then try the next device
        If Left$(cBuffer, dwRet) = "COMIML" Then
            dwRet = 0
        Else
            nDevice = nDevice + 1
        End If
    End If

Wend
' nDevice should now contain the IML device number or -1 for no entry


'In IML.INI we have
' [COMIML]
' Mod0 = xxxx
' Mod1 = yyyy   etc


'Clear the hardware section and device section of old entries
'when we write module 0
If nModnum = 0 Then
    For nIndex = 0 To 9
        'The hardware section
        cKey = "Mod" + CStr(nIndex)
        cSection = "COMIML"
        Call WritePrivateProfileStringA(cSection, cKey, vbNullString, cIMLINIPath)
        If nDevice <> -1 Then
            'The device section
            cKey = "Module" + CStr(nIndex)
            cSection = "Device" + CStr(nDevice)
            Call WritePrivateProfileStringA(cSection, cKey, vbNullString, cIMLINIPath)
        End If
    Next nIndex
End If


'Write the hardware section
cSection = "COMIML"
cKey = "Mod" + CStr(nModnum)
Call WritePrivateProfileStringA(cSection, cKey, cModule, cIMLINIPath)

'Write the device section
If nDevice <> -1 Then
    cSection = "Device" + CStr(nDevice)
    cKey = "Module" + CStr(nModnum)
    Call WritePrivateProfileStringA(cSection, cKey, cModule, cIMLINIPath)
End If

'Each module has a section
'  [COMIML.xxxx]
'  desc=Short Description
' These are never deleted

cSection = "COMIML." + cModule
cKey = "Desc"
cIMLDesc = cModule + " : " + cDesc
Call WritePrivateProfileStringA(cSection, cKey, cIMLDesc, cIMLINIPath)

cKey = "IMDFile"
Call WritePrivateProfileStringA(cSection, cKey, cIMD, cIMLINIPath)

End Sub

Public Function GetModuleDesc(cFilename As String, cDesc As String) As Integer

Dim cSection As String
Dim cKey As String
Dim cBuffer As String * 80
Dim cDefault As String * 2
Dim dwRet As Long

cSection = "Module"
cKey = "Desc"
cDefault = ""

dwRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)

If dwRet = 0 Then
   GetModuleDesc = 0
   cDesc = "No Description"
Else
   GetModuleDesc = 1
   cDesc = Left$(cBuffer, dwRet)
End If


End Function

'Put the module description for Confiml in the module file

Public Sub WriteModuleDescription(cFilename As String, cDesc As String)

Dim cSection As String
Dim cKey As String

cSection = "Module"
cKey = "Desc"

Call WritePrivateProfileStringA(cSection, cKey, cDesc, cFilename)

End Sub

'Searches IML.INI to find a COMIML device
'Sets the number if found into nIMLDevice, -1 if not found

Public Sub GetIMLDeviceNumber()

Dim cSection As String
Dim cKey As String
Dim cBuffer As String * 80
Dim cDefault As String * 2
Dim dwRet As Long
Dim nDevice As Integer

    For nDevice = 0 To 10
        cDefault = ""
        cSection = "Device" + CStr(nDevice)
        cKey = "settings"
        dwRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cIMLINIPath)
 
        If dwRet <> 0 Then
            If Left$(cBuffer, 6) = "COMIML" Then
                nIMLDevice = nDevice
                Exit Sub
            End If
        End If
    
    Next nDevice
    
    nIMLDevice = -1

End Sub

Public Sub SaveNamesFile()

Dim cFilename As String
Dim nModnum As Integer
Dim nChannel As Integer
Dim nRet As Long
Dim cModule As String
Dim cDesc As String
Dim cName As String
Dim cBuffer As String * 80
Dim cSection As String
Dim cKey As String
Dim cDefault As String

Open "IMDnames.txt" For Output As #1


nModnum = 0
While GetIMLModuleName(cModule, cDesc, cFilename, nModnum)

    Print #1, "MODULE  " + cModule
    Print #1, ""

    nChannel = 0
    Do
        'section is [Chan0]
        cSection = "Chan" + CStr(nChannel)
    
        cKey = "Message"    'Read the message number
        nRet = GetPrivateProfileIntA(cSection, cKey, -1, cFilename)
    
        If nRet = -1 Then
            Exit Do
        End If

        cKey = "Name"   'Read the channel name
        cDefault = Chr$(0)
        nRet = GetPrivateProfileStringA(cSection, cKey, cDefault, cBuffer, 80, cFilename)
        cName = Left$(cBuffer, nRet)

        Print #1, "Channel " + CStr(nChannel) + "    " + cName
    
        nChannel = nChannel + 1
    
    Loop

    Print #1, ""
    Print #1, ""
    nModnum = nModnum + 1

Wend

Close #1

End Sub


Public Sub CreateIMSFile(cIMSFile As String, nDevice As Integer, nModule As Integer)

    Dim nChan As Integer
    Dim cLine As String
    Dim cDevMod As String
    
    'Calculate the initial 3 bytes of the channel number
    cDevMod = CStr(nModule)
    If Len(cDevMod) = 1 Then
        cDevMod = "0" + cDevMod
    End If
    cDevMod = CStr(nDevice) + cDevMod
    
    Open cIMLDirPath + "\" + cIMSFile For Output As #1

    Print #1, "F 01 IMSLIB 6.00"
    Print #1, "D"
    
    For nChan = 0 To ChannelDiag.nChans - 1
    
        ChannelDiag.LoadChannel (nChan)
        cLine = "I " + cDevMod
        If nModule < 10 Then
            cLine = cLine + "0"
        End If
        cLine = cLine + CStr(nChan)
        
        cLine = cLine + " N="
        cLine = cLine + ChannelDiag.cName
        cLine = cLine + " E=1 M=A O=0 S=1 U="
        
        Print #1, cLine
        
    Next nChan
    
    
    Close #1

End Sub
