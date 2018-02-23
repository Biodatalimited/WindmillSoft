VERSION 5.00
Begin VB.Form Mainform 
   Caption         =   "Windmill Modem Manager"
   ClientHeight    =   2880
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   7140
   Icon            =   "Mainform.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   2880
   ScaleWidth      =   7140
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton DeleteNumber 
      Height          =   255
      Left            =   4800
      TabIndex        =   4
      ToolTipText     =   "Removes a number from the List"
      Top             =   960
      Width           =   495
   End
   Begin VB.CommandButton Exit 
      Height          =   255
      Left            =   4800
      TabIndex        =   3
      ToolTipText     =   "Saves the selected number for use by Setup800 and Dump8XX"
      Top             =   1920
      Width           =   495
   End
   Begin VB.CommandButton AddNumber 
      Height          =   255
      Left            =   4800
      TabIndex        =   2
      ToolTipText     =   "Adds a number to the list"
      Top             =   480
      Width           =   495
   End
   Begin VB.ListBox TelList 
      Height          =   1425
      Left            =   240
      TabIndex        =   1
      ToolTipText     =   "List of numbers of distant 800 systems"
      Top             =   600
      Width           =   4095
   End
   Begin VB.CommandButton WmStart 
      Height          =   255
      Left            =   4800
      TabIndex        =   0
      ToolTipText     =   "Dials the selected number from Windmill"
      Top             =   1440
      Width           =   495
   End
   Begin VB.Line Line4 
      X1              =   4560
      X2              =   120
      Y1              =   2640
      Y2              =   2640
   End
   Begin VB.Line Line3 
      X1              =   120
      X2              =   120
      Y1              =   2640
      Y2              =   120
   End
   Begin VB.Line Line2 
      X1              =   120
      X2              =   4560
      Y1              =   120
      Y2              =   120
   End
   Begin VB.Line Line1 
      X1              =   4560
      X2              =   4560
      Y1              =   120
      Y2              =   2640
   End
   Begin VB.Label Status 
      Height          =   375
      Left            =   240
      TabIndex        =   10
      Top             =   2160
      Width           =   4095
   End
   Begin VB.Label Label5 
      Caption         =   "Telephone No               Description"
      Height          =   255
      Left            =   360
      TabIndex        =   9
      Top             =   240
      Width           =   3975
   End
   Begin VB.Label Label4 
      Caption         =   "OK"
      Height          =   255
      Left            =   5520
      TabIndex        =   8
      Top             =   1920
      Width           =   855
   End
   Begin VB.Label Label3 
      Caption         =   "Start Windmill"
      Height          =   255
      Left            =   5520
      TabIndex        =   7
      Top             =   1440
      Width           =   1335
   End
   Begin VB.Label Label2 
      Caption         =   "Remove from the list"
      Height          =   255
      Left            =   5520
      TabIndex        =   6
      Top             =   960
      Width           =   1575
   End
   Begin VB.Label Label1 
      Caption         =   "Add to the list"
      Height          =   255
      Left            =   5520
      TabIndex        =   5
      Top             =   480
      Width           =   1455
   End
End
Attribute VB_Name = "Mainform"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub AddNumber_Click()
    Dim cTelephone As String
    Dim cBuffer As String
    Dim cDescription As String
    Dim cCaption As String
    Dim nIndex As Integer
    
    cCaption = "Enter a Telephone Number"
    
    cBuffer = InputBox(cCaption)
    If cBuffer = "" Then Exit Sub
       
    'Remove spaces from string
    cTelephone = ""
    For nIndex = 1 To Len(cBuffer)
        If Mid$(cBuffer, nIndex, 1) <> " " Then
            cTelephone = cTelephone + Mid$(cBuffer, nIndex, 1)
        End If
    Next nIndex
       
    cCaption = "Enter a site Description"
    cDescription = InputBox(cCaption)
    
    While Len(cTelephone) < 20
        cTelephone = cTelephone + " "
    Wend
    
    cTelephone = cTelephone + vbTab + cDescription
    'Trim overlong strings
    If Len(cTelephone) > 75 Then
        cTelephone = Left$(cTelephone, 75)
    End If
       
    TelList.AddItem cTelephone
    
    SaveNewNumber (cTelephone)
    
End Sub

Private Sub DeleteNumber_Click()
    
    Dim nIndex As Integer
       
    For nIndex = 0 To TelList.ListCount - 1
        If TelList.Selected(nIndex) = True Then
            RemoveNumber (TelList.List(nIndex))
            TelList.RemoveItem (nIndex)
            Exit For
        End If
    Next nIndex
 
End Sub

Private Sub Exit_Click()

    Dim nIndex As Integer
    
    'Cope with empty list
    'Delete previous selection
    If TelList.ListCount = 0 Then
        SaveSelectedNumber ("")
        End
    End If
       
    For nIndex = 0 To TelList.ListCount - 1
        If TelList.Selected(nIndex) = True Then
            SaveSelectedNumber (TelList.List(nIndex))
            End
        End If
    Next nIndex
    
    Call MsgBox("No Selected Number", vbExclamation)
        
End Sub

Private Sub Form_Load()


    Dim cTelephone As String
    Dim nIndex As Integer
    
   
    'Read the numbers and put them on the list
    For nIndex = 0 To MAX_NUMS
      cTelephone = GetNumberFromINI(nIndex)
      If cTelephone <> "" Then
        TelList.AddItem cTelephone
      End If
    Next nIndex
    
    'Read the selected one
    cTelephone = GetSelectionFromINI()
    
    For nIndex = 0 To TelList.ListCount - 1
        If TelList.List(nIndex) = cTelephone And cTelephone <> "" Then
            TelList.Selected(nIndex) = True
            Exit For
        End If
    Next nIndex
     
    For nIndex = 1 To Len(cTelephone)
        If Mid$(cTelephone, nIndex, 1) = " " Then
            Exit For
        End If
    Next nIndex
         
    
End Sub


Private Sub WmStart_Click()
    
    Dim nIndex As Integer
    Dim cCaption As String
    
    
    'Cope with empty list
    If TelList.ListCount = 0 Then
        Call MsgBox("No Number Selected", vbExclamation)
        Exit Sub
    End If
       
    'Save the selected number and start Windmill
    For nIndex = 0 To TelList.ListCount - 1
        If TelList.Selected(nIndex) = True Then
            SaveSelectedNumber (TelList.List(nIndex))
            Status.Caption = "Dialling Selected number -  please wait"
            DoEvents
            Mainform.MousePointer = vbHourglass
            Call Dial
            Mainform.MousePointer = vbArrow
            cCaption = "If the Windmill Handlers have successfully loaded" + vbCr + _
            "you can now run Windmill Programs."
            Call MsgBox(cCaption, vbOKOnly)
           
            End
        End If
    Next nIndex
    
    'There was no selected number
    Call MsgBox("No Selected Number", vbExclamation)

End Sub



