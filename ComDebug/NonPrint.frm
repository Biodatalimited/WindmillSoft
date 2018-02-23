VERSION 5.00
Begin VB.Form NonPrint 
   Caption         =   " Non Printing Characters"
   ClientHeight    =   3165
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4950
   LinkTopic       =   "Form1"
   ScaleHeight     =   3165
   ScaleWidth      =   4950
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton InsertCell 
      Caption         =   "Insert Cell"
      Height          =   495
      Left            =   2760
      TabIndex        =   15
      Top             =   2280
      Width           =   1575
   End
   Begin VB.CommandButton DeleteCell 
      Caption         =   "Delete Cell"
      Height          =   495
      Left            =   480
      TabIndex        =   14
      Top             =   2280
      Width           =   1575
   End
   Begin VB.CommandButton Command5 
      Caption         =   "Command5"
      Height          =   255
      Left            =   2640
      TabIndex        =   8
      Top             =   1560
      Width           =   495
   End
   Begin VB.CommandButton ButtonTAB 
      Height          =   255
      Left            =   2640
      TabIndex        =   7
      Top             =   1200
      Width           =   495
   End
   Begin VB.CommandButton Command3 
      Caption         =   "Command3"
      Height          =   255
      Left            =   2640
      TabIndex        =   6
      Top             =   840
      Width           =   495
   End
   Begin VB.CommandButton ButtonLF 
      Height          =   255
      Left            =   2640
      TabIndex        =   5
      Top             =   480
      Width           =   495
   End
   Begin VB.CommandButton ButtonCR 
      Height          =   255
      Left            =   2640
      TabIndex        =   4
      Top             =   120
      Width           =   495
   End
   Begin VB.Frame Frame1 
      Caption         =   "Non Print Entry"
      Height          =   1335
      Left            =   120
      TabIndex        =   0
      Top             =   240
      Width           =   2055
      Begin VB.TextBox Nonprinttext 
         Height          =   285
         Left            =   1320
         TabIndex        =   3
         Top             =   570
         Width           =   375
      End
      Begin VB.OptionButton OptionHex 
         Caption         =   "Hex"
         Height          =   255
         Left            =   360
         TabIndex        =   2
         Top             =   360
         Width           =   1095
      End
      Begin VB.OptionButton OptionDecimal 
         Caption         =   "Decimal"
         Height          =   195
         Left            =   360
         TabIndex        =   1
         Top             =   840
         Width           =   1095
      End
   End
   Begin VB.Line Line1 
      X1              =   120
      X2              =   4680
      Y1              =   2040
      Y2              =   2040
   End
   Begin VB.Label label5 
      Caption         =   "Delete"
      Height          =   255
      Left            =   3240
      TabIndex        =   13
      Top             =   1560
      Width           =   1455
   End
   Begin VB.Label Label4 
      Caption         =   "TAB"
      Height          =   255
      Left            =   3240
      TabIndex        =   12
      Top             =   1200
      Width           =   1575
   End
   Begin VB.Label Label3 
      Caption         =   "ETX"
      Height          =   255
      Left            =   3240
      TabIndex        =   11
      Top             =   840
      Width           =   1575
   End
   Begin VB.Label LineFeed 
      Caption         =   "LineFeed"
      Height          =   255
      Left            =   3240
      TabIndex        =   10
      Top             =   480
      Width           =   1575
   End
   Begin VB.Label Label1 
      Caption         =   "Carriage Return"
      Height          =   255
      Left            =   3240
      TabIndex        =   9
      Top             =   120
      Width           =   1335
   End
End
Attribute VB_Name = "NonPrint"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Form_Load()

    OptionHex.Value = False
    
End Sub

Private Sub ButtonCR_Click()
    Call GridWriteCell("013")
    NonPrint.Visible = False
End Sub

Private Sub ButtonLF_Click()
    Call GridWriteCell("010")
    NonPrint.Visible = False
End Sub

'This routine validates each key press as it is made
'in the text box. If it's not valid kill it and beep

Private Sub Nonprinttext_KeyPress(nKey As Integer)

    Dim cCode As String
    Dim nTemp As Integer
       
    cCode = Nonprinttext.Text
       
    'Backspace always OK
    If nKey = 8 Then
        Exit Sub
    End If
             
    If (nKey = 13) And OptionHex.Value Then
        'Convert from hex to decimal'
        nTemp = HexToInt(Left$(cCode, 1))
        If Len(cCode) = 2 Then
            nTemp = nTemp * 16
            nTemp = nTemp + HexToInt(Right$(cCode, 1))
        End If
        cCode = CStr(nTemp)
    End If
             
    'CR ends the input
    If nKey = 13 Then
        Select Case Len(cCode)
            Case 0
                cCode = ""
            Case 1
                cCode = "00" + cCode
            Case 2
                cCode = "0" + cCode
            Case 3
        End Select
        Call GridWriteCell(cCode)
        nKey = 0
        NonPrint.Visible = False
        Exit Sub
    End If
             
    If OptionHex.Value Then
        'if its not a valid hex char then chop it
        Select Case Chr$(nKey)
            Case "0" To "9"
            Case "a" To "f"
                'Convert to upper case
                nKey = nKey - 32
            Case "A" To "F"
            Case Else
                Beep
                nKey = 0 'Kill the entry
        End Select
        'Can't have more than 2 hex chars
        If Len(cCode) = 2 Then
            Beep
            nKey = 0
        End If
    Else
        'if its not a valid decimal char then chop it
        Select Case Chr$(nKey)
            Case "0" To "9"
            Case Else
                Beep
                nKey = 0 'Kill the entry
        End Select
        'Can't have more than 3 decimal chars
        If Len(cCode) = 3 Then
            Beep
            nKey = 0
        End If
        If Len(cCode) = 2 Then
            cCode = cCode + Chr$(nKey)
            If Val(cCode) > 255 Then
                Beep
                nKey = 0
            End If
        End If
    End If


End Sub


Private Function HexToInt(cHex As String) As Integer

    Select Case cHex
        Case "0" To "9"
            HexToInt = Asc(cHex) - Asc("0")
        Case "A" To "F"
            HexToInt = Asc(cHex) - Asc("A") + 10
    End Select

End Function


Private Sub OptionHex_Click()
    Nonprinttext.Text = ""
End Sub

Private Sub OptionDecimal_Click()
    Nonprinttext.Text = ""
End Sub
