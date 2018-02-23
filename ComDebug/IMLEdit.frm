VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form IMLEdit 
   Caption         =   "Windmill comDebug IML Device Editor"
   ClientHeight    =   3765
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6195
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   3765
   ScaleWidth      =   6195
   StartUpPosition =   3  'Windows Default
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   0
      Top             =   3480
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.Frame Frame2 
      Height          =   615
      Left            =   240
      TabIndex        =   6
      Top             =   3000
      Width           =   4095
      Begin VB.Label Label2 
         Caption         =   "Select Instrument Modules to make up an IML Device"
         Height          =   255
         Left            =   120
         TabIndex        =   7
         Top             =   240
         Width           =   3855
      End
   End
   Begin VB.Frame Frame1 
      Height          =   3255
      Left            =   4440
      TabIndex        =   2
      Top             =   360
      Width           =   1575
      Begin VB.CommandButton DescButton 
         Caption         =   "Description"
         Height          =   375
         Left            =   240
         TabIndex        =   8
         Top             =   1840
         Width           =   1095
      End
      Begin VB.CommandButton OKButton 
         Caption         =   "OK"
         Height          =   375
         Left            =   240
         TabIndex        =   5
         Top             =   2640
         Width           =   1095
      End
      Begin VB.CommandButton RemoveButton 
         Caption         =   "Remove"
         Height          =   375
         Left            =   240
         TabIndex        =   4
         Top             =   1040
         Width           =   1095
      End
      Begin VB.CommandButton AddButton 
         Caption         =   "Add"
         Height          =   375
         Left            =   240
         TabIndex        =   3
         Top             =   240
         Width           =   1095
      End
   End
   Begin VB.ListBox ModuleList 
      Height          =   2400
      Left            =   240
      TabIndex        =   0
      Top             =   480
      Width           =   4095
   End
   Begin VB.Label Label1 
      Caption         =   "Num : Module  -  Description"
      Height          =   255
      Left            =   240
      TabIndex        =   1
      Top             =   120
      Width           =   3975
   End
End
Attribute VB_Name = "IMLEdit"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim cModule(10) As String
Dim cDesc(10) As String
Dim cIMDFile(10) As String

'Dim cPath As String
Dim bEdited As Boolean

Dim nModnum As Integer


Private Sub AddButton_Click()

    Dim cModName As String
    Dim cDescrip As String
    Dim nIndex As Integer
    Dim cTemp As String
    Dim cFilename As String
    
    If nModnum = 10 Then
        MsgBox ("Maximum 10 modules")
        Exit Sub
    End If
    
    CommonDialog1.Flags = cdlOFNHideReadOnly
    CommonDialog1.Filter = "Instrument Files .imd | *.imd"
    CommonDialog1.DialogTitle = "Select Instrument File"
    CommonDialog1.InitDir = cIMLDirPath
    CommonDialog1.ShowOpen
    
    If CommonDialog1.FileName = "" Then
        Exit Sub
    End If
    
    'Read the module desc from the imd file
    cFilename = CommonDialog1.FileName
    Call GetModuleDesc(cFilename, cDescrip)
    
    'Prepare the module name
    'First trim the .imd from the string
    nIndex = Len(cFilename)
    While Mid$(cFilename, nIndex, 1) <> "."
        nIndex = nIndex - 1
    Wend
    ' The -1 gets rid of the .
    cModName = Left$(cFilename, nIndex - 1)
    
    'Now trim the path from the string
    nIndex = Len(cModName)
    While Mid$(cModName, nIndex, 1) <> "\"
        nIndex = nIndex - 1
    Wend
    
    'We have the module name
    cModName = Right$(cModName, Len(cModName) - nIndex)
    
    
    If Len(cModName) > 10 Then
        cTemp = "Windmill uses the .IMD file name as a module name." + vbCr + vbCr _
            + "It is limited to 10 letters or numbers without spaces." + vbCr + vbCr _
            + "Please save your file under another name."
            
        MsgBox (cTemp)
        Exit Sub
    End If
    
    For nIndex = 1 To Len(cModName)
        If Mid$(cModName, nIndex, 1) = " " Then
            cTemp = "Windmill uses the .IMD file name as a module name." + vbCr + vbCr _
            + "It is limited to 10 letters or numbers without spaces." + vbCr + vbCr _
            + "Please save your file under another name."
            
        MsgBox (cTemp)
        
            Exit Sub
        End If
    Next nIndex
        
    'Check for duplicates
    For nIndex = 0 To nModnum - 1
        If cModName = cModule(nIndex) Then
            MsgBox ("This Instrument is already in the Device")
            Exit Sub
        End If
    Next nIndex
    
    'Put it in the list
    cModule(nModnum) = cModName
    cDesc(nModnum) = cDescrip
    cIMDFile(nModnum) = cFilename
    nModnum = nModnum + 1
    FillList
    
    bEdited = True

End Sub

Private Sub DescButton_Click()

Dim nIndex As Integer
Dim cBuffer As String

nIndex = ModuleList.ListIndex

If nIndex = -1 Then
    Exit Sub
End If

cBuffer = "Edit Module Description. This appears in ConfIML."
cDesc(nIndex) = InputBox(cBuffer, "Module Description", cDesc(nIndex))
   
'Put the description in the IMD file
Call WriteModuleDescription(cIMDFile(nIndex), cDesc(nIndex))

FillList
    
End Sub

Private Sub Form_Load()

Dim nRet As Integer
Dim cFilename As String

nModnum = 0
bEdited = False

'Get the current modules from IML.INI

Do
    nRet = GetIMLModuleName(cModule(nModnum), cDesc(nModnum), cIMDFile(nModnum), nModnum)
    
    If cModule(nModnum) = "Default" Then
        nModnum = 0
        Exit Do
    End If
    If nRet = 0 Then
        Exit Do
    End If
        
    'Read the module desc from the imd file
    'Should be the same as that from iml.ini except that
    'it does not have the module name prepended
    Call GetModuleDesc(cIMDFile(nModnum), cDesc(nModnum))
       
    nModnum = nModnum + 1
Loop
   
 
'nModnum is now the number of modules

FillList   'Put them on screen

End Sub


Private Sub FillList()

Dim nIndex As Integer
Dim cText As String
    
ModuleList.Clear

    For nIndex = 0 To nModnum - 1
        cText = "  " + CStr(nIndex) + "  :  " + cModule(nIndex)
        cText = cText + "  -  " + cDesc(nIndex)
        ModuleList.AddItem cText
    Next nIndex
    
    If nModnum > 0 Then
        ModuleList.Selected(nModnum - 1) = True
    End If
    
End Sub

Private Sub IMSbutton_Click()
    
    Dim nIndex As Integer
    Dim cReply As String
    
    CommonDialog1.FileName = ""
    CommonDialog1.Flags = cdlOFNHideReadOnly
    CommonDialog1.Filter = "IMS Files .ims | *.ims"
    CommonDialog1.DialogTitle = "Save IMS File. Limit the name to 8 Characters"
    CommonDialog1.InitDir = cIMLDirPath
    CommonDialog1.ShowOpen
    
    If CommonDialog1.FileName = "" Then
        Exit Sub
    End If
    
    For nIndex = 0 To 12
    
        If Mid$(CommonDialog1.FileName, Len(CommonDialog1.FileName) - nIndex, 1) = "\" Then
            Exit For
        End If
    
    Next nIndex
    
    If nIndex > 12 Then
        Call MsgBox("Please choose a shorter Filename", vbOKOnly)
        Exit Sub
    End If
    
    
    Open CommonDialog1.FileName For Output As #1
    
    Print #1, "F 01 IMSLIB 6.00"
    Print #1, "D"

    GetIMLDeviceNumber
    
    For nIndex = 0 To nModnum - 1
        cReply = GetIMSChannels(cIMDFile(nIndex), nIndex)
    Next nIndex
    
    Print #1, cReply
    
    Close #1

End Sub

Private Sub Form_Unload(Cancel As Integer)
    Startup.Visible = True
End Sub

Private Sub OKButton_Click()

Dim nIndex As Integer
Dim cIMLDesc As String

For nIndex = 0 To nModnum - 1
    Call WriteIMLModuleName(cModule(nIndex), cDesc(nIndex), cIMDFile(nIndex), nIndex)
Next nIndex

If bEdited Then
    If nModnum = 0 Then
        MsgBox ("You must have at least one Module")
    End If
End If

Call SaveNamesFile

Call Unload(IMLEdit)

End Sub

'Remove the selected module

Private Sub RemoveButton_Click()

Dim nIndex As Integer

nIndex = ModuleList.ListIndex

If nIndex = -1 Then
    Exit Sub
End If

While nIndex < 9
    cModule(nIndex) = cModule(nIndex + 1)
    cDesc(nIndex) = cDesc(nIndex + 1)
    nIndex = nIndex + 1
Wend

cModule(nIndex) = ""
cDesc(nIndex) = ""

nModnum = nModnum - 1

FillList

bEdited = True

End Sub
