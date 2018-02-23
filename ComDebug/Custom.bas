Attribute VB_Name = "Custom"

Option Explicit

Declare Function CustomSend Lib "IMLComm.dll" Alias "_CustomSend@24" (ByVal nType As Long, _
        ByVal cMsg As String, ByVal nMsgSize As Long, ByVal cValue As String, ByVal cOutBuf As String, ByVal cSpare As String) As Long

Public Const CUSTOM_SPLIT = 50



'Get a custom string from the Grid
'Return "" if not custom
'If it is then return \A(xx)(cc)
' Where xx is Custom number
' cc is channel number if required

Public Function CustomGetString(cCode As String, cChannel As String) As String

    
    If Left$(cCode, 3) <> cCustom Then
        CustomGetString = ""
        Exit Function
    End If
        
    CustomGetString = "\A(" + Right$(cCode, 2) + ")"
        
    'Values 50 and above will have channel numbers
    If Val(Right$(cCode, 2)) >= CUSTOM_SPLIT Then
        CustomGetString = CustomGetString + "(" + cChannel + ")"
    End If

End Function

'Interpret a prompt string into the code eg "Cus50" and the channel number if needed
'szStr is the prompt string starting at the A
'cCode and cChannel are the reply strings
'Return the number of chars to shift on the Prompt string

Public Function CustomLoadString(szStr As String, cCode As String, cChannel As String) As Integer

    Dim nIndex As Integer
    
    'Move on just 1 if its impossible
    CustomLoadString = 1
    cCode = ""
    cChannel = ""
    
    If Len(szStr) < 5 Then
        Exit Function
    End If

    If Left(szStr, 2) <> "A(" Then
        Exit Function
    End If

    If Mid$(szStr, 5, 1) <> ")" Then
        Exit Function
    End If
    
    'This is a custom entry
    cCode = cCustom + Mid$(szStr, 3, 2)
    CustomLoadString = 5
       
    'Is the string long enough to have a channel number
    If Len(szStr) < 6 Then
        Exit Function
    End If

    'Has it a following channel number
    If Mid$(szStr, 6, 1) <> "(" Then
        Exit Function
    End If
    
    'Extract all chars to final )
    For nIndex = 7 To Len(szStr)
       
        If Mid$(szStr, nIndex, 1) = ")" Then
            CustomLoadString = nIndex
            Exit Function
        End If
        
        cChannel = cChannel + Mid$(szStr, nIndex, 1)

    Next nIndex
    

End Function

'Get a value string from the user

Public Function CustomGetUserValue(cType As String, cChannel As String, cMessage As String) As String

    Dim nCustomIndex As Integer
    Dim nChannel As Integer
    Dim cChannelText As String
    Dim cValue As String
    Dim cOutBuf As String * 40
    Dim nBytes As Integer
    
    CustomGetUserValue = ""
    nCustomIndex = Val(Right$(cType, 2))
    nChannel = Val(cChannel)
    
    If nChannel >= CUSTOM_SPLIT Then
        
        Call ChannelDiag.LoadChannel(nChannel)
        cChannelText = "Enter a Value for " + cChannel + "  " + ChannelDiag.cName
        cValue = InputBox(cChannelText, "Custom Value")
       
    End If
        
    'Send the Custom Index
    'Send the message so far plus its size ( may contain 0s )
    'Send the value to insert
    'The output buffer is fixed size
    'cChannelText is not used
    
    nBytes = CustomSend(nCustomIndex, cMessage, Len(cMessage), cValue, cOutBuf, cChannelText)
    
    CustomGetUserValue = Left$(cOutBuf, nBytes)

End Function


Public Function CustomGridEntry(cTitle As String) As String

    Dim cReply As String
    
    CustomGridEntry = ""
    
    cReply = InputBox("Enter Custom Index", cTitle)
    
    If (Len(cReply) <> 2) Or (Val(cReply) < 10) Or (Val(cReply) > 99) Then
        CustomGridEntry = ""
        Exit Function
    End If
    
    CustomGridEntry = cCustom + cReply


End Function
